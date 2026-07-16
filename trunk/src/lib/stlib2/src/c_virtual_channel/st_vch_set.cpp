// -----------------------------------------------------------------------------
/**
 @file st_vch_set.cpp

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

#include "st_vch_set.h"
#include "c_vch_set.h"
#include "c_virtual_channel.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "stlib2.h"
#include "c_buffer_acq.h"
#include "vch_error_code.h"

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
int ST_VCH_SET_ReceiverMode (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_ReceiverMode(data)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_Pulser (int n_pulses, float pulse_width, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_Pulser(n_pulses, pulse_width)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PulseWidth (float pulse_width, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PulseWidth(pulse_width)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PulsesNumber (int n_pulses, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PulsesNumber(n_pulses)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PRFEnabled (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PRFEnabled(data)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_SamplingFrequency (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_SamplingFrequency (data)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_Attenuator (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_Attenuator(data)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PRFTime (float data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PRFTime(data)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_AcqTime (float acq_start_time, float acq_end_time, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_AcqTime(acq_start_time, acq_end_time)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_SignalTest (int data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_SignalTest(data)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PulserEnabled (bool data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PulserEnabled(data)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_Name (char *data, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_Name(data)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PlaneWaveImagingCompounding_Angles (float *angle, int n_angles, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PWICFocalLaw_Angles(angle, n_angles)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PlaneWaveImagingCompounding (float start_angle, float step_angle, float end_angle, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PWICFocalLaw(start_angle, step_angle, end_angle)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_FulMatrixCapture (int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_FMCFocalLaw()) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_DELETE_FocalLaws (int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->DELETE_FocalLaws()) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_VCH_RESET_AcquisitionCounter (void)
{
int i;

   for (i=0; i<gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL)
         gb_virtual_channel[i]->RESET_AcquisitionCounter();
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SET_PAConfig (unsigned short n_element, float pitch, float frequency, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PAConfig (n_element, pitch, frequency)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetHWRFFilterEnable(int data, int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_RFFilterEnabled(data == 1)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetHWRFFilter(float low_frequency, float high_frequency, int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_RFFilter(low_frequency, high_frequency)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetProcessing(int processing_type, int n_signals, int wait_prf, int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_Processing(processing_type, n_signals, wait_prf)) < 0)
      return RLOG(result);
   return result;
}

