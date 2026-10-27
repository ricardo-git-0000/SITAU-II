// -----------------------------------------------------------------------------
/**
 @file main.cpp

 @brief Librería \c stlib2.dll.<br>
 
 Este código es una demostración del uso de algunas funciones de la
 librería \c stlib2.dll.
 
 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date       Changes
 ----- ---- ---------- ---------------------------------------------------------
 1.00a (rg) 15/03/2023 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================

#include <conio.h>
#include <vector>
#include <time.h>
#include <string>

using namespace std;

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ======================================================================

#include "api_pcie.h"
#include "c_timeout.h"

static string ErrorText(int error_code) // Consulta la misma descripcion que utiliza MATLAB.
{ // Mantiene el buffer local para no compartir mensajes entre llamadas.
    char description[4096] = { 0 }; // Reserva espacio para el catalogo de errores.
    if (ST2_GetError(error_code, description) != 1) sprintf_s(description, sizeof(description), "(Error Code %d) Codigo de error no documentado", error_code); // Hace explicitos los codigos desconocidos.
    return string(description); // Devuelve un mensaje independiente del buffer local.
} // Fin del formateador de la aplicacion de consola.

// ______________________________________________________________________
// ------ T I P O S   D E   D A T O S
// ======================================================================

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

/*!< Modo de adquisición: 
     - (true) Modo continuo
     - (false) realiza un numero de adquisiciones. */
//bool gb_acquisition_continuous = true;
bool gb_acquisition_continuous = false;


/*!< Numero de adquisiciones. */
int gb_buffer_blocks_number = 3;
int gb_buffer_images_block_number = 20;
int gb_images_buffer_number = 500;
/*!< Selección de canal de comunicación:
     - (0) GEthernet
     - (1) GTX
     - (2) GEthernet + GTX */
int gb_data_link = 1; 

int gb_thread_acq = 1;
int gb_thread_acq_sleep = 0;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int OpenSys (char* path, char *ip_address, int tcp_port_cfg, int tcp_port_acq, int pcie_enabled)
{
int result;
char str[255];
char firmware_ethernet[255], firmware_boot[255], firmware_uci_hw[255], firmware_uci_sw[255], firmware_base[255], firmware_module[255];

	ST2_InfoLibraryVersion(str);
	printf("\r\nSTLIB2.DLL Version %s", str);
	PCIe_GetLibraryVersion(str);
	printf("\r\nAPI_PCIe.DLL Version %s", str);

	printf("\r\n Opening the SITAU system... ");
	if ((result = ST2_OpenSys ( path, ip_address, tcp_port_cfg, tcp_port_acq, pcie_enabled)) < 0) 
	{
		printf("ERROR while executing ST_OpenSys");
	    return result;
	}
	printf("OK");  

	//Sleep(5000);

	if ((result = ST2_InfoFirmwareVersion (&firmware_ethernet[0], &firmware_uci_sw[0], &firmware_uci_hw[0], &firmware_module[0])) < 0)
		printf("\r\nERROR Getting Firmware Version");
	else
	{
		printf("\r\n SITAU Ethernet Firmware Version: %s", firmware_ethernet);
		printf("\r\n SITAU UCI SW Firmware Version:   %s", firmware_uci_sw);
		printf("\r\n SITAU UCI HW Firmware Version:   %s", firmware_uci_hw);
		printf("\r\n SITAU MODULE Firmware Version:   %s", firmware_module);
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_GEStatus(void)
{
   int result;
   bool end_acquisition = false;
   char chr;
   unsigned __int64 status_code;
   unsigned long rd_address, wr_address, n_overflows, n_acquisitions, ind_acquisitions, size_buffer_command;
   unsigned char sitau_status, trigger_source;
   C_Timeout* timer_status = new C_Timeout(2000);

   printf("\r\n Starting Acquisition... ");
   if ((result = ST2_Start(1)) < 0)
   {
	   printf("Error Starting Acquisition");
	   return;
   }
   printf("OK Starting Acquisition");

   while (!end_acquisition)
   {
      if (_kbhit())
      {
         chr = _getch();
         end_acquisition = true;
      }
      else
      {
         if (timer_status->Timeout())
         {
            // Lectura del estado
            if ((result = ST2_GetStatus(&status_code, &rd_address, &wr_address, &n_overflows, &sitau_status, &trigger_source, &n_acquisitions, &ind_acquisitions, &size_buffer_command)) < 0)
            {
               printf("\r\n %s - ST_GetStatus() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
            }
            printf("\r\n Status Code (0x%.16llX) - RD Addr (0x%.8X) - WR Addr (0x%.8X) - N. Overflows (%lu) - SITAU Status (%d) - Trig. Source (%d) - N. Acquisitions (%lu) - Ind. Acquisitions (%lu) - Buffer Size (%lu)", status_code, rd_address, wr_address, n_overflows, sitau_status, trigger_source, n_acquisitions, ind_acquisitions, size_buffer_command);
            timer_status->Start();
         }
      }
   }
   // Detiene el proceso de adquisición.
   ST2_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_LastImage(void)
{
	int result, n_blocks_read, n_samples, n_focal_laws, n_ch, n_trigger_lines;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0, image_size, focal_law_size;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada, n_data, images, time_stamp, index_trg;
	unsigned short *punteros_datos_leyes_focales[1024];
	unsigned long long image_addr;
	short int *ptr_data = NULL;

	printf("\r\n Starting Acquisition... ");
	if ((result = ST2_Start(1)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

  if ((image_size = ST2_MC_GetMatrixCaptureSize()) < 0)
  {
    printf("Error ST2_MC_GetMatrixCaptureSize()");
    return;
  }
  if ((focal_law_size = ST2_MC_GetSingleImageSize()) < 0)
  {
    printf("Error ST2_MC_GetSingleImageSize()");
    return;
  }
  if ((n_samples = ST2_GetAScanDataNumber()) < 0)
  {
    printf("Error ST2_GetAScanDataNumber()");
    return;
  }
	if ((n_focal_laws = ST2_GetFocalLawNumber()) < 0)
	{
		printf("Error ST2_GetFP_FocalLaw_Number()");
		return;
	}
	if ((n_ch = ST2_GetChannelsNumber()) < 0)
	{
		printf("Error ST2_GetChannelsNumber()");
		return;
	}
	if ((n_trigger_lines = ST2_GetAcquisitionsNumber()) < 0)
	{
		printf("Error ST2_GetAcquisitionsNumber()");
		return;
	}

	if ((ptr_data = (short int *)malloc(image_size * 2)) == NULL)
	{
		printf("Error malloc()");
		return;
	}

	while (!end_acquisition)
	{
		if (_kbhit())
		{
			chr = _getch();
			end_acquisition = true;
		}
		else
		{
			
			if ((result = ST2_MC_GetBuffer_LastMatrixCapture(ptr_data, image_size, &index_trg)) < 0)
			{
				printf("\r\n %s - ST2_GetBufferFP_LastImage() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
				end_acquisition = true;
			}
			else if (result > 0) printf("\r\nImagen extraida del buffer: Index Trigger (%d) Samples Read (%d) --> (%d Samples/Image - %d Samples/FocalLaw - %d Samples/A-Scan - %d FocalLaws)", index_trg, result, image_size, focal_law_size, n_samples, n_focal_laws);
		}
	}
	ST2_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_GetBufferData_Burst(bool sorted)
{
	int result, fmc_size, n_bursts, n_samples, n_fl, images_per_burst, burst_size, counter_img, counter_start;
	unsigned long i;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada, n_data, images, time_stamp, index_trg;
	unsigned short *punteros_datos_leyes_focales[1024];
	unsigned long long image_addr;
	short int *ptr_data = NULL;
	float mean;
	int min_data, max_data;
	unsigned long min_data_idx, max_data_idx;
	bool end_acquisition = false, wait_acquisition = false;
	int acq_mode;
	unsigned long counter_burst, counter_images, counter_images_burst;
	FILE *f;
	char msg[1024];
	C_Timeout * timer = NULL;
	double mbPerSeconds;

	f = fopen("test.txt", "wt");

	if ((result = ST2_LoadXMLFileConfig("SITAU2_PCIe_TEST.XML")) < 0)
	{
		snprintf(msg, sizeof(msg), "\r\n %s - ST2_LoadXMLFileConfig() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
		printf("%s", msg); // Imprime el mensaje como texto literal.
		fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
		return;
	}

	fmc_size = ST2_MC_GetMatrixCaptureSize();
	images_per_burst = ST2_GetAcquisitionsBurstNumber();
	burst_size = fmc_size * images_per_burst;

	//acq_mode = ST2_GetAcquisitionMode();
	//n_fl = ST2_GetFocalLawNumber();
	//n_samples = ST2_GetAScanDataNumber();
	//n_bursts = ST2_GetBurstsNumber();
	//fmc_size = ST2_MC_GetMatrixCaptureSize();
	//images_per_burst = ST2_GetBurstsNumber();// ST2_GetFMCMatrixBurstNumber();
	//burst_size = fmc_size;// *images_per_burst;

	//if (acq_mode == 1) printf("ACQUISITION MODE: CONTINUOUS");
	//else printf("ACQUISITION MODE: FIXSD");
	//printf("\r\n Focal Law Number: %d", n_fl);
	//printf("\r\n Samples Number: %d", n_samples);
	//printf("\r\n Burst Number: %d", n_bursts);
	//printf("\r\n MC Burst Number: %d", images_per_burst);
	//printf("\r\n MC Size: %d", fmc_size);
	//printf("\r\n Burst Size: %d", burst_size);

	if ((ptr_data = (short int *)malloc(burst_size * 2)) == NULL)
	{
		snprintf(msg, sizeof(msg), "\r\n Error malloc (%d Bytes)", burst_size * 2);
		printf("%s", msg); // Imprime el mensaje como texto literal.
		fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
		return;
	}

	counter_start = 0;
	if (sorted) printf("\r\n (%d) Starting Acquisition (SORTED)... ", counter_start);
	else printf("\r\n (%d) Starting Acquisition (UNSORTED)... ", counter_start);
	if ((result = ST2_Start(sorted ? 1 : 0)) < 0)
	{
		snprintf(msg, sizeof(msg), "\r\n %s - ST2_Start() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
		printf("%s", msg); // Imprime el mensaje como texto literal.
		fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
		return;
	}
	printf("OK");

	timer = new C_Timeout();
	timer->Start();
	counter_burst = counter_images = counter_images_burst = mbPerSeconds = 0;
	while (!end_acquisition)
	{
		if (_kbhit())
		{
			chr = _getch();
			if (chr == 's' || chr == 'S')
			{
				if (sorted) printf("\r\n (%d) Starting Acquisition (SORTED)... ", counter_start);
				else printf("\r\n (%d) Starting Acquisition (UNSORTED)... ", counter_start);
				if ((result = ST2_Start(sorted ? 1 : 0)) < 0)
				{
					printf("Error");
					return;
				}
				printf("OK");
				counter_start++;
			}
			else end_acquisition = true;
		}
		else
		{
			if ((result = PCIe_GetBufferData_Burst(ptr_data, burst_size, 3000)) < 0)
			{
				snprintf(msg, sizeof(msg), "\r\n %s - PCIe_GetBufferData_Burst() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
				printf("%s", msg); // Imprime el mensaje como texto literal.
				fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				end_acquisition = true;
			}
			else
			{
				if (result > 0)
				{
					counter_images += result;
					counter_images_burst += result;
					if (counter_images_burst >= images_per_burst)
					{
						counter_images_burst = 0;
						counter_burst++;
					}
					mbPerSeconds = fmc_size / (timer->Time() * 1000000) * counter_images * 2;

					snprintf(msg, sizeof(msg), "\n%.2f GB/s - PCIe_GetBufferData_Burst(): %d acquired images - %d Bursts", mbPerSeconds, result, counter_burst);
					printf("%s", msg); // Imprime el mensaje como texto literal.
					fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				}
				else
				{
					snprintf(msg, sizeof(msg), "\nPCIe_GetBufferData_Burst(): TIMEOUT");
					printf("%s", msg); // Imprime el mensaje como texto literal.
					fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				}
			}
		}
	}
	delete timer;
	ST2_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_TestTransfer_FullBW(int sorted)
{
	int result, fmc_size, n_bursts, n_samples, n_fl, images_per_burst, counter;
	unsigned long i;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long n_images_buffer, olink_mask_event, dma_mask_event, pci_mask_event;
	int errores_cabecera_totales = 0, acq_mode;
	unsigned long acquired_image_index, imagen_liberada, n_data, images, time_stamp, index_trg;
	unsigned short *punteros_datos_leyes_focales[1024];
	unsigned long long image_addr;
	short int *ptr_data = NULL;
	float mean;
	int min_data, max_data;
	unsigned long idx, min_data_idx, max_data_idx;
	unsigned long long addr_data = 0;
	unsigned long timestamp = 0;
	unsigned long idx_burst = 0;
	bool wait_acquisition = false;
	int burst_number, n_channels, n_images_burst, n_angles;
	float burst_size, sizeGb, t_sec, acq_size;
	C_Timeout *timer;

	//if ((result = ST2_LoadXMLFileConfig("SITAU2_TRANSF_FULLBW.XML")) < 0) return;
	if ((result = ST2_LoadXMLFileConfig("SITAU2_PCIe_TEST.XML")) < 0) return;


	if ((result = ST2_SetPulseAmplitude(30)) < 0) return;

	burst_number = ST2_GetBurstsNumber();
	n_samples = ST2_GetAScanDataNumber();
	n_channels = ST2_GetProbeElementNumber();
	n_images_burst = ST2_GetAcquisitionsBurstNumber();
	n_angles = ST2_MC_GetPlaneWaveImaging_AnglesNumber();

	burst_size = n_samples * n_channels * n_angles * n_images_burst;
	sizeGb = burst_size * 2 / (float)1e9;

	printf("\r\n Focal Law Number: %d", n_angles);
	printf("\r\n Samples Number: %d", n_samples);
	printf("\r\n Burst Number: %d", burst_number);
	printf("\r\n Burst Size: %f GB", sizeGb);

	acq_mode = ST2_GetAcquisitionMode();
	switch (acq_mode)
	{
		case 0: // ACQUISITION_MODE_FIXED
			printf("\r\n ACQUISITION_MODE_FIXED");
			break;
		case 1:	// ACQUISITION_MODE_CONTINOUS
			printf("\r\n ACQUISITION_MODE_CONTINOUS");
			break;
		default:
			printf("\r\n ACQUISITION MODE Unknown");
			break;
	}

	if ((ptr_data = (short int *)malloc(burst_size * 2)) == NULL)
	{
		printf("Error malloc()");
		return;
	}

	printf("\r\n Starting Acquisition SORTED = %d... ", sorted);
	if ((result = ST2_Start(sorted)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK\n");

	timer = new C_Timeout();
	timer->Start();
	counter = 0;
	while (!end_acquisition)
	{
		if (_kbhit())
		{
			chr = _getch();
			end_acquisition = true;
		}
		else
		{
			if ((result = PCIe_GetBufferAddress_Burst(&addr_data, &idx_burst, &timestamp, &n_images_buffer, 3000)) < 0)
			{
				printf("\n %s - PCIe_GetBufferAddress_Burst()\n", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
				end_acquisition = true;
			}
			if (result > 0)
			{
				PCIe_FreeBufferAddress(result);
				counter++;
				t_sec = timer->Time() / 1000;
				acq_size = n_samples * n_channels * n_angles * n_images_buffer * 2 / (float)1e9;
				if (acq_size != sizeGb)
				{
					printf("\nError: Se reciben %f GB de los %f GB programados", acq_size, sizeGb);
					end_acquisition = true;
				}
				else
				{
					printf("\r(%06.3f s) (%04d/%04d) TimeStamp: %08d -> Transfer %.2f GB/s", t_sec, counter, burst_number, timestamp, counter * sizeGb / t_sec);
					if (acq_mode  == 0 && counter >= burst_number)
					{
						end_acquisition = true;
						printf("\nEnd Acquisition.\n");
					}
				}
			}

		}
	}
	delete timer;
	ST2_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_GetBufferAddress_Burst(bool sorted)
{
	int result, fmc_size, n_bursts, n_samples, n_fl, images_per_burst, burst_size, counter_img, counter_start;
	unsigned long i;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long n_images_buffer, olink_mask_event, dma_mask_event, pci_mask_event;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada, n_data, images, time_stamp, index_trg;
	unsigned short *punteros_datos_leyes_focales[1024];
	unsigned long long image_addr;
	float mean;
	int min_data, max_data;
	unsigned long idx, min_data_idx, max_data_idx;
	unsigned long long addr_data = 0, addr_data_bak = 0;
	unsigned long timestamp = 0;
	unsigned long idx_burst = 0;
	unsigned long counter_burst;
	bool wait_acquisition = false;
	FILE *f;
	char msg[1024];
	C_Timeout * timer = NULL;
	double mbPerSeconds;
	
	f = fopen("test.txt", "wt");

	if ((result = ST2_LoadXMLFileConfig("SITAU2_PCIe_TEST.XML")) < 0)
	{
		snprintf(msg, sizeof(msg), "\r\n %s - ST2_LoadXMLFileConfig() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
		printf("%s", msg); // Imprime el mensaje como texto literal.
		fprintf(f, "%s", msg);		 // Guarda la descripcion sin interpretar caracteres de formato.
		return;
	}
  
	fmc_size = ST2_MC_GetMatrixCaptureSize();
	images_per_burst = ST2_GetAcquisitionsBurstNumber();
	burst_size = fmc_size * images_per_burst;

	counter_start = 0;
	if (sorted) printf("\r\n (%d) Starting Acquisition (SORTED)... ", counter_start);
	else printf("\r\n (%d) Starting Acquisition (UNSORTED)... ", counter_start);
	if ((result = ST2_Start(sorted ? 1 : 0)) < 0)
	{
		snprintf(msg, sizeof(msg), "\r\n %s - ST2_Start() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
		printf("%s", msg); // Imprime el mensaje como texto literal.
		fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
		return;
	}
	printf("OK");

	timer = new C_Timeout();
	timer->Start();
	counter_burst = mbPerSeconds = 0;
	while (!end_acquisition)
	{
		if (_kbhit())
		{
			chr = _getch();
			if (chr == 's' || chr == 'S')
			{
				if (sorted) printf("\r\n (%d) Starting Acquisition (SORTED)... ", counter_start);
				else printf("\r\n (%d) Starting Acquisition (UNSORTED)... ", counter_start);
				if ((result = ST2_Start(sorted ? 1 : 0)) < 0)
				{
					snprintf(msg, sizeof(msg), "\r\n %s - ST2_Start() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
					printf("%s", msg); // Imprime el mensaje como texto literal.
					fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
					return;
				}
				printf("OK");
				counter_start++;
			}
			else end_acquisition = true;
		}
		else
		{
			if ((result = PCIe_GetBufferAddress_Burst(&addr_data, &idx_burst, &timestamp, &n_images_buffer, 2000)) < 0)
			{
				snprintf(msg, sizeof(msg), "\r\n %s - PCIe_GetBufferAddress_Burst() ", ErrorText(result).c_str()); // Incluye el significado del codigo devuelto por la API.
				printf("%s", msg); // Imprime el mensaje como texto literal.
				fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				end_acquisition = true;
			}
			else
			{
				if (result > 0)
				{
					PCIe_FreeBufferAddress(result);

					counter_burst++;
					mbPerSeconds = burst_size / (timer->Time() * 1000000) * counter_burst * 2;

					snprintf(msg, sizeof(msg), "\n[%d] %.2f GB/s - PCIe_GetBufferAddress_Burst(): %d acquired images - %d Images/Burst - AddrData(0x%llX) - AddrTimeStamp[%d]", idx_burst, mbPerSeconds, result, n_images_buffer, addr_data, timestamp);
					printf("%s", msg); // Imprime el mensaje como texto literal.
					fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				}
				else
				{
					snprintf(msg, sizeof(msg), "\nPCIe_GetBufferAddress_Burst(): TIMEOUT");
					printf("%s", msg); // Imprime el mensaje como texto literal.
					fprintf(f, "%s", msg); // Guarda la descripcion sin interpretar caracteres de formato.
				}
			}
		}
	}
	delete timer;
	ST2_Stop();
	fclose(f);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_BurstAddres(void)
{
  int result, n_acq_images, n_blocks_read, n_samples, n_focal_laws, n_ch, n_trigger_lines;
  bool end_acquisition = false;
  char chr;
  unsigned __int64 status_code;
  unsigned char sitau_status, trigger_source;
  unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
  unsigned long long status_mask = 0, image_size, focal_law_size;
  int errores_cabecera_totales = 0;
  unsigned long acquired_image_index, imagen_liberada, n_data, images, time_stamp, index_trg;
  unsigned short *punteros_datos_leyes_focales[1024];
  unsigned long long image_addr;
  unsigned long long address;
  unsigned long timeout_ms = 1000;
  int counter_start = 0;

  printf("\r\n (%d) Starting Acquisition... ", counter_start);
  if ((result = ST2_Start(1)) < 0)
  {
    printf("Error");
    return;
  }
  printf("OK");
  counter_start++;

  if ((image_size = ST2_MC_GetMatrixCaptureSize()) < 0)
  {
    printf("Error ST2_MC_GetMatrixCaptureSize()");
    return;
  }
  if ((focal_law_size = ST2_MC_GetSingleImageSize()) < 0)
  {
    printf("Error ST2_MC_GetSingleImageSize()");
    return;
  }
  if ((n_samples = ST2_GetAScanDataNumber()) < 0)
  {
    printf("Error ST2_GetAScanDataNumber()");
    return;
  }
  if ((n_focal_laws = ST2_GetFocalLawNumber()) < 0)
  {
    printf("Error ST2_GetFP_FocalLaw_Number()");
    return;
  }
  if ((n_ch = ST2_GetChannelsNumber()) < 0)
  {
    printf("Error ST2_GetChannelsNumber()");
    return;
  }
  if ((n_trigger_lines = ST2_GetAcquisitionsNumber()) < 0)
  {
    printf("Error ST2_GetAcquisitionsNumber()");
    return;
  }

  while (!end_acquisition)
  {
    if (_kbhit())
    {
      chr = _getch();
	  if (chr == 's' || chr == 'S')
	  {
		  printf("\r\n (%d) Starting Acquisition... ", counter_start);
		  if ((result = ST2_Start(1)) < 0)
		  {
			  printf("Error");
			  return;
		  }
		  printf("OK");
		  counter_start++;
	  }
      end_acquisition = true;
    }
    else
    {
      if ((n_acq_images = PCIe_GetBufferAddress_Burst_2(&address, timeout_ms)) < 0)
      {
        printf("\r\n %s - PCIe_GetBuffer_BurstAddress() ", ErrorText(n_acq_images).c_str()); // Incluye el significado del codigo devuelto por la API.
        end_acquisition = true;
      }
      else
      {
        if (n_acq_images > 0) printf("\r\n(%d) Imagenes extraidas del buffer - (Address: %llu) - %d FCMs", n_acq_images, address);
        //PCIe_FreeBufferAddress(n_acq_images);
      }
    }
  }
  ST2_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void PrintMenu(void)
{
	printf("\r\n");
	//printf("\r\n   '0' Key: Test Transfer Full BW (UNSORTED)");
	//printf("\r\n   '1' Key: Test Transfer Full BW (SORTED)");
	printf("\r\n   '0' Key: Test PCIe_GetBufferData_Burst() (SORTED)");
	printf("\r\n   '1' Key: Test PCIe_GetBufferData_Burst() (UNSORTED)");
	printf("\r\n   '2' Key: Test PCIe_GetBufferAddress_Burst() (SORTED)");
	printf("\r\n   '3' Key: Test PCIe_GetBufferAddress_Burst() (UNSORTED)");
	//printf("\r\n\r\n   'd'/'D' Key: GEthernet - Reads Data");
   //printf("\r\n   'a'/'A' Key: GEthernet - Reads Address");
   //printf("\r\n   'r'/'R' Key: Re-Open");
   //printf("\r\n   't'/'T' Key: Test Transfer Full BW");
   //printf("\r\n   'p'/'P' Key: GEthernet - Reads burst buffer address");
   printf("\r\n   Other key:   EXIT\r\n");
}
// -----------------------------------------------------------------------------
/**


*/
// -----------------------------------------------------------------------------
void main(void)
{
bool end_app;
char chr;
int result, n_focal_laws, n_samples, n_images, n_images_buffer, n_focal_laws_block;
unsigned long olink_mask_enabled = 0x0U, dma_mask_enabled = 0x0U, pci_mask_enabled = 0x0U;

// -----------------------------------------------------------------------------
// ABRE LA COMUNICACION CON EL EQUIPO

   if ((result = ST2_SetNoHardware(0)) < 0) return;
   //ST2_EnabledLoadUCIFirmware(0);
   //ST2_SITAUHardwareEnabled(1);      // <= Programación de FPGAs

try
{
	if ((result = OpenSys(".\\BIN\\", "192.168.2.10", 55902, 50460, 1)) < 0) return;	
	//if ((result = OpenSys(".\\bin_nobf_debug\\", "192.168.2.10", 55902, 50460, 1)) < 0) return;

// -----------------------------------------------------------------------------
// CONFIGURA LA ADQUISICION

	//if ((result = ST2_LoadXMLFileConfig("..\\..\\MatLab\\sitau2_matlab\\SITAU2_test.XML")) < 0) return;
	//if ((result = ST2_LoadXMLFileConfig("SITAU2_PCIe_RF.XML")) < 0) return;

	//if ((result = ST2_SetPulseAmplitude(40)) < 0) return;

// -----------------------------------------------------------------------------
// BUCLE DE ADQUISICION

	PrintMenu();
	end_app = false;
	while (!end_app)
	{
		if (_kbhit())
		{
			chr = _getch();
			if (chr == '0')
			{
				Demo_PCIe_GetBufferData_Burst(true);
				PrintMenu();
			}
			else if (chr == '1')
			{
				Demo_PCIe_GetBufferData_Burst(false);
				PrintMenu();
			}
			else if (chr == '2')
			{
				Demo_PCIe_GetBufferAddress_Burst(true);
				PrintMenu();
			}
			else if (chr == '3')
			{
				Demo_PCIe_GetBufferAddress_Burst(false);
				PrintMenu();
			}
			else 
				end_app = true;
			//if (chr == 'a' || chr == 'A')
			//{
			//	Demo_PCIe_GetBufferAddress_Burst();
			//	PrintMenu();
			//}
			//else if (chr == 'd' || chr == 'D')
			//{
			//	Demo_PCIe_GetBufferData_Burst();
			//	//Demo_LastImage();
			//	PrintMenu();
			//}
			//else if (chr == 'r' || chr == 'R')
			//{
			//	printf("\r\nClosing System... ");
			//	ST2_CloseSys();
			//	printf("OK");
			//	Sleep(5000);
			//	printf("\r\nOpenning System... ");
			//	if ((result = ST2_SetNoHardware(0)) < 0) return;
			//	ST2_EnabledLoadUCIFirmware(2);
			//	ST2_SITAUHardwareEnabled(2);      // <= Programación de FPGAs
			//	if ((result = OpenSys("BIN\\", "192.168.2.10", 55902, 50460, 1)) < 0) return;
			//	if ((result = ST2_LoadXMLFileConfig("..//..//MatLab//sitau2_matlab//SITAU2_test.XML")) < 0) return;
			//	PrintMenu();
			//}

      //else if (chr == 'p' || chr == 'P')
      //{
      //  Demo_PCIe_BurstAddres();
      //  PrintMenu();
      }
      //else end_app = true;
		//}
	}

   // -----------------------------------------------------------------------------  
   // CIERRA LA COMUNICACION CON EL EQUIPO 
}
__finally
{
	printf("\r\nClosing System... ");
	ST2_CloseSys();
	printf("OK");
}
   return;
}

