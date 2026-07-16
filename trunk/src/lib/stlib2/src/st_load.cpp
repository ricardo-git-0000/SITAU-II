// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdio.h>
#include <math.h>
#include <limits>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_load.h"
#include "st_info.h"
#include "st_range.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_calc.h"

#include "st_reg.h"
#include "st_get.h"
#include "c_log_file.h"
#include "utils.h"
#include "st_error_code.h"
#include "c_virtual_channel.h"
#include "st_vch_load.h"
#include "st_vch_set.h"
#include "st_vch_get.h"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S ---------------------------
// ======================================================================

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S -----------------
// ======================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadPRF (float data, int id_virtual_channel)
{
	int result = ES2_NONE;

	if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
	if (id_virtual_channel < 0 || id_virtual_channel >= gb_virtual_channel.size() || gb_virtual_channel[id_virtual_channel] == NULL)
		return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

	if ((result = gb_virtual_channel[id_virtual_channel]->LOAD_PRFTime((float)1/data)) < 0) return RLOG(result);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadPRFBurst(float data, int id_virtual_channel)
{
	int result = ES2_NONE;

	if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
	if (id_virtual_channel < 0 || id_virtual_channel >= gb_virtual_channel.size() || gb_virtual_channel[id_virtual_channel] == NULL)
		return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

	if ((result = gb_virtual_channel[id_virtual_channel]->LOAD_PRFBurstTime((float)1 / data)) < 0) return RLOG(result);
	return result;
}
/* ----------------------------------------------------------------------
   USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_LoadPulser (int npulses, float wpulse, int auto_width)
{
int result;

	if ((result = ST_VCH_LOAD_Pulser(npulses, wpulse)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
Envía el mensaje de configuración de los registros de una BASE.

@return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_LoadGain(float data, int id_virtual_channel)
{
int result;

	if ((result = ST_VCH_LOAD_Gain(data, id_virtual_channel)) < 0)
      return RLOG(result);
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadSignalTest(int data)
{
int n_bases;

   if ((n_bases = ST_VCH_GET_BASENumber()) <= 0) return ELOG(charES2_BASE_Number, ES2_BASE_Number);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadDisabledChannel (int disabled, int channel)
{
int n_channel;

   if ((n_channel = ST_VCH_GET_ChannelsNumber()) < 0) return RLOG(n_channel);
	if (disabled!=0 && disabled!=1)
		return ELOG(charES2_RANGO,ES2_RANGO);

	if (channel<0 || channel>=n_channel)
		return ELOG(charES2_RANGO,ES2_RANGO);
	return ES2_NONE;
}





