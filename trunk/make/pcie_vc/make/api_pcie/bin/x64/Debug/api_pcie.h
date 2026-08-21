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
// ------ C O N S T A N T S
// =============================================================================

#define AQUISITION_MODE_FIXED			0
#define AQUISITION_MODE_CONTINOUS		1
#define AQUISITION_MODE_FIXED_BURST		2
#define AQUISITION_MODE_CONTINOUS_BURST	3

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

	int _API_PCIe_DLL ST2_OpenSys(char* path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq, int pcie_enabled);
	int _API_PCIe_DLL ST2_CloseSys(void);
	int _API_PCIe_DLL ST2_SetNoHardware(int data);
	int _API_PCIe_DLL ST2_GetNoHardware(void);
	void _API_PCIe_DLL ST2_InfoLibraryVersion(char *data);
	int _API_PCIe_DLL ST2_InfoFirmwareVersion(char *firmware_ethernet, char *firmware_uci_sw, char *firmware_uci_hw, char *firmware_mk32_hw);
	void _API_PCIe_DLL PCIe_GetLibraryVersion(char *data);
	void _API_PCIe_DLL PCIe_GetSystemInfo(char *id, char *olink_signature, char *fuse_dma_signature, char *emul_signature);
	int _API_PCIe_DLL ST2_GetChannelsNumber(void);
	int _API_PCIe_DLL ST2_LoadXMLFileConfig(char *file);
	int _API_PCIe_DLL ST2_SaveXMLFileConfig(char *file);
	int _API_PCIe_DLL ST2_PrintFileConfig(char *file);
	int _API_PCIe_DLL ST2_GetError(int error_code, char *str);
	int _API_PCIe_DLL ST2_GetLastError(char *str);

	int _API_PCIe_DLL ST2_GetStatus(unsigned __int64* status_code, unsigned long* rd_address, unsigned long* wr_address, unsigned long* n_overflows, unsigned char* sitau_status, unsigned char* trigger_source, unsigned long* n_acquisitions, unsigned long* ind_acquisitions, unsigned long* size_buffer_command);
	int _API_PCIe_DLL ST2_GetStatusFrame(unsigned char* status_mem, int size_mem);
	int _API_PCIe_DLL ST2_SetTemporaryFilesPath(char *path);
	void _API_PCIe_DLL ST2_EnabledLoadUCIFirmware(int data);
	int _API_PCIe_DLL ST2_SITAUHardwareEnabled(int data);

	int _API_PCIe_DLL ST2_SetHWFormatData(int data);

// _____________________________________________________________________________
// ------ Acqusition Data Functions
// =============================================================================

void _API_PCIe_DLL ST2_MC_SetGEBufferSize(int n_acquisitions);

// ------ Control Acquisition Functions ----------------------------------------

int _API_PCIe_DLL ST2_GetAScanDataNumber(void);
int _API_PCIe_DLL ST2_MC_GetAScanNumber(void);
int _API_PCIe_DLL ST2_MC_GetMatrixCaptureSize(void);
int _API_PCIe_DLL ST2_MC_GetSingleImageSize(void);
int _API_PCIe_DLL ST2_SaveAcquisitionData(char *file);

// ------ Start / Stop Automatic Acquisition Functions -------------------------

int _API_PCIe_DLL ST2_SetDelaySync(float data);
float _API_PCIe_DLL ST2_GetDelaySync(void);
int _API_PCIe_DLL ST2_SetTriggerSource(int data);
int _API_PCIe_DLL ST2_GetTriggerSource(void);
int _API_PCIe_DLL ST2_Start(int data_sorted);
int _API_PCIe_DLL ST2_Stop(void);
int _API_PCIe_DLL ST2_Continue(void);
int _API_PCIe_DLL ST2_SetPauseAfterBurst(int data);
int _API_PCIe_DLL ST2_GetPauseAfterBurst(void);
int _API_PCIe_DLL ST2_FullAcquisition(int *fl_image_counter);
int _API_PCIe_DLL ST2_Acquiring(void);
int _API_PCIe_DLL PCIe_LastImageRead(void);

// ------ Giga-Ethernet Acqusition Data Functions ----------------------------

int _API_PCIe_DLL ST2_SaveFileAcquisition(char *file);
int _API_PCIe_DLL ST2_LoadFileAcquisition(char *file);

int _API_PCIe_DLL ST2_MC_GetBuffer_SingleImage(short int* data, unsigned long size_data, int index_focal_law, unsigned long index_trigger);
int _API_PCIe_DLL ST2_MC_GetBuffer_LastMatrixCapture(short int* data, unsigned long size_data, unsigned long *index_trigger);
int _API_PCIe_DLL ST2_MC_GetBuffer_MatrixCapture(short int* data, unsigned long size_data, unsigned long index_trigger);

int _API_PCIe_DLL ST2_PA_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux);
int _API_PCIe_DLL ST2_PA_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux);

int _API_PCIe_DLL ST2_TFM_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux);
int _API_PCIe_DLL ST2_TFM_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux);

// ------ PCIe Acqusition Data Functions ----------------------------

int _API_PCIe_DLL PCIe_GetBufferData_Burst(short int *data, unsigned long size_data, unsigned long timeout_ms);
int _API_PCIe_DLL PCIe_GetBufferAddress_Burst_2(unsigned long long *addr_data, unsigned long timeout_ms);
int _API_PCIe_DLL PCIe_GetBufferAddress(unsigned long long *addr_data, unsigned long *addr_timestamp, unsigned long *n_images_buffer, unsigned long timeout_ms);
int _API_PCIe_DLL PCIe_GetBufferAddress_Burst(unsigned long long *addr_data, unsigned long *idx_burst, unsigned long *timestamp, unsigned long *n_images_buffer, unsigned long timeout_ms);
int _API_PCIe_DLL PCIe_GetBufferData_FMCMatrix(short int *data, unsigned long size_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms);
int _API_PCIe_DLL PCIe_GetBufferAddress_FMCMatrix(unsigned long long *addr_data, unsigned long *current_image_index, unsigned long *time_stamp, int timeout_ms);
int _API_PCIe_DLL PCIe_FreeBufferAddress(int n_images);
unsigned long long _API_PCIe_DLL PCIe_GetBuffer_FMCMatrixNumber(void);
int _API_PCIe_DLL PCIe_SaveFileBuffer(char *file);

// _____________________________________________________________________________
// ------ Focal Law Parameters (Full Parallel)
// =============================================================================

int _API_PCIe_DLL ST2_MC_SetEmissionFocalLawFile(char* file);
int _API_PCIe_DLL ST2_MC_GetFocalLawType(void);
int _API_PCIe_DLL ST2_MC_SetFullMatrixCapture(void);
int _API_PCIe_DLL ST2_MC_SetPlaneWaveImaging_Angles_deg(float *angle, int n_angles);
int _API_PCIe_DLL ST2_MC_SetPlaneWaveImaging_Angles_rad(float *angle, int n_angles);
int _API_PCIe_DLL ST2_MC_SetPlaneWaveImaging(float start_angle, float step_angle, float end_angle);
float _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_StartAngle(void);
float _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_StepAngle(void);
float _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_EndAngle(void);
int _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_Angles_rad(float* data, int size_data);
int _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_Angles_deg(float* data, int size_data);
int _API_PCIe_DLL ST2_MC_GetPlaneWaveImaging_AnglesNumber(void);
int _API_PCIe_DLL ST2_GetFocalLawNumber(void);
int _API_PCIe_DLL ST2_MC_GetFocalLaw_AScanNumber(int id_focal_law);

// _____________________________________________________________________________
// ------ Acquisition Parameters Functions
// =============================================================================

int _API_PCIe_DLL ST2_SetAcquisitionType(int data);
int _API_PCIe_DLL ST2_GetAcquisitionType(void);
int _API_PCIe_DLL ST2_SetAcquisitionMode(int data);
int _API_PCIe_DLL ST2_GetAcquisitionMode(void);
int _API_PCIe_DLL ST2_SetAcquisitionsNumber(unsigned long n_blocks, unsigned long images_block);
int _API_PCIe_DLL ST2_GetAcquisitionsBurstNumber(void);
int _API_PCIe_DLL ST2_GetBurstsNumber(void);
int _API_PCIe_DLL ST2_GetAcquisitionsNumber(void);
int _API_PCIe_DLL ST2_SetGain(float data);
float _API_PCIe_DLL ST2_GetGain(void);
int _API_PCIe_DLL ST2_SetMaterialVelocity(float data);
float _API_PCIe_DLL ST2_GetMaterialVelocity(void);
int _API_PCIe_DLL ST2_SetAcqDistance(float start_distance, float end_distance, int acquisition_time_mode);
int _API_PCIe_DLL ST2_SetAcqTime(float start_time, float end_time);
float _API_PCIe_DLL ST2_GetAcqStartDistance(void);
float _API_PCIe_DLL ST2_GetAcqEndDistance(void);
float _API_PCIe_DLL ST2_GetAcqStartTime(void);
float _API_PCIe_DLL ST2_GetAcqEndTime(void);
float _API_PCIe_DLL ST2_GetAcqTime(void);
int _API_PCIe_DLL ST2_GetAcquisitionTimeMode(void);
int	_API_PCIe_DLL ST2_SetSamplingFrequency(int data);
int _API_PCIe_DLL ST2_GetDownSamplingFactor(void);
float _API_PCIe_DLL ST2_GetSamplingFrequency(void);
float _API_PCIe_DLL ST2_GetPRF(void);
int _API_PCIe_DLL ST2_SetPRF(float data);
int _API_PCIe_DLL ST2_GetImageEntryPoint_X(float *x_coordinate, int size_data);
int _API_PCIe_DLL ST2_GetImageEntryPoint_Y(float *y_coordinate, int size_data);
int _API_PCIe_DLL ST2_GetImageEntryPoint_Z(float *z_coordinate, int size_data);
int _API_PCIe_DLL ST2_GetImageEntryPoint_Angle(float *angle, int size_data);

// _____________________________________________________________________________
// ------ Digital Signal Processing Functions
// =============================================================================

int _API_PCIe_DLL ST2_SetProcessing(int processing_type, int n_signals, int wait_prf);
int _API_PCIe_DLL ST2_GetProcessingType(void);
int _API_PCIe_DLL ST2_GetProcessingSignalsNumber_AVR(void);
int _API_PCIe_DLL ST2_SetHWRFFilterEnable(int data);
int _API_PCIe_DLL ST2_GetHWRFFilterEnable(void);
int _API_PCIe_DLL ST2_SetHWRFFilter(float low_frequency, float high_frequency);
float _API_PCIe_DLL ST2_GetHWRFFilterLow(void);
float _API_PCIe_DLL ST2_GetHWRFFilterHigh(void);

// _____________________________________________________________________________
// ------ Pulser Functions
// =============================================================================

int _API_PCIe_DLL ST2_SetHighVoltageMaximum(int data);
int  _API_PCIe_DLL ST2_GetHighVoltageMaximum(void);
int _API_PCIe_DLL ST2_SetPulseAmplitude(int data);
int _API_PCIe_DLL ST2_GetPulseAmplitude(void);
int _API_PCIe_DLL ST2_SetPulserEnable(int data);
int _API_PCIe_DLL ST2_GetPulserEnable(void);
int _API_PCIe_DLL ST2_SetPulseWidth(float pulse_width);
int _API_PCIe_DLL ST2_SetPulsesNumber(int n_pulses);
float _API_PCIe_DLL ST2_GetPulseWidth(void);
int _API_PCIe_DLL ST2_GetPulsesNumber(void);

// _____________________________________________________________________________
// ------ Phased Array Probe Parameters
// =============================================================================

int _API_PCIe_DLL ST2_SetProbeConfig(int n_element, float pitch, float frequency);
float _API_PCIe_DLL ST2_GetProbePitch(void);
int _API_PCIe_DLL ST2_GetProbeElementNumber(void);
float _API_PCIe_DLL ST2_GetProbeFrequency(void);

// _____________________________________________________________________________
// ------ (fUS) Functional Ultrasound Imaging
// =============================================================================

float _API_PCIe_DLL ST2_FUS_CentralFrequency(void);
float _API_PCIe_DLL ST2_FUS_RapportFocalAperture(void);
float _API_PCIe_DLL ST2_FUS_ApodizationCoefficient(void);
float _API_PCIe_DLL ST2_FUS_LateralPositionX(void);
float _API_PCIe_DLL ST2_FUS_PixelSizeX(void);
int _API_PCIe_DLL ST2_FUS_PixelsX(void);
float _API_PCIe_DLL ST2_FUS_DepthZ(void);
int _API_PCIe_DLL ST2_FUS_PixelsZ(void);
float _API_PCIe_DLL ST2_FUS_PixelSizeZ(void);
int _API_PCIe_DLL ST2_FUS_SinglePlane(void);
int _API_PCIe_DLL ST2_FUS_SamplesWavelength(void);
int _API_PCIe_DLL ST2_FUS_InterpolationCoefficients(void);

// _____________________________________________________________________________
// ------ Range Functions
// =============================================================================

int _API_PCIe_DLL ST2_RangePA_MaxSignalAmplitude(int id_vch);
int _API_PCIe_DLL ST2_RangePA_MinSignalAmplitude(int id_vch);
int _API_PCIe_DLL ST2_RangeMaxFocalLaws(void);

// _____________________________________________________________________________
// ------ Focal Law Parameters (Phased Array)
// =============================================================================

int _API_PCIe_DLL ST2_PA_SetLinearScanConfig(int step, int aperture, int startelement, int endelement, float angle);
int _API_PCIe_DLL ST2_PA_SetAngularScanConfig(float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle);
int _API_PCIe_DLL ST2_PA_GetAperture(void);
int _API_PCIe_DLL ST2_PA_GetScanType(void);
int _API_PCIe_DLL ST2_PA_GetLinearScanStep(void);
int _API_PCIe_DLL ST2_PA_GetScanStartElement(void);
int _API_PCIe_DLL ST2_PA_GetLinearScanEndElement(void);
float _API_PCIe_DLL ST2_PA_GetLinearScanAngle(void);
float _API_PCIe_DLL ST2_PA_GetAngularScanStartAngle(void);
float _API_PCIe_DLL ST2_PA_GetAngularScanEndAngle(void);
float _API_PCIe_DLL ST2_PA_GetAngularScanStepAngle(void);
int _API_PCIe_DLL ST2_PA_SetEmissionFocalPoint(float emission_focus);
float _API_PCIe_DLL ST2_PA_GetEmissionFocalPoint(void);

#ifdef __cplusplus
}
#endif

#endif
