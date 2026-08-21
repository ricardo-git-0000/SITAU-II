#ifndef fp_c_virtual_channelH
#define fp_c_virtual_channelH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <system.hpp>
#endif

#include "tad.h"
#include "c_acq_data.h"
#include "c_xml_config.h"

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_vch_config.h"
#include "c_base.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "c_buffer_tfm_scan_hw_ut.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define VCH_PROCESSING_TYPE_MAX        3
#define VCH_PROCESSING_N_SIGNALS_MAX   255

#define VCH_MAX_LINES	            256 //!< Número máximo de líneas
#define VCH_MAX_VIRTUAL_CHANNELS    10

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ CVirtualChannel class
// _____________________________________________________________________________
// =============================================================================

class CVirtualChannel
{
private:

   bool update_dsp_scan_data_;
   bool memory_resize_enabled_;
   bool gb_cfg_change_info;
   bool update_register_;
   bool update_focal_law_fp_;
   bool update_focal_law_pa_;
   bool update_tfm_forward_;
   bool update_tfm_backward_;
   bool update_tgc_;
   bool update_fir_filter_;
   bool update_next_vch_amplia_command_;
   bool update_previous_vch_amplia_command_;
   bool updating_amplia_command_;
   CVirtualChannel *previous_;
   CVirtualChannel *next_;

   vector<unsigned long> amplia_command_;

   int Set_Registers(void);

   int SET_EmissionFocalLaws(void);

   int Set_TGC(void);

   int Set_FIRFilter(void);

   // c_vch_config.cpp _________________________________________________________

   int DecoFileVersion (   AnsiString version_file,
                           unsigned short *field_1,
                           unsigned short *field_2,
                           unsigned short *field_3,
                           unsigned short *field_4);

   int Load_AcqTime (float acq_start_time, float acq_end_time, bool update=false);

protected:

public:

   CBufferMC_ScanHWUT *buffer_mc_hwut_;
   CBufferPA_ScanHWUT *buffer_pa_hwut_;
   CBufferTFM_ScanHWUT* buffer_tfm_hwut_;
   TFP_VCH_SysConfig sys_;
   TFP_VCH_ConfigData cfg_;
   CAcqData acq_data_;

   CVirtualChannel (int id, int id_pa_sys, int id_connector, AnsiString name, bool hw_format);

   ~CVirtualChannel();

   int PRINT_Beamformer(void);

   int PRINT_TFM_Backward(void);

   int PRINT_TFM_FMC_Forward(void);
   int PRINT_TFM_PWI_Forward(void);

   int ResetRegister(void);

   int HardwareSetup(int id_pa_sys, int id_connector);

   void UpdateConfig(void);

   int Setup(int id_pa_sys, int id_connector, AnsiString name);

   int FP_UpdateAcquisitionSize (void);
   
   int PA_UpdateAcquisitionSize (void);

   int TFM_UpdateAcquisitionSize(void);
   
   int UpdateAcquisitionSize (void);

   float AcquisitionDistanceMax (void);

   int MinDownSamplingFactor (float *fs_max);

   int Set(bool update=false);

   CPASystem *GetPASystem(void) {return sys_.pa_sys;}

   void Release (void);

   int UpdateAScanSamplesPosition(TFP_VCH_AcquisitionConfig *acq);

   void DSP_UpdateScanDataEnabled(bool data) {update_dsp_scan_data_ = data;}

   int GetSamplesIncrement (void);

   // c_vch_tgc.cpp ____________________________________________________________

   int SET_DigitalTGC_Attenuator(int data);
   int LOAD_DigitalTGC_Attenuator(int data);
   int SET_LOG2_AVERAGE(int data);
   int SET_LNA_GAIN(int data);
   int LOAD_LOG2_AVERAGE(int data);
   int LOAD_LNA_GAIN(int data);
   int LOAD_LNA_GAIN(float data);
   int SET_PGA_GAIN(int data);
   int LOAD_PGA_GAIN(int data);
   int GET_DigitalTGC_Attenuator(void);
   int GET_LOG2_AVERAGE(void);
   int GET_LNA_GAIN(void);
   int GET_PGA_GAIN(void);
   int LOAD_TGC_Enabled(int data);
   int SET_TGC_Enabled(int data);
   int GET_TGC_Enabled(void) {return cfg_.tgc_enabled;}
   int LOAD_TGC_AttenuationCoefficient(float attn_coefficient);
   int SET_TGC_AttenuationCoefficient(float data);
   float GET_TGC_AttenuationCoefficient(void);
   int GET_PA_ParallelBeamformedLines (void);
   int GET_PA_IncreasedAcquiredSamples (void) {return cfg_.acq.increased_acq_samples;}

   int GET_TFM_ParallelBeamformedLines(void) { return cfg_.tfm_backward.parallel_lines; }
   //int GET_TFM_EmissionStartElement(void) { return cfg_.tfm.emi_start_element; }
   //int GET_TFM_EmissionEndElement(void) { return cfg_.tfm.emi_end_element; }
   //int GET_TFM_EmissionStepElement(void) { return cfg_.tfm.emi_step_element; }
   int GET_TFM_BackwardLinesNumber(void)  { return (int)cfg_.tfm_backward.beamformer.size(); }
   float GET_TFM_BackwardStartDistance(void) { return cfg_.tfm.start_x_distance; }
   float GET_TFM_BackwardEndDistance(void) { return cfg_.tfm.end_x_distance; }
   float GET_TFM_BackwardStartTime_X(void) { return cfg_.tfm.start_x_time; }
   float GET_TFM_BackwardEndTime_X(void) { return cfg_.tfm.end_x_time; }
   float GET_TFM_BackwardAngle(void) { return cfg_.tfm.angle; }
   int GET_TFM_ForwardStartElement(void);
   int GET_TFM_ForwardEndElement(void);
    int GET_TFM_ForwardStepElement(void);
    //int GET_TFM_BackwardStartAperture(void) { return 0; }
    //int GET_TFM_BackwardAperture(void) { return 128; }

   // --------------------------------------------------------------------------
   // c_vch_config.cpp _________________________________________________________

   int CONFIG_Copy (CVirtualChannel *vch);
   int LoadXMLConfigData (TXMLCFG_VirtualChannel *xml_vch, int n_bursts, int n_images_per_burst, int id_vch);
   int LoadXMLHardwareData(void);
   int GetXMLConfigData (TXMLCFG_VirtualChannel *xml_vch);
   //int FP_PRINT_EmissionFocalLawFile(char* file = NULL);
   int FP_PRINT_EmissionFocalLawDelaysFile(void);

   int PrintFileConfig(FILE *f, bool sitau_info=false);
   int CONFIG_PrintFilePASystem(FILE *f, int id_pa_sys);

   //float SamplePositionToDistance_float (short int samples);
   //float SamplePositionToTime_float (short int samples);
   float TimeToDistance (float data);
   float DistanceToTime (float data);
   float SamplesToTime (int data);
   //float SamplesToTime_HWGate (int data);
   //float SamplesToDistance_HWGate (int data);
   int TimeToSamples (float data);
   //unsigned long TimeToSamples_HWGate (float data);
   float SamplesToDistance (int data);
   float PA_ADDataToFSH (short int data);
   float FP_ADDataToFSH (short int data);
   int PA_FSHToADData (float data);
   int FP_FSHToADData (float data);

   // --------------------------------------------------------------------------
   // c_vch_set.cpp ____________________________________________________________

   int GET_Id(void) {return cfg_.id;}
   int SET_ReceiverMode (int data);
   int SET_Pulser (int n_pulses, float pulse_width);
   int SET_PulseWidth (float pulse_width);
   int SET_PulsesNumber (int n_pulses);
   int SET_PRFEnabled(int data);
   int SET_SamplingFrequency (int data);
   int SET_Attenuator (unsigned short data);
   int SET_PRFTime(float data);
   int SET_MaterialVelocity (float data);
   int SET_AcqTime (float acq_start_time, float acq_end_time);
   int SET_AcqDistance (float acq_start_distance, float acq_end_distance, int pulse_echo);
   int SET_Gain (float gain_db);
   int SET_AFEGain(int attn, int lna, int pga);
   int SET_SignalTest (int data);
   int SET_PulserEnabled (bool data);
   int SET_Name(char *name);
   int SET_PWICFocalLaw_Angles(float *angle, int n_angles);
   int SET_TFM_PWICFocalLaw_Angles(float* angle, int n_angles);
   int SET_PWICFocalLaw(float start_angle, float step_angle, float end_angle);
   int SET_TFM_PWIForwardConfig(float start_angle, float step_angle, float end_angle);
   int SET_FMCFocalLaw(void);
   int DELETE_FocalLaws(void);
   void RESET_AcquisitionCounter(void);
   int SET_PAConfig (unsigned short n_element, float pitch, float frequency);
   int SET_RFFilter(float low_frequency, float high_frequency);
   int SET_RFFilterEnabled(bool data);
   int SET_ReductionFactor(int data);
   int SET_Processing(int processing_type, int n_signals, int wait_prf);
   int FP_SET_EmissionFocalLawFile(char* file);
   //int TFM_SET_Config(vector<int> &rec_elements, vector<int> &emi_elements, float start_x, float end_x, int n_lines, float angle);
   int TFM_SET_BackwardConfig(float start_x, float end_x, int n_lines, float angle);
   int TFM_SET_FMC_ForwardConfig(vector<int>& emi_elements);
   int TFM_SET_FMC_ForwardConfig(int start_element, int end_element, int step_element);

   // --------------------------------------------------------------------------
   // c_vch_load.cpp ___________________________________________________________

   int LOAD_DefaultConfig (void);
   int LOAD_ReceiverMode (int data, bool update_range=true);
   int LOAD_Pulser (int n_pulses, float pulse_width);
   int LOAD_PulseWidth (float data);
   int LOAD_PulsesNumber (int data);
   int LOAD_PRFEnabled(int data);
   int LOAD_SamplingFrequency (int data);
   int LOAD_Attenuator (unsigned short data);
   int LOAD_PRFTime(float data);
   int LOAD_PRFBurstTime(float data);
   int LOAD_MaterialVelocity (float data);
   int LOAD_AcqTime (float acq_start_time, float acq_end_time, bool update=false);
   int LOAD_AcqDistance (float acq_start_distance, float acq_end_distance, int pulse_echo, bool update=false);
   int LOAD_AcqSamples(int acq_start_samples, int n_acq_samples, bool update = false);
   int RELOAD_AcqDistance (void);
   int LOAD_Gain (float gain_db);
   int RELOAD_Gain (void);
   int LOAD_SignalTest (unsigned short data);
   int LOAD_PulserEnabled (bool data);
   int RELOAD_PWICFocalLaw(void);
   int LOAD_PWICFocalLaw_Angles(float *angles, int n_angles);
   int TFM_LOAD_PWI_Forward_Angles(float* angles, int n_angles);
   int LOAD_PWICFocalLaw(float start_angle, float step_angle, float end_angle);
   int TFM_LOAD_PWI_Forward(float start_angle, float step_angle, float end_angle);
   int LOAD_FMCFocalLaw(void);
   int LOAD_Beamformer(void);
   //int TFM_HWMemory(void);
   //int MC_LOAD_EmissionFocalLaw (void);
   //int PA_LOAD_EmissionFocalLaw(void);
   //int TFM_LOAD_EmissionFocalLaw(void);
   int LOAD_EmissionFocalLaw(vector <vector< T_VCH_EmitterFocalLaw > > &emission_focal_law);
   int LOAD_PAConfig (unsigned short n_element, float pitch, float frequency);
   int LOAD_PinToChannel(vector<unsigned short>& data);
   int SET_AcquisitionType(int data);
   int GET_AcquisitionType (void) {return cfg_.acquisition_type;}
   int SET_SignalMode(int data);
   int GET_SignalMode(void) { return cfg_.signal_mode; }
   int LOAD_RFFilter(float low_frequency, float high_frequency);
   int RELOAD_RFFilter(void);
   int LOAD_RFFilterEnabled(bool data);
   int LOAD_Processing(int processing_type, int n_signals, int wait_prf);
   int FP_LOAD_EmissionFocalLawFile(const char* file);
   int RELOAD_FocalLaw(void);
   //int TFM_LOAD_FMC_Config(vector<int> &rec_elements, vector<int> &emi_elements, float start_x, float end_x, int n_lines, float angle);
   int TFM_LOAD_Backward(float start_x, float end_x, int n_lines, float angle);
   int TFM_BackwardHWMemory(void);
   int TFM_LOAD_FMC_Forward(vector<int>& elements);
   int TFM_LOAD_FMC_Forward(int start_element, int end_element, int step_element);
   int TFM_FMC_Forward_HWMemory(void);
   int TFM_PWI_Forward_HWMemory(void);
   int LOAD_AcquisitionType(int data);
   int RELOAD_AcquisitionType(void);

   // --------------------------------------------------------------------------
   // c_vch_fl_pa.cpp ____________________________________________________________

   int LOAD_PA_FocalLaw (void);
   int SET_Beamformer (void);
   int TFM_SET_FMC_Forward(void);
   int TFM_SET_PWI_Forward(void);
   int SET_TFMBackward(void);
   int LOAD_PA_LinearScanConfig (int step, int aperture, int startelement, int endelement, float angle);
   int LOAD_PA_AngularScanConfig (float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle);
   int SET_PA_LinearScanConfig (int step, int aperture, int startelement, int endelement, float angle);
   int SET_PA_AngularScanConfig (float step, unsigned short aperture, unsigned short startelement, float startangle, float endangle);
   //int LOAD_PA_DynamicFocusingEnable (int data);
   //int SET_PA_DynamicFocusingEnable (int data);
   //int GET_PA_DynamicFocusingEnable (void);
   int GET_PA_ReceptionAperture (void);
   int GET_PA_EmissionAperture (void);
   int GET_PA_ScanType (void);
   int GET_PA_LinearStep (void);
   int GET_PA_ReceptionStartElement (void);
   int GET_PA_EmissionStartElement (void);
   int GET_PA_LinearEndElement (void);
   float GET_PA_LinearAngle (void);
   float GET_PA_AngularStartAngle (void);
   float GET_PA_AngularEndAngle (void);
   float GET_PA_AngularStep (void);
   int GET_PA_FocusingEnabled (void);
   float GET_PA_EmissionFocalPoint (void);
   float GET_PA_ReceptionFocalPoint (void);
   //int GET_PA_ReceptionAutoFocusEnabled (void);
   //int GET_PA_EmissionAutoFocusEnabled (void);
   int GET_ImageEntryPoint_X (float *x_coordinate, int size_data);
   int GET_ImageEntryPoint_Z (float *z_coordinate, int size_data);
   int GET_ImageEntryPoint_Angle (float *angle, int size_data);
   int GET_PA_ReceptionFocalLawNumber(void);
   int GET_PA_EmissionFocalLawNumber (void);
   int GET_PA_T0_Max(void);
   int GET_ProcessingType(void);
   int GET_ProcessingSignalsNumber_EMI(void);
   int GET_ProcessingSignalsNumber_AVR(void);
   int GET_ProcessingParameter_AVR(void);
   int GET_ReductionFactor(void);
   int LOAD_PA_FocalPoint(float emission_focus, float reception_focus);
   int LOAD_PA_EmissionFocalPoint(float emission_focus);
   int SET_PA_FocalPoint(float emission_focus, float reception_focus);
   int SET_PA_EmissionFocalPoint(float emission_focus);

   // --------------------------------------------------------------------------
   // c_vch_get.cpp ____________________________________________________________

   int GET_SignalTest(void);
   int GET_AcquisitionTimeMode(void);
   int GET_Attenuator (void);
   float GET_SamplingFrequency (void);
   int GET_DownSamplingFactor (void);
   bool GET_AutoPulseWidth (void) {return cfg_.auto_pulse_width;}
   bool GET_PulserEnabled(void);
   float GET_PulseWidth(void);
   int GET_PulsesNumber(void);
   float GET_Gain(void) {return cfg_.gain_db;}
   int GET_AFEGain(int* attn, int* lna, int* pga);
   int GET_FP_AScanDataNumber (void);
   int GET_AScanDataNumber(void);
   int GET_FP_ImageDataNumber (int id_focal_law);
   int GET_AcqStartSamples(void);
   float GET_AcqStartTime (void);
   float GET_AcqEndTime (void);
   float GET_AcqTime (void);
   float GET_AcqStartDistance (void);
   float GET_AcqEndDistance (void);
   unsigned long GET_FP_AcquisitionSize(void) {return cfg_.fp_acquisition_size;}
   int GET_Name(char *name);
   float GET_TimeResolution(void);
   float GET_DistanceResolution(void);
   int GET_ActiveChannelsNumber(void);
   int GET_MultiplexedChannelsNumber(void);
   int GET_ChannelsNumber(void);
   int GET_MODULENumber(void);
   int GET_BASENumber(void);
   float GET_PRFTime(void);
   float GET_PRFBurstTime(void);
   int GET_PRFEnabled(void);
   int GET_PRFBurstEnabled(void);
   int GET_AScanSamplesPositionTime_double (vector<double> &data);//, int ch);
   int GET_AScanSamplesPositionTime_float (vector<float> &data);//, int ch);
   int GET_AScanSamplesPositionDistance_double (vector<double> &data);//, int ch);
   int GET_AScanSamplesPositionDistance_float (vector<float> &data);//, int ch);
   double GET_AScanSamplePositionTime_double (int ind_sample);
   float GET_AScanSamplePositionTime_float (int ind_sample);
   double GET_AScanSamplePositionDistance_double (int ind_sample);
   float GET_AScanSamplePositionDistance_float (int ind_sample);
   int GET_MC_AScanNumber(void);
   int GET_MC_FMCMatrix_Size(void);
   int GET_MC_SingleImage_Size(void);
   int GET_FocalLawNumber(void);
   int GET_MC_FocalLaw_AScanNumber(int id_focal_law);
   float GET_MaterialVelocity (void);// {return cfg_.material_velocity;}
   unsigned long GET_AcquisitionCounter(void);
   int GET_LastFocalLawAcquired(void);
   float GET_Probe_Pitch(void);// {return cfg_.probe_pitch;}
   int GET_Probe_ElementNumber(void);// {return cfg_.probe_n_elements;}
   float GET_Probe_Frequency(void);// {return cfg_.probe_frequency;}
   int GET_AcqSamplesTime (float *samples_time, int size_data);
   int GET_BeamformedSamplesTime (float *samples_time, int size_data);
   int GET_AcqSamplesDistance (float *samples_distance, int size_data);
   int GET_BeamformedSamplesDistance (float *samples_distance, int size_data);
   int GET_MC_ImageEntryPoint(float *data, int size_data);
   int GET_MC_ImageEntryPoint(vector<float>& data);
   float GET_PWIC_StartAngle(void) {return cfg_.fp_pwic_start_angle;}
   float GET_PWIC_StepAngle(void) {return cfg_.fp_pwic_step_angle;}
   float GET_PWIC_EndAngle(void) {return cfg_.fp_pwic_end_angle;}
   float GET_TFM_PWIC_StartAngle(void) { return cfg_.tfm.pwic_start_angle; }
   float GET_TFM_PWIC_StepAngle(void) { return cfg_.tfm.pwic_step_angle; }
   float GET_TFM_PWIC_EndAngle(void) { return cfg_.tfm.pwic_end_angle; }

   int GET_MC_FocalLawType(void) {return cfg_.fp_focal_laws_type;}
   int GET_TFM_FocalLawType(void) { return cfg_.tfm.focal_laws_type; }
   float GET_HWRFFilterLow(void) { return cfg_.fir.low_frequency; }
   float GET_HWRFFilterHigh(void) { return cfg_.fir.high_frequency; }
   bool GET_HWRFFilterEnable(void) { return cfg_.fir.enabled; }
   //float GET_HWRFFilterLow(void) { return cfg_.acquisition_type == ACQUISITION_TYPE_PA ? cfg_.fir.low_frequency : cfg_.fp_fir.low_frequency; }
   //float GET_HWRFFilterHigh(void) { return cfg_.acquisition_type == ACQUISITION_TYPE_PA ? cfg_.fir.high_frequency : cfg_.fp_fir.high_frequency; }
   //bool GET_HWRFFilterEnable(void) { return cfg_.acquisition_type == ACQUISITION_TYPE_PA ? cfg_.fir.enabled : cfg_.fp_fir.enabled; }
   AnsiString GET_FP_EmissionFocalLawFile(void) { return cfg_.fp_focal_law_file; }
   //int GET_AcquisitionType(void) { return cfg_.acquisition_type_kk; }
   int GET_PinToChannel(unsigned short* pin_to_channel, int n_pin);
   int GET_PinToChannelSize(void);
   int GET_PCIe_FocalLawsBlockNumber(void) { return (int)cfg_.gtx_nfl_blocks; }
   int GET_PCIe_BlocksBurstNumber(void) { return (int)cfg_.gtx_n_blocks_burst; }
   vector<float> GET_PWIC_Angles(void) { return cfg_.fp_pwic_angles; }
   vector<float> GET_TFM_PWIC_Angles(void) { return cfg_.tfm.pwic_angles; }
   int GET_PWIC_AnglesNumber(void) { return (int)cfg_.fp_pwic_angles.size(); }
   int GET_TFM_PWIC_AnglesNumber(void) { return (int)cfg_.tfm.pwic_angles.size(); }
   int FP_GET_EmissionFocalLawFile(char* file);

   int SaveFileAcquisition(fstream& f);
   int SaveFileConfig(fstream& f);

   int FL_PWIEmission(vector <vector< T_VCH_EmitterFocalLaw > >& fl, float* angle, int n_angles, float pitch, float velocity, int n_ch);
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern vector<CVirtualChannel *> gb_virtual_channel;
extern vector<CVirtualChannel *> gb_vch_load_config;
extern int gb_selected_virtual_channel;

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

float ST_VCH_SamplesToTime (int data, int id_vch=-1);

int ST_VirtualChannels_MemoryResize(int id_vch=-1, int n_acquisitions=0);

int ST_VirtualChannelsDestroy (void);

int ST_VirtualChannelsDelete (void);

int ST_VirtualChannelDelete(int id_virtual_channel);

int ST_VirtualChannelsDefault (void);

int ST_VirtualChannelsAdd(char *name, int id_pa_sys=0, int id_connector=0);

int ST_SetVirtualChannel(bool update=false);

int ST_SetVirtualChannel(int id_virtual_channel, bool update=false);

int ST_GetFP_AcquisitionSize(vector<unsigned long> &acq_size);

int ST_VirtualChannel_Set(void);

int ST_VCH_SamplePositionToDistance_vdouble (   short int *samples,
                                    unsigned long n_data,
                                    vector<double> &position_distance,
                                    int id_vch);

float ST_VCH_SamplePositionToTime_float (short int samples, int id_vch=-1);

int ST_VCH_SamplePositionToTime_vdouble (   short int *samples,
                                    unsigned long n_data,
                                    vector<double> &position_time,
                                    int id_vch);

int STPA_ADDataToFSH_vdouble (short int *samples, unsigned long n_data, vector<double> &fsh, int id_vch);

int STFP_ADDataToFSH_vdouble (short int *samples, unsigned long n_data, vector<double> &fsh, int id_vch);

int STPA__VCH_FSHToADData (float data, int id_vch);

int STFP_VCH_FSHToADData (float data, int id_vch);

#endif
