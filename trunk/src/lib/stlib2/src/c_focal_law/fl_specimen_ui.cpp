// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <math.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_specimen_ui.h"
#include "fl_scan_ui.h"
#include "fl_error_code.h"
#include "c_log_file.h"
#include "fl_calc.h"

using namespace std;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CFL_SpecimenUI::CFL_SpecimenUI (float velocity) 
   : CFL_Specimen (velocity)
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_SpecimenUI::SetVelocity (float data)
{
	int result;

	if ((result = CFL_Specimen::SetVelocity(data)) < 0) return RLOG(result);
	return EFL_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CFL_SpecimenUI::CONFIG_Copy (CFL_SpecimenUI *specimen)
{
   if (specimen == NULL) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);
   velocity_ = specimen->GetVelocity();
   return EFL_NONE;
}


