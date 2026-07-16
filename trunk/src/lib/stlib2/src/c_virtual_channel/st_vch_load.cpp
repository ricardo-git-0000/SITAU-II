// -----------------------------------------------------------------------------
/**
 @file st_vch_load.cpp

 @brief Este archivo contiene la implementación de las funciones de conversión de datos de la curva TGC.

 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 11/29/10 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_vch_load.h"
#include "c_vch_load.h"
#include "c_virtual_channel.h"
#include "st_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_ReceiverMode (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_ReceiverMode(data)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_Pulser (int n_pulses, float pulse_width, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_Pulser(n_pulses, pulse_width)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_PRFEnabled (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_PRFEnabled(data)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_SamplingFrequency (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_SamplingFrequency (data)) < 0)
      return RLOG(result);
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_VCH_LOAD_MaterialVelocity (float data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_MaterialVelocity(data)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_AcqTime (float acq_start_time, float acq_end_time, int ch, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_AcqTime(acq_start_time, acq_end_time, ch)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_LOAD_Gain (float gain_db, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->LOAD_Gain(gain_db)) < 0) return RLOG(result);
   return result;
}

