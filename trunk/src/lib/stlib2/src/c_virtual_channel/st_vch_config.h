#ifndef st_vch_configH
#define st_vch_configH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdio.h>
#if !defined(_MSC_VER)
   #include <vcl.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define STCFG_VERSION_         "ST-0000.0000.0000.0001"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

long ST_VCH_SaveConfig (char *file, int create_file);

//int ST_VCH_PrintFileConfig (char *file, bool sitau_info=false);

//int ST_VCH_PrintFileConfig_PASystem (char *file);

extern bool gb_vch_load_config_update;

#endif
