#ifndef st_vch_loadH
#define st_vch_loadH

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

int ST_VCH_LOAD_ReceiverMode (int data, int id_vch=-1);

int ST_VCH_LOAD_Pulser (int n_pulses, float pulse_width, int id_vch=-1);

int ST_VCH_LOAD_PRFEnabled (int data, int id_vch=-1);

int ST_VCH_LOAD_SamplingFrequency (int data, int id_vch=-1);

int ST_VCH_LOAD_MaterialVelocity (float data, int id_vch=-1);

int ST_VCH_LOAD_AcqTime (float acq_start_time, float acq_end_time, int ch, int id_vch=-1);

int ST_VCH_LOAD_Gain (float gain_db, int id_vch=-1);

#endif
