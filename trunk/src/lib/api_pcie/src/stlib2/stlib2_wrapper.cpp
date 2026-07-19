// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdlib.h>
#include <string.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "stlib2.h"
#include "stlib2_wrapper.h"
#include "api_pcie.h"
#include "pcie_error_code.h"
#include "log.h"
#include "pcie.h"
#include "api_pcie.h"
#include "c_timeout.h"
#include "pcie_thread_acq.h"
#include "stlib2.h"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

vector<unsigned short> gb_pin_to_channel;

// _____________________________________________________________________________
// ------ System Functions
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_OpenSys(char* path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq, int pcie_enabled)
{
	int result, n_ch;

	//gb_drv_handle = nullptr;
	gb_pcie_n_samples = 0; // Número de muestras programado por el usuario
	gb_pcie_n_images_buffer = 0;
	gb_pcie_buffer_n_blocks_buf = 0; // Número de bloques recibidos por PCIe con su cabecera
	//gb_size_block = 0; // Tamaño de bloques recividos por PCIe con su cabecera (En Bytes)
	gb_pcie_n_blocks_acq_total = -1; // Número total de bloques a adquirir con su cabecera, cuando se completa este número finaliza la adquisición
	gb_pcie_n_beats_frames = 0;
	gb_pcie_n_focal_laws_block = 0;
	gb_check_pcie_header = 1;
	gb_driver_enabled = true;
	gb_pcie_enabled = true;
	gb_moving_average_kernel_size = 5;
	gb_pcie_buffer_size_block = 0;
	gb_pcie_closed = 1;

	if (pcie_enabled == 1 && ST_GetNoHardware() == 1)
	{
		//pcie_enabled = 0;
		gb_driver_enabled = false;
	}

	gb_pcie_enabled = pcie_enabled == 1;
	if (gb_pcie_enabled)
	{
		if ((result = PCIe_Open()) < 0) return RLOG(result);
		if ((result = PCIe_EnableRx(0)) < 0) return RLOG(result);
		gb_pcie_closed = 0;
	}
	if ((result = ST_OpenSys(path_bin, ip_address, tcp_port_cfg, tcp_port_acq)) < 0) return RLOG(result);
	if (pcie_enabled == 1)
	{
		if ((result = ST_SetStatusConfigPCIe(1)) < 0) return RLOG(result);
	}
	if ((n_ch = ST_GetChannelsNumber()) < 0) return RLOG(n_ch);
	if (n_ch == 0) return ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
	gb_pin_to_channel.resize(n_ch);
	if ((result = ST_GetProbePinToChannel(&gb_pin_to_channel[0], (int)gb_pin_to_channel.size(), 0)) < 0) return RLOG(result);
	if (gb_pcie_enabled)
	{
		if ((result = PCIe_EnableRx(1)) < 0) return RLOG(result);
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_CloseSys(void)
{
	ST_CloseSys();
	if (gb_pcie_enabled) PCIe_Close();
	if (gb_drv_handle != nullptr)
	{
		if (gb_driver_enabled && gb_drv_handle->dma_buffer_leakage() > 0) { ELOG(charEPCIE_BUFFER_LEAKAGE, EPCIE_BUFFER_LEAKAGE); }
		delete gb_drv_handle;
		gb_drv_handle = nullptr;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetNoHardware(int data)
{
	return ST_SetNoHardware(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetNoHardware(void)
{
	return ST_GetNoHardware();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetTemporaryFilesPath(char *path)
{
	return ST_SetTemporaryFilesPath(path);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetError(int error_code, char *str)
{
	return ST_GetError(error_code, str);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetLastError(char *str)
{
	return ST_GetLastError(str);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SaveXMLFileConfig(char *file)
{
	return ST_SaveXMLFileConfig(file);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetHWFormatData(int data)
{
	return ST_SetHWFormatData(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_LoadXMLFileConfig(char *file)
{
	int result, n_ch;// , n_images, n_focal_laws_block, n_focal_laws, n_samples;// , images_burst_number, bursts_number;
	int images_ddr_number = 800;
	//int data_sorted = 1;
	//int thread_dma_enabled = 0;
	//int thread_olink_enabled = 0;

	// CARGA LA CONFIGURACIÖN DESDE EL ARCHIVO XML

	if ((result = ST_LoadXMLFileConfig(file)) < 0) return RLOG(result);

	// INICIALIZA EL ENLACE PCIe
	//if (gb_pcie_enabled)
	//{
	//	if ((n_samples = ST_MC_GetAScanDataNumber(0)) < 0) return RLOG(n_samples);
	//	if ((n_images = ST_GetAcquisitionsNumber()) < 0) return RLOG(n_images);
		//if ((bursts_number = ST_GetBurstsNumber()) < 0) return RLOG(bursts_number);
		//if ((images_burst_number = ST_GetFMCMatrixBurstNumber()) < 0) return RLOG(images_burst_number);
		//if ((n_focal_laws = ST_GetFocalLawNumber(0)) < 0) return RLOG(n_focal_laws);
		//if ((n_focal_laws_block = ST_PCIe_GetFocalLawsBlockNumber()) < 0) return RLOG(n_focal_laws_block);

		//if ((result = PCIe_SetBuffers_mod(n_samples, n_focal_laws, images_ddr_number, n_images, n_focal_laws_block)) < 0) return RLOG(result);
		//if ((result = PCIe_image_manager_init(500, n_focal_laws, n_focal_laws_block, n_samples)) < 0) return RLOG(result);
		//if ((result = PCIe_Start(1, 0, 2, data_sorted, 1, images_burst_number, bursts_number, thread_dma_enabled, thread_olink_enabled)) < 0) return RLOG(result);
		//if ((result = PCIe_EnableRx(1)) < 0) return RLOG(result);
	//}

	//if (gb_pcie_enabled)
	{
		//if ((n_samples = ST_MC_GetAScanDataNumber(0)) < 0) return RLOG(n_samples);
		//if ((n_focal_laws = ST_GetFocalLawNumber(0)) < 0) return RLOG(n_focal_laws);
		//if ((n_images = ST_GetAcquisitionsNumber()) < 0) return RLOG(n_images);
		//if ((bursts_number = ST_GetBurstsNumber()) < 0) return RLOG(bursts_number);
		//if ((images_burst_number = ST_GetFMCMatrixBurstNumber()) < 0) return RLOG(images_burst_number);
		//if ((n_focal_laws_block = ST_PCIe_GetFocalLawsBlockNumber()) < 0) return RLOG(n_focal_laws_block);
		//if ((result = PCIe_SetBuffers_mod(n_samples, n_focal_laws, images_ddr_number, n_images, n_focal_laws_block)) < 0) return RLOG(result);
		//if ((result = PCIe_image_manager_init(500, n_focal_laws, n_focal_laws_block, n_samples)) < 0) return RLOG(result);
		//if ((result = PCIe_Start(1, 0, 2, data_sorted, 1, images_burst_number, bursts_number, thread_dma_enabled, thread_olink_enabled)) < 0) return RLOG(result);
		if ((n_ch = ST_GetChannelsNumber()) < 0) return RLOG(n_ch);
		if (n_ch == 0) return ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
		gb_pin_to_channel.resize(n_ch);
		if ((result = ST_GetProbePinToChannel(&gb_pin_to_channel[0], (int)gb_pin_to_channel.size(), 0)) < 0) return RLOG(result);
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SaveFileAcquisition(char *file)
{
	return ST_SaveFileAcquisition(file);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_LoadFileAcquisition(char *file)
{
	return ST_LoadFileAcquisition(file);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PrintFileConfig(char *file)
{
	return ST_PrintFileConfig(file, true);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PrintPASystemConfig(void)
{
	return ST_PrintPASystemConfig();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST2_InfoLibraryVersion(char *data)
{
	ST_InfoLibraryVersion(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_InfoFirmwareVersion(char *firmware_ethernet, char *firmware_uci_sw, char *firmware_uci_hw, char *firmware_mk32_hw)
{
	return ST_InfoFirmwareVersion(firmware_ethernet, firmware_uci_sw, firmware_uci_hw, firmware_mk32_hw);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST2_EnabledLoadUCIFirmware(int data)
{
	ST_EnabledLoadUCIFirmware(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SITAUHardwareEnabled(int data)
{
	return ST_SITAUHardwareEnabled(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetAcquisitionMode(int data)
{
	return ST_SetAcquisitionMode(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAcquisitionMode(void)
{
	return ST_GetAcquisitionMode();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetAcquisitionType(int data)
{
	return ST_SetAcquisitionType(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAcquisitionType(void)
{
	return ST_GetAcquisitionType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetStatus(unsigned __int64* status_code, unsigned long* rd_address, unsigned long* wr_address, unsigned long* n_overflows, unsigned char* sitau_status, unsigned char* trigger_source, unsigned long* n_acquisitions, unsigned long* ind_acquisitions, unsigned long* size_buffer_command)
{
	return ST_GetStatus(status_code, rd_address, wr_address, n_overflows, sitau_status, trigger_source, n_acquisitions, ind_acquisitions, size_buffer_command);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetStatusFrame(unsigned char* status_mem, int size_mem)
{
	return ST_GetStatusFrame((unsigned __int8*)status_mem, size_mem);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetChannelsNumber(void)
{
	return ST_GetChannelsNumber();
}

// _____________________________________________________________________________
// ------ Acqusition Data Functions
// =============================================================================

// ------ Control Acquisition Functions ----------------------------------------

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAScanDataNumber(void)
{
	return ST_GetAScanDataNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetAScanNumber(void)
{
	return ST_MC_GetAScanNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetMatrixCaptureSize(void)
{
	return ST_MC_GetMatrixCaptureSize();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetSingleImageSize(void)
{
	return ST_MC_GetSingleImageSize();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SaveAcquisitionData(char *file)
{
	return ST_SaveAcquisitionData(file);
}

// ------ Start / Stop Automatic Acquisition Functions -------------------------

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST2_StartPRFScan(void)
//{
//	return ST_StartPRFScan();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAcquisitionsNumber(void)
{
	return ST_GetAcquisitionsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAcquisitionsBurstNumber(void)
{
	return ST_GetAcquisitionsBurstNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetBurstsNumber(void)
{
	return ST_GetBurstsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetAcquisitionsNumber(unsigned long n_blocks, unsigned long images_block)
{
	return ST_SetAcquisitionsNumber(n_blocks, images_block);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetDelaySync(float data)
{
	return ST_SetDelaySync(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetDelaySync(void)
{
	return ST_GetDelaySync();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_Start(int data_sorted)
{
	int bursts_number, images_burst_number, n_samples, n_focal_laws, n_focal_laws_block, n_images;
	int result;// , data_sorted = 1;
	int thread_dma_enabled = 1;
	int thread_olink_enabled = 1;
	int images_ddr_number = 800;
	int dma_blocks_number = 500;
	int acq_mode = ST2_GetAcquisitionMode();
	C_Timeout *timeout = nullptr;

	if ((images_ddr_number = ST_XML_GetBuffer_PCIeFIFOPointersNumber()) < 0) images_ddr_number = 512;
	if ((dma_blocks_number = ST_XML_GetBuffer_PCIeDMABlocksNumber()) < 0) dma_blocks_number = 500;

	//// INICIALIZA EL ENLACE PCIe
	//if (gb_pcie_enabled)
	//{
	//	if ((n_samples = ST_MC_GetAScanDataNumber(0)) < 0) return RLOG(n_samples);
	//	//if ((n_images = ST_GetAcquisitionsNumber()) < 0) return RLOG(n_images);
	//	if ((bursts_number = ST_GetBurstsNumber()) < 0) return RLOG(bursts_number);
	//	if ((images_burst_number = ST_GetFMCMatrixBurstNumber()) < 0) return RLOG(images_burst_number);
	//	if ((n_focal_laws = ST_GetFocalLawNumber(0)) < 0) return RLOG(n_focal_laws);
	//	if ((n_focal_laws_block = ST_PCIe_GetFocalLawsBlockNumber()) < 0) return RLOG(n_focal_laws_block);

	//	//if ((result = PCIe_SetBuffers_mod(n_samples, n_focal_laws, images_ddr_number, n_images, n_focal_laws_block)) < 0) return RLOG(result);
	//	if ((result = PCIe_image_manager_init(500, n_focal_laws, n_focal_laws_block, n_samples)) < 0) return RLOG(result);
	//	if ((result = PCIe_Start(1, 0, 2, data_sorted, 1, images_burst_number, bursts_number, thread_dma_enabled, thread_olink_enabled)) < 0) return RLOG(result);
	//	//if ((result = PCIe_EnableRx(1)) < 0) return RLOG(result);
	//}

	////if (gb_pcie_enabled && (result = PCIe_EnableRx(1)) < 0) return RLOG(result);

	//if ((result = ST_Start()) < 0) return RLOG(result);


	//if (acq_mode == 1 && PCIe_ThreadIsRunning() == 1) 
		ST2_Stop();

	// INICIALIZA EL ENLACE PCIe
	if (gb_pcie_enabled)
	{
		if (ST_PCIe_GetBlocksBurstNumber() > 256) return ELOG(charEPCIE_GTX_BLOCKS, EPCIE_GTX_BLOCKS);

		if (gb_pcie_closed)
		{
			if ((result = PCIe_Open()) < 0) return RLOG(result);
			if ((result = PCIe_EnableRx(1)) < 0) return RLOG(result);
			gb_pcie_closed = 0;
		}

		if ((n_samples = ST_MC_GetAScanDataNumber(0)) < 0) return RLOG(n_samples);
		if ((n_images = ST_GetAcquisitionsNumber()) < 0) return RLOG(n_images);
		if ((bursts_number = ST_GetBurstsNumber()) < 0) return RLOG(bursts_number);
		if ((images_burst_number = ST_GetAcquisitionsBurstNumber()) < 0) return RLOG(images_burst_number);
		if ((n_focal_laws = ST_GetFocalLawNumber(0)) < 0) return RLOG(n_focal_laws);
		if ((n_focal_laws_block = ST_PCIe_GetFocalLawsBlockNumber()) < 0) return RLOG(n_focal_laws_block);

		if ((result = PCIe_SetBuffers_mod(n_samples, n_focal_laws, images_ddr_number, n_images, n_focal_laws_block)) < 0) return RLOG(result);
		if ((result = PCIe_image_manager_init(dma_blocks_number, n_focal_laws, n_focal_laws_block, n_samples)) < 0) return RLOG(result);
		if ((result = PCIe_Start(1, 0, 2, data_sorted, 1, images_burst_number, bursts_number, thread_dma_enabled, thread_olink_enabled)) < 0) return RLOG(result);
		//if ((result = PCIe_EnableRx(1)) < 0) return RLOG(result);
	}
	//if (gb_pcie_enabled && (result = PCIe_EnableRx(1)) < 0) return RLOG(result);

	if ((result = ST_Start())< 0) return RLOG(result);

	timeout = new C_Timeout(2000);
	timeout->Start();
	while (!ThrAcq_IsRunning() && !timeout->Timeout());
	if (timeout->Timeout()) printf("\r\nTIMEOUT");
	delete timeout;
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetTriggerSource(int data)
{
	int result;

	if ((result = ST_SetTriggerSource(data)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetTriggerSource(void)
{
	int result;

	if ((result = ST_GetTriggerSource()) < 0) return RLOG(result);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_Stop(void)
{
	if (gb_pcie_enabled)
	{
    ST_Continue();
		PCIe_Stop();
		PCIe_Close();
		gb_pcie_closed = 1;
	}
	ST_Stop();
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_Pause(void)
{
	return ST_Pause();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_Continue(void)
{
	return ST_Continue();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPauseAfterBurst(int data)
{
  return ST_SetPauseAfterBurst(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetPauseAfterBurst(void)
{
  return ST_GetPauseAfterBurst();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FullAcquisition(int *fl_image_counter)
{
	return ST_FullAcquisition(fl_image_counter);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_Acquiring(void)
{
	return ST_Acquiring();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST2_MC_GetBuffer_LastSingleImage(short int* data, unsigned long size_data, unsigned long* index_focal_law, unsigned long* index_trigger)
//{
//	return ST_MC_GetBuffer_LastSingleImage(data, size_data, index_focal_law, index_trigger);
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetBuffer_SingleImage(short int* data, unsigned long size_data, int index_focal_law, unsigned long index_trigger)
{
	return ST_MC_GetBuffer_SingleImage(data, size_data, index_focal_law, index_trigger);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetBuffer_LastMatrixCapture(short int* data, unsigned long size_data, unsigned long *index_trigger)
{
	return ST_MC_GetBuffer_LastMatrixCapture(data, size_data, index_trigger);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetBuffer_MatrixCapture(short int* data, unsigned long size_data, unsigned long index_trigger)
{
	return ST_MC_GetBuffer_MatrixCapture(data, size_data, index_trigger);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux)
{
	return ST_PA_GetBuffer_LastImage(data, size_data, n_data, index_trg, index_aux);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux)
{
	return ST_PA_GetBuffer_Image(data, size_data, n_data, index_trg, index_aux);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_TFM_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux)
{
	return ST_TFM_GetBuffer_LastImage(data, size_data, n_data, index_trg, index_aux);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_TFM_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux)
{
	return ST_TFM_GetBuffer_Image(data, size_data, n_data, index_trg, index_aux);
}

// _____________________________________________________________________________
// ------ Focal Law Parameters (Full Parallel)
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_SetEmissionFocalLawFile(char* file)
{
	return ST_MC_SetEmissionFocalLawFile(file);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_SetFullMatrixCapture(void)
{
	return ST_MC_SetFullMatrixCapture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_SetPlaneWaveImaging_Angles_deg(float *angle, int n_angles)
{
	return ST_MC_SetPlaneWaveImaging_Angles_deg(angle, n_angles);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_SetPlaneWaveImaging_Angles_rad(float *angle, int n_angles)
{
	return ST_MC_SetPlaneWaveImaging_Angles_rad(angle, n_angles);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_SetPlaneWaveImaging(float start_angle, float step_angle, float end_angle)
{
	return ST_MC_SetPlaneWaveImaging(start_angle, step_angle, end_angle);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_MC_GetPlaneWaveImaging_StartAngle(void)
{
	return ST_MC_GetPlaneWaveImaging_StartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_MC_GetPlaneWaveImaging_StepAngle(void)
{
	return ST_MC_GetPlaneWaveImaging_StepAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_MC_GetPlaneWaveImaging_EndAngle(void)
{
	return ST_MC_GetPlaneWaveImaging_EndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetPlaneWaveImaging_Angles_rad(float* data, int size_data)
{
	return ST_MC_GetPlaneWaveImaging_Angles_rad(data, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetPlaneWaveImaging_Angles_deg(float* data, int size_data)
{
	return ST_MC_GetPlaneWaveImaging_Angles_deg(data, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetPlaneWaveImaging_AnglesNumber(void)
{
	return ST_MC_GetPlaneWaveImaging_AnglesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetFocalLawType(void)
{
	return ST_MC_GetFocalLawType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetFocalLawNumber(void)
{
	return ST_GetFocalLawNumber(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_MC_GetFocalLaw_AScanNumber(int id_focal_law)
{
	return ST_MC_GetFocalLaw_AScanNumber(id_focal_law);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_RangePA_MaxSignalAmplitude(int id_vch)
{
	return ST_RangePA_MaxSignalAmplitude(id_vch);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_RangePA_MinSignalAmplitude(int id_vch)
{
	return ST_RangePA_MinSignalAmplitude(id_vch);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_RangeMaxFocalLaws(void)
{
	return ST_RangeMaxFocalLaws();
}

// _____________________________________________________________________________
// ------ Focal Law Parameters (Phased Array)
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_SetLinearScanConfig(int step, int aperture, int startelement, int endelement, float angle)
{
	return ST_PA_SetLinearScanConfig(step, aperture, startelement, endelement, angle);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_SetAngularScanConfig(float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle)
{
	return ST_PA_SetAngularScanConfig(step, aperture, startelement, startangle, endangle);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetAperture(void)
{
	return ST_PA_GetAperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetScanType(void)
{
	return ST_PA_GetScanType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetLinearScanStep(void)
{
	return ST_PA_GetLinearScanStep();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetScanStartElement(void)
{
	return ST_PA_GetScanStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_GetLinearScanEndElement(void)
{
	return ST_PA_GetLinearScanEndElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_PA_GetLinearScanAngle(void)
{
	return ST_PA_GetLinearScanAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_PA_GetAngularScanStartAngle(void)
{
	return ST_PA_GetAngularScanStartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_PA_GetAngularScanEndAngle(void)
{
	return ST_PA_GetAngularScanEndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_PA_GetAngularScanStepAngle(void)
{
	return ST_PA_GetAngularScanStepAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_PA_SetEmissionFocalPoint(float emission_focus)
{
	return ST_PA_SetEmissionFocalPoint(emission_focus);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_PA_GetEmissionFocalPoint(void)
{
	return ST_PA_GetEmissionFocalPoint();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetImageEntryPoint_X(float *x_coordinate, int size_data)
{
	return ST_GetImageEntryPoint_X(x_coordinate, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST2_GetImageEntryPoint_Y(float *y_coordinate, int size_data)
//{
//	return ST_GetImageEntryPoint_Y(y_coordinate, size_data);
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetImageEntryPoint_Z(float *z_coordinate, int size_data)
{
	return ST_GetImageEntryPoint_Z(z_coordinate, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetImageEntryPoint_Angle(float *angle, int size_data)
{
	return ST_GetImageEntryPoint_Angle(angle, size_data);
}

// _____________________________________________________________________________
// ------ Acquisition Parameters Functions
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetGain(float data)
{
	return ST_SetGain(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetGain(void)
{
	return ST_GetGain();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetMaterialVelocity(float data)
{
	return ST_SetMaterialVelocity(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetMaterialVelocity(void)
{
	return ST_GetMaterialVelocity();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetAcqDistance(float start_distance, float end_distance, int acquisition_time_mode)
{
	return ST_SetAcqDistance(start_distance, end_distance, acquisition_time_mode);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetAcqTime(float start_time, float end_time)
{
	return ST_SetAcqTime(start_time, end_time);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetAcqStartDistance(void)
{
	return ST_GetAcqStartDistance();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetAcqEndDistance(void)
{
	return ST_GetAcqEndDistance();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetAcqStartTime(void)
{
	return ST_GetAcqStartTime();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetAcqEndTime(void)
{
	return ST_GetAcqEndTime();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetAcqTime(void)
{
	return ST_GetAcqTime();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetAcquisitionTimeMode(void)
{
	return ST_GetAcquisitionTimeMode();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetSamplingFrequency(int data)
{
	return ST_SetSamplingFrequency(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetSamplingFrequency(void)
{
	return ST_GetSamplingFrequency();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetDownSamplingFactor(void)
{
	return ST_GetDownSamplingFactor(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetPRF(void)
{
	return ST_GetPRF();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPRF(float data)
{
	return ST_SetPRF(data);
}

// _____________________________________________________________________________
// ------ Digital Signal Processing Functions
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetProcessing(int processing_type, int n_signals, int wait_prf)
{
	return ST_SetProcessing(processing_type, n_signals, wait_prf, 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetProcessingType(void)
{
	return ST_GetProcessingType(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetProcessingSignalsNumber_AVR(void)
{
	return ST_GetProcessingSignalsNumber_AVR(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetHWRFFilterEnable(int data)
{
	return ST_SetHWRFFilterEnable(data, 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetHWRFFilterEnable(void)
{
	return ST_GetHWRFFilterEnable(0);

}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetHWRFFilter(float low_frequency, float high_frequency)
{
	return ST_SetHWRFFilter(low_frequency, high_frequency, 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetHWRFFilterLow(void)
{
	return ST_GetHWRFFilterLow(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetHWRFFilterHigh(void)
{
	return ST_GetHWRFFilterHigh(0);
}

// _____________________________________________________________________________
// ------ Pulser Functions
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetHighVoltageMaximum(int data)
{
	ST_SetHighVoltageMaximum(data);
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetHighVoltageMaximum(void)
{
	return ST_GetHighVoltageMaximum();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPulseAmplitude(int data)
{
	return ST_SetPulseAmplitude(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetPulseAmplitude(void)
{
	return ST_GetPulseAmplitude();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPulserEnable(int data)
{
	return ST_SetPulserEnable(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetPulserEnable(void)
{
	return ST_GetPulserEnable();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPulseWidth(float data)
{
	return ST_SetPulseWidth(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetPulsesNumber(int data)
{
	return ST_SetPulsesNumber(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetPulseWidth(void)
{
	return ST_GetPulseWidth();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetPulsesNumber(void)
{
	return ST_GetPulsesNumber();
}

// _____________________________________________________________________________
// ------ Phased Array Probe Parameters
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_SetProbeConfig(int n_element, float pitch, float frequency)
{
	return ST_SetProbeConfig(n_element, pitch, frequency);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetProbePitch(void)
{
	return ST_GetProbePitch();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_GetProbeElementNumber(void)
{
	return ST_GetProbeElementNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_GetProbeFrequency(void)
{
	return ST_GetProbeFrequency();
}

// _____________________________________________________________________________
// ------ (fUS) Functional Ultrasound Imaging
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_CentralFrequency(void)
{
	return ST_FUS_CentralFrequency();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_RapportFocalAperture(void)
{
	return ST_FUS_RapportFocalAperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_ApodizationCoefficient(void)
{
	return ST_FUS_ApodizationCoefficient();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_LateralPositionX(void)
{
	return ST_FUS_LateralPositionX();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_PixelSizeX(void)
{
	return ST_FUS_PixelSizeX();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FUS_PixelsX(void)
{
	return ST_FUS_PixelsX();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_DepthZ(void)
{
	return ST_FUS_DepthZ();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FUS_PixelsZ(void)
{
	return ST_FUS_PixelsZ();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST2_FUS_PixelSizeZ(void)
{
	return ST_FUS_PixelSizeZ();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FUS_SinglePlane(void)
{
	return ST_FUS_SinglePlane();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FUS_SamplesWavelength(void)
{
	return ST_FUS_SamplesWavelength();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST2_FUS_InterpolationCoefficients(void)
{
	return ST_FUS_InterpolationCoefficients();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST2_MC_SetGEBufferSize(int n_acquisitions)
{
	ST_MC_SetGEBufferSize(n_acquisitions);
}