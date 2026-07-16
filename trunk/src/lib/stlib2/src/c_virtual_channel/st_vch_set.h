#ifndef st_vch_setH
#define st_vch_setH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

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

int ST_VCH_SET_Name (char *data, int id_vch=-1);

int ST_VCH_SET_ReceiverMode (int data, int id_vch=-1);

int ST_VCH_SET_Pulser (int n_pulses, float pulse_width, int id_vch=-1);

int ST_VCH_SET_PulseWidth (float pulse_width, int id_vch=-1);

int ST_VCH_SET_PulsesNumber (int n_pulses, int id_vch=-1);

int ST_VCH_SET_PRFEnabled (int data, int id_vch=-1);

int ST_VCH_SET_SamplingFrequency (int data, int id_vch=-1);

int ST_VCH_SET_Attenuator (int data, int id_vch=-1);

int ST_VCH_SET_PRFTime (float data, int id_vch=-1);

int ST_VCH_SET_AcqTime (float acq_start_time, float acq_end_time, int id_vch=-1);

int ST_VCH_SET_SignalTest (int data, int id_vch=-1);

int ST_VCH_SET_PulserEnabled (bool data, int id_vch=-1);

//int ST_VCH_SET_OffsetTriggerLine (int data, int id_vch=-1);

int ST_VCH_SET_PlaneWaveImagingCompounding_Angles (float *angle, int n_angles, int id_vch=-1);

int ST_VCH_SET_PlaneWaveImagingCompounding (float start_angle, float step_angle, float end_angle, int id_vch=-1);

int ST_VCH_SET_FulMatrixCapture (int id_vch=-1);

int ST_VCH_DELETE_FocalLaws (int id_vch=-1);

void ST_VCH_RESET_AcquisitionCounter (void);

int ST_VCH_SET_PAConfig (unsigned short n_element, float pitch, float frequency, int id_vch);

#endif
