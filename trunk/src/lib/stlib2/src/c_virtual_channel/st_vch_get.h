#ifndef st_vch_getH
#define st_vch_getH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_VCH_GET_PulsesNumber (int id_vch=-1);

float ST_VCH_GET_PulseWidth (int id_vch=-1);

int ST_VCH_GET_AutoPulseWidth (int id_vch=-1);

float ST_VCH_GET_MaterialVelocity ( int id_vch=-1);

int ST_VCH_GET_ChannelNumber (int id_vch=-1);

int ST_VCH_GET_FP_ImageDataNumber (int id_focal_law, int id_vch=-1);

float ST_VCH_GET_AcqStartTime (int id_vch=-1);

float ST_VCH_GET_AcqEndTime (int id_vch=-1);

float ST_VCH_GET_AcqTime (int id_vch=-1);

float ST_VCH_GET_AcqStartDistance (int id_vch=-1);

float ST_VCH_GET_AcqEndDistance (int id_vch=-1);

int ST_VCH_GET_SignalTest (int id_vch=-1);

int ST_VCH_GET_VirtualChannelName (char *name, int id_vch=-1);

int ST_VCH_GET_PulserEnabled (int id_vch=-1);

float ST_VCH_GET_TimeResolution (int id_vch=-1);

float ST_VCH_GET_DistanceResolution (int id_vch=-1);

int ST_VCH_GET_ActiveChannelsNumber (int id_vch=-1);

int ST_VCH_GET_MultiplexedChannelsNumber (int id_vch=-1);

int ST_VCH_GET_ChannelsNumber (int id_vch=-1);

int ST_VCH_GET_MODULENumber (int id_vch=-1);

int ST_VCH_GET_BASENumber (int id_vch=-1);

float ST_VCH_GET_PRFTime(int id_vch=-1);

int ST_VCH_GET_PRFEnabled(int id_vch=-1);

float ST_VCH_GET_SamplingFrequency (int id_vch=-1);

int ST_VCH_GET_DownSamplingFactor (int id_vch=-1);

int ST_VCH_GET_Attenuator (int id_vch=-1);

int ST_VCH_GET_AScanSamplesPositionTime_double (vector<double> &data, int id_vch=-1);

int ST_VCH_GET_AScanSamplesPositionTime_float (vector<float> &data, int id_vch=-1);

int ST_VCH_GET_AScanSamplesPositionDistance_float (vector<float> &data, int id_vch=-1);

int ST_VCH_GET_AScanSamplesPositionDistance_double (vector<double> &data, int id_vch=-1);

double ST_VCH_GET_AScanSamplePositionTime_double (int ind_sample, int id_vch=-1);

float ST_VCH_GET_AScanSamplePositionTime_float (int ind_sample, int id_vch=-1);

double ST_VCH_GET_AScanSamplePositionDistance_double (int ind_sample, int id_vch=-1);

float ST_VCH_GET_AScanSamplePositionDistance_float (int ind_sample, int id_vch=-1);

int ST_VCH_GET_PASystemInfo ( int *pa_system_id,
                              int *pa_connector_id,
                              int *pa_pin_start,
                              int id_vch=-1);

float ST_VCH_GET_PA_Pitch (int id_vch=-1);

int ST_VCH_GET_PA_ElementNumber (int id_vch=-1);

float ST_VCH_GET_PA_Frequency (int id_vch=-1);

int ST_VCH_GET_FP_FocalLawType (int id_vch=-1);

float ST_VCH_GET_PWIC_StepAngle (int id_vch=-1);

int ST_VCH_GET_PA_T0_Max (int id_vch=-1);

#endif

