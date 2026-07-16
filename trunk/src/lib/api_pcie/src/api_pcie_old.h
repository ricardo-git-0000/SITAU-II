#ifndef api_pcieH
#define api_pcieH

// _____________________________________________________________________________
// ------ C O N S T A N T   D E F I N I T I O N
// =============================================================================

#ifdef _WIN32
#ifdef APIPCIE_EXPORTS
#define _API_PCIe_DLL __declspec (dllexport)
#else
#define _API_PCIe_DLL __declspec (dllimport)
#endif
#endif

// _____________________________________________________________________________
// ------ F U N C T I O N S   P R O T O T Y P E
// =============================================================================

#ifdef __cplusplus
extern "C"
{
#endif

// _____________________________________________________________________________
// ------ System Functions
// =============================================================================

int _API_PCIe_DLL PCIe_Open(void);
int _API_PCIe_DLL PCIe_Close(void);
void _API_PCIe_DLL PCIe_GetLibraryVersion(char *data);
int _API_PCIe_DLL PCIe_EnableEvent(unsigned long olink_mask, unsigned long dma_mask, unsigned long pci_mask);
int _API_PCIe_DLL PCIe_GetEvent(unsigned long *olink_mask, unsigned long *dma_mask, unsigned long *pci_mask);
int _API_PCIe_DLL PCIe_GetStatus(unsigned long long *status_mask);

// _____________________________________________________________________________
// ------ PCIe Comunication Functions
// =============================================================================

int _API_PCIe_DLL PCIe_GetImagesNumber(void);
int _API_PCIe_DLL PCIe_SetBuffers(int n_samples, int n_focal_laws_image, int n_images, int n_focal_laws_block);
int _API_PCIe_DLL PCIe_SetBuffers_mod(int n_samples, int n_focal_laws_image, int n_images_buffer, int n_images_total, int n_focal_laws_block);
int _API_PCIe_DLL PCIe_Start(int thread_acq_enabled, int time_sleep, int priority, int sorted, int check_header, unsigned long n_images_block, unsigned long n_blocks);
int _API_PCIe_DLL PCIe_Stop(void);

// _____________________________________________________________________________
// ------ PCIe Acqusition Data Functions
// =============================================================================

int _API_PCIe_DLL PCIe_GetBlockData(short *data, unsigned long size_data, short *addr_block);
int _API_PCIe_DLL PCIe_GetAddressBlocks(int *n_read_blocks, unsigned long long *addr_blocks, int n_max_blocks);
int _API_PCIe_DLL PCIe_GetPtrBlocks(int *n_read_blocks, void **ptr_blocks, int n_max_blocks);
int _API_PCIe_DLL PCIe_FreePtrBlocks(int n_blocks);
int _API_PCIe_DLL PCIe_GetBlocksReceived(void);
int _API_PCIe_DLL PCIe_GetReadlmagesNumber(void);
int _API_PCIe_DLL PCIe_GetAcquiredBlocksNumber(void);
int _API_PCIe_DLL PCIe_EnableRx(int enable);
int _API_PCIe_DLL PCIe_FreeOneImage_Pointers(unsigned long *image_index_fred);
int _API_PCIe_DLL PCIe_GetOneImage_Pointers(short int **focal_laws_pointers, unsigned long *current_image_index);
void _API_PCIe_DLL PCIe_CheckPCIeBlockSaw(void *ptr_block, unsigned long long *channel_errors);
int _API_PCIe_DLL PCIe_CheckPCIeHeader_fast(void *ptr_block);
int _API_PCIe_DLL PCIe_image_manager_init(int fifo_pointers, int focal_laws_per_image, int focal_laws_per_block, int frame_length);
int _API_PCIe_DLL PCIe_GetUnreadBloksNumber(int timeout);
int _API_PCIe_DLL PCIe_GetChannelsNumber(void);
int _API_PCIe_DLL PCIe_GetModulesNumber(void);
int _API_PCIe_DLL PCIe_GetImageSize(void);
int _API_PCIe_DLL PCIe_GetImageData(short int *data, unsigned long size_data, unsigned long *current_image_index);
int _API_PCIe_DLL PCIe_GetImageData_Sorted(short int *data, unsigned long size_data, unsigned long *current_image_index);
int _API_PCIe_DLL PCIe_FreeImageData(void);
int _API_PCIe_DLL PCIe_SaveImageData(char *file, short int *data, unsigned long n_data);
int _API_PCIe_DLL PCIe_CreateFileData(char *file);
int _API_PCIe_DLL PCIe_AddFileImageData(char *file, short int *data, unsigned long n_data);
int _API_PCIe_DLL PCIe_GetBuffer_ImageBlock(short int *data, unsigned long size_data, unsigned long *current_image_index, int timeout_ms);
int _API_PCIe_DLL PCIe_GetAddressBuffer_ImageBlock(unsigned long long *addr_block, unsigned long *current_image_index, int timeout_ms);
int _API_PCIe_DLL PCIe_FreeAddressBuffer_ImageBlock(void);
unsigned long _API_PCIe_DLL PCIe_GetBuffer_ImagesRead(void);
unsigned long _API_PCIe_DLL PCIe_GetBuffer_ImagesWritten(void);
int _API_PCIe_DLL PCIe_SaveFileBuffer(char *file);

// _____________________________________________________________________________
// ------ SITAU 2 - System Functions
// =============================================================================

int _API_PCIe_DLL ST2_OpenSys(char* path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq);
int _API_PCIe_DLL ST2_CloseSys(void);
int _API_PCIe_DLL ST2_SetNoHardware(int data);
int _API_PCIe_DLL ST2_GetNoHardware(void);
int _API_PCIe_DLL ST2_SetTemporaryFilesPath(char *path);
int _API_PCIe_DLL ST2_GetError(int error_code, char *str);
int _API_PCIe_DLL ST2_GetLastError(char *str);
int _API_PCIe_DLL ST2_LoadXMLFileConfig(char *file);
int _API_PCIe_DLL ST2_SaveXMLFileConfig(char *file);
int _API_PCIe_DLL ST2_PrintFileConfig(char *file);
int _API_PCIe_DLL ST2_PrintConfig(void);
int _API_PCIe_DLL ST2_PrintPASystemConfig(void);
void _API_PCIe_DLL ST2_InfoLibraryVersion(char *data);
int _API_PCIe_DLL ST2_InfoFirmwareVersion(char *firmware_boot, char *firmware_ethernet, char *firmware_uci_sw, char *firmware_uci_hw, char *firmware_base, char *firmware_module);
void _API_PCIe_DLL ST2_EnabledLoadUCIFirmware(int data);
int _API_PCIe_DLL ST2_SITAUHardwareEnabled(int data);
int _API_PCIe_DLL ST2_GetFocalLawsPCIeBlockNumber(int id_vch);
int _API_PCIe_DLL ST2_SetAcquisitionMode(int data);
int _API_PCIe_DLL ST2_GetAcquisitionMode(void);
int _API_PCIe_DLL ST2_SetDataLinkMode(int data);
int _API_PCIe_DLL ST2_GetDataLinkMode(void);
int _API_PCIe_DLL ST2_GetStatus(unsigned __int64* status_code, unsigned long* rd_address, unsigned long* wr_address, unsigned long* n_overflows, unsigned char* sitau_status, unsigned char* trigger_source, unsigned long* n_acquisitions, unsigned long* ind_acquisitions, unsigned long* size_buffer_command);

// _____________________________________________________________________________
// ------ Acqusition Data Functions
// =============================================================================

// ------ Control Acquisition Functions ----------------------------------------

int _API_PCIe_DLL ST2_GetFP_AScanDataNumber(int id_vch);
int _API_PCIe_DLL ST2_SaveAcquisitionData(char *file);

// ------ Start / Stop Automatic Acquisition Functions -------------------------

int _API_PCIe_DLL ST2_StartPRFScan(void);
int _API_PCIe_DLL ST2_GetPRFAcquisitionsNumber(void);
int _API_PCIe_DLL ST2_SetPRFAcquisitionsNumber(int data);
int _API_PCIe_DLL ST2_SetDelaySync(float data);
float _API_PCIe_DLL ST2_GetDelaySync(void);
int _API_PCIe_DLL ST2_Start(void);
int _API_PCIe_DLL ST2_Stop(void);
int _API_PCIe_DLL ST2_FullAcquisition(int *fl_image_counter);
int _API_PCIe_DLL ST2_Acquiring(void);

// ------ Manual User Trigger Acquisition Functions ----------------------------

int _API_PCIe_DLL ST2_GetBufferFP_LastFocalLawImage(int id_focal_law, short int *data, unsigned long size_data, unsigned long *n_data, unsigned long* index_trigger, unsigned long* index_auxiliar, int *end_acquisition);
int _API_PCIe_DLL ST2_GetBufferFP_FocalLawImage(int id_focal_law, short int *data, unsigned long size_data, unsigned long *n_data, unsigned long index_trigger, bool hw_format);
int _API_PCIe_DLL ST2_GetBufferPA_LastImage(short int *data, unsigned long size_data, unsigned long *n_data);

// _____________________________________________________________________________
// ------ Focal Law Parameters (Full Parallel)
// =============================================================================

int _API_PCIe_DLL ST2_SetFullMatrixCapture(void);
int _API_PCIe_DLL ST2_SetPlaneWaveImagingCompounding_Angles(float *angle, int n_angles);
int _API_PCIe_DLL ST2_SetPlaneWaveImagingCompounding(float start_angle, float step_angle, float end_angle);
float _API_PCIe_DLL ST2_GetPlaneWaveImagingCompounding_StartAngle(void);
float _API_PCIe_DLL ST2_GetPlaneWaveImagingCompounding_StepAngle(void);
float _API_PCIe_DLL ST2_GetPlaneWaveImagingCompounding_EndAngle(void);
int _API_PCIe_DLL ST2_GetFP_FocalLaw_Number(void);
int _API_PCIe_DLL ST2_GetFP_FocalLaw_AScanNumber(int id_focal_law);


int _API_PCIe_DLL ST2_RangePA_MaxSignalAmplitude(int id_vch);
int _API_PCIe_DLL ST2_RangePA_MinSignalAmplitude(int id_vch);
int _API_PCIe_DLL ST2_RangeMaxFocalLaws(void);
int _API_PCIe_DLL ST2_SetPulseAmplitude(int data);

#ifdef __cplusplus
}
#endif

#endif
