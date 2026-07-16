// -----------------------------------------------------------------------------
/**
 @file c_vch_get.cpp

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

#include "c_vch_get.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "stlib2.h"
#include "stlib_dev.h"
//#include "c_hw.h"
#include "sitau_hw.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CVCH_Load class Methods
// _____________________________________________________________________________
// =============================================================================
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//CVCH_Load::CVCH_Load()
//{
//}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//CVCH_Load::~CVCH_Load()
//{
//}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_Name(char *name)
{
   strcpy(name, cfg_.name.c_str());
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_SignalTest(void)
{
   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AcquisitionTimeMode(void)
{
int tr_sel = 0;

   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if (cfg_.acq_time_mode == ACQ_PulseEcho && tr_sel > 0) 
       cfg_.acq_time_mode = ACQ_Transmission;
   return cfg_.acq_time_mode;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_Attenuator (void)
{
   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return 0;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
bool CVirtualChannel::GET_PulserEnabled(void)
{
   if (sys_.pulser == NULL) return (float)ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return sys_.pulser->reg_.enabled == 0 ? false : true;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
float CVirtualChannel::GET_PulseWidth(void)
{
   if (sys_.pulser == NULL) return (float)ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return sys_.pulser->reg_.pulse_width_ns;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_PulsesNumber(void)
{
   if (sys_.pulser == NULL) return (float)ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return sys_.pulser->reg_.n_pulses;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_AFEGain(int* attn, int* lna, int* pga)
{
    //if ((result = sys_.afe->LoadRegisterAFE_DIG_TGC_ATT_GAIN(data)) < 0) return RLOG(result);
    return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_SamplingFrequency (void)
{
unsigned short reg_value = 0;

   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return (float)FCLK / (float)(reg_value + 1);  
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_DownSamplingFactor (void)
{
unsigned short reg_value = 0;

   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   return reg_value + 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_DistanceResolution(void)
{
float result = (float)1.;

   if (cfg_.acq.ascan_samples_position_distance_float.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (cfg_.acq.ascan_samples_position_distance_float.size() > 0)
      result = cfg_.acq.ascan_samples_position_distance_float[1] - cfg_.acq.ascan_samples_position_distance_float[0];
   else {ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);}
   return result;
//float result = (float)1.;
//
//   if (ch >= cfg_.acq.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
//   if (cfg_.acq[ch].ascan_samples_position_distance_float.size() == 0)
//      UpdateAScanSamplesPosition(&cfg_.acq[ch]);
//   if (cfg_.acq[ch].ascan_samples_position_distance_float.size() > 0)
//      result = cfg_.acq[ch].ascan_samples_position_distance_float[1] - cfg_.acq[ch].ascan_samples_position_distance_float[0];
//   else {ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);}
//   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ActiveChannelsNumber(void)
{
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   return sys_.pa_sys->GetActiveChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MultiplexedChannelsNumber(void)
{
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   return sys_.pa_sys->GetMultiplexedChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ChannelsNumber(void)
{
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   return sys_.pa_sys->GetChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MODULENumber(void)
{
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   return sys_.pa_sys->GetMODULENumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_BASENumber(void)
{
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   return sys_.pa_sys->GetBASENumber();
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
float CVirtualChannel::GET_PRFTime(void)
{
float max_prf;

   if (cfg_.prf_enabled == 0 || cfg_.prf_time == 0)
   {
      max_prf = ST_RangeMaxPRF();
      if (max_prf > 0) return 1. / max_prf;
      else return 0;
   }
   return cfg_.prf_time;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
float CVirtualChannel::GET_PRFBurstTime(void)
{
    float max_prf;

    if (cfg_.prf_burst_enabled == 0 || cfg_.prf_burst_time == 0)
    {
        max_prf = ST_RangeMaxPRF();
        if (max_prf > 0) return 1. / max_prf;
        else return 0;
    }
    return cfg_.prf_burst_time;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_PRFEnabled(void)
{
   return cfg_.prf_enabled;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_PRFBurstEnabled(void)
{
    return cfg_.prf_burst_enabled;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AScanSamplesPositionTime_double (vector<double> &data)
{
   if (cfg_.acq.ascan_samples_position_time_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   data.assign (  cfg_.acq.ascan_samples_position_time_double.begin(),
                  cfg_.acq.ascan_samples_position_time_double.end());
   return EVCH_NONE;
   //if (ch >= cfg_.acq.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);     
   //if (cfg_.acq[ch].ascan_samples_position_time_double.size() == 0)
   //   UpdateAScanSamplesPosition(&cfg_.acq[ch]);
   //data.assign (  cfg_.acq[ch].ascan_samples_position_time_double.begin(),
   //               cfg_.acq[ch].ascan_samples_position_time_double.end());
   //return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AScanSamplesPositionTime_float (vector<float> &data)//, int ch)
{
   if (cfg_.acq.ascan_samples_position_time_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   data.assign (  cfg_.acq.ascan_samples_position_time_float.begin(),
                  cfg_.acq.ascan_samples_position_time_float.end());
   return EVCH_NONE;
   //if (ch >= cfg_.acq.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);     
   //if (cfg_.acq[ch].ascan_samples_position_time_double.size() == 0)
   //   UpdateAScanSamplesPosition(&cfg_.acq[ch]);
   //data.assign (  cfg_.acq[ch].ascan_samples_position_time_float.begin(),
   //               cfg_.acq[ch].ascan_samples_position_time_float.end());
   //return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AScanSamplesPositionDistance_double (vector<double> &data)
{
   if (cfg_.acq.ascan_samples_position_time_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   data.assign (  cfg_.acq.ascan_samples_position_distance_double.begin(),
                  cfg_.acq.ascan_samples_position_distance_double.end());
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AScanSamplesPositionDistance_float (vector<float> &data)
{
   if (cfg_.acq.ascan_samples_position_time_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   data.assign (  cfg_.acq.ascan_samples_position_distance_float.begin(),
                  cfg_.acq.ascan_samples_position_distance_float.end());
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double CVirtualChannel::GET_AScanSamplePositionTime_double (int ind_sample)
{
double result = 0;

   if (cfg_.acq.ascan_samples_position_time_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (ind_sample >= 0 && ind_sample < cfg_.acq.ascan_samples_position_time_double.size())
      result = cfg_.acq.ascan_samples_position_time_double[ind_sample];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AScanSamplePositionTime_float (int ind_sample)
{
float result = 0;

   if (cfg_.acq.ascan_samples_position_time_float.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (ind_sample >= 0 && ind_sample < cfg_.acq.ascan_samples_position_time_float.size())
      result = cfg_.acq.ascan_samples_position_time_float[ind_sample];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double CVirtualChannel::GET_AScanSamplePositionDistance_double (int ind_sample)
{
double result = 0;

   if (cfg_.acq.ascan_samples_position_distance_double.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (ind_sample >= 0 && ind_sample < cfg_.acq.ascan_samples_position_distance_double.size())
      result = cfg_.acq.ascan_samples_position_distance_double[ind_sample];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AScanSamplePositionDistance_float (int ind_sample)
{
float result = 0;

   if (cfg_.acq.ascan_samples_position_distance_float.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (ind_sample >= 0 && ind_sample < cfg_.acq.ascan_samples_position_distance_float.size())
      result = cfg_.acq.ascan_samples_position_distance_float[ind_sample];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_TimeResolution(void)
{
float result = (float)1.;

   if (cfg_.acq.ascan_samples_position_time_float.size() == 0)
      UpdateAScanSamplesPosition(&cfg_.acq);
   if (cfg_.acq.ascan_samples_position_time_float.size() > 0)
      result = cfg_.acq.ascan_samples_position_time_float[1] - cfg_.acq.ascan_samples_position_time_float[0];
   else {ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);}
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_AScanNumber (void)
{
int i, result = 0;

   for (i = 0; i < (int)cfg_.fp_focal_law.size(); i++) result += cfg_.fp_focal_law[i].n_ascan;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_FMCMatrix_Size(void)
{
    return GET_MC_SingleImage_Size() * cfg_.mc_emission_focal_law.size();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_SingleImage_Size(void)
{
    if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
    return sys_.pa_sys->GetChannelsNumber() * cfg_.acq.n_acq_samples;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_FocalLawNumber (void)
{
    int result;

    switch (cfg_.acquisition_type)
    {
    case ACQUISITION_TYPE_MC: result = (int)cfg_.mc_emission_focal_law.size(); break;
    case ACQUISITION_TYPE_PA: result = (int)cfg_.pa_emission_focal_law.size(); break;
    case ACQUISITION_TYPE_TFM: result = (int)cfg_.tfm_emission_focal_law.size(); break;
    default: result = ELOG(charEVCH_ACQUISITION_TYPE, EVCH_ACQUISITION_TYPE); break;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_FocalLaw_AScanNumber(int id_focal_law)
{
int result = 0;

if (id_focal_law < cfg_.fp_focal_law.size()) result = cfg_.fp_focal_law[id_focal_law].n_ascan;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_FP_AScanDataNumber (void)
{
   return cfg_.acq.n_acq_samples;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AScanDataNumber (void)
{
    if (cfg_.acquisition_type == ACQUISITION_TYPE_PA || cfg_.acquisition_type == ACQUISITION_TYPE_TFM) return cfg_.acq.n_beamformed_samples;
    else return cfg_.acq.n_acq_samples;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_FP_ImageDataNumber (int id_focal_law)
{
int result=0;

   if (id_focal_law < cfg_.fp_focal_law.size()) 
      result = cfg_.fp_focal_law[id_focal_law].image_size;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AcqStartSamples(void)
{
    return cfg_.acq.n_acq_start_samples;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AcqStartTime (void)
{
   return cfg_.acq.acq_start_time;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AcqEndTime (void)
{
   return cfg_.acq.acq_end_time;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AcqTime (void)
{
   return cfg_.acq.acq_end_time - cfg_.acq.acq_start_time;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AcqStartDistance (void)
{
   return cfg_.acq.acq_start_distance;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_AcqEndDistance (void)
{
   return cfg_.acq.acq_end_distance;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CVirtualChannel::GET_AcquisitionCounter(void)
{
unsigned long result = 0;

   if (buffer_mc_hwut_ != NULL) result = buffer_mc_hwut_->GetAcquisitionCounter();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_LastFocalLawAcquired(void)
{
unsigned long result = 0;

   if (buffer_mc_hwut_ != NULL) result = buffer_mc_hwut_->GetLastFocalLawAcquired();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AcqSamplesTime (float *samples_time, int size_data)
{
   int size, id_virtual_channel = 0;

   if ((size = (int)cfg_.acq.acq_samples_time_.size()) < 0) return RLOG(size);
   if (size == 0) return 0;
   if (size > size_data) size = size_data;
   memcpy(samples_time,&cfg_.acq.acq_samples_time_[0],size*sizeof(float));
	return size;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_BeamformedSamplesTime (float *samples_time, int size_data)
{
   int size, id_virtual_channel = 0;

   if ((size = (int)cfg_.acq.beamformed_samples_time_.size()) < 0) return RLOG(size);
   if (size == 0) return 0;
   if (size > size_data) size = size_data;
   memcpy(samples_time,&cfg_.acq.beamformed_samples_time_[0],size*sizeof(float));
	return size;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_AcqSamplesDistance (float *samples_distance, int size_data)
{
int size;

   if ((size = (int)cfg_.acq.acq_samples_distance_.size()) < 0) return RLOG(size);
   if (size == 0) return 0;
   if (size > size_data) size = size_data;
   memcpy(samples_distance,&cfg_.acq.acq_samples_distance_[0],size*sizeof(float));
	return size;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_BeamformedSamplesDistance (float *samples_distance, int size_data)
{
int size;

   if ((size = (int)cfg_.acq.beamformed_samples_distance_.size()) < 0) return RLOG(size);
   if (size == 0) return 0;
   if (size > size_data) size = size_data;
   memcpy(samples_distance,&cfg_.acq.beamformed_samples_distance_[0],size*sizeof(float));
	return size;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_ImageEntryPoint(float *data, int size_data)
{
int i=0, n_ch =0;
float probe_pitch;

   if ((probe_pitch = GET_Probe_Pitch()) < 0) return RLOG(probe_pitch);
   if ((n_ch = GET_ChannelsNumber()) < 0) return RLOG(probe_pitch);
   //if (id_focal_law < cfg_.fp_focal_law.size()) n_lines = cfg_.fp_focal_law[id_focal_law].n_ascan;
   if (probe_pitch > 0)
   {
      for (i=0; i<size_data && i< n_ch; i++) data[i] = i * probe_pitch;
   }
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_MC_ImageEntryPoint(vector<float> &data)
{
    int i = 0, n_ch = 0;
    float probe_pitch;

    if ((probe_pitch = GET_Probe_Pitch()) < 0) return RLOG(probe_pitch);
    if ((n_ch = GET_ChannelsNumber()) < 0) return RLOG(probe_pitch);
    data.resize(n_ch, 0);
    //if (id_focal_law < cfg_.fp_focal_law.size()) n_lines = cfg_.fp_focal_law[id_focal_law].n_ascan;
    if (probe_pitch > 0)
    {
        for (i = 0; i < n_ch; i++) data[i] = i * probe_pitch;
    }
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_ParallelBeamformedLines (void)
{
int n_beamformers, n_parallel_lines;

   n_parallel_lines = cfg_.scan != NULL ? cfg_.scan->GetParallelBeamformedLines() : 1;
   n_beamformers = STHW_GetBeamformersNumber();
   return n_parallel_lines == 1 ? 1 : n_parallel_lines / n_beamformers;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_TFM_ForwardStartElement(void)
{
    int result = 0;

    if (cfg_.tfm.emi_elements.size() > 0) result = cfg_.tfm.emi_elements.front();
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_TFM_ForwardEndElement(void)
{
    int result = 0;

    if (cfg_.tfm.emi_elements.size() > 0) result = cfg_.tfm.emi_elements.back();
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_TFM_ForwardStepElement(void)
{
    int result = 0;

    if (cfg_.tfm.emi_elements.size() > 1) return cfg_.tfm.emi_elements[1] - cfg_.tfm.emi_elements[0];
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PA_T0_Max(void) 
{
   if (sys_.base[0] != NULL) return sys_.base[0]->reg_.beamformer.t0_max;
   else return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_MaterialVelocity(void) 
{
   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL)
   else return cfg_.specimen->GetVelocity();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_Probe_Pitch(void) 
{
   if (cfg_.pa_probe == NULL) return ELOG(charEVCH_PA_NULL, EVCH_PA_NULL)
   else return cfg_.pa_probe->GetPitch();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_Probe_ElementNumber(void) 
{
   if (cfg_.pa_probe == NULL) return ELOG(charEVCH_PA_NULL, EVCH_PA_NULL)
   else return cfg_.pa_probe->GetElementNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::GET_Probe_Frequency(void) 
{
   if (cfg_.pa_probe == NULL) return ELOG(charEVCH_PA_NULL, EVCH_PA_NULL)
   else return cfg_.pa_probe->GetFrequency();
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_ProcessingType(void)
{
   return cfg_.processing_type;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_ProcessingSignalsNumber_EMI()
{
   return cfg_.n_signals_processing_emi;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_ProcessingSignalsNumber_AVR()
{
   return cfg_.n_signals_processing_avr;// ST_RegisterToProcessingSignalsNumber_AVR(cfg_.n_signals_processing_avr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_ReductionFactor(void)
{
   int result = 0;

   //if (sys_.base.size() > 0) result = sys_.base[0]->GetWMREDU();
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::GET_ProcessingParameter_AVR()
{
   return cfg_.parameter_processing_avr;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PinToChannel(unsigned short* pin_to_channel, int n_pin)
{
   int n_data = 0;

   if (cfg_.pa_probe != NULL)
   {
       n_data = cfg_.pa_probe->pin_to_channel_.size() > n_pin ? n_pin : cfg_.pa_probe->pin_to_channel_.size();
       if (n_data > 0) memcpy(pin_to_channel, &cfg_.pa_probe->pin_to_channel_[0], n_data * sizeof(unsigned short));
   }
   return n_data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GET_PinToChannelSize(void)
{
   if (cfg_.pa_probe != NULL) return (int)cfg_.pa_probe->pin_to_channel_.size();
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//vector<float> CVirtualChannel::GET_PWIC_Angles(void)
//{
//    return cfg_.fp_pwic_angles;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::GET_PWIC_AnglesNumber(void)
//{
//    return (int)cfg_.fp_pwic_angles.size();
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_GET_EmissionFocalLawFile(char* file)
{
    if (cfg_.fp_focal_law_file != "") strcpy(file, cfg_.fp_focal_law_file.c_str());
    else file = "";
    return EVCH_NONE;
}



















