#include "pcie_layout.h" // Comparte recorridos y diagnosticos con las pruebas sin hardware.
// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <stdlib.h>
#include <string>
#include <cstdint>
#include <vector>
#include <vcclr.h>  
#include <stdio.h>
#include <time.h>

using namespace std;

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "api_pcie.h"
#include "pcie.h"
#include "pcie_error_code.h"
#include "log.h"
#include "pcie_utils.h"
#include "fuse_api.h"
#include "c_image_handle.h"
#include "stlib2.h"
#include "stlib2_wrapper.h"
#include "pcie_tad.h"
#include "fileformat.h"
#include "pcie_thread_acq.h"
#include "pcie_thread_ge.h"
#include "c_timeout.h"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

fuse *gb_drv_handle = nullptr;
int gb_pcie_n_samples = 0; // Número de muestras programado por el usuario
int gb_pcie_n_images_buffer = 0;
//int gb_n_focal_laws_image = 0; // Número de leyes focales por imagen programado por el usuario
//int gb_n_images = 0; // Número de imagenes total a adquirir

int gb_pcie_buffer_n_blocks_buf = 0; // Número de bloques recibidos por PCIe con su cabecera
//size_t gb_size_block = 0; // Tamaño de bloques recividos por PCIe con su cabecera (En Bytes)
int gb_pcie_n_blocks_acq_total = -1; // Número total de bloques a adquirir con su cabecera, cuando se completa este número finaliza la adquisición

int gb_pcie_n_beats_frames = 0;
int gb_pcie_n_focal_laws_block = 0; // Leyes reales por bloque, sin duplicacion de transporte.
static int gb_pcie_transport_units_per_law = 0; // Factor fijado por la geometria de cada arranque, nunca por una cabecera.

int gb_check_pcie_header = 1;

bool gb_driver_enabled = true;
bool gb_pcie_enabled = true;
int gb_moving_average_kernel_size = 5;

vector<dma_buffer_t> gb_ptr_blocks; // Array de punteros a los bloques devueltos al usuario
T_block_header gb_last_acquierd_header;// = { .header_struct.ch_id_last.bits.last_block = 1 };
vector<short int *> gb_focal_laws_pointers;

//int gb_pcie_buffer_n_blocks_buf = 0;
size_t gb_pcie_buffer_size_block = 0;
int gb_pcie_closed = 1;
bool gb_pcie_continuous = false;

// ______________________________________________________________________
// ------ I M L E M E N T A C I Ó N   D E   F U N C I O N E S
// ======================================================================

// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_ResolveGeometry(T_PCIeGeometry &geometry) // Consulta una sola vez cada dimension de stlib2 para esta adquisicion.
{ // No abre dispositivos, reserva buffers ni conserva una geometria global obsoleta.
    geometry = T_PCIeGeometry(); // Invalida la salida tambien si falla un getter.
    const int acquisition_type = ST_GetAcquisitionType(); // El contrato de esta entrega es MC paralelo.
    if (acquisition_type < 0) return RLOG(acquisition_type); // Conserva errores del getter.
    if (acquisition_type != 0) return ELOG(charEPCIE_Geometry, EPCIE_Geometry); // ACQUISITION_TYPE_MC=0; PA y TFM requieren otro contrato.
    int channel_count, module_count, probe_elements, map_size, samples_per_channel, focal_laws_per_image, focal_laws_per_block, result; // Usa variables locales para construir una instantanea validada.
    if ((channel_count = ST_GetChannelsNumber()) < 0) return RLOG(channel_count); // Reutiliza los canales efectivos derivados de SITAU.XML.
    if ((module_count = ST_GetModulesNumber()) < 0) return RLOG(module_count); // Distingue canales multiplexados de modulos fisicos.
    if ((probe_elements = ST_GetProbeElementNumber()) < 0) return RLOG(probe_elements); // Limita esta entrega a sonda completa.
    if ((map_size = ST_GetProbePinToChannelSize(0)) < 0) return RLOG(map_size); // Comprueba la longitud resuelta tras la ampliacion de stlib2.
    if ((samples_per_channel = ST_MC_GetAScanDataNumber(0)) < 0) return RLOG(samples_per_channel); // Consulta las muestras del canal virtual cero.
    if ((focal_laws_per_image = ST_GetFocalLawNumber(0)) < 0) return RLOG(focal_laws_per_image); // Consulta las leyes de la imagen.
    if ((focal_laws_per_block = ST_PCIe_GetFocalLawsBlockNumber()) < 0) return RLOG(focal_laws_per_block); // Consulta las leyes por bloque calculadas por stlib2.
    result = PCIe_BuildGeometry(channel_count, module_count, probe_elements, map_size, samples_per_channel, focal_laws_per_image, focal_laws_per_block, geometry); // Centraliza formulas y limites comprobables sin hardware.
    if (result < 0) return RLOG(result); // Propaga el codigo de geometria o capacidad al catalogo MATLAB.
    return EPCIE_NONE; // La fase 2 consumira tambien los tamanos de transporte calculados.
} // La salida permanece local al arranque que la solicita.

int PCIe_Open(void)
{
int result, n_fl;
char str[255];

	if (!gb_pcie_enabled) return EPCIE_NONE;
	PCIe_GetLibraryVersion(str);
	LOG_Open(str);
	if (gb_drv_handle != nullptr) 
	{ 
		if (gb_drv_handle != nullptr)
		{
			if (gb_drv_handle->dma_buffer_leakage() > 0) { ELOG(charEPCIE_BUFFER_LEAKAGE, EPCIE_BUFFER_LEAKAGE); }
			//if ((result = gb_drv_handle->destroy_dma_buffers()) < 0) { RLOG(result); }
			//if ((result = gb_drv_handle->close()) < 0) { RLOG(result); }
			delete gb_drv_handle;
			gb_drv_handle = nullptr;
		}
		ThrAcq_Delete();
		if (gb_image_handle != nullptr)
		{
			delete gb_image_handle;
			gb_image_handle = nullptr;
		}
		gb_image_handle = nullptr;

		if (gb_buffer_data != nullptr) delete gb_buffer_data;
		gb_buffer_data = nullptr;
	}

	gb_pcie_buffer_n_blocks_buf = 0;
	gb_pcie_buffer_size_block = 0;
	if (gb_driver_enabled && (gb_drv_handle = new fuse(0, 0, 0)) == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if (gb_driver_enabled && (result = gb_drv_handle->open()) < 0)
	{
		delete gb_drv_handle;
		gb_drv_handle = nullptr;
		gb_pcie_enabled = false;
		return RLOG(result);
	}
	
	//if ((result = gb_drv_handle->dma_hw_flush()) < 0) return RLOG(result);

	if (gb_driver_enabled && (result = gb_drv_handle->dma_hw_reset(FUSE_HW_RST_DMA)) < 0) { RLOG(result); }
  //if ((result = gb_drv_handle->dma_hw_init()) < 0) { RLOG(result); }
  

	Sleep(1);

	if ((n_fl = ST2_RangeMaxFocalLaws()) > 0) gb_focal_laws_pointers.resize(n_fl);
	//if ((result = PCIe_EventDefault()) < 0) return RLOG(result);
	gb_pcie_closed = 0;

	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetFreeBlocksNumber(void)
{
	int result, buffer_leakage = 0;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if (gb_driver_enabled && (buffer_leakage = gb_drv_handle->dma_buffer_leakage()) < 0) return RLOG(buffer_leakage);
	result = gb_pcie_buffer_n_blocks_buf - buffer_leakage;
	return result < 0 ? gb_pcie_buffer_n_blocks_buf : result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_Close(void)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	gb_pcie_buffer_n_blocks_buf = 0;
	gb_pcie_buffer_size_block = 0;
	ThrGE_Delete();
	ThrAcq_Delete();
	//if (gb_drv_handle != nullptr)
 // {
	//  if (gb_driver_enabled && gb_drv_handle->dma_buffer_leakage() > 0) { ELOG(charEPCIE_BUFFER_LEAKAGE, EPCIE_BUFFER_LEAKAGE); }
	//  //if ((result = gb_drv_handle->destroy_dma_buffers()) < 0) { RLOG(result); }
	//  //if ((result = gb_drv_handle->close()) < 0) { RLOG(result); }
	//  delete gb_drv_handle;
	//  gb_drv_handle = nullptr;
 // }
  if (gb_image_handle != nullptr)
  {
	  delete gb_image_handle;
	  //ELOG("delete buffer", EPCIE_USER);
  }
  gb_image_handle = nullptr;

  if (gb_buffer_data != nullptr) delete gb_buffer_data;
  gb_buffer_data = nullptr;
  
  LOG_Close();

  gb_pcie_n_samples = 0; // Número de muestras programado por el usuario
  gb_pcie_n_images_buffer = 0;
  gb_pcie_buffer_n_blocks_buf = 0; // Número de bloques recibidos por PCIe con su cabecera
  //gb_size_block = 0; // Tamaño de bloques recividos por PCIe con su cabecera (En Bytes)
  gb_pcie_n_blocks_acq_total = -1; // Número total de bloques a adquirir con su cabecera, cuando se completa este número finaliza la adquisición
  gb_pcie_n_beats_frames = 0;
  gb_pcie_n_focal_laws_block = 0;
  //gb_check_pcie_header = 1;
  //gb_driver_enabled = true;
  //gb_pcie_enabled = true;
  //gb_moving_average_kernel_size = 5;
  gb_pcie_buffer_size_block = 0;
  gb_pcie_continuous = false;
  //gb_pcie_closed = 1;

  return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
void PCIe_GetLibraryVersion(char *data)
{
  strcpy(data, _VERSION_DLL_MAJOR);
  strcat(data, ".");
  strcat(data, _VERSION_DLL_MINOR);
  strcat(data, ".");
  strcat(data, _VERSION_DLL_RELEASE);
  strcat(data, ".");
  strcat(data, _VERSION_DLL_BUILD);
  strcat(data, " ");
  strcat(data, _VERSION_DLL_DATE);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
void PCIe_GetSystemInfo(char *id, char *olink_signature, char *fuse_dma_signature, char *emul_signature)
{
  uint16_t pci_subsys_id = 0;
  uint8_t pci_rev_id = 0;
  int version = 0, subversion = 0, revision = 0, magic_number = 0;

  if (gb_pcie_enabled && gb_driver_enabled && gb_drv_handle != nullptr)
  {
    gb_drv_handle->get_pcie_info(pci_subsys_id, pci_rev_id);
	sprintf(id, "PCI subsystem ID = 0x%4.4X, revision ID = 0x%2.2X", pci_subsys_id, pci_rev_id);
	
	gb_drv_handle->olink_get_signature(version, subversion, revision, magic_number);
	sprintf(olink_signature, "Optical link : version % d.%d, revision %d, magic number = 0x%4.4X", version, subversion, revision, magic_number);

	gb_drv_handle->dma_get_signature(version, subversion, revision, magic_number);
	sprintf(fuse_dma_signature, "FUSE DMA: version %d.%d, revision %d, magic number = 0x%4.4X", version, subversion, revision, magic_number);

	gb_drv_handle->emul_get_signature(version, subversion, revision, magic_number);
	sprintf(emul_signature, "Emulator: version %d.%d, revision %d, magic number = 0x%4.4X\n", version, subversion, revision, magic_number);
  }
  else
  {
	  sprintf(id, "PCI subsystem ID = 0x%4.4X, revision ID = 0x%2.2X", pci_subsys_id, pci_rev_id);
	  sprintf(olink_signature, "Optical link : version % d.%d, revision %d, magic number = 0x%4.4X", version, subversion, revision, magic_number);
	  sprintf(fuse_dma_signature, "FUSE DMA: version %d.%d, revision %d, magic number = 0x%4.4X", version, subversion, revision, magic_number);
	  sprintf(emul_signature, "Emulator: version %d.%d, revision %d, magic number = 0x%4.4X\n", version, subversion, revision, magic_number);
  }
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetStatus(unsigned long long *status_mask)
{
  int result;
  fuse_status_t mask = 0;

  if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
  if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);

  if (gb_driver_enabled && (result = gb_drv_handle->dma_get_status(mask, 1)) < 0) return RLOG(result);
  *status_mask = (unsigned long long) mask;
  return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_IsRunning(void)
{
  int result;

  if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
  if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
  if ((result = gb_drv_handle->dma_is_running()) < 0) return RLOG(result);
  return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
//int PCIe_GetImagesNumber(void)
//{
//  return gb_n_images;
//}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
//int PCIe_GetFocalLawsNumber(void)
//{
//  return gb_n_focal_laws_image;
//}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_SetGeometryBuffers(const T_PCIeGeometry &geometry, int buffer_blocks, int total_images) // Recalcula todos los parametros en cada arranque parado.
{ // Evita una cache parcial al alternar 128 y 256 con las mismas muestras.
    if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED); // Exige transporte PCIe.
    if (buffer_blocks <= 0 || total_images < 0) return ELOG(charEPCIE_Geometry, EPCIE_Geometry); // Cero imagenes conserva el modo continuo.
    const unsigned long long total_laws = static_cast<unsigned long long>(total_images) * geometry.focal_laws_per_image; // Calcula sin overflow de int.
    const unsigned long long total_blocks = (total_laws + geometry.focal_laws_per_block - 1) / geometry.focal_laws_per_block; // Redondea hacia arriba el ultimo bloque.
    if (total_blocks > 2147483647ULL) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // Respeta los contadores legacy de adquisicion.
    if (buffer_blocks < (geometry.focal_laws_per_image + geometry.focal_laws_per_block - 1) / geometry.focal_laws_per_block + 1) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // La reserva debe permitir completar una imagen que cruza bloques.
    int result; // Propaga fallos antes de publicar la nueva configuracion.
    if (gb_driver_enabled) // La simulacion no necesita un driver real.
    { // Reconfigura y reserva solo con DMA detenido.
        if (!gb_drv_handle) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL); // Comprueba el propietario del buffer.
        if ((result = gb_drv_handle->set_transport_modules(geometry.module_count)) < 0) return RLOG(result); // Reserva y lectura comparten cuatro u ocho tramas.
        if (gb_drv_handle->calc_hw_image_size(geometry.beats_per_module, geometry.focal_laws_per_block) != geometry.block_size_bytes) return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // Contrasta unidades antes de reservar.
        if ((result = gb_drv_handle->destroy_dma_buffers()) < 0) return RLOG(result); // Descarta memoria y FIFO de la configuracion anterior.
        if ((result = gb_drv_handle->create_dma_buffers(buffer_blocks, static_cast<size_t>(geometry.block_size_bytes))) < 0) return RLOG(result); // Reserva exactamente la longitud que leera DMA.
    } // No conserva una cache dependiente de la instancia del driver.
    gb_pcie_n_samples = geometry.samples_per_channel; // Publica dimensiones tras reservar correctamente.
    gb_pcie_n_images_buffer = buffer_blocks; // Conserva el significado historico: numero de bloques DMA.
    gb_pcie_buffer_n_blocks_buf = buffer_blocks; // Actualiza umbrales de pausa.
    gb_pcie_buffer_size_block = static_cast<size_t>(geometry.block_size_bytes); // Capacidad fisica por bloque.
    gb_pcie_n_blocks_acq_total = static_cast<int>(total_blocks); // Actualiza incluso si solo cambia el numero de imagenes.
    gb_pcie_n_beats_frames = geometry.beats_per_module; // Mantiene el formato de cabecera por modulo.
    gb_pcie_n_focal_laws_block = geometry.focal_laws_per_block; // Conserva la capacidad real para DMA y secuencia.
    gb_pcie_transport_units_per_law = geometry.transport_units_per_law; // Actualiza tambien al volver de 256 a 128.
    gb_ptr_blocks.assign(buffer_blocks, nullptr); // Elimina punteros residuales de otra adquisicion.
    return EPCIE_NONE; // Configuracion completa para este arranque.
} // Fin de reserva por geometria.
int PCIe_SetBuffers_mod(int n_samples, int n_focal_laws_image, int n_images_buffer, int n_images_total, int n_focal_laws_block) // Conserva la firma historica.
{ // Los clientes antiguos tambien deben coincidir con stlib2.
    T_PCIeGeometry geometry = T_PCIeGeometry(); // Obtiene el contrato actual.
    int result = PCIe_ResolveGeometry(geometry); // Valida topologia y capacidades.
    if (result < 0) return RLOG(result); // Propaga fallo de configuracion.
    if (n_samples != geometry.samples_per_channel || n_focal_laws_image != geometry.focal_laws_per_image || n_focal_laws_block != geometry.focal_laws_per_block) return ELOG(charEPCIE_Geometry, EPCIE_Geometry); // Rechaza argumentos obsoletos.
    return PCIe_SetGeometryBuffers(geometry, n_images_buffer, n_images_total); // Reutiliza la ruta parametrizada.
} // Fin del adaptador historico.
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_Start(int thread_acq_enabled, int time_sleep, int priority, int sorted, int check_header, unsigned long n_images_block, unsigned long n_blocks, int thread_dma_enabled, int thread_olink_enabled)
{
	int result = 0, n_hw_img = 0, frame_len = 0, hw_image_len = 0;
	size_t hw_img_size = 0;
	C_Timeout* timer_thread = nullptr;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	//if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	gb_check_pcie_header = check_header;
	gb_buffer_n_images_block = n_images_block;
	gb_buffer_n_blocks = n_blocks;
	gb_pcie_continuous = ST2_GetAcquisitionMode() == 1;
	//if (gb_driver_enabled)
	//{
	//	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if ((result = ThrGE_Create(gb_drv_handle, priority, thread_dma_enabled == 1, thread_olink_enabled == 1)) < 0) RLOG(result);
	//}
	try
	{
		if (thread_acq_enabled == 1 && gb_process_acq == nullptr)
		{
			if ((result = ThrAcq_Create(priority, time_sleep, sorted)) < 0) return RLOG(result);
			timer_thread = new C_Timeout(5000);
			timer_thread->Start();
			while (!ThrAcq_IsRunning() && !timer_thread->Timeout());
			if ((result = ThrAcq_Error()) < 0) return RLOG(result); // No arranca DMA si fallo la reserva del buffer de usuario.
			if (!ThrAcq_IsRunning()) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer); // Rechaza timeout de inicializacion antes de transmitir.
		}
		else if (gb_buffer_data != nullptr) gb_buffer_data->Clear();
		else return ELOG(charEPCIE_BUFFER_NULL, EPCIE_BUFFER_NULL);
		gb_process_acq->no_hw_image_index_ = 0;
		gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block = 1;
		//if ((result = gb_drv_handle->close_dev_files()) < 0) return RLOG(result);
		//if ((result = gb_drv_handle->open()) < 0) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
		//if (gb_drv_handle == nullptr) PCIe_Open();// return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
		//if (gb_drv_handle == nullptr)  return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);

		if (gb_driver_enabled)
		{
			if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
			if ((result = gb_drv_handle->dma_start(gb_pcie_continuous == 1 ? -1 : gb_pcie_n_blocks_acq_total, gb_pcie_n_beats_frames, gb_pcie_n_focal_laws_block)) < 0)
			{
				if (result == -2)
				{
					RLOG(result);
				}
				else return RLOG(result);
			}
		}
		//if (gb_driver_enabled && (result = gb_drv_handle->dma_start(-1, gb_pcie_n_beats_frames, gb_pcie_n_focal_laws_block)) < 0) return RLOG(result);
	}
	__finally
	{
		if (timer_thread != nullptr) delete timer_thread;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_Start_1(int thread_acq_enabled, int time_sleep, int priority, int sorted, int check_header, unsigned long n_images_block, unsigned long n_blocks, int thread_dma_enabled, int thread_olink_enabled)
{
	int result = 0, acq_mode, n_hw_img = 0, frame_len = 0, hw_image_len = 0;
	size_t hw_img_size = 0;
	C_Timeout* timer_thread = nullptr;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	gb_check_pcie_header = check_header;
	gb_buffer_n_images_block = n_images_block;
	gb_buffer_n_blocks = n_blocks;
	acq_mode = ST2_GetAcquisitionMode();
	if ((result = ThrGE_Create(gb_drv_handle, priority, thread_dma_enabled == 1, thread_olink_enabled == 1)) < 0) RLOG(result);
	try
	{
		if (thread_acq_enabled == 1)
		{
			if ((result = ThrAcq_Create(priority, time_sleep, sorted)) < 0) return RLOG(result);
			timer_thread = new C_Timeout(5000);
			timer_thread->Start();
			while (!ThrAcq_IsRunning() && !timer_thread->Timeout());
			if ((result = ThrAcq_Error()) < 0) return RLOG(result); // No arranca DMA si fallo la reserva del buffer de usuario.
			if (!ThrAcq_IsRunning()) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer); // Rechaza timeout de inicializacion antes de transmitir.
		}
		gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block = 1;
		//if ((result = gb_drv_handle->close_dev_files()) < 0) return RLOG(result);
		//if ((result = gb_drv_handle->open()) < 0) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
		//if (gb_drv_handle == nullptr) PCIe_Open();// return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
		//if (gb_drv_handle == nullptr)  return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
		if (gb_driver_enabled && (result = gb_drv_handle->dma_start(acq_mode == 1 ? -1 : gb_pcie_n_blocks_acq_total, gb_pcie_n_beats_frames, gb_pcie_n_focal_laws_block)) < 0) return RLOG(result);
		//if (gb_driver_enabled && (result = gb_drv_handle->dma_start(-1, gb_pcie_n_beats_frames, gb_pcie_n_focal_laws_block)) < 0) return RLOG(result);
	}
	__finally
	{
		if (timer_thread != nullptr) delete timer_thread;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_Stop(void)
{
	int result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	ThrAcq_Delete();
	//if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if (gb_drv_handle != nullptr && gb_drv_handle->dma_is_running())
	{
		if ((result = gb_drv_handle->dma_stop()) < 0) return RLOG(result);
		//if ((result = gb_drv_handle->destroy_dma_thread()) < 0) return RLOG(result);
		//if ((result = gb_drv_handle->close_dev_files()) < 0) return RLOG(result);
		//gb_pcie_closed = 1;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
//int PCIe_GetImages(short *images, int n_images)
//{
//  return EPCIE_NONE;
//}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetAddressBlocks(int *n_read_blocks, unsigned long long *addr_blocks, int n_max_blocks)
{
	return PCIe_GetPtrBlocks(n_read_blocks, (void **)addr_blocks, n_max_blocks);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetPtrBlocks(int *n_read_blocks, void **ptr_blocks, int n_max_blocks)
{
	int n_blocks;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (n_read_blocks != nullptr) *n_read_blocks = 0;
	//return 0;
	if (n_max_blocks >= gb_ptr_blocks.size()) return ELOG(charEPCIE_PARAMETER, EPCIE_PARAMETER);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);

	if ((n_blocks = fuse_dma_buffer_rdy(gb_drv_handle)) < 0)
		return RLOG(n_blocks);

	if (n_blocks > 0)
	{
		if ((n_blocks = fuse_get_dma_buffer(gb_drv_handle, &gb_ptr_blocks[0], n_max_blocks)) < 0)
			return RLOG(n_blocks);
		if (n_blocks > 0)
		{
			if (n_read_blocks != nullptr) *n_read_blocks = n_blocks;
			if (ptr_blocks != nullptr) memcpy(ptr_blocks, &gb_ptr_blocks[0], sizeof(void *) * n_blocks);
		}
		else { ELOG("WARNING: n_blocks == 0", EPCIE_USER); }

	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_FreePtrBlocks(int n_blocks)
{
	int i, result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	for (i = 0; i < n_blocks && i < gb_ptr_blocks.size(); i++)
	{
		if ((result = gb_drv_handle->free_dma_buffer(gb_ptr_blocks[i])) < 0) return RLOG(result);
	}
	return n_blocks;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
long long PCIe_GetAcquiredBlocksNumber(void)
{
	long long result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if ((result = gb_drv_handle->dma_buffer_recv()) < 0) return RLOG((int)result);
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetUnreadBlocksNumber(void)
{
	int result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if ((result = gb_drv_handle->dma_buffer_rdy()) < 0) return RLOG(result);
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_EnableRx(int enable) 
{
	int result;

	if (!gb_driver_enabled) return EPCIE_NONE;
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	if (enable == 0)
	{
		ELOG("PCIe_EnableRx = 0", -1);
	}
	else if (enable == 1)
	{
		ELOG("PCIe_EnableRx = 1", -1);
	}
	else
	{
		ELOG("PCIe_EnableRx = Unknown", -1);
	}
	if ((result = gb_drv_handle->dma_hw_enable(enable)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_CheckPCIeHeader_fast(void *ptr_block)
{
	T_block_header *new_header_structed_p;
	int header_errors = 0;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_check_pcie_header == 0) return EPCIE_NONE;

	new_header_structed_p = (T_block_header *)(ptr_block); // Lee sin reescribir la cabecera de transporte.
    unsigned int received_real_laws = 0; // El total y los indices de imagen permanecen en leyes reales.
    if (!new_header_structed_p || PCIe_DecodeBlockLaws(new_header_structed_p->header_struct.block_focal_laws_word.bits.block_focal_laws, gb_pcie_transport_units_per_law, received_real_laws) < 0 || received_real_laws != static_cast<unsigned int>(gb_pcie_n_focal_laws_block)) return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // Rechaza medias leyes y capacidades distintas de la configurada.

	if (new_header_structed_p->header_struct.magic_number != HEADER_MAGIC_NUMBER)
	{
		snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Bad Magic Number\nExpected magic number: %08X, received %08X", HEADER_MAGIC_NUMBER, new_header_structed_p->header_struct.magic_number);
		ELOG(gb_log_msg, EPCIE_USER);
    return ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); // No acepta una cabecera corrupta.
	}
	if (!gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block)  //Aquí se comprueba en realidad que esta es la PRIMERA TRAMA DE UN ENVÍO!!! Se hace mirando si la última trama recibida tuvo el LAST a 1! Puede que este método no sea el mejor y se cambie en el futuro
	{
		unsigned long expected_image_index, expected_focal_law_index;
		unsigned long prev_image_index, prev_block_focal_laws, prev_image_focal_law_total, prev_image_focal_law_index;

		if (gb_last_acquierd_header.header_struct.image_focal_law_word.bits.image_focal_law_total == 0)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: image_focal_law_total = 0");
			return ELOG(gb_log_msg, EPCIE_USER);
		}

		prev_image_index = gb_last_acquierd_header.header_struct.image_index;
		prev_block_focal_laws = static_cast<unsigned long>(gb_pcie_n_focal_laws_block - 1); // La cabecera previa ya fue validada; avanza en leyes reales, codificadas menos uno.
		prev_image_focal_law_total = gb_last_acquierd_header.header_struct.image_focal_law_word.bits.image_focal_law_total;
		prev_image_focal_law_index = gb_last_acquierd_header.header_struct.image_focal_law_word.bits.image_focal_law_index;

		expected_image_index = prev_image_index + ((prev_image_focal_law_index + prev_block_focal_laws + 1) / prev_image_focal_law_total);//No es necesario FLOOR porque la división en C ya es floor
		expected_focal_law_index = (prev_image_focal_law_index + prev_block_focal_laws + 1) % prev_image_focal_law_total;

		// Identificación de bloque, aumenta de uno en uno
		if ((gb_last_acquierd_header.header_struct.block_index + 1) != new_header_structed_p->header_struct.block_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Block Index (Previous %d, Received %d)", gb_last_acquierd_header.header_struct.block_index, new_header_structed_p->header_struct.block_index);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (gb_last_acquierd_header.header_struct.block_beats_word.bits.block_beats != new_header_structed_p->header_struct.block_beats_word.bits.block_beats)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Block Beats (Previous %d, Received %d)", gb_last_acquierd_header.header_struct.block_beats_word.bits.block_beats, new_header_structed_p->header_struct.block_beats_word.bits.block_beats);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (gb_last_acquierd_header.header_struct.block_focal_laws_word.bits.block_focal_laws != new_header_structed_p->header_struct.block_focal_laws_word.bits.block_focal_laws)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Block Focal Laws (Previous %d, Received %d)", gb_last_acquierd_header.header_struct.block_focal_laws_word.bits.block_focal_laws, new_header_structed_p->header_struct.block_focal_laws_word.bits.block_focal_laws);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (expected_image_index != new_header_structed_p->header_struct.image_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Image Index (Previous %d, Expected %d, Received %d)", prev_image_index, expected_image_index, new_header_structed_p->header_struct.image_index);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (expected_focal_law_index != new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Image Focal Law Index (Previous %d, Expected %d, Received %d)", prev_image_focal_law_index, expected_focal_law_index, new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_index);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (new_header_structed_p->header_struct.ch_id_last.bits.last_block)
		{
			if (new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total > static_cast<unsigned int>(gb_pcie_n_focal_laws_block)) // El ultimo bloque declara leyes validas reales.
			{
				snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (last block): Valid Image Focal Laws in Last Block (Expected less than maximum %d, Received %d)", gb_last_acquierd_header.header_struct.block_focal_laws_word.bits.block_focal_laws, new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total);
				return ELOG(gb_log_msg, EPCIE_USER);
			}
		}
		else
		{
			if (prev_image_focal_law_total != new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total)
			{
				snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Total Image Focal Laws (Previous %d, Received %d)", prev_image_focal_law_total, new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total);
				return ELOG(gb_log_msg, EPCIE_USER);
			}
		}

		if (gb_last_acquierd_header.header_struct.ch_id_last.bits.channel_id != new_header_structed_p->header_struct.ch_id_last.bits.channel_id)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER: Channel Id (Previous %d, Received %d)", gb_last_acquierd_header.header_struct.ch_id_last.bits.channel_id, new_header_structed_p->header_struct.ch_id_last.bits.channel_id);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
	}
	else
	{
		if (new_header_structed_p->header_struct.block_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Block Index (Expected 0 - Received %d)", new_header_structed_p->header_struct.block_index);
			ELOG(gb_log_msg, EPCIE_USER);
			//return
		}
		if (!new_header_structed_p->header_struct.block_beats_word.bits.block_beats)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Block Beats (Expected not 0, Received %d)", new_header_structed_p->header_struct.block_beats_word.bits.block_beats);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		//if (!new_header_structed_p->header_struct.block_focal_laws_word.bits.block_focal_laws)
		//{
		//	snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Block Focal Laws (Expected not 0, Received %d)", new_header_structed_p->header_struct.block_focal_laws_word.bits.block_focal_laws);
		//	return ELOG(gb_log_msg, EPCIE_USER);
		//}
		if (new_header_structed_p->header_struct.image_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Image Index (Expected 0, Received %d)", new_header_structed_p->header_struct.image_index);
			ELOG(gb_log_msg, EPCIE_USER);
			//return;
		}
		if (new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_index)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Image Focal Law Index (Expected 0, Received %d)", new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_index);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
		if (!new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total)
		{
			snprintf(gb_log_msg, gb_log_msg == NULL ? 0 : SIZE_MSG_LOG, "ERROR HEADER (first block): Total Image Focal Laws (Expected not 0, Received %d)", new_header_structed_p->header_struct.image_focal_law_word.bits.image_focal_law_total);
			return ELOG(gb_log_msg, EPCIE_USER);
		}
	}
	memcpy((void *)&gb_last_acquierd_header, (const void *)new_header_structed_p, sizeof(T_block_header));
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
void PCIe_CheckPCIeBlockSaw(void *ptr_block, unsigned long long *channel_errors) // Adaptador legacy: el llamante garantiza la capacidad de ambos punteros.
{ // La funcion pura PCIe_CheckSaw permite diagnosticos internos con limites explicitos.
    T_PCIeGeometry geometry = T_PCIeGeometry(); // Usa la configuracion actual para dimensionar el diagnostico.
    int result = PCIe_ResolveGeometry(geometry); // No deduce ocho modulos del nombre del patron.
    if (result < 0) { RLOG(result); return; } // No interpreta memoria con una geometria invalida.
    if (!ptr_block || !channel_errors || gb_pcie_buffer_size_block != geometry.block_size_bytes) { ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); return; } // Exige un bloque de la adquisicion configurada.
    const T_block_header *header = static_cast<const T_block_header *>(ptr_block); // Solo consulta la cabecera tras comprobar punteros.
    unsigned int received_real_laws = 0; // El diagnostico usa la misma conversion que el gestor.
    if (PCIe_DecodeBlockLaws(header->header_struct.block_focal_laws_word.bits.block_focal_laws, geometry.transport_units_per_law, received_real_laws) < 0 || header->header_struct.magic_number != HEADER_MAGIC_NUMBER || header->header_struct.block_beats_word.bits.block_beats + 1 != geometry.beats_per_module || received_real_laws != static_cast<unsigned int>(geometry.focal_laws_per_block)) { ELOG(charEPCIE_BlockLayout, EPCIE_BlockLayout); return; } // Contrasta dimensiones antes del recorrido.
    const unsigned int valid_laws = header->header_struct.ch_id_last.bits.last_block ? header->header_struct.image_focal_law_word.bits.image_focal_law_total : geometry.focal_laws_per_block; // No recorre relleno del ultimo bloque.
    result = PCIe_CheckSaw(reinterpret_cast<const short *>(header + 1), static_cast<size_t>((geometry.block_size_bytes - 32) / 2), geometry, valid_laws, channel_errors, geometry.channel_count); // El contrato legacy exige tantos contadores como canales.
    if (result < 0) { RLOG(result); } // Registra fallos porque la firma historica devuelve void.
} // Fin del diagnostico compatible.
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_InitGeometryManager(int fifo_pointers, const T_PCIeGeometry &geometry) // Inicializa el gestor con el mismo contrato que DMA.
{ // Se llama despues de parar la adquisicion y reservar buffers.
    if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED); // Exige PCIe.
    if (fifo_pointers <= 0 || (geometry.focal_laws_per_image + geometry.focal_laws_per_block - 1) / geometry.focal_laws_per_block + 1 > fifo_pointers) return ELOG(charEPCIE_GeometrySize, EPCIE_GeometrySize); // Reserva margen para una imagen que cruza bloques.
    if (!gb_image_handle) gb_image_handle = new CImageHandle(gb_drv_handle, fifo_pointers, geometry); // El gestor conserva una copia de la geometria.
    else gb_image_handle->image_manager_init(gb_drv_handle, fifo_pointers, geometry); // Reinicia indices al cambiar de adquisicion.
    return EPCIE_NONE; // Gestor preparado.
} // Fin de inicializacion parametrizada.
int PCIe_image_manager_init(int fifo_pointers, int focal_laws_per_image, int focal_laws_per_block, int frame_length) // Conserva la firma historica.
{ // Comprueba que el cliente usa la configuracion actual.
    T_PCIeGeometry geometry = T_PCIeGeometry(); // Construye la instantanea.
    int result = PCIe_ResolveGeometry(geometry); // Reutiliza validaciones.
    if (result < 0) return RLOG(result); // Propaga fallos.
    if (focal_laws_per_image != geometry.focal_laws_per_image || focal_laws_per_block != geometry.focal_laws_per_block || frame_length != geometry.samples_per_channel) return ELOG(charEPCIE_Geometry, EPCIE_Geometry); // No mezcla dimensiones.
    return PCIe_InitGeometryManager(fifo_pointers, geometry); // Delega en la implementacion comun.
} // Fin del adaptador.
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferData_Burst(short int *data, unsigned long size_data, unsigned long timeout_ms)
{
	int result, i_image = 0, n_images_burst, status;
	C_Timeout *timer = nullptr;
	unsigned long i_buffer = 0;
	unsigned long n_samples, n_ch, n_fl, size_img;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (!ThrAcq_IsRunning(&status))
	{
		char str[255];
		sprintf(str, "Error, Acquisistion process is not running. Status: %d", status);
		return ELOG(str, EPCIE_THREAD_ACQ);
	}
	if (gb_image_handle == nullptr) return ELOG(charEPCIE_IMAGE_HANDLE_NULL, EPCIE_IMAGE_HANDLE_NULL);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
	if ((result = ST2_GetFocalLawNumber()) < 0) return RLOG(result);
	n_fl = result;
	if ((result = ST2_GetAScanDataNumber()) < 0) return RLOG(result);
	n_samples = result;
	if ((result = ST_GetChannelsNumber()) < 0) return RLOG(result);
	n_ch = result;
	size_img = n_samples * n_ch * n_fl;

	n_images_burst = ST_GetAcquisitionsBurstNumber();
	try
	{
		timer = new C_Timeout((double)timeout_ms);
		timer->Start();
		while (!timer->Timeout())
		{
			if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
			if ((result = gb_buffer_data->GetBufferNextImage(&data[i_buffer], size_data - i_buffer, nullptr, nullptr, 3000)) < 0) return RLOG(result);
			if (result > 0)
			{
				i_image++;
				if (i_image >= n_images_burst) 
					return i_image;
				i_buffer += size_img;
				if (i_buffer + size_img > size_data)
					return i_image;
			}
		}
	}
	__finally
	{
		if (timer != nullptr) delete timer;
	}
	return i_image;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetPtrBurstData(short int **data, unsigned long *idx_burst, unsigned long *timestamp, unsigned long *n_images_buffer, unsigned long timeout_ms)
{
	int result, i_image = 0, n_images_burst, status;
	//C_Timeout *timer = nullptr;
	unsigned long i_buffer = 0;
	unsigned long n_images_available, n_samples, n_ch, n_fl, size_img;
	short int *ptr = nullptr;

	if (!gb_pcie_enabled) return EPCIE_NONE;
	if (data == nullptr) return ELOG(charEPCIE_BUFFER_NULL, EPCIE_BUFFER_NULL);

	*data = nullptr;
	if (timestamp != nullptr) *timestamp = 0;
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (!ThrAcq_IsRunning(&status))
	{
		char str[255];
		sprintf(str, "Error, Acquisistion process is not running. Status: %d", status);
		return ELOG(str, EPCIE_THREAD_ACQ);
	}
	if (gb_image_handle == nullptr) return ELOG(charEPCIE_IMAGE_HANDLE_NULL, EPCIE_IMAGE_HANDLE_NULL);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);

	n_images_burst = ST_GetAcquisitionsBurstNumber();
	if ((result = ST2_GetFocalLawNumber()) < 0) return RLOG(result);
	n_fl = result;
	if ((result = ST2_GetAScanDataNumber()) < 0) return RLOG(result);
	n_samples = result;
	if ((result = ST_GetChannelsNumber()) < 0) return RLOG(result);
	n_ch = result;
	size_img = n_samples * n_ch * n_fl;

	try
	{
		//timer = new C_Timeout((double)timeout_ms);
		//timer->Start();
		if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
		gb_buffer_data->BackupNextImageRD();
		//while (!timer->Timeout())
		{
			if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
			if ((ptr = gb_buffer_data->GetPtrBufferNextBurst(idx_burst, timestamp, n_images_buffer, &n_images_available, timeout_ms)) != nullptr)
			{
				if (*data == nullptr) *data = ptr;
				return n_images_available;
			}
			else if ((result = ThrAcq_Error()) < 0) return RLOG(result);
		}
	}
	__finally
	{
		//if (timer != nullptr) delete timer;
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferAddress_Burst_2(unsigned long long *addr_data, unsigned long timeout_ms)
{
	return PCIe_GetPtrBurstData((short int **)addr_data, nullptr, nullptr, nullptr, timeout_ms);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferAddress(unsigned long long *addr_data, unsigned long *addr_timestamp, unsigned long *n_images_buffer, unsigned long timeout_ms)
{
	return PCIe_GetPtrBurstData((short int **)addr_data, nullptr, addr_timestamp, n_images_buffer, timeout_ms);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferAddress_Burst(unsigned long long *addr_data, unsigned long *idx_burst, unsigned long *timestamp, unsigned long *n_images_buffer, unsigned long timeout_ms)
{
  return PCIe_GetPtrBurstData((short int **)addr_data, idx_burst, timestamp,n_images_buffer, timeout_ms);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_FreeImageData(void)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_image_handle == nullptr) return ELOG(charEPCIE_IMAGE_HANDLE_NULL, EPCIE_IMAGE_HANDLE_NULL);
	return gb_image_handle->FreeOneImage_Pointers(NULL);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_FreeOneImage_Pointers(unsigned long *image_index_fred)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_image_handle != nullptr) return gb_image_handle->FreeOneImage_Pointers(image_index_fred);
	return 0;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
//int PCIe_GetChannelsNumber(void)
//{
//	return N_CHANNELS_MODULE;
//}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
//int PCIe_GetModulesNumber(void)
//{
//	return N_MODULES;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PCIe_SaveImageData(char *file, short int *data, unsigned long n_data)
{
	int result, data_int, acquisition_number, j, k, n_fl, n_ascan, n_samples, n_data_fl;
	unsigned long i_data;
	short int *fp_bscan = NULL, *pa_bscan = NULL;
	unsigned short data_ushort;
	char str[255];
	gcroot<TFileStream^> f;
	time_t t;

	try
	{
		if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
		if (file == NULL) return ELOG(charEPCIE_File, EPCIE_File);
		f = OpenFileStream(file, FS_Create);
		if (f.operator ->() == nullptr) return ELOG(charEPCIE_File, EPCIE_File);

		data_ushort = 0x1234;

		// Mascara Big-Little Endian
		data_ushort = 0x1234;
		if (FS_Write(f, (char*)&data_ushort, sizeof(unsigned short)) != sizeof(unsigned short)) return ELOG(charEPCIE_File, EPCIE_File);

		// Date, Time
		t = time(NULL);
		strftime(str, 20, "%d/%m/%Y %H:%M:%S", localtime(&t));
		if ((result = FS_Write(f, str, 20) != 20)) return RLOG(result);

		// Signal Range Max. Amplitude
		data_int = ST2_RangePA_MaxSignalAmplitude(0);
		if (FS_Write(f, (char*)&data_int, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		// Signal Range Min. Amplitude
		data_int = ST2_RangePA_MinSignalAmplitude(0);
		if (FS_Write(f, (char*)&data_int, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		// Number of acquisitions
		acquisition_number = ST2_GetAcquisitionsNumber();
		if (FS_Write(f, (char*)&acquisition_number, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		// Number of focal laws
		n_fl = ST2_GetFocalLawNumber();
		if (FS_Write(f, (char*)&n_fl, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		for (j = i_data = 0; j < n_fl; j++)
		{
			// Number of A-Scan signals
			n_ascan = ST2_MC_GetFocalLaw_AScanNumber(j);
			if (FS_Write(f, (char*)&n_ascan, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

			// Number of the samples of the each A-Scan
			n_samples = ST2_GetAScanDataNumber();
			if (FS_Write(f, (char*)&n_samples, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

			// Number of the samples of the focal law image
			n_data_fl = (int)(n_samples * n_ascan);
			if (FS_Write(f, (char*)&n_data_fl, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

			if (i_data + n_data_fl >= n_data) return ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);

			// Acquisition data
			for (k = 0; k < n_data_fl; k++)
			{
				if (FS_Write(f, (char*)&data, sizeof(short int)) != sizeof(short int)) return ELOG(charEPCIE_File, EPCIE_File);
			}
		}
	}
	__finally
	{
		delete f;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PCIe_CreateFileData(char *file)
{
	TFileStream^ f;

	try
	{
		if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
		if (file == NULL) return ELOG(charEPCIE_File, EPCIE_File);
		if ((f = FS_CreateFileImageData(file)) == nullptr) return ELOG(charEPCIE_File, EPCIE_File);
	}
	__finally
	{
		delete f;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PCIe_AddFileImageData(char *file, short int *data, unsigned long n_data)
{
	int result;
	gcroot<TFileStream^> f;

	try
	{
		if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
		if (file == NULL) return ELOG(charEPCIE_File, EPCIE_File);
		f = OpenFileStream(file, FS_OpenReadWrite);
		if (f.operator ->() == nullptr) return ELOG(charEPCIE_File, EPCIE_File);

		if ((result = FS_AddFileImageData(f, data, n_data)) < 0) return RLOG(result);
	}
	__finally
	{
		delete f;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferData_FMCMatrix(short int *data, unsigned long size_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms)
{
	int result = 0;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
	if ((result = gb_buffer_data->GetBufferNextImage(data, size_data, current_image_index, time_stamp, timeout_ms)) < 0) return RLOG(result);
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_GetBufferAddress_FMCMatrix(unsigned long long *addr_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms)
{
	int result = 0;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
	addr_data = (unsigned long long *)gb_buffer_data->GetPtrBufferNextImage(current_image_index, time_stamp, timeout_ms);
	return addr_data != nullptr ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_FreeBufferAddress(int n_images)
{
	int result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_NoBuffer, EPCIE_NoBuffer);
	if ((result = gb_buffer_data->FreePtrBufferImages(n_images)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
unsigned long long PCIe_GetBuffer_FMCMatrixNumber(void)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_buffer_data == nullptr) return 0;
	return gb_buffer_data->GetImagesCounter(nullptr);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_SaveFileBuffer(char *file)
{
	int result;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_buffer_data == nullptr) return ELOG(charEPCIE_EmptyBuffer, EPCIE_EmptyBuffer);
	if ((result = gb_buffer_data->SaveFileBuffer(file)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_ThreadIsRunning(void)
{
	int result = 1;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (!ThrAcq_IsRunning())
	{
		ThrAcq_Delete();
		result = 0;
	}
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_DMAThreadIsRunning(void)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr) return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	return gb_drv_handle->dma_is_running();
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_ThreadIsStopped(void)
{
	int result = 0;

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (ThrAcq_IsStopped())
	{
		ThrAcq_Delete();
		result = 1;
	}
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_InitBoard(void)
{
	int result;
	uint32_t mode;
	C_Timeout *timeout = new C_Timeout(500);

	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	if (gb_drv_handle == nullptr)
	{
		printf("Error"); 
		return ELOG(charEPCIE_DRV_NULL, EPCIE_DRV_NULL);
	}

	if ((result = gb_drv_handle->dma_hw_enable(0)) < 0) { RLOG(result); }

	ST_Stop();

	timeout->Start();
	while (!timeout->Timeout());

	mode = FUSE_HW_RST_DMA;
	//mode = FUSE_HW_RST_DMA | FUSE_HW_RST_MEM;
	if ((result = gb_drv_handle->dma_hw_reset(mode)) < 0) { RLOG(result); }

	//if ((result = gb_drv_handle->dma_rx_enable(1)) < 0) { RLOG(result); }
	PCIe_Close();
	//PCIe_Open();
	printf("OK");
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIE_MovingAverage_KernelSize(int data)
{
	if (!gb_pcie_enabled) return ELOG(charEPCIE_DISABLED, EPCIE_DISABLED);
	gb_moving_average_kernel_size = data;
	if (gb_buffer_data != nullptr) gb_buffer_data->MovingAverage_KernelSize(data);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int PCIe_LastImageRead(void)
{
	bool flag_acq = false;

	if (gb_pcie_enabled)
	{
		if (!gb_driver_enabled) flag_acq = true;
		else flag_acq = gb_drv_handle != nullptr ? gb_drv_handle->dma_is_running() : 0;
	}
	if (gb_pcie_continuous) return (gb_buffer_data != nullptr ? gb_buffer_data->LastImageRead() : 0) || !flag_acq;
	return (gb_buffer_data != nullptr ? gb_buffer_data->LastImageRead() : 0);
}
