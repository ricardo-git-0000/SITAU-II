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

using namespace std;

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ======================================================================

#include "api_pcie.h"
#include "c_timeout.h"

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
int gb_buffer_blocks_number = 4;
int gb_buffer_images_block_number = 50;
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
int OpenSys (char* path, char *ip_address, int tcp_port_cfg, int tcp_port_acq)
{
int result;
char str[255];
char firmware_ethernet[255], firmware_boot[255], firmware_uci_hw[255], firmware_uci_sw[255], firmware_base[255], firmware_module[255];

	ST2_InfoLibraryVersion(str);
	printf("\r\nSTLIB2.DLL Version %s", str);
	PCIe_GetLibraryVersion(str);
	printf("\r\nAPI_PCIe.DLL Version %s", str);

	printf("\r\n Opening the PCIe Communication... ");
	if ((result = PCIe_Open()) < 0)
	{
	    printf("ERROR while opening the PCIe infrastructure");	
	    return result;
	}
	printf("OK");

	printf("\r\n Opening the SITAU system... ");
	PCIe_EnableRx(0);
	if ((result = ST2_OpenSys ( path, ip_address, tcp_port_cfg, tcp_port_acq)) < 0) 
	{
		printf("ERROR while executing ST_OpenSys");
	    PCIe_EnableRx(1);
	    return result;
	}
	printf("OK");  

	Sleep(5000);

	if ((result = ST2_InfoFirmwareVersion (firmware_boot, firmware_ethernet, firmware_uci_sw, firmware_uci_hw, firmware_base, firmware_module)) < 0)
		printf("\r\nERROR Getting Firmware Version");
	else
	{
		printf("\r\n SITAU Boot Firmware Version:     %s", firmware_boot);
		printf("\r\n SITAU Ethernet Firmware Version: %s", firmware_ethernet);
		printf("\r\n SITAU UCI SW Firmware Version:   %s", firmware_uci_sw);
		printf("\r\n SITAU UCI HW Firmware Version:   %s", firmware_uci_hw);
		printf("\r\n SITAU BASE Firmware Version:     %s", firmware_base);
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
   if ((result = ST2_Start()) < 0)
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
               printf("\r\n Error Code (%d) - ST_GetStatus() ", result);
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
void Demo_PCIe(void)
{
	int result, n_blocks_read;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned long rd_address, wr_address, n_overflows, size_buffer_command;
	unsigned char sitau_status, trigger_source;
	vector<void *> ptr_blocks; // Array de punteros a los bloques devueltos al usuario
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0;
	int errores_cabecera_totales = 0;
	unsigned long long *errores_canales;	
	int n_ch = PCIe_GetChannelsNumber();
	int n_mod = PCIe_GetModulesNumber();
	C_Timeout* timer_status = new C_Timeout(2000);
	
	errores_canales = new unsigned long long[n_ch * n_mod];
	for (int i = 0; i < n_ch * n_mod; i++) errores_canales[i] = 0;
		

	/*gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block = 1;*/

	try
	{

		ptr_blocks.resize(5000);

		printf("\r\n Starting PCIe Communication... ");
		if ((result = PCIe_Start(0, 0, 0, 0, 1, gb_buffer_images_block_number, gb_buffer_blocks_number)) < 0)
		{
			printf("Error");
			return;
		}
		printf("OK");

		PCIe_EnableRx(1);

		printf("\r\n Starting Acquisition... ");
		if ((result = ST2_StartPRFScan()) < 0)
		{
			printf("Error");
			return;
		}
		printf("OK");

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
#if 0
					// Lectura del estado
					if ((result = ST_GetStatus(&status_code, &rd_address, &wr_address, &n_overflows, &sitau_status, &trigger_source, &n_acquisitions, &ind_acquisitions, &size_buffer_command)) < 0) {
						printf("\r\n Error Code (%d) - ST_GetStatus() ", result);
					}
					printf("\r\n GE Status Code (0x%016llX) - RD Addr (0x%08X) - WR Addr (0x%08X) - N. Overflows (%lu) - SITAU Status (%d) - Trig. Source (%d) - N. Acquisitions (%lu) - Ind. Acquisitions (%lu) - Buffer Size (%lu)", status_code, rd_address, wr_address, n_overflows, sitau_status, trigger_source, n_acquisitions, ind_acquisitions, size_buffer_command);
#endif

#if 0
					if ((result = PCIe_GetStatus(&status_mask)) < 0) {
						printf("\r\n Error Code (%d) - PCIe_GetStatus() ", result);
					}
#endif

#if 0
					if ((result = PCIe_GetEvent(&olink_mask_event, &dma_mask_event, &pci_mask_event)) < 0) {
						printf("\r\n Error Code (%d) - PCIe_GetEvent() ", result);
					}
#endif
#if 0
#endif
#if 0
					printf("\r\n PCIe Status Code (0x%016llX) - Optical Link Event (0x%08X) - DMA Event (0x%08X) - PCI Event (0x%08X)", status_mask, olink_mask_event, dma_mask_event, pci_mask_event);
#endif
					timer_status->Start();
				}

				//system("pause");
				//if ((result = PCIe_GetPtrBlocks(&n_blocks_read, &ptr_blocks[0], (int)ptr_blocks.size())) < 0)
				if ((result = PCIe_GetPtrBlocks(&n_blocks_read, &ptr_blocks[0], (int)20)) < 0)
				{
					printf("\r\n Error Code (%d) - PCIe_GetPtrBlocks() ", result);
				}
				else
				{
					if (n_blocks_read > 0)
					{
						int total;

						total = PCIe_GetAcquiredBlocksNumber();
						printf("[%s] bloques devueltos = %.5d Numero total de hw_img recibidas = %.5d\n", __FUNCTION__, n_blocks_read, total);
					}
					//printf("\r\n Bloques devueltos: %d", n_blocks_read);
				}

				if (n_blocks_read == 0)
				{
					Sleep(5);
					putchar('0');

				}

				//else
				if (n_blocks_read > 0)
				{
					int proc_cabeceras_index;
					for (proc_cabeceras_index = 0; proc_cabeceras_index < n_blocks_read; proc_cabeceras_index++)
					{
						errores_cabecera_totales += PCIe_CheckPCIeHeader_fast(ptr_blocks[proc_cabeceras_index]);
						PCIe_CheckPCIeBlockSaw(ptr_blocks[proc_cabeceras_index], errores_canales);
					}

					if ((result = PCIe_FreePtrBlocks(n_blocks_read)) < 0)
					{
						printf("\r\n Error Code (%d) - PCIe_FreePtrBlocks() ", result);
					}
					else
					{
						printf("[%s] Procesados (liberados) %.5d bloques\n", __FUNCTION__, result);
					}
				}
			}
		}
	}
	__finally
	{
		// Detiene el proceso de adquisición.
		ST2_Stop();
		PCIe_Stop();

		printf("Errores de información por canal\n");
		for (int i = 0; i < n_ch * n_mod; i++) printf("Channel %3d = %I64u\n", i, errores_canales[i]);
		printf("errores_cabecera_totales=%d\n", errores_cabecera_totales);
		delete errores_canales;
		delete timer_status;
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_Image_manager(int focal_laws_per_image, int focal_laws_per_block, int frame_length)
{
	int result, n_blocks_read;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	//vector<void *> ptr_blocks; // Array de punteros a los bloques devueltos al usuario
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0;
	int errores_cabecera_totales = 0;
	//Images_hander_t image_manager;
	unsigned long acquired_image_index, imagen_liberada;
	short int *punteros_datos_leyes_focales[1024];
	//gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block = 1;
	//T_block_header *last_block_header;

	C_Timeout* timer_status = new C_Timeout(2000);

	if ((result = PCIe_image_manager_init(500, focal_laws_per_image, focal_laws_per_block, frame_length)) < 0)
		return;

	printf("\r\n Starting PCIe Communication... ");
	if ((result = PCIe_Start(0, 0, 0, 0, 1, gb_buffer_images_block_number, gb_buffer_blocks_number)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	PCIe_EnableRx(1);

	printf("\r\n Starting Acquisition... ");
	if ((result = ST2_StartPRFScan()) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

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
				timer_status->Start();
			}

			if ((result = PCIe_GetOneImage_Pointers(punteros_datos_leyes_focales, &acquired_image_index)) < 0)
			{
				printf("\r\n Error Code (%d) - PCIe_GetOneImage_Pointers() ", result);
			}
			else if (result)
			{
				//int imagen_liberada;
				printf("\r\n[%s]Imagen extraida = %3d", __FUNCTION__, acquired_image_index);
				if ((result = PCIe_FreeOneImage_Pointers(&imagen_liberada)) < 0)
					printf("\r\n Error Code (%d) - PCIe_FreeOneImage_Pointers() ", result);
				else if (result)
					printf("\r\n[%s]Índice de imagen liberada = %d", __FUNCTION__, imagen_liberada);
				else
				{
					putchar('-');
					Sleep(30);
				}

			}
			else
			{
				putchar('o');
				Sleep(30);
			}
		}
	}
	// Detiene el proceso de adquisición.
	ST2_Stop();
	PCIe_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_Image_Data(int focal_laws_per_image, int focal_laws_per_block, int frame_length)
{
	int result, n_blocks_read, image_size;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada;
	unsigned short *punteros_datos_leyes_focales[1024];
	vector<short int> image;

	C_Timeout* timer_status = new C_Timeout(2000);

	if ((result = PCIe_image_manager_init(500, focal_laws_per_image, focal_laws_per_block, frame_length)) < 0)
		return;

	printf("\r\n Starting PCIe Communication... ");
	if ((result = PCIe_Start(0, 0, 0, 0, 1, gb_buffer_images_block_number, gb_buffer_blocks_number)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	PCIe_EnableRx(1);

	printf("\r\n Starting Acquisition... ");
	if ((result = ST2_StartPRFScan()) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	if ((image_size = PCIe_GetImageSize()) < 0) 
	{
		printf("Error");
		return;
	}

	if ((result = PCIe_CreateFileData("acquisition.bin")) < 0)
	{
		printf("Error");
		return;
	}

	image.resize(image_size);

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
				timer_status->Start();
			}

			if ((result = PCIe_GetImageData_Sorted(&image[0], image.size(), &acquired_image_index)) < 0)
			{
				printf("\r\n Error Code (%d) - PCIe_GetImageData() ", result);
			}
			else if (result > 0)
			{
				printf("[%s]Imagen extraida = %3d \n", __FUNCTION__, acquired_image_index);

				if ((result = PCIe_FreeOneImage_Pointers(&imagen_liberada)) < 0)
					printf("\r\n Error Code (%d) - PCIe_FreeOneImage_Pointers() ", result);
				else if (result)
					printf("[%s]Índice de imagen liberada = %d\n", __FUNCTION__, imagen_liberada);
				else
				{
					putchar('-');
					Sleep(30);
				}
				//if ((result = PCIe_AddFileImageData("acquisition.bin", &image[0], image.size())) < 0)
				//{
				//	printf("\r\n Error Code (%d) - PCIe_AddFileImageData() ", result);
				//}
			}
			else
			{
				putchar('o');
				Sleep(30);
			}
		}
	}

	PCIe_SaveFileBuffer("prueba.bin");
	// Detiene el proceso de adquisición.
	ST2_Stop();
	PCIe_Stop();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_Image_Data_Thread(int focal_laws_per_image, int focal_laws_per_block, int frame_length)
{
	int result, n_blocks_read, image_size;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada, n_data, images_rd, images_wr;
	unsigned short *punteros_datos_leyes_focales[1024];
	vector<short int> image_block;

	C_Timeout* timer_status = new C_Timeout(2000);

	if ((result = PCIe_image_manager_init(500, focal_laws_per_image, focal_laws_per_block, frame_length)) < 0)
		return;

	printf("\r\n Starting PCIe Communication... ");
	if ((result = PCIe_Start(1, 0, 0, 1, 1, gb_buffer_images_block_number, gb_buffer_blocks_number)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	PCIe_EnableRx(1);

	printf("\r\n Starting Acquisition... ");
	if ((result = ST2_StartPRFScan()) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	if ((image_size = PCIe_GetImageSize()) < 0)
	{
		printf("Error");
		return;
	}

	image_block.resize(gb_buffer_images_block_number * image_size);

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
				timer_status->Start();
			}

			if ((result = PCIe_GetBuffer_ImageBlock(&image_block[0], image_block.size(), &acquired_image_index, 10000)) < 0)
			{
				printf("\r\n Error Code (%d) - PCIe_GetImageData() ", result);
			}
			else if (result > 0)
			{
				images_rd = PCIe_GetBuffer_ImagesRead();
				images_wr = PCIe_GetBuffer_ImagesWritten();
				printf("\r\n[%s] Imagen extraida del buffer = %d - %d/%d", __FUNCTION__, acquired_image_index, images_rd, images_wr);
			}
			else
			{
				images_rd = PCIe_GetBuffer_ImagesRead();
				images_wr = PCIe_GetBuffer_ImagesWritten();
				printf("\r\n[%s] No hay bloque disponible - Buffer RD(%d) / Buffer WR(%d)", __FUNCTION__, images_rd, images_wr);
				//putchar('o');
				Sleep(300);
			}
		}
	}
	ST2_Stop();
	PCIe_Stop();

	if ((image_size = PCIe_SaveFileBuffer("buffer.bin")) < 0)
	{
		printf("Error");
		return;
	}
	// Detiene el proceso de adquisición.
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void Demo_PCIe_Image_Address_Thread(int focal_laws_per_image, int focal_laws_per_block, int frame_length)
{
	int result, n_blocks_read, image_size;
	bool end_acquisition = false;
	char chr;
	unsigned __int64 status_code;
	unsigned char sitau_status, trigger_source;
	unsigned long olink_mask_event, dma_mask_event, pci_mask_event;
	unsigned long long status_mask = 0;
	int errores_cabecera_totales = 0;
	unsigned long acquired_image_index, imagen_liberada, n_data, images_rd, images_wr;
	unsigned short *punteros_datos_leyes_focales[1024];
	//vector<unsigned long long> image_addr;
	unsigned long long image_addr;

	C_Timeout* timer_status = new C_Timeout(2000);

	if ((result = PCIe_image_manager_init(500, focal_laws_per_image, focal_laws_per_block, frame_length)) < 0)
		return;

	printf("\r\n Starting PCIe Communication... ");
	if ((result = PCIe_Start(1, 0, 2, 0, 1, gb_buffer_images_block_number, gb_buffer_blocks_number)) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	PCIe_EnableRx(1);

	printf("\r\n Starting Acquisition... ");
	if ((result = ST2_StartPRFScan()) < 0)
	{
		printf("Error");
		return;
	}
	printf("OK");

	if ((image_size = PCIe_GetImageSize()) < 0)
	{
		printf("Error");
		return;
	}

	//image_addr.resize(gb_buffer_blocks_number);

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
				timer_status->Start();
			}

			if ((result = PCIe_GetAddressBuffer_ImageBlock(&image_addr, &acquired_image_index, 10000)) < 0)
			{
				printf("\r\n Error Code (%d) - PCIe_GetAddressBuffer_ImageBlock() ", result);
			}
			else if (result > 0)
			{
				images_rd = PCIe_GetBuffer_ImagesRead();
				images_wr = PCIe_GetBuffer_ImagesWritten();
				printf("\r\n[%s] Imagen extraida del buffer = %d - %d/%d", __FUNCTION__, acquired_image_index, images_rd, images_wr);
				if ((result = PCIe_FreeAddressBuffer_ImageBlock()) < 0)
				{
					printf("\r\n Error Code (%d) - PCIe_FreeAddressBuffer_ImageBlock() ", result);
				}
			}
			else
			{
				images_rd = PCIe_GetBuffer_ImagesRead();
				images_wr = PCIe_GetBuffer_ImagesWritten();
				printf("\r\n[%s] No hay bloque disponible - Buffer RD(%d) / Buffer WR(%d)", __FUNCTION__, images_rd, images_wr);
				//putchar('o');
				Sleep(300);
			}
		}
	}
	ST2_Stop();
	PCIe_Stop();

	if ((image_size = PCIe_SaveFileBuffer("buffer.bin")) < 0)
	{
		printf("Error");
		return;
	}
	// Detiene el proceso de adquisición.
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void PrintMenu(void)
{
   printf("\r\n\r\n   's'/'S' Key: GEthernet Status Reading");
   printf("\r\n   'p'/'P' Key: PCIe");
   printf("\r\n   'i'/'I' Key: PCIe with Image Manager");
   printf("\r\n   'd'/'D' Key: PCIe with Image Data");
   printf("\r\n   't'/'T' Key: PCIe with Image Data and Acquisition Thread");
   printf("\r\n   'a'/'A' Key: PCIe with Image Address and Acquisition Thread");
   printf("\r\n   Other key:   EXIT");
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
   ST2_EnabledLoadUCIFirmware(1);
   ST2_SITAUHardwareEnabled(1);      // <= Programación de FPGAs

try
{
   if ((result = OpenSys("BIN\\", "192.168.2.10", 55902, 50460)) < 0) return;	

// -----------------------------------------------------------------------------
// CONFIGURA LA ADQUISICION

   if ((result = ST2_LoadXMLFileConfig("SITAU2.XML")) < 0) return;
   //if ((result = ST2_LoadXMLFileConfig("C:\\projects\\deus_i_win_driver\\final_integration\\pcie\\SITAU2.XML")) < 0) return;

   ST2_PrintConfig();

   if (gb_acquisition_continuous)
     ST2_SetAcquisitionMode(1);
   else
   {
      if ((result = ST2_SetAcquisitionMode(0)) < 0) return;
      if ((result = ST2_SetPRFAcquisitionsNumber(gb_buffer_blocks_number * gb_buffer_images_block_number)) < 0) return;
   }
   if ((result = ST2_SetDataLinkMode(gb_data_link)) < 0) return;
   n_images_buffer = gb_images_buffer_number;

// CONFIGURA LOS BUFFERS PCIe

   if ((n_samples = ST2_GetFP_AScanDataNumber(-1)) < 0) return;
   if ((n_focal_laws = ST2_GetFP_FocalLaw_Number()) < 0) return;
   if ((n_images = ST2_GetPRFAcquisitionsNumber()) < 0) return;
   if ((n_focal_laws_block = ST2_GetFocalLawsPCIeBlockNumber(0)) < 0) return;

   printf("\r\n Enabling PCIe Communication Events... ");
   if ((result = PCIe_EnableEvent(olink_mask_enabled, dma_mask_enabled, pci_mask_enabled)) < 0)
   {
	   printf("Error");
	   return;
   }
   printf("OK");

   printf("\r\n Creating PCIe Communication Buffers (%d samples - %d focal laws - %d images - %d focal laws per block)... ", n_samples, n_focal_laws, n_images, n_focal_laws_block);
   if ((result = PCIe_SetBuffers_mod(n_samples, n_focal_laws, n_images_buffer, n_images, n_focal_laws_block)) < 0)
   {
	   printf("Error PCIe_SetBuffers");
	   return;
   }
   printf("OK PCIe_SetBuffers");

// -----------------------------------------------------------------------------
// BUCLE DE ADQUISICION

   PrintMenu();
   end_app = false;
   while (!end_app)
   {
      if (_kbhit())
      {
         chr = _getch();
         if (chr == 's' || chr == 'S')
         {
            Demo_GEStatus();
            PrintMenu();
         }
		 else if (chr == 'p' || chr == 'P')
		 {
			 Demo_PCIe();
			 PrintMenu();
		 }
		 else if (chr == 'i' || chr == 'I')
		 {
			 Demo_PCIe_Image_manager(n_focal_laws, n_focal_laws_block, n_samples);
		 }
		 else if (chr == 'd' || chr == 'D')
		 {
			 Demo_PCIe_Image_Data(n_focal_laws, n_focal_laws_block, n_samples);
		 }
		 else if (chr == 't' || chr == 'T')
		 {
			 Demo_PCIe_Image_Data_Thread(n_focal_laws, n_focal_laws_block, n_samples);
		 }
		 else if (chr == 'a' || chr == 'A')
		 {
			 Demo_PCIe_Image_Address_Thread(n_focal_laws, n_focal_laws_block, n_samples);
		 } 
		 else end_app = true;
      }
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

