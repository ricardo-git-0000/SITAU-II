#ifndef fp_t_pa_system_tadH
#define fp_t_pa_system_tadH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_info.h"
#include "t_base_tad.h"
#include "t_sitau_hw.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct T_PASystemConnectorConfig {
   AnsiString name;

   bool enabled;
   bool multiplexed;

   int n_active_channels;
   int n_multiplexed_channels;
   int n_total_channels;

   unsigned short start_channel;
} T_PASystemConnectorConfig;

typedef struct T_PASystemConfig {
   AnsiString name;
   int id;

   bool enabled;
   bool multiplexed;

   unsigned char id_frontend;
   int n_bases;
   unsigned char id_base_first;
   unsigned char id_base_last;
   int n_modules;
   int n_modules_base;
   unsigned char id_mod_first;
   unsigned char id_mod_last;
   unsigned short id_fl_first;
   unsigned short id_fl_last;
   unsigned char start_element;
   unsigned short actual_focal_law;

   int n_active_channels;
   int n_multiplexed_channels;
   int n_total_channels;

   vector<TInfoChannel> element_to_channel;
   vector<unsigned short> pin_to_channel;
   vector<unsigned short> disabled_channel;

   vector<T_PASystemConnectorConfig> connector;

   TXMLHW_FRONTEND* frontend;
} T_PASystemConfig;

typedef struct T_PASystem_CompareFocalLaw {
   int id_vch;
   int n_fl;
} T_PASystem_CompareFocalLaw;

typedef struct T_PASystem_BASE_WNLIN {
   int lact;
   int nlin;
} T_PASystem_BASE_WNLIN;

typedef struct T_PASystem_VCH {
   int id;
   int wnlin_lact;
   int wnlin_nlin;
   int n_fl;
} T_PASystem_VCH;

typedef struct T_PASystem_BASE_ShareVCH {
   vector<vector<T_PASystem_VCH> > base;
   vector<vector<vector<T_PASystem_VCH> > > module;
} T_PASystem_ShareVCH;


// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
