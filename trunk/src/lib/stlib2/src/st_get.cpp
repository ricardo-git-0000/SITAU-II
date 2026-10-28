#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if !defined(_MSC_VER)
   #include <vcl.h>
   #include <dstring.h>
#endif
#include <stdio.h>
#include <math.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_get.h"
#include "stlib2.h"
#include "c_log_file.h"
//#include "stlib_dev.h"
//#include "st_calc.h"
//#include "st_info.h"
#include "st_reg.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "st_thread_acq.h"
#include "st_thread_scan.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"
//#include "c_acq_data.h"
//#include "c_buffer_mc_scan_hw_ut.h"
//#include "c_buffer_pa_scan_hw_ut.h"
//#include "c_buffer_scan.h"
#include "c_virtual_channel.h"
#include "st_vch_get.h"
//#include "st_vch_fl_pa.h"
#include "st_vch_tgc.h"
#include "sitau_hw.h"
#include "st_encoder.h"
#include "st_encoder_error_code.h"
#include "c_datalogger.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetSignalTest(void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_SignalTest(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetTimeResolution(void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_TimeResolution(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetDistanceResolution(void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_DistanceResolution(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetDistanciaEntreMuestras(void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_DistanceResolution(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetDigitalTGC_Attenuator(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_DigitalTGC_Attenuator(id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetLog2_Average(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_LOG2_AVERAGE(id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetLNAGain(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_LNA_GAIN(id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPGAGain(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PGA_GAIN(id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetTGCEnable(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_TGC_Enabled(id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetTGCAttenuationCoefficient(void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_TGC_AttenuationCoefficient(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetTGCConfigType (void)
{
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPulsesNumber (void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_PulsesNumber(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetPulseWidth (void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_PulseWidth(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPulseAmplitude (void)
{
   return stVAR.high_voltage;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_GetAcquisitionMode(void)
{
   return UCIP.acquisition_mode;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_GetStatusConfigPCIe(void)
{
   return UCIP.status_pcie;
} 
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPulserEnable (void)
{
int result, id_virtual_channel = 0;

	if ((result = ST_VCH_GET_PulserEnabled(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetGain (void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_Gain();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAFEGain(int* attn, int* lna, int* pga)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AFEGain(attn, lna, pga);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetAcqStartDistance (void) 
{
int ch = 0, id_virtual_channel = 0;

   return ST_VCH_GET_AcqStartDistance(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetAcqEndDistance (void) 
{
int ch = 0, id_virtual_channel = 0;

   return ST_VCH_GET_AcqEndDistance(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//float ST_GetAcqDistance (void) 
//{
//int ch = 0, id_virtual_channel = 0;
//
//   return ST_VCH_GET_AcqDistance(id_virtual_channel);
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetAcqEndTime (void) 
{
int ch = 0, id_virtual_channel = 0;

   return ST_VCH_GET_AcqEndTime(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetAcqTime (void) 
{
int ch = 0, id_virtual_channel = 0;

   return ST_VCH_GET_AcqTime(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetAcqStartTime (void) 
{
int ch = 0, id_virtual_channel = 0;

   return ST_VCH_GET_AcqStartTime(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**
 Devuelve el nÃºmero A-Scan que componen la imagen phased array
 configurada en el canal virtual seleccionado con la funciÃ³n \c ST_SetVirtualChannel().

 @return NÃºmero de seÃ±ales A-Scan que componen de una imagen phased array.
         Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_MC_GetAScanNumber(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_MC_AScanNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetFocalLawNumber(int id_vch)
{
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_FocalLawNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetTriggerFocalLawsNumber(void)
{
    int i, result = 0;

    for (i=0; i< gb_virtual_channel.size(); i++) result += gb_virtual_channel[i]->GET_FocalLawNumber();
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetReceptionFocalLawNumber(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_ReceptionFocalLawNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetEmissionFocalLawNumber(int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   return gb_virtual_channel[id_vch]->GET_PA_EmissionFocalLawNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetFocalLaw_AScanNumber (int id_focal_law)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_MC_FocalLaw_AScanNumber(id_focal_law);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetModulesNumber(void) // Expone la topologia fisica para validar la geometria PCIe.
{ // Usa el mismo canal virtual cero que ST_GetChannelsNumber.
   if (gb_virtual_channel.empty() || gb_virtual_channel[0] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel); // Rechaza un sistema sin canal configurado.
   return gb_virtual_channel[0]->GET_MODULENumber(); // Propaga el resultado de la configuracion hardware ya cargada.
} // No modifica la configuracion ni el transporte TCP.

int ST_GetChannelsNumber(void)
{
int result = 1;
int id_virtual_channel = 0;

	if ((result = ST_VCH_GET_ChannelNumber(id_virtual_channel)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetMatrixCaptureSize(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_MC_FMCMatrix_Size();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetSingleImageSize(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_MC_SingleImage_Size();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetMaterialVelocity (void)
{
float result;
int id_virtual_channel = 0;

	if ((result = ST_VCH_GET_MaterialVelocity(id_virtual_channel)) < 0) return (float)RLOG(result);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAScanDataNumber(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AScanDataNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetTriggerSource (void) {return UCIP.trigger_source;}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPauseAfterBurst(void) { return UCIP.pause_after_burst; }
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAutoContinueMode(void) { return UCIP.auto_continue_mode; }
// -----------------------------------------------------------------------------
/**
 Devuelve el nÃºmero de datos de cada imagen phased array configurada en el canal
 virtual seleccionado con la funciÃ³n \c ST_SetVirtualChannel().

 @return NÃºmero de datos de una imagen phased array.
         Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_GetFP_ImageDataNumber (void)
{
int result;
int id_virtual_channel = 0;

   if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
   if ((result = ST_VCH_GET_FP_ImageDataNumber(id_virtual_channel)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetSamplingFrequency (void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_SamplingFrequency(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetDownSamplingFactor (int id_virtual_channel)
{
int result;

   if ((result = ST_VCH_GET_DownSamplingFactor(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Acquiring (void)
{
int result = 0;

   if (gb_process_acq != NULL)
   {
      if (gb_process_acq_scan != NULL)
      {
         if (gb_process_acq->End() == 0 && gb_process_acq_scan->End() == 0)
            result = 1;
         else
         {
             if ((result = ST_Stop()) < 0) { RLOG(result); }
         }
      }
      else
      {
          if ((result = ST_Stop()) < 0) { RLOG(result); }
      }
   }
   else if (gb_process_acq_scan != NULL)
   {
       if ((result = ST_Stop()) < 0) { RLOG(result); }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_AcquiringError (void)
{
int result = 0;

   if (gb_process_acq != NULL)
   {
      if (gb_process_acq_scan == NULL)
      {
          if ((result = ST_Stop()) < 0) { RLOG(result); }
          return -1;
      }
      else if (gb_process_acq->End() == 1)
      {
          if (gb_buffer_hw == NULL ||
              (gb_buffer_hw->MC_BufferAcquisitionsNumber() <= 0 &&
              gb_buffer_hw->PA_BufferAcquisitionsNumber() <= 0 &&
              gb_buffer_hw->TFM_BufferAcquisitionsNumber() <= 0))
          {
              result = gb_process_acq->GetError();
              if ((result = ST_Stop()) < 0) { RLOG(result); }
          }
          return result;
      }
      else if (gb_process_acq_scan->End() == 1)
      {
         result = gb_process_acq_scan->GetError();
         if ((result = ST_Stop()) < 0) return RLOG(result);
         return result;
      }
   }
   else if (gb_process_acq_scan != NULL)
   {
      //ST_Stop();
      return -1;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetPRFTime (int id_virtual_channel)
{
float result;

   if ((result = ST_VCH_GET_PRFTime(id_virtual_channel)) < 0) return (float)RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetPRF (void)
{
float prf_time, result = 0;
int id_virtual_channel = 0;

   if ((prf_time = ST_VCH_GET_PRFTime(id_virtual_channel)) < 0) return (float)RLOG(prf_time);
   if (prf_time > 0) result = (float)1. / prf_time;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetPRFBurst(void)
{
    int id_vch = 0;
    float prf_time, result = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((prf_time = gb_virtual_channel[id_vch]->GET_PRFBurstTime()) < 0) return (float)RLOG(prf_time);
    if (prf_time > 0) result = (float)1. / prf_time;

    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAcquisitionsNumber (void)
{
    return UCIP.trigger_acq_number;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAcquisitionsBurstNumber(void)
{
    return UCIP.n_images_burst;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetBurstsNumber(void)
{
    return UCIP.n_bursts;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAcquisitionTimeMode(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AcquisitionTimeMode();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetDisabledChannel(int channel)
{
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAttenuator(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_GET_Attenuator(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPtrSamplesTimeImage (float *samples_time, int size_mem)
{
float inc_time;
int i, n_samples;

   if ((inc_time = ST_GetTimeResolution()) < 0) return RLOG(Round32(inc_time));
   if ((n_samples = ST_GetTimeResolution()) < 0) return RLOG(n_samples);
   if (size_mem < n_samples) return ELOG(charES2_RANGO, ES2_RANGO);
   if (n_samples == 0) return 0;
//   inc_time = ST_GetAcqTime() / n_samples;
   for (i=0; i<n_samples; i++) samples_time[i] = i * inc_time;
	return n_samples;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_GetPtrSamplesDistanceImage (float *samples_time, int size_mem)
//{
//float inc_distance = ST_GetDistanciaEntreMuestras();
//int i, n_samples = ST_GetAScanDataNumber();
//
//   if (size_mem < n_samples) return ELOG(charES2_RANGO, ES2_RANGO);
//   if (n_samples == 0) return 0;
//   for (i=0; i<n_samples; i++) samples_time[i] = i * inc_distance;
//	return n_samples;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_USER_GetPtrSamplesDistanceImage (vector<float> &samples_time)
//{
//int i, n_samples = ST_VCH_GET_USER_AScanDataNumber();
//float inc_distance = ST_VCH_GET_USER_DistanceResolution();
//
//   if (n_samples > 0)
//   {
//		samples_time.resize(n_samples);
//      for (i=0; i<n_samples; i++) samples_time[i] = i * inc_distance;
//   }
//   return n_samples;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_GetRAW_PtrSamplesDistanceImage (vector<float> &samples_time)
//{
//int i, n_samples = ST_VCH_GET_RAW_AScanDataNumber();
//float inc_distance = ST_GetDistanciaEntreMuestras(-1);
//
//   if (n_samples > 0)
//   {
//		samples_time.resize(n_samples);
//      for (i=0; i<n_samples; i++) samples_time[i] = i * inc_distance;
//   }
//   return n_samples;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEnabledConfigFileText(void)
{
   return STDL_GetEnabled() ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool ST_GetSelectLogData_HWCircularBuffer (void)
{
   if (gb_buffer_hw != NULL) return gb_buffer_hw->SelectLogData_HWCircularBuffer();
   return false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool ST_GetSelectLogData_AcqBufferCounter (void)
{
   if (gb_buffer_hw != NULL) return gb_buffer_hw->SelectLogData_AcqBufferCounter();
   return false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool ST_GetSelectLogData_AcqBufferSize (void)
{
   if (gb_buffer_hw != NULL) return gb_buffer_hw->SelectLogData_AcqBufferSize();
   return false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool ST_GetSelectLogData_AcqBufferNumber (void)
{
   if (gb_buffer_hw != NULL) return gb_buffer_hw->SelectLogData_AcqBufferNumber();
   return false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool ST_FP_GetSelectLogData_GetBufferCounter (void)
{
   if (gb_fp_buffer_acq != NULL) return gb_fp_buffer_acq->SelectLogData_GetBufferCounter();
   return false;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el canal virtual seleccinado previamente por la funciÃ³n \c ST_SetSelectedVirtualChannel()

 @return Canal virtual actualmente seleccionado.\n
         Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int ST_GetVirtualChannelName (char *name, int id_vch)
{
   return ST_VCH_GET_VirtualChannelName (name, id_vch);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetNoHardware (void)
{
   if (gb_no_hw) return 1;
   else return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanLinesTrigger (void)
{
    //int result;

    if (UCIP.trigger_source == TRG_SCAN_PRF || UCIP.trigger_source == TRG_SCAN_EXT)
    {
        if (UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS && UCIP.trigger_acq_number < MIN_ACQUISITIONS_MODE_CONTINOUS) return MIN_ACQUISITIONS_MODE_CONTINOUS;
        return UCIP.trigger_acq_number;
    }
    //else if (UCIP.trigger_source == TRG_SCAN_ENC)
    //{
    //    if (gb_encoder_trigger < 0 || gb_encoder_trigger >= gb_encoder.size() || gb_encoder[gb_encoder_trigger] == NULL) return ELOG(charEENC_IdEncoder, EENC_IdEncoder);
    //    if ((result = gb_encoder[gb_encoder_trigger]->GetScanLines()) < 0) return RLOG(result);
    //    return result;
    //}
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_GetScanLinesTrigger(unsigned long *mc_n_trg, unsigned long *pa_n_trg, unsigned long* pa_n_trg)
//{
//    int i, n_trg;
//    unsigned long mc=0, pa=0, tfm=0;
//
//    if ((n_trg = ST_GetScanLinesTrigger()) < 0) return RLOG(n_trg);
//    for (i = 0; i < gb_virtual_channel.size(); i++)
//    {
//        if (gb_virtual_channel[i] != NULL)
//        {
//            if (gb_virtual_channel[i]->cfg_.acquisition_type == ACQUISITION_TYPE_PA) pa = n_trg;
//            else if (gb_virtual_channel[i]->cfg_.acquisition_type == ACQUISITION_TYPE_MC) mc = n_trg;
//            else if (gb_virtual_channel[i]->cfg_.acquisition_type == ACQUISITION_TYPE_TFM) tfm = n_trg;
//        }
//    }
//    if (mc_n_trg != NULL) *mc_n_trg = mc;
//    if (pa_n_trg != NULL) *pa_n_trg = pa;
//    if (tfm_n_trg != NULL) *tfm_n_trg = tfm;
//    return ES2_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//unsigned long ST_GetScanFocalLawsNumber(void)
//{
//    int i, n_trg = 0, n_vch_fl = 0;
//
//    if ((n_trg = ST_GetScanLinesTrigger()) < 0) return RLOG(n_trg);
//    for (i = 0; i < gb_virtual_channel.size(); i++)
//    {
//        if (gb_virtual_channel[i] != NULL) n_vch_fl += gb_virtual_channel[i]->cfg_.emission_focal_law.size();
//    }
//    return n_trg * n_vch_fl;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanLinesAuxiliar (void)
{
    //int result;

    //if (UCIP.trigger_source == TRG_SCAN_ENC)
    //{
    //    if (gb_encoder_auxiliar < 0 || gb_encoder_auxiliar >= gb_encoder.size() || gb_encoder[gb_encoder_auxiliar] == NULL)
    //        return ELOG(charEENC_IdEncoder, EENC_IdEncoder);
    //    if (gb_auxiliar_scan_enabled)
    //    {
    //        if ((result = gb_encoder[gb_encoder_auxiliar]->GetScanLines()) < 0) return RLOG(result);
    //    }
    //    else result = 1;
    //    return result;
    //}
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAuxiliarEncoderEnabled(void)
{
    //if (gb_buffer_acq != NULL) return gb_buffer_acq->GetAuxiliarEncoderEnabled();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetProbePitch(void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PA_Pitch(id_virtual_channel)) < 0) return RLOG(Round32(result));
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProbeElementNumber(void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PA_ElementNumber(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetProbeFrequency(void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PA_Frequency(id_virtual_channel)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetFP_SamplesTime (float *samples_time, int size_data)
{
int id_vch = 0;//result

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AcqSamplesTime(samples_time, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetPA_SamplesTime (float *samples_time, int size_data)
{
    int id_vch = 0; //result

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_BeamformedSamplesTime(samples_time, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetFP_SamplesDistance (float *samples_distance, int size_data, int ch)
{
int id_vch = 0;//result

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AcqSamplesDistance(samples_distance, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetSamplesDistance(float *samples_distance, int size_data, int ch)
{
int id_vch = 0;//result

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_BeamformedSamplesDistance(samples_distance, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetImageEntryPoint(float * data, int size_data)
{
    int id_vch = 0;//result

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_MC_ImageEntryPoint(data, size_data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetFocalLawType(void)
{
int id_virtual_channel = 0;

   return ST_VCH_GET_FP_FocalLawType(id_virtual_channel);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetFocalLawType(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_FocalLawType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_MC_GetPlaneWaveImaging_StartAngle(void)
{
int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    return gb_virtual_channel[id_vch]->GET_PWIC_StartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_GetPlaneWaveImaging_StartAngle(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    return gb_virtual_channel[id_vch]->GET_TFM_PWIC_StartAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_MC_GetPlaneWaveImaging_StepAngle(void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PWIC_StepAngle(id_virtual_channel)) < 0) return RLOG(Round32(result));
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_GetPlaneWaveImaging_StepAngle(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_PWIC_StepAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_MC_GetPlaneWaveImaging_EndAngle (void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    return gb_virtual_channel[id_vch]->GET_PWIC_EndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_GetPlaneWaveImaging_EndAngle(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    return gb_virtual_channel[id_vch]->GET_TFM_PWIC_EndAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAcquisitionType(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_AcquisitionType();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetSignalMode(void)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_SignalMode();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetBeamformerConfig (int *n_parallel_lines)
{
//int result;
int id_virtual_channel = 0;

   //if (cfg_hw == NULL)  return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);   
   if (id_virtual_channel >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if (gb_virtual_channel[id_virtual_channel] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if (n_parallel_lines != NULL) *n_parallel_lines = gb_virtual_channel[id_virtual_channel]->cfg_.scan->GetParallelBeamformedLines();
   return STHW_GetBeamformersNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetPlaneWaveImaging_Angles_rad(float* data, int size_data)
{
    int i;
    int id_virtual_channel = 0;
    vector<float> angles;

    if (id_virtual_channel >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_virtual_channel] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    angles = gb_virtual_channel[id_virtual_channel]->GET_PWIC_Angles();
    for (i = 0; i < angles.size() && i < size_data; i++) data[i] = angles[i] * M_PI / 180;
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetPlaneWaveImaging_Angles_rad(float* data, int size_data)
{
    int i;
    int id_virtual_channel = 0;
    vector<float> angles;

    if (id_virtual_channel >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_virtual_channel] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    angles = gb_virtual_channel[id_virtual_channel]->GET_TFM_PWIC_Angles();
    for (i = 0; i < angles.size() && i < size_data; i++) data[i] = angles[i] * M_PI / 180;
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetPlaneWaveImaging_Angles_deg(float* data, int size_data)
{
    int i;
    int id_virtual_channel = 0;
    vector<float> angles;

    if (id_virtual_channel >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_virtual_channel] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    angles = gb_virtual_channel[id_virtual_channel]->GET_PWIC_Angles();
    for (i = 0; i < angles.size() && i < size_data; i++) data[i] = angles[i];
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetPlaneWaveImaging_Angles_deg(float* data, int size_data)
{
    int i;
    int id_virtual_channel = 0;
    vector<float> angles;

    if (id_virtual_channel >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_virtual_channel] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    angles = gb_virtual_channel[id_virtual_channel]->GET_TFM_PWIC_Angles();
    for (i = 0; i < angles.size() && i < size_data; i++) data[i] = angles[i];
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetPlaneWaveImaging_AnglesNumber(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_PWIC_AnglesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetPlaneWaveImaging_AnglesNumber(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_PWIC_AnglesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetT0Max (void)
{
float result;
int id_virtual_channel = 0;

   if ((result = ST_VCH_GET_PA_T0_Max(id_virtual_channel)) < 0) return RLOG(Round32(result));
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetParallelLinesBlock(void)
{
   return MOD_MAX_CH;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetParallelLinesBlock(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_ParallelBeamformedLines();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_GetEmissionStartElement(void)
//{
//    int id_vch = 0;
//
//    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    return gb_virtual_channel[id_vch]->GET_TFM_EmissionStartElement();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_GetEmissionEndElement(void)
//{
//    int id_vch = 0;
//
//    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    return gb_virtual_channel[id_vch]->GET_TFM_EmissionEndElement();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_GetEmissionStepElement(void)
//{
//    int id_vch = 0;
//
//    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    return gb_virtual_channel[id_vch]->GET_TFM_EmissionStepElement();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetBackwardLinesNumber(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_BackwardLinesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetAcqDistance(float *start_x, float *start_z, float* end_x, float* end_z)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (start_x != NULL) *start_x = gb_virtual_channel[id_vch]->GET_TFM_BackwardStartDistance();
    if (end_x != NULL) *end_x = gb_virtual_channel[id_vch]->GET_TFM_BackwardEndDistance();
    if (start_z != NULL) *start_z = gb_virtual_channel[id_vch]->GET_AcqStartDistance();
    if (end_z != NULL) *end_z = gb_virtual_channel[id_vch]->GET_AcqEndDistance();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_BackwardStartDistance(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_BackwardStartDistance();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_BackwardEndDistance(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_BackwardEndDistance();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_BackwardAngle(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_BackwardAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetAcqTime(float* start_x, float* start_z, float* end_x, float* end_z)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (start_x != NULL) *start_x = gb_virtual_channel[id_vch]->GET_TFM_BackwardStartTime_X();
    if (end_x != NULL) *end_x = gb_virtual_channel[id_vch]->GET_TFM_BackwardEndTime_X();
    if (start_z != NULL) *start_z = gb_virtual_channel[id_vch]->GET_AcqStartTime();
    if (end_z != NULL) *end_z = gb_virtual_channel[id_vch]->GET_AcqEndTime();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TFM_GetBackwardAngle(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size())
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    if (gb_virtual_channel[id_vch] == NULL)
    {
        ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
        return 0;
    }
    return gb_virtual_channel[id_vch]->GET_TFM_BackwardAngle();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetForwardStartElement(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_ForwardStartElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetForwardEndElement(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_ForwardEndElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetForwardStepElement(void)
{
    int id_vch = 0;

    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    return gb_virtual_channel[id_vch]->GET_TFM_ForwardStepElement();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_GetBackwardStartAperture(void)
//{
//    int id_vch = 0;
//
//    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    return gb_virtual_channel[id_vch]->GET_TFM_BackwardStartAperture();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_GetBackwardAperture(void)
//{
//    int id_vch = 0;
//
//    if (id_vch >= gb_virtual_channel.size()) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if (gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    return gb_virtual_channel[id_vch]->GET_TFM_BackwardAperture();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetParallelLinesBlock(void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetDelaySync(void)
{
   return UCIP.delay_sync_time;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProbePinToChannel(unsigned short* pin_to_channel, int n_pin, int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if ((result = gb_virtual_channel[id_vch]->GET_PinToChannel(pin_to_channel, n_pin)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetProbePinToChannelSize(int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if ((result = gb_virtual_channel[id_vch]->GET_PinToChannelSize()) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetPRFTrigger(void)
{
    return UCIP.trigger_prf_time > 0 ? (float)1 / (float)UCIP.trigger_prf_time : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_CentralFrequency(void)
{
    return gb_fus.central_frequency;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_RapportFocalAperture(void)
{
    return gb_fus.rapport_focal_aperture;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_ApodizationCoefficient(void)
{
    return gb_fus.apodization_coefficient;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_LateralPositionX(void)
{
    return gb_fus.lateral_position_x;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_PixelSizeX(void)
{
    return gb_fus.pixel_size_x;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FUS_PixelsX(void)
{
    return gb_fus.pixels_x;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_DepthZ(void)
{
    return gb_fus.depth_z;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FUS_PixelsZ(void)
{
    return gb_fus.pixels_z;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_FUS_PixelSizeZ(void)
{
    return gb_fus.pixel_size_z;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FUS_SinglePlane(void)
{
    return gb_fus.single_plane;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FUS_SamplesWavelength(void)
{
    return gb_fus.samples_wavelength;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FUS_InterpolationCoefficients(void)
{
    return gb_fus.interpolation_coefficients;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanCarriageReturn(void)
{
    int result = 0;

    //if (gb_buffer_acq != NULL) result = gb_buffer_acq->GetScanCarriageReturn() ? 1 : 0;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanStartLeft(void)
{
    int result = 0;

    //if (gb_buffer_acq != NULL) result = gb_buffer_acq->GetScanStartLeft() ? 1 : 0;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanStartUp(void)
{
    int result = 0;

    //if (gb_buffer_acq != NULL) result = gb_buffer_acq->GetScanStartUp() ? 1 : 0;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetHWFormatData(void)
{
    return gb_hw_format ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetMaxAcquisitionsNumber(void)
{    
    int id_vch = 0, n_samples, n_ascan;

    if ((n_samples = ST_MC_GetAScanDataNumber(id_vch)) < 0) return RLOG(n_samples);
    if ((n_ascan = ST_MC_GetAScanNumber()) < 0) return RLOG(n_ascan);
    if (n_samples > 0 && n_ascan > 0) return (unsigned long)(1024 * 1024 * 1000 * 4) / (2 * n_samples * n_ascan);
    else return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_CurrentIndexTrigger(void)
//{
//    if (gb_process_acq_scan != NULL) return (int)gb_process_acq_scan->CurrentIndexTrigger();   
//    return 0;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_CurrentIndexFocalLaw(void)
{
    int result = 0;

    if (gb_process_acq_scan != NULL)
    {
        switch (ST_GetAcquisitionType())
        {
        case ACQUISITION_TYPE_MC: result = gb_process_acq_scan->MC_CurrentIndexFocalLaw(); break;
        case ACQUISITION_TYPE_PA: result = gb_process_acq_scan->PA_CurrentIndexFocalLaw(); break;
        case ACQUISITION_TYPE_TFM: result = gb_process_acq_scan->TFM_CurrentIndexFocalLaw(); break;
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetEmissionFocalLawFile(char* file)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->FP_GET_EmissionFocalLawFile(file)) < 0) return RLOG(result);
    return result;
}





