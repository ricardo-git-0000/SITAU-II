// -----------------------------------------------------------------------------
/**
 @file st_vch_pa.cpp

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

#include "st_vch_get.h"
#include "c_vch_get.h"
#include "c_virtual_channel.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "stlib2.h"

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
int ST_VCH_GET_PulsesNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PulsesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_PulseWidth (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PulseWidth();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_AutoPulseWidth (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if (gb_virtual_channel[id_vch]->GET_AutoPulseWidth()) return 1;
   else return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_MaterialVelocity (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_MaterialVelocity();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_ChannelNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ChannelsNumber();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_MC_GetAScanDataNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_FP_AScanDataNumber();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_FP_ImageDataNumber (int id_focal_law, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_FP_ImageDataNumber(id_focal_law);
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AcqStartTime (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_AcqStartTime();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AcqEndTime (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_AcqEndTime();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AcqTime (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_AcqTime();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AcqStartDistance (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_AcqStartDistance();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AcqEndDistance (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_AcqEndDistance();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_PRFTime(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PRFTime();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_PRFEnabled(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PRFEnabled();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_SamplingFrequency (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_SamplingFrequency();
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_DownSamplingFactor (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_DownSamplingFactor();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_Attenuator (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_Attenuator();
}
// -----------------------------------------------------------------------------
/**
 Devuelve el canal virtual seleccinado previamente por la función \c ST_SetSelectedVirtualChannel()

 @return Canal virtual actualmente seleccionado.\n
         Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_VirtualChannelName (char *name, int id_vch)
{
int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if ((result = gb_virtual_channel[id_vch]->GET_Name(name)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_PulserEnabled (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PulserEnabled() ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_TimeResolution (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_TimeResolution();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_DistanceResolution (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_DistanceResolution();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_SignalTest (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_SignalTest();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_ActiveChannelsNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ActiveChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_MultiplexedChannelsNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_MultiplexedChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_ChannelsNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_MODULENumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_MODULENumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_BASENumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_BASENumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_AScanSamplesPositionTime_double (vector<double> &data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_AScanSamplesPositionTime_double(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_AScanSamplesPositionTime_float (vector<float> &data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_AScanSamplesPositionTime_float(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_AScanSamplesPositionDistance_float (vector<float> &data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_AScanSamplesPositionDistance_float(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_AScanSamplesPositionDistance_double (vector<double> &data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_AScanSamplesPositionDistance_double(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double ST_VCH_GET_AScanSamplePositionTime_double (int ind_sample, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
   {
      ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
      return 0;
   }
   return gb_virtual_channel[id_vch]->GET_AScanSamplePositionTime_double(ind_sample);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AScanSamplePositionTime_float (int ind_sample, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
   {
      ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
      return 0;
   }
   return gb_virtual_channel[id_vch]->GET_AScanSamplePositionTime_float(ind_sample);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double ST_VCH_GET_AScanSamplePositionDistance_double (int ind_sample, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
   {
      ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
      return 0;
   }
   return gb_virtual_channel[id_vch]->GET_AScanSamplePositionDistance_double(ind_sample);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_AScanSamplePositionDistance_float (int ind_sample, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
   {
      ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
      return 0;
   }
   return gb_virtual_channel[id_vch]->GET_AScanSamplePositionDistance_float(ind_sample);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_PASystemInfo ( int *pa_system_id,
                              int *pa_connector_id,
                              int *pa_pin_start,
                              int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if (pa_system_id != NULL) *pa_system_id = gb_virtual_channel[id_vch]->sys_.id_pa_sys;
   if (pa_connector_id != NULL) *pa_connector_id = gb_virtual_channel[id_vch]->sys_.id_connector;
   if (pa_pin_start != NULL) *pa_pin_start = 0;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_PA_Pitch (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_Probe_Pitch();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_PA_ElementNumber (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_Probe_ElementNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_PA_Frequency (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_Probe_Frequency();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_FP_FocalLawType (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_MC_FocalLawType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_GET_PWIC_StepAngle (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PWIC_StepAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_GET_PA_T0_Max (int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_T0_Max();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetHWRFFilterLow(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_HWRFFilterLow();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetHWRFFilterHigh(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_HWRFFilterHigh();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetHWRFFilterEnable(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   return gb_virtual_channel[id_vch]->GET_HWRFFilterEnable() ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProcessingType(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ProcessingType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProcessingParameter_AVR(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ProcessingParameter_AVR();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProcessingSignalsNumber_AVR(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ProcessingSignalsNumber_AVR();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProcessingSignalsNumber_EMI(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_ProcessingSignalsNumber_EMI();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PCIe_GetBlocksBurstNumber(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_PCIe_BlocksBurstNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PCIe_GetFocalLawsBlockNumber(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_PCIe_FocalLawsBlockNumber();
}




