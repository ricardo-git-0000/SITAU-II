#ifndef stlib2H
#define stlib2H

// _____________________________________________________________________________
// ------ C O N S T A N T   D E F I N I T I O N
// =============================================================================

#ifdef _WIN32
#ifdef _DLLCode
#define _TYPE __declspec (dllexport)
#else
#define _TYPE __declspec (dllimport)
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

int _TYPE ST_GetLastMessage(char* message);
int  _TYPE ST_OpenSys (char* path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq);
int  _TYPE ST_OpenSys_Progress (char* str);
int  _TYPE ST_OpenSys_Cancel(void);
int  _TYPE ST_GetProgressMessage(char* str);
int  _TYPE ST_GetProgressLastMessage(char* str, int* delay_ms);
int  _TYPE ST_CloseSys (void);
int  _TYPE ST_MC_GetParallelLinesBlock(void);
int  _TYPE ST_PA_GetParallelLinesBlock(void);
int  _TYPE ST_SetNoHardware (int data);
int  _TYPE ST_GetNoHardware (void);
int  _TYPE ST_SetTemporaryFilesPath (char *path);
int  _TYPE ST_GetError (int error_code, char *str);
int  _TYPE ST_GetLastError (char *str);
void _TYPE ST_SetEnabledConfigFileText(int data);
int _TYPE ST_GetEnabledConfigFileText(void);

int  _TYPE ST_LoadXMLFileConfig (char *file);
int  _TYPE ST_SaveXMLFileConfig (char *file);
int _TYPE ST_SaveFileAcquisition(char* file);
int _TYPE ST_LoadFileAcquisition(char* file);
int _TYPE ST_SaveAcquisitionData(char* file);
int _TYPE ST_FileAcquisition_Progress(char* str);

int  _TYPE ST_PrintFileConfig(char *file, int hardware_config);
int  _TYPE ST_PrintConfig(void);
int  _TYPE ST_PrintPASystemConfig(void);

void _TYPE ST_InfoLibraryVersion (char *data);
int  _TYPE ST_InfoFirmwareVersion (char *firmware_ethernet, char *firmware_uci_sw, char *firmware_uci_hw, char *firmware_module);
void _TYPE ST_EnabledLoadUCIFirmware (int data);
int  _TYPE ST_SITAUHardwareEnabled (int data);

void _TYPE ST_ConfigTerminalLog (bool ACQUIRING, bool ACQUIRING_FRAME, bool PROTOCOL, bool PRG_BASE_WMPAR, bool PRG_MOD_WMPAR, bool PRG_REG, bool AMPLIA_SEND_COMMAND);
void _TYPE ST_ResetTerminalLog (void);

void _TYPE ST_EnabledLogFile(int data);

int _TYPE ST_SetUserFlashNetConfig(char* ip_address, int port_cconfig, int port_data, char *mac, int dhcp);
int _TYPE ST_GetUserFlashNetConfig(char* ip_address, int* port_config, int* port_data, char* mac, int* dhcp);
int _TYPE ST_GetActualNetConfig(char* ip_address, int* port_config, int *port_data, char* mac, int *dhcp);

int _TYPE ST_PCIe_GetBlocksBurstNumber(void);
int _TYPE ST_PCIe_GetFocalLawsBlockNumber(void);
int _TYPE ST_SetAcquisitionMode(int data);
int _TYPE ST_GetAcquisitionMode(void);

int _TYPE ST_SetStatusConfig(int enabled, int period_ms, int pcie);
int _TYPE ST_SetStatusConfigPCIe(int pcie);
int _TYPE ST_GetStatusConfigPCIe(void);
int _TYPE ST_GetStatus(unsigned __int64* status_code, unsigned long* rd_address, unsigned long* wr_address, unsigned long* n_overflows, unsigned char* sitau_status, unsigned char* trigger_source, unsigned long* n_acquisitions, unsigned long* ind_acquisitions, unsigned long* size_buffer_command);
int _TYPE ST_GetStatusFrame(unsigned __int8* status_mem, int size_mem);
int _TYPE ST_GetStatusFrame_v1(unsigned __int8 * status_mem, int size_mem);
int _TYPE ST_WriteBootFirmware_1(char* path_bin, char* ip_address, int tcp_port_cfg, int tcp_port_acq, char* file_firmware);
int _TYPE ST_WriteBootFirmware_2(char* ip_address, int tcp_port_cfg, char* file_firmware, int wait_ms);

int _TYPE ST_GetAvailable_PA(void);

// _____________________________________________________________________________
// ------ Acqusition Data Functions
// =============================================================================

void _TYPE ST_MC_SetGEBufferSize(int n_acquisitions);

// ------ Control Acquisition Functions ----------------------------------------

int _TYPE ST_GetAScanDataNumber(void);
int _TYPE ST_MC_GetAScanDataNumber(int id_vch);
int _TYPE ST_MC_GetAScanNumber(void);
int _TYPE ST_MC_GetMatrixCaptureSize(void);
int _TYPE ST_MC_GetSingleImageSize(void);
int _TYPE ST_GetChannelsNumber(void);
int _TYPE ST_GetModulesNumber(void); // Consulta los modulos fisicos del canal virtual cero, sin releer el XML.
int _TYPE ST_GetMaxAcquisitionsNumber(void);
int _TYPE ST_SetMaxAcquisitionsNumber(void);
int _TYPE ST_CurrentIndexFocalLaw(void);

// ------ Start / Stop Automatic Acquisition Functions -------------------------

int _TYPE ST_GetTriggerSource (void);
int _TYPE ST_SetTriggerSource(int data);
int _TYPE ST_GetAcquisitionsNumber(void);
int _TYPE ST_SetAcquisitionsNumber(unsigned long n_bursts, unsigned long images_bursts);
int _TYPE ST_GetAcquisitionsBurstNumber(void);
int _TYPE ST_GetBurstsNumber(void);
int _TYPE ST_SetDelaySync(float data);
float _TYPE ST_GetDelaySync(void);
int _TYPE ST_SetPauseAfterBurst(int data);
int _TYPE ST_GetPauseAfterBurst(void);
int _TYPE ST_SetAutoContinueMode(int data);
int _TYPE ST_GetAutoContinueMode(void);

int _TYPE ST_Start (void);
int _TYPE ST_Stop (void);
int _TYPE ST_Pause(void);
int _TYPE ST_Continue(void);
int _TYPE ST_FullAcquisition (int *fl_image_counter);
int _TYPE ST_Acquiring (void);
int _TYPE ST_IsStopped(void);
int _TYPE ST_MemoryResize(void);
int _TYPE ST_SetHWFormatData(int data);
int _TYPE ST_GetHWFormatData(void);

// ------ Manual User Trigger Acquisition Functions ----------------------------

int _TYPE ST_Trigger (float timeout);

int _TYPE ST_MC_GetBuffer_SingleImage(short int* data, unsigned long size_data, int index_focal_law, unsigned long index_trigger);
int _TYPE ST_MC_GetBuffer_LastMatrixCapture(short int* data, unsigned long size_data, unsigned long *index_trigger);
int _TYPE ST_MC_GetBuffer_MatrixCapture(short int* data, unsigned long size_data, unsigned long index_trigger);

int _TYPE ST_PA_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux);
int _TYPE ST_PA_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux);

int _TYPE ST_TFM_GetBuffer_LastImage(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trg, unsigned long* index_aux);
int _TYPE ST_TFM_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux);

// _____________________________________________________________________________
// ------ Acquisition Parameters Functions
// =============================================================================

int _TYPE ST_SetGain (float data);
float _TYPE ST_GetGain (void);
int _TYPE ST_SetAFEGain(int attn, int lna, int pga);
int _TYPE ST_GetAFEGain(int *attn, int *lna, int *pga);

int _TYPE ST_SetSignalTest (int data);
int _TYPE ST_GetSignalTest (void);

int	_TYPE ST_SetSamplingFrequency (int data);
float _TYPE ST_GetSamplingFrequency (void);
int _TYPE ST_GetDownSamplingFactor(int id_vch);

int _TYPE ST_SetMaterialVelocity (float data);
float _TYPE ST_GetMaterialVelocity (void);

int _TYPE ST_SetAcqDistance (float start_distance, float end_distance, int pulse_echo);
int _TYPE ST_SetAcqTime (float start_time, float end_time);
float _TYPE ST_GetAcqStartDistance (void);
float _TYPE ST_GetAcqEndDistance (void);
float _TYPE ST_GetAcqStartTime (void);
float _TYPE ST_GetAcqEndTime (void);
float _TYPE ST_GetAcqTime (void);
int _TYPE ST_GetAcquisitionTimeMode(void);

float _TYPE ST_GetDistanceResolution(void);

int _TYPE ST_SetAttenuator (int data);
int _TYPE ST_GetAttenuator (void);

int _TYPE ST_SetPRF (float data);
float _TYPE ST_GetPRF (void);

int _TYPE ST_SetPRFBurst(float data);
float _TYPE ST_GetPRFBurst(void);

int _TYPE ST_SetPRFTrigger(float prf_hz);
float _TYPE ST_GetPRFTrigger(void);

int _TYPE ST_GetFP_SamplesTime (float *samples_time, int size_data);
int _TYPE ST_GetPA_SamplesTime (float *samples_time, int size_data);
int _TYPE ST_GetFP_SamplesDistance (float *samples_distance, int size_data, int ch);
int _TYPE ST_PA_GetSamplesDistance (float *samples_distance, int size_data, int ch);
int _TYPE ST_MC_GetImageEntryPoint(float *data, int size_data);

// _____________________________________________________________________________
// ------ Acquisition Parameters Functions
// =============================================================================

int   _TYPE ST_SetDigitalTGC_Attenuator(int data);
int   _TYPE ST_GetDigitalTGC_Attenuator(void);
int   _TYPE ST_SetLNAGain(int data);
int   _TYPE ST_GetLNAGain(void);
int   _TYPE ST_SetPGAGain(int data);
int   _TYPE ST_GetPGAGain(void);
int   _TYPE ST_SetTGCEnable (int data);
int   _TYPE ST_GetTGCEnable (void);
int   _TYPE ST_SetTGCAttenuationCoefficient (float attn_coefficient);
float _TYPE ST_GetTGCAttenuationCoefficient (void);
int   _TYPE ST_SetTGCConfigType (int data);
int   _TYPE ST_GetTGCConfigType (void);
int   _TYPE ST_SetLog2_Average(int data);
int   _TYPE ST_GetLog2_Average(void);
int   _TYPE ST_SetAcquisitionType (int data);
int   _TYPE ST_GetAcquisitionType (void);
int   _TYPE ST_SetSignalMode(int data);
int   _TYPE ST_GetSignalMode(void);

// _____________________________________________________________________________
// ------ Pulser Functions
// =============================================================================

void _TYPE ST_SetHighVoltageMaximum(int data);
int  _TYPE ST_GetHighVoltageMaximum(void); 
int   _TYPE ST_SetPulseAmplitude (int data);
int   _TYPE ST_GetPulseAmplitude (void);
int 	_TYPE ST_SetPulserEnable (int data);
int   _TYPE ST_GetPulserEnable (void);
int   _TYPE ST_SetPulser (int n_pulses, float pulse_width);
int   _TYPE ST_SetPulseWidth (float pulse_width);
int   _TYPE ST_SetPulsesNumber (int n_pulses);
float _TYPE ST_GetPulseWidth (void);
int   _TYPE ST_GetPulsesNumber (void);

// _____________________________________________________________________________
// ------ Digital Signal Processing Functions
// =============================================================================

int  	_TYPE ST_SetHWRFFilterEnable(int data, int id_vch);
int   _TYPE ST_GetHWRFFilterEnable(int id_vch);

int  	_TYPE ST_SetHWRFFilter(float low_frequency, float high_frequency, int id_vch);
float _TYPE ST_GetHWRFFilterLow(int id_vch);
float _TYPE ST_GetHWRFFilterHigh(int id_vch);

int  	_TYPE ST_SetProcessing(int processing_type, int n_signals, int wait_prf, int id_vch);
int   _TYPE ST_GetProcessingType(int id_vch);
int   _TYPE ST_GetProcessingSignalsNumber_AVR(int id_vch);
int   _TYPE ST_GetProcessingParameter_AVR(int id_vch);
int   _TYPE ST_GetProcessingSignalsNumber_EMI(int id_vch);

// _____________________________________________________________________________
// ------ Focal Law Parameters (Matrix Cpature)
// =============================================================================

int   _TYPE ST_MC_SetEmissionFocalLawFile(char* file);
int   _TYPE ST_MC_GetEmissionFocalLawFile(char* file);
int   _TYPE ST_MC_GetFocalLawType (void);
int   _TYPE ST_MC_SetFullMatrixCapture(void);
int   _TYPE ST_MC_SetPlaneWaveImaging_Angles_deg(float *angle, int n_angles);
int   _TYPE ST_MC_SetPlaneWaveImaging_Angles_rad(float* angle, int n_angles);
int   _TYPE ST_MC_SetPlaneWaveImaging(float start_angle, float step_angle, float end_angle);
float _TYPE ST_MC_GetPlaneWaveImaging_StartAngle (void);
float _TYPE ST_MC_GetPlaneWaveImaging_StepAngle (void);
float _TYPE ST_MC_GetPlaneWaveImaging_EndAngle (void);
int _TYPE ST_MC_GetPlaneWaveImaging_Angles_rad(float* data, int size_data);
int _TYPE ST_MC_GetPlaneWaveImaging_Angles_deg(float* data, int size_data);
int _TYPE ST_MC_GetPlaneWaveImaging_AnglesNumber(void);

int _TYPE ST_DeleteFocalLaws(void);
int _TYPE ST_GetFocalLawNumber(int id_vch);
int _TYPE ST_GetTriggerFocalLawsNumber(void);
int _TYPE ST_MC_GetFocalLaw_AScanNumber(int id_fl);

int _TYPE ST_PrintVirtualChannel_BASERegisters(int id_vch);

// _____________________________________________________________________________
// ------ Focal Law Parameters (Phased Array)
// =============================================================================

int _TYPE ST_PA_SetLinearScanConfig(int step, int aperture, int startelement, int endelement, float angle);
int _TYPE ST_PA_SetAngularScanConfig(float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle);
int _TYPE ST_PA_GetAperture(void);
int _TYPE ST_PA_GetScanType(void);
int _TYPE ST_PA_GetLinearScanStep(void);
int _TYPE ST_PA_GetScanStartElement(void);
int _TYPE ST_PA_GetLinearScanEndElement(void);
float _TYPE ST_PA_GetLinearScanAngle(void);
float _TYPE ST_PA_GetAngularScanStartAngle(void);
float _TYPE ST_PA_GetAngularScanEndAngle(void);
float _TYPE ST_PA_GetAngularScanStepAngle(void);
int _TYPE ST_PA_GetFocusingEnabled(void);
int _TYPE ST_PA_SetEmissionFocalPoint(float emission_focus);
float _TYPE ST_PA_GetEmissionFocalPoint(void);
int _TYPE ST_GetImageEntryPoint_X(float* x_coordinate, int size_data);
int _TYPE ST_GetImageEntryPoint_Z(float* z_coordinate, int size_data);
int _TYPE ST_GetImageEntryPoint_Angle(float* angle, int size_data);

int _TYPE ST_PA_GetReceptionFocalLawNumber(int id_vch);
int _TYPE ST_PA_GetEmissionFocalLawNumber(int id_vch);
int _TYPE ST_SetFocalPoint(float emission_focus, float reception_focus, int id_vch); 
int _TYPE ST_GetBeamformerConfig(int* n_parallel_lines);
//int _TYPE ST_SetDynamicFocusingEnable (int data);
//int _TYPE ST_GetDynamicFocusingEnable (void);
float _TYPE ST_GetReceptionFocalPoint (void);
int _TYPE ST_GetT0Max (void);

// _____________________________________________________________________________
// ------ Phased Array Probe Parameters
// =============================================================================

int   _TYPE ST_SetProbeConfig (int n_element, float pitch, float frequency);
float _TYPE ST_GetProbePitch (void);
int   _TYPE ST_GetProbeElementNumber (void);
float _TYPE ST_GetProbeFrequency (void);
int  _TYPE ST_GetProbePinToChannel(unsigned short* pin_to_channel, int n_pin, int id_vch);
int  _TYPE ST_GetProbePinToChannelSize(int id_vch);

// _____________________________________________________________________________
// ------ Range Functions
// =============================================================================

int _TYPE ST_RangeEncodersNumber(void); 
float	_TYPE ST_RangeMaxGain (void);
float _TYPE ST_RangeMinGain (void);

int   _TYPE ST_RangePA_MinSignalAmplitude (int id_vch);
int   _TYPE ST_RangePA_MaxSignalAmplitude (int id_vch);

int   _TYPE ST_RangeFP_MinSignalAmplitude (void);
int   _TYPE ST_RangeFP_MaxSignalAmplitude (void);

float _TYPE ST_RangeMinSignalAmplitude_FSH (int id_vch);
float _TYPE ST_RangeMaxSignalAmplitude_FSH (int id_vch);

int _TYPE ST_RangeMinDownSamplingFactor(void);
int _TYPE ST_RangeMaxDownSamplingFactor(void);

int _TYPE ST_RangeMaxReductionFactor(void);
int _TYPE ST_RangeMinReductionFactor(void);

int   _TYPE ST_RangeMaxPulsesNumber(void);

int   _TYPE ST_RangeMaxProcessingSignals(void);
int   _TYPE ST_RangeMinProcessingSignals(void);

int   _TYPE ST_RangeMaxFocalLaws(void);

// _____________________________________________________________________________
// ------ Conversion Functions
// =============================================================================

float _TYPE ST_SamplePositionToDistance_float (short int data, int id_vch);

float _TYPE STPA_ADDataToFSH_float (short int data, int id_vch);

float _TYPE STFP_ADDataToFSH_float (short int data, int id_vch);

// _____________________________________________________________________________
// ------ Terminal Log Functions
// =============================================================================

int _TYPE ST_SetTerminalLog(unsigned short data);
int _TYPE ST_SetTerminalLog_PRG_REG(int data);
int _TYPE ST_GetTerminalLog_PRG_REG(void);
int _TYPE ST_SetTerminalLog_ACQUIRING(int data);
int _TYPE ST_GetTerminalLog_ACQUIRING(void);
int _TYPE ST_SetTerminalLog_ACQUIRING_FRAME(int data);
int _TYPE ST_GetTerminalLog_ACQUIRING_FRAME(void);
int _TYPE ST_SetTerminalLog_PROTOCOL(int data);
int _TYPE ST_GetTerminalLog_PROTOCOL(void);
int _TYPE ST_SetTerminalLog_BEAMFORMING(int data);
int _TYPE ST_GetTerminalLog_BEAMFORMING(void);

int  _TYPE ST_TerminalPrintVCH_Config(void);

// _____________________________________________________________________________
// ------ Encoder Functions
// =============================================================================

int _TYPE ST_SetEncoderZero(int id_encoder);
int _TYPE ST_SetAllEncoderZero(void);
int _TYPE ST_SetEncoderSign(int id_encoder);
int _TYPE ST_GetEncoderSign(int id_encoder);
int _TYPE ST_SetEncoderTrigger(int id_encoder);
int _TYPE ST_GetEncoderTrigger(void);
int _TYPE ST_SetEncoderAuxiliar(int id_encoder);
int _TYPE ST_GetEncoderAuxiliar(void);
int _TYPE ST_SetEncoderCalibration(int data, int id_encoder);
int _TYPE ST_GetEncoderCalibration(int id_encoder);
int _TYPE ST_GetAuxiliarEncoderEnabled(void);
int _TYPE ST_SetAuxiliarEncoderEnabled(int data);
int _TYPE ST_GetEncoderConnectorName(char* name, int id_encoder);
int _TYPE ST_SetEncoderPosition(long data, int id_encoder);

// ------ Encoder Scan Parameters ----------------------------------------------

int _TYPE ST_SetEncoderScanResolution(float data, int id_encoder);
float	_TYPE ST_GetEncoderScanResolution(int id_encoder);
int _TYPE ST_GetEncoderScanResolutionSteps(int id_encoder);
int _TYPE ST_SetEncoderScanLength(float data, int id_encoder);
float _TYPE ST_GetEncoderScanLength(int id_encoder);

// _____________________________________________________________________________
// ------ Scan Functions
// =============================================================================

int _TYPE ST_GetScanLinesTrigger(void);
int _TYPE ST_GetScanLinesAuxiliar(void);
int _TYPE ST_SetScanResolutionTrigger(float data);
float _TYPE ST_GetScanResolutionTrigger(void);
int _TYPE ST_GetScanResolutionTriggerSteps(void);
int _TYPE ST_SetScanResolutionAuxiliar(float data);
float _TYPE ST_GetScanResolutionAuxiliar(void);
int _TYPE ST_GetScanResolutionAuxiliarSteps(void);
int _TYPE ST_SetScanLengthTrigger(float data);
float _TYPE ST_GetScanLengthTrigger(void);
int _TYPE ST_SetScanLengthAuxiliar(float data);
float _TYPE ST_GetScanLengthAuxiliar(void);
int _TYPE ST_SetScanAuxiliarEnabled(int data);
int _TYPE ST_GetScanAuxiliarEnabled(void);

int _TYPE ST_SetScanCarriageReturn(int data);
int _TYPE ST_GetScanCarriageReturn(void);
int _TYPE ST_SetScanStartLeft(int data);
int _TYPE ST_GetScanStartLeft(void);
int _TYPE ST_SetScanStartUp(int data);
int _TYPE ST_GetScanStartUp(void);

// _____________________________________________________________________________
// ------ (fUS) Functional Ultrasound Imaging
// =============================================================================

float _TYPE ST_FUS_CentralFrequency(void);
float _TYPE ST_FUS_RapportFocalAperture(void);
float _TYPE ST_FUS_ApodizationCoefficient(void);
float _TYPE ST_FUS_LateralPositionX(void);
float _TYPE ST_FUS_PixelSizeX(void);
int   _TYPE ST_FUS_PixelsX(void);
float _TYPE ST_FUS_DepthZ(void);
int   _TYPE ST_FUS_PixelsZ(void);
float _TYPE ST_FUS_PixelSizeZ(void);
int   _TYPE ST_FUS_SinglePlane(void);
int   _TYPE ST_FUS_SamplesWavelength(void);
int   _TYPE ST_FUS_InterpolationCoefficients(void);

// _____________________________________________________________________________
// ------ (TFM) Total Focusing Method
// =============================================================================

int _TYPE ST_GetAvailable_TFM(void);
int _TYPE ST_TFM_SetBackwardConfig(float start_x, float end_x, int n_lines, float angle);
int _TYPE ST_TFM_SeFMCForwardConfig(int start_element, int end_element, int step_element);
int _TYPE ST_TFM_GetParallelLinesBlock(void);
int _TYPE ST_TFM_GetForwardStartElement(void);
int _TYPE ST_TFM_GetForwardEndElement(void);
int _TYPE ST_TFM_GetForwardStepElement(void);
int _TYPE ST_TFM_GetBackwardLinesNumber(void);
int _TYPE ST_TFM_GetAcqDistance(float* start_x, float* start_z, float* end_x, float* end_z);
int _TYPE ST_TFM_GetAcqTime(float* start_x, float* start_z, float* end_x, float* end_z);
float _TYPE ST_TFM_BackwardStartDistance(void);
float _TYPE ST_TFM_BackwardEndDistance(void);
float _TYPE ST_TFM_BackwardAngle(void);
float _TYPE ST_TFM_GetBackwardAngle(void);

int   _TYPE ST_TFM_GetFocalLawType(void);
int   _TYPE ST_TFM_SetPlaneWaveImaging_Angles_deg(float* angle, int n_angles);
int   _TYPE ST_TFM_SetPlaneWaveImaging_Angles_rad(float* angle, int n_angles);
int   _TYPE ST_TFM_SetPWIForwardConfig(float start_angle, float step_angle, float end_angle);
float _TYPE ST_TFM_GetPlaneWaveImaging_StartAngle(void);
float _TYPE ST_TFM_GetPlaneWaveImaging_StepAngle(void);
float _TYPE ST_TFM_GetPlaneWaveImaging_EndAngle(void);
int _TYPE ST_TFM_GetPlaneWaveImaging_Angles_rad(float* data, int size_data);
int _TYPE ST_TFM_GetPlaneWaveImaging_Angles_deg(float* data, int size_data);
int _TYPE ST_TFM_GetPlaneWaveImaging_AnglesNumber(void);

// _____________________________________________________________________________
// ------ XML Hardware Config
// =============================================================================

int _TYPE ST_XML_GetBuffer_PCIeUserBufferBurstsNumber(void);
int _TYPE ST_XML_GetBuffer_PCIeDMABlocksNumber(void);
int _TYPE ST_XML_GetBuffer_PCIeFIFOPointersNumber(void);
int _TYPE ST_XML_GetDataLogger_PCIeUserBuffer(void);
int _TYPE ST_XML_GetDataLogger_Enabled(void);
int _TYPE ST_XML_GetConfig_StatusPeriod(void);
int _TYPE ST_XML_GetConfig_StatusEnabled(void);

#ifdef __cplusplus
}
#endif

#endif
