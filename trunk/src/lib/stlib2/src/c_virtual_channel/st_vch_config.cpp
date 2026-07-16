// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdlib.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_vch_config.h"
#include "st_get.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "st_calc.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "fileformat.h"
#include "c_virtual_channel.h"
#include "sitau_hw.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

bool gb_vch_load_config_update = false;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_VCH_PrintFileConfig_PASystem (char *file)
//{
//int i, j, result, n_fl = 0;
//FILE *f = NULL;
//CPASystem *pa_sys = NULL;
//
//   if (sitau_hw == NULL) return ELOG(charES2_SYSTEM_NULL, ES2_SYSTEM_NULL);
//   if ((f = fopen (file,"wt")) == NULL) return ELOG(charES2_FILE,ES2_FILE);
//   for (i=0; i<sitau_hw->GetPASystemNumber(); i++)
//   {
//      if ((pa_sys = STHW_GetPtrPASystem(i)) != NULL) pa_sys->PrintFileConfig(f);
//      //if ((pa_sys = sitau_hw->GetPtrPASystem(i)) != NULL) pa_sys->PrintFileConfig(f);
//      for (j=n_fl=0; j<gb_virtual_channel.size(); j++)
//      {
//         if (gb_virtual_channel[j] != NULL)
//         {
//            if ((result = gb_virtual_channel[j]->CONFIG_PrintFilePASystem(f, i)) < 0)
//               {RLOG(result);}
//            else n_fl += result;
//         }
//      }
//      fprintf (f, "\n    TOTAL FOCAL LAWS: %d", n_fl);
//   }
//   fclose(f);
//   return ES2_NONE;
//}






