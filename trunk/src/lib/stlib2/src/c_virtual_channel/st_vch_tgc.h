#ifndef st_vch_tgcH
#define st_vch_tgcH

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

int ST_VCH_LOAD_DigitalTGC_Attenuator (int data, int id_vch=-1);

int ST_VCH_GET_DigitalTGC_Attenuator (int id_vch=-1);

int ST_VCH_GET_LOG2_AVERAGE (int id_vch=-1);

int ST_VCH_GET_LNA_GAIN (int id_vch=-1);

int ST_VCH_GET_PGA_GAIN (int id_vch=-1);

int ST_VCH_SET_DigitalTGC_Attenuator (int data, int id_vch=-1);

int ST_VCH_SET_LOG2_AVERAGE (int data, int id_vch=-1);

int ST_VCH_SET_LNA_GAIN (int data, int id_vch=-1);

int ST_VCH_SET_PGA_GAIN (int data, int id_vch=-1);

int ST_VCH_SET_TGC_Enabled (int data, int id_vch=-1);

int ST_VCH_GET_TGC_Enabled (int id_vch=-1);

int ST_VCH_SET_TGC_AttenuationCoefficient(float data, int id_vch = -1);

float ST_VCH_GET_TGC_AttenuationCoefficient(int id_vch = -1);

#endif
