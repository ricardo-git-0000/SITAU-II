// -----------------------------------------------------------------------------
/**
 @file c_vch_load.cpp

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

#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
#endif
#include <math.h>
#if !defined(_MSC_VER)
   #include <vector.h>
#else
   #include <vector>
#endif

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_vch_load.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "c_pa_system.h"
#include "st_calc.h"
#include "st_load.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "t_base_tad.h"
#include "sitau_hw.h"
#include "foc_params.h"
#include "c_datalogger.h"
#include "fl_calc.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CVCH_Load class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_DefaultConfig (void)
{
int n_ch, result, aperture=8, end_element=32;
vector<T_VCH_EmitterFocalLaw> focal_law;

   n_ch = GET_ChannelsNumber();

   cfg_.acq.acq_start_distance = 0;
   cfg_.acq.acq_end_distance = 40;
   cfg_.acq.n_acq_start_samples = 0;
   cfg_.acq.n_acq_samples = 0;
   cfg_.acq.n_beamformed_samples = 0;
   cfg_.acq.acq_start_time = 0;
   cfg_.acq.acq_end_time = 0;
   cfg_.acq.increased_acq_samples = 200;
   cfg_.acq_time_mode = ACQ_PulseEcho;

   if ((result = LOAD_ReceiverMode(ACQ_PulseEcho, false)) < 0) {RLOG(result);}
   if ((result = LOAD_MaterialVelocity ((float)DCFG_VEL)) < 0) {RLOG(result);}

   if ((aperture = n_ch/4) > 32) aperture = 32;
   if ((end_element = n_ch - 1) > 127) end_element = 127;

   if ((result = LOAD_PRFTime (1/DCFG_PRF)) < 0) {RLOG(result);}
   if ((result = LOAD_Pulser(DCFG_PUL_N, DCFG_PUL_W)) < 0) {RLOG(result);}
   if ((result = LOAD_Gain (DCFG_GAIN)) < 0) { RLOG(result); }
   if ((result = LOAD_AcqDistance(DCFG_DINI, DCFG_DFIN, ACQ_PulseEcho, true)) < 0) { RLOG(result); }
   if ((result = LOAD_PA_AngularScanConfig(1, 32, 0, -45, 45)) < 0) { RLOG(result); }
   if ((result = LOAD_FMCFocalLaw()) < 0) { RLOG(result); }

   cfg_.tfm_backward.parallel_lines = 4;
   //vector<int> rec_elements, emi_elements;

   //emi_elements.push_back(0);
   //for (i = 0; i < 128; i++) rec_elements.push_back(i);
   //if ((result = TFM_LOAD_FMC_Forward(0, 127, 1)) < 0) return RLOG(result);
   if ((result = TFM_LOAD_Backward(0, 20, 32, 0)) < 0) return RLOG(result);
   cfg_.tfm.focal_laws_type = VCH_FP_FL_TYPE_FMC;
   if ((result = TFM_LOAD_FMC_Forward(0, 127, 1)) < 0) return RLOG(result);

   //if ((result = TFM_LOAD_FMC_Config(rec_elements, emi_elements, 0, 20, 32, 0)) < 0) { RLOG(result); }

   cfg_.acquisition_type = ACQUISITION_TYPE_MC;
   cfg_.signal_mode = 0;


   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_ReceiverMode (int data, bool update_range)
{
int result;
unsigned short reg_value;

    if (data < 0 || data > 2) return ELOG(charEVCH_Parameter,EVCH_Parameter);
    if (data == 0) reg_value = 0;
    else reg_value = 1;

    cfg_.acq_time_mode = data;
	if (update_range && (result = RELOAD_AcqDistance()) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_Pulser (int n_pulses, float pulse_width)
{
int result;

   if (sys_.pulser != NULL)
   {
      if ((result = sys_.pulser->LoadRegisterPULSER_N_PULSES(n_pulses)) < 0) return RLOG(result);
      if ((result = sys_.pulser->LoadValuePULSER_WIDTH(pulse_width)) < 0) return RLOG(result);
      update_register_ = update_register_ ? true : sys_.pulser->UpdateRegister();
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PulseWidth (float data)
{
int result;

   if (sys_.pulser != NULL)
   {
      if ((result = sys_.pulser->LoadValuePULSER_WIDTH(data)) < 0) return RLOG(result);
      update_register_ = update_register_ ? true : sys_.pulser->UpdateRegister();
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PulsesNumber (int data)
{
int result;

   if (sys_.pulser != NULL)
   {
      if ((result = sys_.pulser->LoadRegisterPULSER_N_PULSES(data)) < 0) return RLOG(result);
      update_register_ = update_register_ ? true : sys_.pulser->UpdateRegister();
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PRFEnabled(int data)
{
   if (data != cfg_.prf_enabled)
   {
      update_register_ = true;
      cfg_.prf_enabled = data;
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_SamplingFrequency (int data)
{
int result, probe_n_elements;
float material_velocity, probe_pitch;

   if ((probe_pitch = GET_Probe_Pitch()) < 0) return RLOG(probe_pitch);
   if ((probe_n_elements = GET_Probe_ElementNumber()) < 0) return RLOG(probe_n_elements);

   if ((material_velocity = GET_MaterialVelocity()) < 0) return RLOG(Round32(material_velocity));
   if (data < 1) data = 1;	
   cfg_.sampling_frequency = GET_SamplingFrequency();
   if ((result = RELOAD_AcqDistance()) < 0) return RLOG(result);
   cfg_.probe_aperture_samples = Round32((probe_n_elements * probe_pitch * (float)1000) / material_velocity) * cfg_.sampling_frequency;
   if ((result = RELOAD_AcqDistance()) < 0) return RLOG(result);
   return EVCH_NONE;   
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_Attenuator (unsigned short data)
{
	return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PRFTime(float data)
{
   if (data != cfg_.prf_time)
   {
      update_register_ = true;
      cfg_.prf_time = data;
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_PRFBurstTime(float data)
{
    if (data != cfg_.prf_burst_time)
    {
        update_register_ = true;
        cfg_.prf_burst_time = data;
    }
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_MaterialVelocity (float data)
{
int result, probe_n_elements;
float probe_pitch;

    if (data <= 0) return ELOG(charEVCH_Parameter, EVCH_Parameter);
   if ((probe_pitch = GET_Probe_Pitch()) < 0) return RLOG(probe_pitch);
   if ((probe_n_elements = GET_Probe_ElementNumber()) < 0) return RLOG(probe_n_elements);

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   if ((result = cfg_.specimen->SetVelocity(data)) < 0) return RLOG(result);

   cfg_.probe_aperture_samples = Round32((probe_n_elements * probe_pitch * (float)1000) / data) * cfg_.sampling_frequency;
   cfg_.tfm.start_x_time = cfg_.tfm.start_x_distance / data;
   cfg_.tfm.end_x_time = cfg_.tfm.end_x_distance / data;
   if ((result = RELOAD_AcqDistance()) < 0) return RLOG(result);
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::Load_AcqTime (float acq_start_time, float acq_end_time, bool update)
{
int acq_time_mode, result;
float factor = 1000.;
unsigned short n_acq_samples, n_beamformed_samples;
float material_velocity;
   
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity();

   if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if (update || acq_start_time != cfg_.acq.acq_start_time || acq_end_time != cfg_.acq.acq_end_time)
   {
      acq_time_mode = GET_AcquisitionTimeMode();
      if (acq_time_mode == ACQ_PulseEcho || acq_time_mode == ACQ_PitchCatch) factor = (float)2000.;

      // Calculo el número de muestras por cada línea
		n_beamformed_samples = Round32 ((acq_end_time - acq_start_time) * (float)FCLK);

      // La base siempre devuelve una cantidad par de datos
      if (n_beamformed_samples % 2 != 0) n_beamformed_samples++;
      if (cfg_.acquisition_type != ACQUISITION_TYPE_MC) n_acq_samples = n_beamformed_samples + cfg_.acq.increased_acq_samples;//cfg_.probe_aperture_samples;
      else n_acq_samples = n_beamformed_samples;
      if (n_acq_samples % 2 != 0) n_acq_samples++;

		//Comprobar que el número de muestras después del diezmado no es menor de 31
		//ni mayor de 20478
        if (n_acq_samples < MIN_NMU_OUT) return ELOG(charEVCH_NMU_Underflow, EVCH_NMU_Underflow);
	    if (n_acq_samples > MAX_NMU_OUT) return ELOG(charEVCH_NMU_Overflow, EVCH_NMU_Overflow);

      update_register_ = cfg_.acq.n_acq_samples != n_acq_samples;
      update_register_ = update_register_ ? true : cfg_.acq.n_beamformed_samples != n_beamformed_samples;

      cfg_.acq.acq_start_time = acq_start_time;
      cfg_.acq.acq_end_time = acq_end_time;
      cfg_.acq.n_acq_start_samples = Round32((float)cfg_.acq.acq_start_time * (float)FCLK);
      cfg_.acq.n_acq_samples = n_acq_samples;
      cfg_.acq.n_beamformed_samples = n_beamformed_samples;
      if (material_velocity <= 0) cfg_.acq.acq_start_distance = cfg_.acq.acq_start_time;
      else cfg_.acq.acq_start_distance = material_velocity * cfg_.acq.acq_start_time / factor;
      if (material_velocity <= 0) cfg_.acq.acq_end_distance = cfg_.acq.acq_end_time;
      else cfg_.acq.acq_end_distance = material_velocity * cfg_.acq.acq_end_time / factor;

      if (sys_.afe != NULL)
      {
         if ((result = sys_.afe->LoadRegisterBUSSAR_N_SAMPLES(cfg_.acq.n_acq_samples)) < 0) return RLOG(result);
         if ((result = sys_.afe->LoadRegisterBUSSAR_WATER_DELAY(cfg_.acq.n_acq_start_samples)) < 0) return RLOG(result);
         update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
      }

      UpdateAcquisitionSize();
      GetSamplesIncrement();
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_AcqSamples(int acq_start_samples, int n_acq_samples, bool update)
{
    int acq_time_mode, result;
    float factor = 1000.;
    unsigned short n_beamformed_samples;
    float material_velocity;

    if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();

    if (sys_.base.size() == 0 || sys_.base[0] == NULL) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
    if (update || acq_start_samples != cfg_.acq.n_acq_start_samples || n_acq_samples != cfg_.acq.n_acq_start_samples)
    {
        acq_time_mode = GET_AcquisitionTimeMode();
        if (acq_time_mode == ACQ_PulseEcho || acq_time_mode == ACQ_PitchCatch) factor = (float)2000.;

        // Calculo el número de muestras por cada línea
        n_beamformed_samples = n_acq_samples;

        // La base siempre devuelve una cantidad par de datos
        if (n_beamformed_samples % 2 != 0) n_beamformed_samples++;
        if (cfg_.acquisition_type != ACQUISITION_TYPE_MC) n_acq_samples = n_beamformed_samples + cfg_.acq.increased_acq_samples;
        else n_acq_samples = n_beamformed_samples;
        if (n_acq_samples % 2 != 0) n_acq_samples++;

        //Comprobar que el número de muestras después del diezmado no es menor de 31
        //ni mayor de 20478
        if (n_acq_samples < MIN_NMU_OUT) return ELOG(charEVCH_NMU_Underflow, EVCH_NMU_Underflow);
        if (n_acq_samples > MAX_NMU_OUT) return ELOG(charEVCH_NMU_Overflow, EVCH_NMU_Overflow);

        update_register_ = cfg_.acq.n_acq_samples != n_acq_samples;
        update_register_ = update_register_ ? true : cfg_.acq.n_beamformed_samples != n_beamformed_samples;
        cfg_.acq.n_acq_start_samples = acq_start_samples;
        cfg_.acq.n_acq_samples = n_acq_samples;
        cfg_.acq.n_beamformed_samples = n_beamformed_samples;

        cfg_.acq.acq_start_time = (float)cfg_.acq.n_acq_start_samples / (float)FCLK;
        cfg_.acq.acq_end_time = (float)(cfg_.acq.n_acq_start_samples + cfg_.acq.n_acq_samples) / (float)FCLK;
        if (material_velocity <= 0) cfg_.acq.acq_start_distance = cfg_.acq.acq_start_time;
        else cfg_.acq.acq_start_distance = material_velocity * cfg_.acq.acq_start_time / factor;
        if (material_velocity <= 0) cfg_.acq.acq_end_distance = cfg_.acq.acq_end_time;
        else cfg_.acq.acq_end_distance = material_velocity * cfg_.acq.acq_end_time / factor;

        if (sys_.afe != NULL)
        {
            if ((result = sys_.afe->LoadRegisterBUSSAR_N_SAMPLES(cfg_.acq.n_acq_samples)) < 0) return RLOG(result);
            if ((result = sys_.afe->LoadRegisterBUSSAR_WATER_DELAY(cfg_.acq.n_acq_start_samples)) < 0) return RLOG(result);
            update_register_ = update_register_ ? true : sys_.afe->UpdateRegister();
        }

        UpdateAcquisitionSize();
        GetSamplesIncrement();
    }
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_AcqTime (float acq_start_time, float acq_end_time, bool update)
{
int result, bak_result;
float bak_acq_start_time, bak_acq_end_time;
   
   bak_acq_start_time = cfg_.acq.acq_start_time;
   bak_acq_end_time = cfg_.acq.acq_end_time;

   if ((result = Load_AcqTime (acq_start_time, acq_end_time, update)) < 0)
   {
      if ((bak_result = Load_AcqTime (bak_acq_start_time, bak_acq_end_time, update)) < 0) {RLOG(bak_result);}
      return RLOG(result);
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::LOAD_AcqDistance (float acq_start_distance, float acq_end_distance, int pulse_echo, bool update)
{
    int result;// , acq_time_mode;
float factor = 1000.;
float acq_start_time, acq_end_time;
float material_velocity;

    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();
    if (material_velocity <= 0) ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    if (    update ||
            acq_start_distance != cfg_.acq.acq_start_distance ||
            acq_end_distance != cfg_.acq.acq_end_distance ||
            cfg_.acq_time_mode != pulse_echo ||
            cfg_.acq.n_acq_samples == 0 ||
            cfg_.acq.n_beamformed_samples == 0)
   {
      if (acq_end_distance - acq_start_distance > AcquisitionDistanceMax()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
      if (acq_end_distance - acq_start_distance < 0) return ELOG(charEVCH_Parameter, EVCH_Parameter);

      cfg_.acq_time_mode = pulse_echo;
      if (cfg_.acq_time_mode == ACQ_PulseEcho || cfg_.acq_time_mode == ACQ_PitchCatch) factor = 2000.;
      //acq_time_mode = GET_AcquisitionTimeMode();
      //if (acq_time_mode == ACQ_PulseEcho || acq_time_mode == ACQ_PitchCatch) factor = 2000.;
      acq_start_time = factor * acq_start_distance / material_velocity;
      acq_end_time = factor * acq_end_distance / material_velocity;

      if ((result = LOAD_AcqTime (acq_start_time, acq_end_time, update)) < 0) return RLOG(result);

      if (cfg_.scan != NULL) 
      {
         if ((result = cfg_.scan->LOAD_AcqDistance(acq_start_distance, acq_end_distance)) < 0)
            return RLOG(result);
      }
   }
   return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::RELOAD_AcqDistance(void)
{
int result;

    if ((result = LOAD_AcqDistance (cfg_.acq.acq_start_distance, cfg_.acq.acq_end_distance, cfg_.acq_time_mode, true)) < 0)
        return RLOG(result);
    if ((result = RELOAD_FocalLaw()) < 0) return RLOG(result);
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::RELOAD_FocalLaw(void)
{
   int result;

    switch (cfg_.acquisition_type)
    {
        case ACQUISITION_TYPE_MC:
            if (cfg_.fp_focal_laws_type == VCH_FP_FL_TYPE_USER && cfg_.fp_focal_law_file != "")
            {
                if ((result = FP_LOAD_EmissionFocalLawFile(cfg_.fp_focal_law_file.c_str())) < 0) return RLOG(result);
            }
            else if (cfg_.fp_focal_laws_type == VCH_FP_FL_TYPE_PWI)
            {
                if ((result = RELOAD_PWICFocalLaw()) < 0) return RLOG(result);
            }
            else
            {
                if ((result = LOAD_FMCFocalLaw()) < 0) return RLOG(result);
            }
            break;
        case ACQUISITION_TYPE_TFM:
            switch (cfg_.tfm.focal_laws_type)
            {
            case VCH_FP_FL_TYPE_FMC:
                if ((result = TFM_LOAD_Backward(cfg_.tfm.start_x_distance, cfg_.tfm.end_x_distance, cfg_.tfm.n_lines, cfg_.tfm.angle)) < 0) return RLOG(result);
                if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0) return RLOG(result);
                //if ((result = TFM_LOAD_FMC_Config(cfg_.tfm.rec_elements, cfg_.tfm.emi_elements, cfg_.tfm.start_x_distance, cfg_.tfm.end_x_distance, cfg_.tfm.n_lines, cfg_.tfm.angle)) < 0) return RLOG(result);
                break;
            case VCH_FP_FL_TYPE_PWI:
                if ((result = TFM_LOAD_Backward(cfg_.tfm.start_x_distance, cfg_.tfm.end_x_distance, cfg_.tfm.n_lines, cfg_.tfm.angle)) < 0) return RLOG(result);
                if ((result = TFM_LOAD_PWI_Forward(cfg_.tfm.pwic_start_angle, cfg_.tfm.pwic_end_angle, cfg_.tfm.pwic_step_angle)) < 0) return RLOG(result);
                break;
            }
            break;
        case ACQUISITION_TYPE_PA:
            if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
            if ((result = cfg_.scan->RELOAD_FocalLaw()) < 0) return RLOG(result);
            if ((result = LOAD_PA_FocalLaw()) < 0) return RLOG(result);
            break;
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**
LNA --> Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved
PGA --> Bit[13]: 0:24 dB; 1:30 dB
ATT --> Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1

      GAIN(dB)  ATT     LNA     PGA
      -6        -42     12      24
       0        -36     12      24
       6        -30     12      24
      12        -24     12      24
      18        -18     12      24
      24        -12     12      24
      30        -6      12      24
      36        0       12      24
      42        0       18      24
      48        0       24      24
      54        0       24      30

      GAIN(dB)  ATT     LNA     PGA
       0        -42     12      24
       6        -36     12      24
      12        -30     12      24
      18        -24     12      24
      24        -18     12      24
      30        -12     12      24
      36        -6      12      24
      42        0       12      24
      48        0       18      24
      54        0       24      24
      60        0       24      30

@return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_Gain (float gain_db)
{
unsigned short gain_hi = 0;
float gaincat=0;
int result;
bool load_gain_angular_sensitivity = true;
bool load_gain_aperture_factor = true;
bool load_gain_aperture_factor_div2 = true;
div_t d;
int pga, lna, att, pga_reg, lna_reg, att_reg;

    if (gain_db < 0) gain_db = 0;
    else if (gain_db > 60) gain_db = 60;
    else
    {
        d = div(gain_db, 6);
        gain_db = d.rem < 3 ? d.quot * 6 : (d.quot + 1) * 6;
    }

    if (gain_db < 60)
    {
        if (gain_db < 48)
        {
            lna = 12;
            if (gain_db < 6) { att = -42; cfg_.gain_db = 0; }
            else if (gain_db < 12) { att = -36; cfg_.gain_db = 6; }
            else if (gain_db < 18) { att = -30; cfg_.gain_db = 12; }
            else if (gain_db < 24) { att = -24; cfg_.gain_db = 18; }
            else if (gain_db < 30) { att = -18; cfg_.gain_db = 24; }
            else if (gain_db < 36) { att = -12; cfg_.gain_db = 30; }
            else if (gain_db < 42) { att = -6; cfg_.gain_db = 36; }
            else { att = 0; cfg_.gain_db = 42; }
        }
        else
        {
            att = 0;
            if (gain_db >= 54) { lna = 24; cfg_.gain_db = 54; }
            else { lna = 18; cfg_.gain_db = 48; }
        }
        pga = 24;
    }
    else
    {
        cfg_.gain_db = 60;
        att = 0;
        lna = 24;
        pga = 30;
    }
    cfg_.gain_db = gain_db;

    switch (lna)
    {
    case 18: lna_reg = 0; break;
    case 24: lna_reg = 1; break;
    case 12: lna_reg = 2; break;
    default: return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency); break;
    }
    switch (pga)
    {
    case 24: pga_reg = 0; break;
    case 30: pga_reg = 1; break;
    default: return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency); break;
    }
    switch (att)
    {
    case 0: att_reg = 0; break;
    case -6: att_reg = 1; break;
    case -12: att_reg = 2; break;
    case -18: att_reg = 3; break;
    case -24: att_reg = 4; break;
    case -30: att_reg = 5; break;
    case -36: att_reg = 6; break;
    case -42: att_reg = 7; break;
    default: return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency); break;
    }

    if ((result = LOAD_LNA_GAIN(lna_reg)) < 0) return RLOG(result);
    if ((result = LOAD_PGA_GAIN(pga_reg)) < 0) return RLOG(result);
    if ((result = LOAD_DigitalTGC_Attenuator(att_reg)) < 0) return RLOG(result);
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::RELOAD_Gain (void)
{
int result;

   if ((result = LOAD_Gain(cfg_.gain_db)) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_SignalTest (unsigned short data)
{
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PulserEnabled (bool data)
{
int result;
unsigned char value;

   value = data ? 1 : 0;   
   if (sys_.pulser != NULL)
   {
      if ((result = sys_.pulser->LoadRegisterPULSER_ENABLED(value)) < 0) return RLOG(result);
      update_register_ = update_register_ ? true : sys_.pulser->UpdateRegister();
   }
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::RELOAD_PWICFocalLaw(void)
{
int result;

   if (cfg_.fp_pwic_angles.size() > 0)
   {
      if ((result = LOAD_PWICFocalLaw_Angles(&cfg_.fp_pwic_angles[0], (int)cfg_.fp_pwic_angles.size())) < 0) return RLOG(result);
   }
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PWICFocalLaw_Angles (float *angle, int n_angles)
{
float p, v;
int result, i, c, n_ch = GET_ChannelsNumber();
float material_velocity, probe_pitch, emitter_delay;

    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();
    if ((probe_pitch = GET_Probe_Pitch()) < 0) return RLOG(probe_pitch);
    n_ch = GET_ChannelsNumber();

    if (angle == NULL) return ELOG(charEVCH_PtrAccess_NULL, EVCH_PtrAccess_NULL);  
    if (n_ch > 0)
    {
        if ((v = material_velocity / 1000) <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        if ((p = probe_pitch) <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        cfg_.mc_emission_focal_law.resize(n_angles);
        for (i=0; i<n_angles; i++)
        {
            cfg_.mc_emission_focal_law[i].resize(n_ch);
            for (c=0; c<n_ch; c++)
		    {
			    if (angle[i] < 0) emitter_delay = sinf(angle[i] * (float)M_PI / (float)180.) * p / v * (c + 1 - n_ch);
			    if (angle[i] >= 0) emitter_delay = sinf(angle[i] * (float)M_PI / (float)180.) * p / v * (c + 1 - 1);
                cfg_.mc_emission_focal_law[i][c].emitter_delay_ticks = emitter_delay * FCLK4_MHz;
                cfg_.mc_emission_focal_law[i][c].emitter_delay = emitter_delay;
                cfg_.mc_emission_focal_law[i][c].emitter_enabled = 1;
		    }
        }
        cfg_.fp_focal_laws_type = VCH_FP_FL_TYPE_PWI;
        cfg_.fp_pwic_angles.assign(angle, angle + n_angles);

        if (cfg_.fp_pwic_angles.size() > 1)
        {
            cfg_.fp_pwic_start_angle = cfg_.fp_pwic_angles.front();
            cfg_.fp_pwic_end_angle = cfg_.fp_pwic_angles.back();
            cfg_.fp_pwic_step_angle = cfg_.fp_pwic_angles[1] - cfg_.fp_pwic_angles[0];
        }
        else if (cfg_.fp_pwic_angles.size() == 1)
        {
            cfg_.fp_pwic_start_angle = cfg_.fp_pwic_angles.front();
            cfg_.fp_pwic_end_angle = cfg_.fp_pwic_start_angle;
            cfg_.fp_pwic_step_angle = 0;
        }
        else
        {
            cfg_.fp_pwic_start_angle = 0;
            cfg_.fp_pwic_end_angle = 0;
            cfg_.fp_pwic_step_angle = 0;
        }

        cfg_.fp_pwic_start_angle = cfg_.fp_pwic_angles.front();
        cfg_.fp_pwic_end_angle = cfg_.fp_pwic_angles.back();
        if (cfg_.fp_pwic_angles.size() > 1) cfg_.fp_pwic_step_angle = cfg_.fp_pwic_angles[1] - cfg_.fp_pwic_angles[0];
        else cfg_.fp_pwic_step_angle = 0;

        if ((result = FP_UpdateAcquisitionSize()) < 0) return RLOG(result);
        if (cfg_.acquisition_type == ACQUISITION_TYPE_MC)
        {
            if ((result = LOAD_EmissionFocalLaw(cfg_.mc_emission_focal_law)) < 0) return RLOG(result);
        }
        //if ((result = LOAD_RegistersFocalLaw()) < 0) return RLOG(result);
    }
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_LOAD_PWI_Forward_Angles(float* angle, int n_angles)
{
    int result, n_ch;
    float material_velocity, probe_pitch, first_x, first_z, last_x, last_z;
    vector<float> v_pix_mm;
    vector<float> v_piz_mm;
    vector<float> v_sin_ang;
    vector<float> v_cos_ang;
    float end_x = cfg_.tfm.end_x_distance;
    float start_x = cfg_.tfm.start_x_distance;
    float start_z = cfg_.acq.acq_start_distance;
    float end_z = cfg_.acq.acq_end_distance;
    float angle_rad = cfg_.tfm.angle * M_PI / 180;
    int n_lines = cfg_.tfm.n_lines;

    if (angle == NULL) return ELOG(charEVCH_PtrAccess_NULL, EVCH_PtrAccess_NULL);
    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    if (cfg_.pa_probe == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();
    n_ch = GET_ChannelsNumber();
    if ((probe_pitch = cfg_.pa_probe->GetPitch()) < 0) return RLOG(probe_pitch);
    if ((result = cfg_.pa_probe->FirstElementCoordinates(&first_x, &first_z)) < 0) return RLOG(probe_pitch);
    if ((result = cfg_.pa_probe->LastElementCoordinates(&last_x, &last_z)) < 0) return RLOG(probe_pitch);

    if (n_ch > 0)
    {
        // LEYES FOCALS EN EMISIÓN

        if ((result = FL_PWIEmission(cfg_.tfm_emission_focal_law, angle, n_angles, probe_pitch, material_velocity / 1000, n_ch)) < 0) return RLOG(result);

        cfg_.tfm.pwic_angles.assign(angle, angle + n_angles);

        if (cfg_.tfm.pwic_angles.size() > 1)
        {
            cfg_.tfm.pwic_start_angle = cfg_.tfm.pwic_angles.front();
            cfg_.tfm.pwic_end_angle = cfg_.tfm.pwic_angles.back();
            cfg_.tfm.pwic_step_angle = cfg_.tfm.pwic_angles[1] - cfg_.tfm.pwic_angles[0];
        }
        else if (cfg_.tfm.pwic_angles.size() == 1)
        {
            cfg_.tfm.pwic_start_angle = cfg_.tfm.pwic_angles.front();
            cfg_.tfm.pwic_end_angle = cfg_.tfm.pwic_start_angle;
            cfg_.tfm.pwic_step_angle = 0;
        }
        else
        {
            cfg_.tfm.pwic_start_angle = 0;
            cfg_.tfm.pwic_end_angle = 0;
            cfg_.tfm.pwic_step_angle = 0;
        }

        cfg_.tfm.pwic_start_angle = cfg_.tfm.pwic_angles.front();
        cfg_.tfm.pwic_end_angle = cfg_.tfm.pwic_angles.back();
        if (cfg_.tfm.pwic_angles.size() > 1) cfg_.tfm.pwic_step_angle = cfg_.tfm.pwic_angles[1] - cfg_.tfm.pwic_angles[0];
        else cfg_.tfm.pwic_step_angle = 0;

        if ((result = FP_UpdateAcquisitionSize()) < 0) return RLOG(result);
        //if (cfg_.acquisition_type == ACQUISITION_TYPE_MC)
        {
            if ((result = LOAD_EmissionFocalLaw(cfg_.tfm_emission_focal_law)) < 0) return RLOG(result);
        }

        cfg_.tfm.focal_laws_type = VCH_FP_FL_TYPE_PWI;

        if (n_lines > 0)
        {
            start_z = cfg_.acq.acq_start_distance;
            end_z = cfg_.acq.acq_end_distance;

            for (float resolution = (end_x - start_x) / n_lines, i = 0; i < n_lines; i++) v_pix_mm.push_back(start_x + resolution * i);
            for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_piz_mm.push_back(start_z);

            if ((result = tfm_pwi_forward_parameters(first_x, first_z, last_x, last_z, cfg_.tfm.pwic_angles, 0, v_pix_mm, v_piz_mm, angle_rad, material_velocity / 1000, GET_SamplingFrequency(), sys_.tfm_pwi_forward.forwardBeamformer, sys_.tfm_pwi_forward.anglesBeamformer)) < 0)
                return RLOG(result);

            if ((result = TFM_PWI_Forward_HWMemory()) < 0) return RLOG(result);
            update_tfm_backward_ = true;
        }
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PWICFocalLaw (float start_angle, float step_angle, float end_angle)
{
float angle;
int result;

    cfg_.fp_pwic_angles.clear();
    if (step_angle == 0) cfg_.fp_pwic_angles.push_back(start_angle);
    else
    {
        if (start_angle + step_angle >= end_angle) end_angle = start_angle + step_angle;
        for (angle = start_angle; angle <= end_angle; angle += step_angle) cfg_.fp_pwic_angles.push_back(angle);
    }

   if ((result = LOAD_PWICFocalLaw_Angles(&cfg_.fp_pwic_angles[0], cfg_.fp_pwic_angles.size())) < 0)
      return RLOG(result);

   cfg_.fp_pwic_start_angle = start_angle;
   cfg_.fp_pwic_step_angle = step_angle;
   cfg_.fp_pwic_end_angle = end_angle;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_LOAD_PWI_Forward(float start_angle, float step_angle, float end_angle)
{
    float angle;
    int result;

    cfg_.tfm.pwic_angles.clear();
    if (step_angle == 0) cfg_.tfm.pwic_angles.push_back(start_angle);
    else
    {
        if (start_angle + step_angle >= end_angle) end_angle = start_angle + step_angle;
        for (angle = start_angle; angle <= end_angle; angle += step_angle) cfg_.tfm.pwic_angles.push_back(angle);
    }

    if ((result = TFM_LOAD_PWI_Forward_Angles(&cfg_.tfm.pwic_angles[0], cfg_.tfm.pwic_angles.size())) < 0)
        return RLOG(result);

    cfg_.tfm.pwic_start_angle = start_angle;
    cfg_.tfm.pwic_step_angle = step_angle;
    cfg_.tfm.pwic_end_angle = end_angle;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_FMCFocalLaw(void)
{
int c, l, result;
int n_ch = GET_ChannelsNumber();

   if (n_ch > 0)
   {
      cfg_.fp_focal_law.clear();
      cfg_.mc_emission_focal_law.resize(n_ch);
      for (l=0; l < n_ch; l++)
      {
         cfg_.mc_emission_focal_law[l].resize(n_ch);
         for (c=0; c < n_ch; c++)
         {
            if (c == l) cfg_.mc_emission_focal_law[l][c].emitter_enabled = 1;
            else cfg_.mc_emission_focal_law[l][c].emitter_enabled = 0;
            cfg_.mc_emission_focal_law[l][c].emitter_delay = 0;
            cfg_.mc_emission_focal_law[l][c].emitter_delay_ticks = 0;
         }
      }
      cfg_.fp_focal_laws_type = VCH_FP_FL_TYPE_FMC;

      if ((result = FP_UpdateAcquisitionSize()) < 0) return RLOG(result);
      if (cfg_.acquisition_type == ACQUISITION_TYPE_MC)
      {
          if ((result = LOAD_EmissionFocalLaw(cfg_.mc_emission_focal_law)) < 0) return RLOG(result);
      }

      //if ((result = LOAD_RegistersFocalLaw()) < 0) return RLOG(result);
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_Beamformer(void)
{
   int l, c_fl, c_sitau, id_base, c_base, n_fl, n_bases, n_ch_sitau;
   init_word_t init_word;
   bool last, reset;
   vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
   vector<unsigned short> start_element;
   TFP_VCH_BASE_BeamformerParameters beamformer_default = { 0, 0, 0, 0, 0, 1 };
   vector<T_VCH_EmitterFocalLaw> efl;
   bool pin_to_channel;

#if !defined(_PA)
   return EVCH_NONE;
#endif

   if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
   if ((n_ch_sitau = GET_ChannelsNumber()) < 0) return RLOG(n_ch_sitau);
   if ((n_bases = GET_BASENumber()) < 0) return RLOG(n_bases);
   if ((n_fl = cfg_.pa_focal_law.beamformer.size()) == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (n_ch_sitau == 0 || n_bases == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

   //if ((result = STHW_GetPinToChannel(sys_.id_pa_sys, sys_.id_connector, pin_to_channel)) < 0) return RLOG(result);
   if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
   else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;

   for (id_base = 0; id_base < sys_.base.size(); id_base++)
   {
      sys_.base[id_base]->reg_.beamformer.n_fl = n_fl;
      //sys_.base[id_base]->reg_.beamformer.n_fl_hw = n_fl;
      sys_.base[id_base]->reg_.beamformer.n_fl_parallel = cfg_.scan->GetParallelBeamformedLines();
      sys_.base[id_base]->reg_.beamformer.bf_parameters.resize(n_fl);
      for (l = 0; l < n_fl; l++) sys_.base[id_base]->reg_.beamformer.bf_parameters[l].resize(MOD_MAX_CH);
      sys_.base[id_base]->reg_.beamformer.t0_max = 0;
      sys_.base[id_base]->reg_.beamformer.n_lword = 0;
      sys_.base[id_base]->reg_.beamformer.shift_bits = 3;
   }

   // Rellena los parámetros del beamformer por cada base teniendo en cuenta la asignación de pines
   for (l = 0; l < n_fl; l++)
   {
      for (c_fl = 0; c_fl < n_ch_sitau; c_fl++)
      {
         if (pin_to_channel && c_fl < cfg_.pa_probe->pin_to_channel_.size()) c_sitau = cfg_.pa_probe->pin_to_channel_[c_fl];
         else c_sitau = c_fl;
         if (c_fl >= cfg_.pa_focal_law.beamformer[l].size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
         if ((id_base = c_sitau / MOD_MAX_CH) >= sys_.base.size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
         if ((c_base = c_sitau - id_base * MOD_MAX_CH) >= MOD_MAX_CH) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
         sys_.base[id_base]->reg_.beamformer.bf_parameters[l][c_base] = cfg_.pa_focal_law.beamformer[l][c_fl];
      }
   }

    // Genera la memoria de parámetros por cada base
    for (id_base = 0; id_base < sys_.base.size(); id_base++)
    {
        for (l = 0; l < n_fl; l++)
        {
            for (c_base = 0; c_base < MOD_MAX_CH; c_base++)
            {                
                unsigned long t0_max_new = (sys_.base[id_base]->reg_.beamformer.bf_parameters[l][c_base].T_0 - sys_.base[id_base]->reg_.beamformer.bf_parameters[l][c_base].Z_0 * 4);
                if (sys_.base[id_base]->reg_.beamformer.t0_max < t0_max_new) sys_.base[id_base]->reg_.beamformer.t0_max = t0_max_new;
                copy_foc_line_to_init_word(&init_word, &sys_.base[id_base]->reg_.beamformer.bf_parameters[l][c_base]);
                last = (c_base + 1) % C_N_PARALLEL_INIT ? false : true;
                reset = c_base == 0;
                sys_.base[id_base]->reg_.beamformer.n_lword += copy_init_word_to_buffer(&sys_.base[id_base]->reg_.beamformer.bf_memory[0], init_word, sys_.base[id_base]->reg_.beamformer.n_lword, last, reset);
            }
        }
    }

    for (id_base = 0; id_base < sys_.base.size(); id_base++) sys_.base[id_base]->reg_.beamformer.t0_max = 2 + sys_.base[id_base]->reg_.beamformer.t0_max / 4;
    update_focal_law_pa_ = true;

    if (STDL_GetPABeamformer()) PRINT_Beamformer();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::TFM_HWMemory(void)
//{
//    int l, c_fl, c_sitau, id_base, c_base, n_fl, n_bases, n_ch_sitau;
//    init_word_t init_word;
//    bool last, reset;
//    vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
//    vector<unsigned short> start_element;
//    TFP_VCH_BASE_BeamformerParameters beamformer_default = { 0, 0, 0, 0, 0, 1 };
//    vector<T_VCH_EmitterFocalLaw> efl;
//    bool pin_to_channel;
//    long t0_max_new;
//
//    if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
//    if ((n_ch_sitau = GET_ChannelsNumber()) < 0) return RLOG(n_ch_sitau);
//    if ((n_bases = GET_BASENumber()) < 0) return RLOG(n_bases);
//    if ((n_fl = cfg_.tfm_backward.beamformer.size()) == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//    if (n_ch_sitau == 0 || n_bases == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//
//    if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
//    else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;
//    for (id_base = 0; id_base < sys_.base.size(); id_base++)
//    {
//        sys_.base[id_base]->reg_.tfm_backward.n_fl = n_fl;
//        sys_.base[id_base]->reg_.tfm_backward.n_fl_parallel = cfg_.tfm_backward.parallel_lines;
//        sys_.base[id_base]->reg_.tfm_backward.bf_parameters.resize(n_fl);
//        for (l = 0; l < n_fl; l++) sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l].resize(MOD_MAX_CH);
//        sys_.base[id_base]->reg_.tfm_backward.t0_max = 0;
//        sys_.base[id_base]->reg_.tfm_backward.n_lword = 0;
//        sys_.base[id_base]->reg_.tfm_backward.shift_bits = 3;
//    }
//
//    // Rellena los parámetros del beamformer por cada base teniendo en cuenta la asignación de pines
//    for (l = 0; l < n_fl; l++)
//    {
//        for (c_fl = 0; c_fl < cfg_.tfm_backward.beamformer[l].size() && c_fl < n_ch_sitau; c_fl++)
//        {            
//            if (pin_to_channel && c_fl < cfg_.pa_probe->pin_to_channel_.size()) c_sitau = cfg_.pa_probe->pin_to_channel_[c_fl];
//            else c_sitau = c_fl;
//
//            if (c_fl >= cfg_.tfm_backward.beamformer[l].size())
//                return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//
//            if ((id_base = c_sitau / MOD_MAX_CH) >= sys_.base.size())
//                return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//
//            if ((c_base = c_sitau - id_base * MOD_MAX_CH) >= MOD_MAX_CH)
//                return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//
//            sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base] = cfg_.tfm_backward.beamformer[l][c_fl];
//        }
//    }
//
//    // -- Tiempos de Vuelta
//    // Genera la memoria de parámetros por cada base
//    for (id_base = 0; id_base < sys_.base.size(); id_base++)
//    {
//        for (l = 0; l < n_fl; l++)
//        {
//            for (c_base = 0; c_base < MOD_MAX_CH; c_base++)
//            {
//
//                t0_max_new = (sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base].T_0 - sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base].Z_0 * 4);
//                if (t0_max_new < 0) t0_max_new = 0;
//                if (sys_.base[id_base]->reg_.tfm_backward.t0_max < t0_max_new) sys_.base[id_base]->reg_.tfm_backward.t0_max = t0_max_new;
//                copy_foc_line_to_init_word(&init_word, &sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base]);
//                last = (c_base + 1) % C_N_PARALLEL_INIT ? false : true;
//                reset = c_base == 0;
//                sys_.base[id_base]->reg_.tfm_backward.n_lword += copy_init_word_to_buffer(&sys_.base[id_base]->reg_.tfm_backward.bf_memory[0], init_word, sys_.base[id_base]->reg_.tfm_backward.n_lword, last, reset);
//            }
//        }
//    }
//
//    // -- Tiempos de ida
//    // Genera la memoria de parámetros por cada base
//    reset = 1;
//    last = 1;
//    sys_.tfm_forward.n_lword = 0;
//    for (c_base = 0; c_base < sys_.tfm_forward.beamformer.size(); c_base++)
//    {
//        for (l = 0; l < sys_.tfm_forward.beamformer[c_base].size(); l++)
//        {
//            //unsigned long t0_max_new = (sys_.tfm_forward.beamformer[l][c_base].T_0 - sys_.tfm_forward.beamformer[l][c_base].Z_0 * 4);
//            //if (sys_.base[id_base]->reg_.tfm_backward.t0_max < t0_max_new) sys_.tfm_forward..parameters.t0_max = t0_max_new;
//            copy_foc_line_to_init_word(&init_word, &sys_.tfm_forward.beamformer[c_base][l]);
//            sys_.tfm_forward.n_lword += copy_init_word_to_buffer_tfm_forward(&sys_.tfm_forward.memory[0], init_word, sys_.tfm_forward.n_lword);
//            //sys_.tfm_forward.n_lword += copy_init_word_to_buffer(&sys_.tfm_forward.memory[0], init_word, sys_.tfm_forward.n_lword, last, reset);
//        }
//    }
//    //for (id_base = 0; id_base < sys_.base.size(); id_base++)
//    //    sys_.base[id_base]->reg_.tfm_backward.t0_max = 2 + sys_.base[id_base]->reg_.tfm_backward.t0_max / 4;
//    update_tfm_forward_ = true;
//    update_tfm_backward_ = true;
//
//    if (STDL_GetTFMBeamformer())
//    {
//        PRINT_TFM_Backward();
//        PRINT_TFM_Forward();
//    }
//    return EVCH_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_BackwardHWMemory(void)
{
    int l, c_fl, c_sitau, id_base, c_base, n_fl, n_bases, n_ch_sitau;
    init_word_t init_word;
    bool last, reset;
    vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
    vector<unsigned short> start_element;
    TFP_VCH_BASE_BeamformerParameters beamformer_default = { 0, 0, 0, 0, 0, 1 };
    vector<T_VCH_EmitterFocalLaw> efl;
    bool pin_to_channel;
    long t0_max_new;

    if (cfg_.scan == NULL) return ELOG(charEVCH_SCAN_NULL, EVCH_SCAN_NULL);
    if ((n_ch_sitau = GET_ChannelsNumber()) < 0) return RLOG(n_ch_sitau);
    if ((n_bases = GET_BASENumber()) < 0) return RLOG(n_bases);
    if ((n_fl = cfg_.tfm_backward.beamformer.size()) == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    if (n_ch_sitau == 0 || n_bases == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

    if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
    else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;
    for (id_base = 0; id_base < sys_.base.size(); id_base++)
    {
        sys_.base[id_base]->reg_.tfm_backward.n_fl = n_fl;
        sys_.base[id_base]->reg_.tfm_backward.n_fl_parallel = cfg_.tfm_backward.parallel_lines;
        sys_.base[id_base]->reg_.tfm_backward.bf_parameters.resize(n_fl);
        for (l = 0; l < n_fl; l++) sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l].resize(MOD_MAX_CH);
        sys_.base[id_base]->reg_.tfm_backward.t0_max = 0;
        sys_.base[id_base]->reg_.tfm_backward.n_lword = 0;
        sys_.base[id_base]->reg_.tfm_backward.shift_bits = 3;
    }

    // Rellena los parámetros del beamformer por cada base teniendo en cuenta la asignación de pines
    for (l = 0; l < n_fl; l++)
    {
        for (c_fl = 0; c_fl < n_ch_sitau && c_fl < cfg_.tfm_backward.beamformer[l].size(); c_fl++)
        {
            if (pin_to_channel && c_fl < cfg_.pa_probe->pin_to_channel_.size()) c_sitau = cfg_.pa_probe->pin_to_channel_[c_fl];
            else c_sitau = c_fl;

            //if (c_fl >= cfg_.tfm_backward.beamformer[l].size())
            //    return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

            if ((id_base = c_sitau / MOD_MAX_CH) >= sys_.base.size())
                return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

            if ((c_base = c_sitau - id_base * MOD_MAX_CH) >= MOD_MAX_CH)
                return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

            sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base] = cfg_.tfm_backward.beamformer[l][c_fl];
        }
    }

    // -- Tiempos de Vuelta
    // Genera la memoria de parámetros por cada base
    for (id_base = 0; id_base < sys_.base.size(); id_base++)
    {
        for (l = 0; l < n_fl; l++)
        {
            for (c_base = 0; c_base < MOD_MAX_CH; c_base++)
            {

                t0_max_new = (sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base].T_0 - sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base].Z_0 * 4);
                if (t0_max_new < 0) t0_max_new = 0;
                if (sys_.base[id_base]->reg_.tfm_backward.t0_max < t0_max_new) sys_.base[id_base]->reg_.tfm_backward.t0_max = t0_max_new;
                copy_foc_line_to_init_word(&init_word, &sys_.base[id_base]->reg_.tfm_backward.bf_parameters[l][c_base]);
                last = (c_base + 1) % C_N_PARALLEL_INIT ? false : true;
                reset = c_base == 0;
                sys_.base[id_base]->reg_.tfm_backward.n_lword += copy_init_word_to_buffer(&sys_.base[id_base]->reg_.tfm_backward.bf_memory[0], init_word, sys_.base[id_base]->reg_.tfm_backward.n_lword, last, reset);
            }
        }
    }
    update_tfm_backward_ = true;

    if (STDL_GetTFMBeamformer()) PRINT_TFM_Backward();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_FMC_Forward_HWMemory(void)
{
    int l, c_base, n_fl, n_bases, n_ch_sitau;
    init_word_t init_word;
    bool last, reset;
    vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
    vector<unsigned short> start_element;
    TFP_VCH_BASE_BeamformerParameters beamformer_default = { 0, 0, 0, 0, 0, 1 };
    vector<T_VCH_EmitterFocalLaw> efl;
    bool pin_to_channel;

    if ((n_ch_sitau = GET_ChannelsNumber()) < 0) return RLOG(n_ch_sitau);
    if ((n_bases = GET_BASENumber()) < 0) return RLOG(n_bases);
    if ((n_fl = cfg_.tfm_backward.beamformer.size()) == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    if (n_ch_sitau == 0 || n_bases == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

    if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
    else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;

    // -- Tiempos de ida
    // Genera la memoria de parámetros por cada base
    reset = 1;
    last = 1;
    sys_.tfm_fmc_forward.n_lword = 0;
    for (c_base = 0; c_base < sys_.tfm_fmc_forward.beamformer.size(); c_base++)
    {
        for (l = 0; l < sys_.tfm_fmc_forward.beamformer[c_base].size(); l++)
        {
            copy_foc_line_to_init_word(&init_word, &sys_.tfm_fmc_forward.beamformer[c_base][l]);
            sys_.tfm_fmc_forward.n_lword += copy_init_word_to_buffer_tfm_forward(&sys_.tfm_fmc_forward.memory[0], init_word, sys_.tfm_fmc_forward.n_lword);
        }
    }
    update_tfm_forward_ = true;
    if (STDL_GetTFMBeamformer()) PRINT_TFM_FMC_Forward();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_PWI_Forward_HWMemory(void)
{
    int l, c_base, n_fl, n_bases, n_ch_sitau;
    unsigned long init_word;
    bool last, reset;
    vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
    vector<unsigned short> start_element;
    TFP_VCH_BASE_BeamformerParameters beamformer_default = { 0, 0, 0, 0, 0, 1 };
    vector<T_VCH_EmitterFocalLaw> efl;
    bool pin_to_channel;

    if ((n_ch_sitau = GET_ChannelsNumber()) < 0) return RLOG(n_ch_sitau);
    if ((n_bases = GET_BASENumber()) < 0) return RLOG(n_bases);
    if ((n_fl = cfg_.tfm_backward.beamformer.size()) == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    if (n_ch_sitau == 0 || n_bases == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

    if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
    else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;

    // -- Tiempos de ida
    // Genera la memoria de parámetros por cada base
    for (l = 0; l < sys_.tfm_pwi_forward.anglesBeamformer.size() && l < MAX_PWI_FL; l++)
    {
        sys_.tfm_pwi_forward.anglesMemory[l] = sys_.tfm_pwi_forward.anglesBeamformer[l].COS_PWI;
    }
    reset = 1;
    last = 1;
    sys_.tfm_pwi_forward.n_lword = 0;
    for (c_base = 0; c_base < sys_.tfm_pwi_forward.forwardBeamformer.size(); c_base++)
    {
        for (l = 0; l < sys_.tfm_pwi_forward.forwardBeamformer[c_base].size(); l++)
        {
            copy_foc_tfm_pwi_line_to_init_word(&init_word, &sys_.tfm_pwi_forward.forwardBeamformer[c_base][l]);
            sys_.tfm_pwi_forward.n_lword += copy_init_word_to_buffer_tfm_pwi_forward(&sys_.tfm_pwi_forward.forwardMemory[0], init_word, sys_.tfm_pwi_forward.n_lword);
        }
    }
    update_tfm_forward_ = true;
    if (STDL_GetTFMBeamformer()) PRINT_TFM_PWI_Forward();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_EmissionFocalLaw(vector <vector< T_VCH_EmitterFocalLaw > > &emission_focal_law)
{
    int b, l, c_fl, c_base, c_sitau, id_base, idx;
    int n_ch = GET_ChannelsNumber();
    int n_fl = emission_focal_law.size();
    bool pin_to_channel;

    if (n_ch > 0 && n_fl > 0)
    {
        if (cfg_.pa_probe->pin_to_channel_assignment_) pin_to_channel = false;
        else pin_to_channel = cfg_.pa_probe != NULL && cfg_.pa_probe->pin_to_channel_.size() > 0;

        for (b = 0; b < sys_.base.size(); b++)
        {
            if (sys_.base[b] != NULL)
            {
                sys_.base[b]->reg_.fp_fl.efl.delay.resize(MOD_MAX_CH * n_fl, 0xFFFF);
                sys_.base[b]->reg_.fp_fl.efl.n_fl = n_fl;
            }
        }

        for (l = 0; l < n_fl; l++)
        {
            for (c_fl = 0; c_fl < emission_focal_law[l].size(); c_fl++)
            {
                if (pin_to_channel && c_fl < cfg_.pa_probe->pin_to_channel_.size()) c_sitau = cfg_.pa_probe->pin_to_channel_[c_fl];
                else c_sitau = c_fl;

                if ((id_base = c_sitau / MOD_MAX_CH) >= sys_.base.size())
                    return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

                if ((c_base = c_sitau - id_base * MOD_MAX_CH) >= MOD_MAX_CH)
                    return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

                if ((idx = l * MOD_MAX_CH + c_base) >= sys_.base[id_base]->reg_.fp_fl.efl.delay.size())
                    return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

                if (emission_focal_law[l][c_fl].emitter_enabled == 1)
                    sys_.base[id_base]->reg_.fp_fl.efl.delay[idx] = emission_focal_law[l][c_fl].emitter_delay_ticks;
                else sys_.base[id_base]->reg_.fp_fl.efl.delay[idx] = 0xFFFF;
                update_focal_law_fp_ = true;
            }
        }
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PAConfig (unsigned short n_element, float pitch, float frequency)
{
int n_ch, result;
float material_velocity, pitch_bak, frequency_bak;
unsigned short n_element_bak;

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity();

   if (pitch <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (n_element <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (frequency <= 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (sys_.pa_sys != NULL)
   {
      if ((n_ch = sys_.pa_sys->GetChannelsNumber()) < 0) return RLOG(n_ch);
   }
   else return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);

   if (n_element > n_ch) n_element = n_ch;
   //cfg_.probe_pitch = pitch;
   //cfg_.probe_n_elements = n_element;
   //cfg_.probe_frequency = frequency;
   cfg_.probe_aperture_samples = Round32((n_element * pitch * (float)1000) / material_velocity) * cfg_.sampling_frequency;

    if (cfg_.pa_probe != NULL) 
    {
        pitch_bak = cfg_.pa_probe->GetPitch();
        frequency_bak = cfg_.pa_probe->GetFrequency();
        n_element_bak = cfg_.pa_probe->GetElementNumber();
        if ((result = cfg_.pa_probe->SetConfig(n_element, pitch, frequency, 0, 0)) < 0)
        {
            cfg_.pa_probe->SetConfig(n_element_bak, pitch_bak, frequency_bak, 0, 0);
            return RLOG(result);
        }
        if ((result = RELOAD_FocalLaw()) < 0)
        {
            cfg_.pa_probe->SetConfig(n_element_bak, pitch_bak, frequency_bak, 0, 0);
            return RLOG(result);
        }
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_PinToChannel(vector<unsigned short> &data)
{
    if (cfg_.pa_probe != NULL) cfg_.pa_probe->PinToChannel(data);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_RFFilterEnabled(bool data)
{
   int result;

   if (cfg_.fir.enabled != data) cfg_.fir.enabled = data;
   //switch (cfg_.acquisition_type)
   //{
   //case ACQUISITION_TYPE_TFM:
   //    break;
   //case ACQUISITION_TYPE_PA:
   //    if (cfg_.fir.enabled != data) cfg_.fir.enabled = data;
   //    break;
   //default:
   //    if (cfg_.fp_fir.enabled != data) cfg_.fp_fir.enabled = data;
   //    break;
   //}
   if ((result = RELOAD_RFFilter()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_RFFilter(float low_frequency, float high_frequency)
{
   int i;

   if (!cfg_.fir.enabled)
   {
       cfg_.fir.low_frequency = low_frequency;
       cfg_.fir.high_frequency = high_frequency;
       if (cfg_.fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
       for (i = 0; i < cfg_.fir.hw_coefficients.size(); i++) cfg_.fir.hw_coefficients[i] = 0;
       cfg_.fir.hw_coefficients[FIR_N_COEFFICIENTS / 2] = 0x7FFF;
       update_fir_filter_ = true;
   }
   else
   {
       if (cfg_.fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
       if (cfg_.fir.coefficients_fft.size() != FIR_N_COEFFICIENTS) cfg_.fir.coefficients_fft.resize(FIR_N_COEFFICIENTS);
       cfg_.fir.factor = CalculaFiltro(low_frequency,
           high_frequency,
           GET_SamplingFrequency() * (float)1000.,
           FIR_N_COEFFICIENTS,
           BASE_BITS_CPB,
           &cfg_.fir.hw_coefficients[0],
           cfg_.fir.coefficients_fft);

       cfg_.fir.low_frequency = low_frequency;
       cfg_.fir.high_frequency = high_frequency;
       update_fir_filter_ = true;
   }

   //switch (cfg_.acquisition_type)
   //{
   //case ACQUISITION_TYPE_TFM:
   //    break;
   //case ACQUISITION_TYPE_PA:
   //    if (!cfg_.fir.enabled)
   //    {
   //        cfg_.fir.low_frequency = low_frequency;
   //        cfg_.fir.high_frequency = high_frequency;
   //        if (cfg_.fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
   //        for (i = 0; i < cfg_.fir.hw_coefficients.size(); i++) cfg_.fir.hw_coefficients[i] = 0;
   //        cfg_.fir.hw_coefficients[FIR_N_COEFFICIENTS / 2] = 0x7FFF;
   //        update_fir_filter_ = true;
   //    }
   //    else //if (cfg_.fir.low_frequency != low_frequency || cfg_.fir.high_frequency != high_frequency)
   //    {
   //        if (cfg_.fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
   //        if (cfg_.fir.coefficients_fft.size() != FIR_N_COEFFICIENTS) cfg_.fir.coefficients_fft.resize(FIR_N_COEFFICIENTS);
   //        cfg_.fir.factor = CalculaFiltro(low_frequency,
   //            high_frequency,
   //            GET_SamplingFrequency() * (float)1000.,
   //            FIR_N_COEFFICIENTS,
   //            BASE_BITS_CPB,
   //            &cfg_.fir.hw_coefficients[0],
   //            cfg_.fir.coefficients_fft);

   //        cfg_.fir.low_frequency = low_frequency;
   //        cfg_.fir.high_frequency = high_frequency;
   //        update_fir_filter_ = true;
   //    }
   //    break;
   //default:
   //    if (!cfg_.fp_fir.enabled)
   //    {
   //        cfg_.fp_fir.low_frequency = low_frequency;
   //        cfg_.fp_fir.high_frequency = high_frequency;
   //        if (cfg_.fp_fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fp_fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
   //        for (i = 0; i < cfg_.fp_fir.hw_coefficients.size(); i++) cfg_.fp_fir.hw_coefficients[i] = 0;
   //        cfg_.fp_fir.hw_coefficients[FIR_N_COEFFICIENTS / 2] = 0x7FFF;
   //        update_fir_filter_ = true;
   //    }
   //    else
   //    {
   //        if (cfg_.fp_fir.hw_coefficients.size() != FIR_N_COEFFICIENTS) cfg_.fp_fir.hw_coefficients.resize(FIR_N_COEFFICIENTS);
   //        if (cfg_.fp_fir.coefficients_fft.size() != FIR_N_COEFFICIENTS) cfg_.fp_fir.coefficients_fft.resize(FIR_N_COEFFICIENTS);
   //        cfg_.fp_fir.factor = CalculaFiltro(low_frequency,
   //            high_frequency,
   //            GET_SamplingFrequency() * (float)1000.,
   //            FIR_N_COEFFICIENTS,
   //            BASE_BITS_CPB,
   //            &cfg_.fp_fir.hw_coefficients[0],
   //            cfg_.fp_fir.coefficients_fft);

   //        cfg_.fp_fir.low_frequency = low_frequency;
   //        cfg_.fp_fir.high_frequency = high_frequency;
   //        update_fir_filter_ = true;
   //    }
   //    break;
   //}

   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::RELOAD_RFFilter(void)
{
   int result;

   if ((result = LOAD_RFFilter(cfg_.fir.low_frequency, cfg_.fir.high_frequency)) < 0)
       return RLOG(result);

   //switch (cfg_.acquisition_type)
   //{
   //case ACQUISITION_TYPE_TFM:
   //    break;
   //case ACQUISITION_TYPE_PA:
   //    if ((result = LOAD_RFFilter(cfg_.fir.low_frequency, cfg_.fir.high_frequency)) < 0)
   //        return RLOG(result);
   //    break;
   //default:
   //    if ((result = LOAD_RFFilter(cfg_.fp_fir.low_frequency, cfg_.fp_fir.high_frequency)) < 0)
   //        return RLOG(result);
   //    break;
   //}
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_Processing(int processing_type, int n_signals, int wait_prf)
{
   int result, n_signals_reg, n_signals_processing;
   int min_signals = ST_RangeMinProcessingSignals();
   int max_signals = ST_RangeMaxProcessingSignals();

   if (processing_type < 0 || processing_type > VCH_PROCESSING_TYPE_MAX)
   {
      processing_type = 0;
      ELOG(charEVCH_Parameter, EVCH_Parameter);
   }

   if (processing_type == 1)
   {
      if (n_signals < 1)
      {
         n_signals = 1;
         ELOG(charEVCH_Parameter, EVCH_Parameter);
      }
      if (n_signals > 8)
      {
         n_signals = 8;
         ELOG(charEVCH_Parameter, EVCH_Parameter);
      }
   }
   else if (processing_type == 2)
   {
      if (n_signals < min_signals)
      {
         n_signals = min_signals;
         ELOG(charEVCH_Parameter, EVCH_Parameter);
      }
      if (n_signals > max_signals)
      {
         n_signals = max_signals - 1;
         ELOG(charEVCH_Parameter, EVCH_Parameter);
      }
   }
   //else if (processing_type == 3)
   //{
      //if (n_signals < min_signals)
      //{
      //   n_signals = min_signals;
      //   ELOG(charEVCH_Parameter, EVCH_Parameter);
      //}
      //if (n_signals > max_signals)
      //{
      //   n_signals = max_signals - 1;
      //   ELOG(charEVCH_Parameter, EVCH_Parameter);
      //}
   //}

   if (processing_type != cfg_.processing_type)
   {
      update_register_ = true;
      cfg_.processing_type = processing_type;
   }
   switch (processing_type)
   {
   case 0: 
      n_signals_reg = 0; 
      if ((result = LOAD_LOG2_AVERAGE(0)) < 0) return RLOG(result);
      break;
   case 1:
      cfg_.parameter_processing_avr = n_signals;
      n_signals_processing = n_signals;// pow(2, n_signals);
      if (n_signals_processing != cfg_.n_signals_processing_avr)
      {
         update_register_ = true;
         cfg_.n_signals_processing_avr = n_signals_processing;
      }
      n_signals_reg = cfg_.n_signals_processing_avr - 1;
      if ((result = LOAD_LOG2_AVERAGE(0)) < 0) return RLOG(result);
      break;
   case 2:
      if (n_signals != cfg_.n_signals_processing_emi)
      {
         update_register_ = true;
         cfg_.n_signals_processing_emi = n_signals;
      }
      n_signals_reg = cfg_.n_signals_processing_emi - 1;
      if ((result = LOAD_LOG2_AVERAGE(0)) < 0) return RLOG(result);
      break;
   case 3:
       if (n_signals > 0) cfg_.fp_processing_type = 1;
       else cfg_.fp_processing_type = 0;
      if ((result = LOAD_LOG2_AVERAGE(n_signals)) < 0) return RLOG(result);
      break;
   }
   if ((wait_prf == 1) != cfg_.wait_prf_signals_processing)
   {
      update_register_ = true;
      cfg_.wait_prf_signals_processing = wait_prf;
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_LOAD_EmissionFocalLawFile(const char *file)
{
   int result, i, j, n_delays, n_fl, n_ch, n_ch_sitau;
   ifstream archivo_entrada(file);
   string line, data_str;
   int init;
   int end;
   vector<float> line_data;
   vector<vector<float> > fl;
   float data;

   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   n_ch_sitau = GET_ChannelsNumber();
   n_delays = 0;
   fl.clear(); 
   while (getline(archivo_entrada, line, '\n'))
   {
      init = end = 0;
      line_data.clear();
      while (end = line.find(" ", init), end >= 0)
      {
         data_str = line.substr(init, end - init);
         replace(data_str.begin(), data_str.end(), ',', '.');
         data = atof(data_str.c_str());
         line_data.push_back(data);
         if (line_data.size() > n_ch_sitau) return ELOG(charEVCH_File, EVCH_File);
         init = end + 1;
      }
      if (init < line.length())
      {
         data_str = line.substr(init, line.length() - init);
         replace(data_str.begin(), data_str.end(), ',', '.');
         data = atof(data_str.c_str());
         line_data.push_back(data);
         if (line_data.size() > n_ch_sitau) return ELOG(charEVCH_File, EVCH_File);
      }
      if (line_data.size() > 0)
      {
         if (fl.size() == 0) n_delays = line_data.size();
         else if (n_delays != line_data.size()) return ELOG(charEVCH_File, EVCH_File);
         fl.push_back(line_data);
      }
   }

   if ((n_fl = fl.size()) > 0)
   {
      cfg_.mc_emission_focal_law.resize(n_fl);
      for (i = 0; i < n_fl; i++)
      {
         if ((n_ch = fl[i].size()) != n_ch_sitau) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
         cfg_.mc_emission_focal_law[i].resize(n_ch_sitau);
         for (j = 0; j < n_ch_sitau; j++)
         {
            cfg_.mc_emission_focal_law[i][j].emitter_delay = fl[i][j];
            cfg_.mc_emission_focal_law[i][j].emitter_delay_ticks = cfg_.mc_emission_focal_law[i][j].emitter_delay * FCLK4_MHz;
            cfg_.mc_emission_focal_law[i][j].emitter_enabled = cfg_.mc_emission_focal_law[i][j].emitter_delay >= 0 ? 1 : 0;
         }
      }
      cfg_.fp_focal_laws_type = VCH_FP_FL_TYPE_USER;
      cfg_.fp_focal_law_file = AnsiString(file);

      if ((result = FP_UpdateAcquisitionSize()) < 0) return RLOG(result);
      if (cfg_.acquisition_type == ACQUISITION_TYPE_MC)
      {
          if ((result = LOAD_EmissionFocalLaw(cfg_.mc_emission_focal_law)) < 0) return RLOG(result);
      }

      //if ((result = LOAD_RegistersFocalLaw()) < 0) return RLOG(result);
   }
   else return ELOG(charEVCH_File, EVCH_File);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::TFM_LOAD_FMC_Config(vector<int>& rec_elements, vector<int>& emi_elements, float start_x, float end_x, int n_lines, float angle)
//{
//    int i, j, l, c, result;
//    vector<float> v_rx_mm;
//    vector<float> v_rz_mm;
//    vector<float> v_ex_mm;
//    vector<float> v_ez_mm;
//    vector<float> v_pix_mm;
//    vector<float> v_piz_mm;
//    vector<float> v_sin_ang;
//    vector<float> v_cos_ang;
//    float angle_rad = angle * M_PI / 180;
//    vector < vector<foc_params_s> > emi_fl;
//    vector < vector<foc_params_s> > rec_fl;
//    vector<int> v_rec_disable;
//    vector<int> v_emi_disable;
//    vector<TFP_VCH_BASE_BeamformerParameters> forward_parameters;
//    float material_velocity;
//    float start_z = cfg_.acq.acq_start_distance;
//    float end_z = cfg_.acq.acq_end_distance;
//    int n_ch = GET_ChannelsNumber();
//
//    v_rec_disable.assign(rec_elements.size(), 0);
//    v_emi_disable.assign(emi_elements.size(), 0);
//
//    for (i = 0; i < rec_elements.size(); i++)
//    {
//        if (rec_elements[i] >= cfg_.pa_probe->x_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
//        v_rx_mm.push_back(cfg_.pa_probe->x_[rec_elements[i]]);
//        if (rec_elements[i] >= cfg_.pa_probe->z_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
//        v_rz_mm.push_back(cfg_.pa_probe->z_[rec_elements[i]]);
//    }
//    for (i = 0; i < emi_elements.size(); i++)
//    {
//        if (emi_elements[i] >= cfg_.pa_probe->x_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
//        v_ex_mm.push_back(cfg_.pa_probe->x_[emi_elements[i]]);
//        if (emi_elements[i] >= cfg_.pa_probe->z_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
//        v_ez_mm.push_back(cfg_.pa_probe->z_[emi_elements[i]]);
//    }
//    for (float resolution = (end_x - start_x) / n_lines, i = 0; i < n_lines; i++) v_pix_mm.push_back(start_x + resolution * i);
//    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_piz_mm.push_back(start_z);// +resolution * i);
//    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_sin_ang.push_back(sin(angle_rad));
//    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_cos_ang.push_back(cos(angle_rad));
//
//    if ((result = tfm_param_arrays(rec_elements.size(), emi_elements.size(), n_lines, 0, &v_rx_mm[0], &v_rz_mm[0], &v_ex_mm[0], &v_ez_mm[0], NULL, NULL, &v_pix_mm[0], &v_piz_mm[0], &v_sin_ang[0], &v_cos_ang[0], cfg_.specimen->GetVelocity() / 1000, GET_SamplingFrequency(), &v_rec_disable[0], &v_emi_disable[0], emi_fl, rec_fl)) < 0)
//        return RLOG(result);
//
//    sys_.tfm_fmc_forward.beamformer.clear();
//    for (i = 0; i < emi_fl.size(); i++)
//    {
//        forward_parameters.resize(emi_fl[i].size());
//        for (j = 0; j < emi_fl[i].size(); j++)
//        {
//            forward_parameters[j].A_0 = emi_fl[i][j].A_0;
//            forward_parameters[j].D_0 = emi_fl[i][j].D_0;
//            forward_parameters[j].E_0 = emi_fl[i][j].E_0;
//            forward_parameters[j].R_0 = emi_fl[i][j].R_0;
//            forward_parameters[j].T_0 = emi_fl[i][j].T_0;
//            forward_parameters[j].Z_0 = emi_fl[i][j].Z_0;
//        }
//        sys_.tfm_fmc_forward.beamformer.push_back(forward_parameters);
//    }
//
//    if (rec_fl.size() != n_lines) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//
//    cfg_.tfm_backward.beamformer.resize(n_lines);
//    for (i = 0; i < n_lines; i++)
//    {
//        cfg_.tfm_backward.beamformer[i].resize(rec_fl[i].size());
//        for (j = 0; j < rec_fl[i].size(); j++)
//        {
//            cfg_.tfm_backward.beamformer[i][j].A_0 = rec_fl[i][j].A_0;
//            cfg_.tfm_backward.beamformer[i][j].D_0 = rec_fl[i][j].D_0;
//            cfg_.tfm_backward.beamformer[i][j].E_0 = rec_fl[i][j].E_0;
//            cfg_.tfm_backward.beamformer[i][j].R_0 = rec_fl[i][j].R_0;
//            cfg_.tfm_backward.beamformer[i][j].T_0 = rec_fl[i][j].T_0;
//            cfg_.tfm_backward.beamformer[i][j].Z_0 = rec_fl[i][j].Z_0;
//        }
//    }
//
//    // LEYS FOCALS EN EMISIÓN
//
//    if (n_ch > 0)
//    {
//        cfg_.fp_focal_law.clear();
//        cfg_.tfm_emission_focal_law.resize(emi_elements.size());
//        for (l = 0; l < cfg_.tfm_emission_focal_law.size(); l++)
//        {
//            cfg_.tfm_emission_focal_law[l].resize(n_ch);
//            for (c = 0; c < n_ch; c++)
//            {
//                cfg_.tfm_emission_focal_law[l][c].emitter_enabled = 0;
//                cfg_.tfm_emission_focal_law[l][c].emitter_delay = 0;
//                cfg_.tfm_emission_focal_law[l][c].emitter_delay_ticks = 0;
//            }
//            if (emi_elements[l] >= 0 && emi_elements[l] < cfg_.tfm_emission_focal_law[l].size())
//                cfg_.tfm_emission_focal_law[l][emi_elements[l]].emitter_enabled = 1;
//        }
//        if (cfg_.acquisition_type == ACQUISITION_TYPE_TFM)
//        {
//            if ((result = LOAD_EmissionFocalLaw(cfg_.tfm_emission_focal_law)) < 0) return RLOG(result);
//        }
//    }
//
//    if ((result = TFM_HWMemory()) < 0) return RLOG(result);
//
//    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
//    material_velocity = cfg_.specimen->GetVelocity();
//    if (material_velocity <= 0) ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
//    
//    cfg_.tfm.start_x_distance = start_x;
//    cfg_.tfm.end_x_distance = end_x;
//    cfg_.tfm.start_x_time = cfg_.tfm.start_x_distance / material_velocity;
//    cfg_.tfm.end_x_time = cfg_.tfm.end_x_distance / material_velocity;
//    cfg_.tfm.angle = angle;
//    cfg_.tfm.n_lines = n_lines;
//    cfg_.tfm.rec_elements.assign(rec_elements.begin(), rec_elements.end());
//    cfg_.tfm.emi_elements.assign(emi_elements.begin(), emi_elements.end());
//    return EVCH_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_LOAD_Backward(float start_x, float end_x, int n_lines, float angle)
{
    int i, result;
    vector<float> v_rx_mm;
    vector<float> v_rz_mm;
    vector<float> v_ex_mm;
    vector<float> v_ez_mm;
    vector<float> v_pix_mm;
    vector<float> v_piz_mm;
    vector<float> v_sin_ang;
    vector<float> v_cos_ang;
    float angle_rad = angle * M_PI / 180;
    vector<int> v_rec_disable;
    vector<int> v_emi_disable;
    float material_velocity;
    float start_z = cfg_.acq.acq_start_distance;
    float end_z = cfg_.acq.acq_end_distance;
    int n_ch;

    if ((n_ch = GET_ChannelsNumber()) < 0) return RLOG(n_ch);

    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();
    if (material_velocity <= 0) ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

    v_rec_disable.assign(n_ch, 0);

    cfg_.tfm.rec_elements.clear();
    for (i = 0; i < n_ch && i < cfg_.pa_probe->x_.size() && i< cfg_.pa_probe->z_.size(); i++)
    {
        //if (i >= cfg_.pa_probe->x_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
        v_rx_mm.push_back(cfg_.pa_probe->x_[i]);
        //if (i >= cfg_.pa_probe->z_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
        v_rz_mm.push_back(cfg_.pa_probe->z_[i]);
        cfg_.tfm.rec_elements.push_back(i);
    }
    for (float resolution = (end_x - start_x) / n_lines, i = 0; i < n_lines; i++) v_pix_mm.push_back(start_x + resolution * i);
    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_piz_mm.push_back(start_z);
    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_sin_ang.push_back(sin(angle_rad));
    for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_cos_ang.push_back(cos(angle_rad));

    //if ((result = tfm_param_arrays(rec_elements.size(), emi_elements.size(), n_lines, 0, &v_rx_mm[0], &v_rz_mm[0], &v_ex_mm[0], &v_ez_mm[0], NULL, NULL, &v_pix_mm[0], &v_piz_mm[0], &v_sin_ang[0], &v_cos_ang[0], cfg_.specimen->GetVelocity() / 1000, GET_SamplingFrequency(), &v_rec_disable[0], &v_emi_disable[0], emi_fl, rec_fl)) < 0)
    //    return RLOG(result);
    //if ((result = tfm_fmc_forward_parameters(emi_elements, 0, v_ex_mm, v_ez_mm, v_pix_mm, v_piz_mm, v_sin_ang, v_cos_ang, cfg_.specimen->GetVelocity() / 1000, GET_SamplingFrequency(), v_emi_disable, sys_.tfm_forward.beamformer)) < 0)
    //    return RLOG(result);
    if ((result = tfm_backward_parameters(n_ch, 0, v_rx_mm, v_rz_mm, v_pix_mm, v_piz_mm, v_sin_ang, v_cos_ang, material_velocity / 1000, GET_SamplingFrequency(), v_rec_disable, cfg_.tfm_backward.beamformer)) < 0)
        return RLOG(result);

    if ((result = TFM_BackwardHWMemory()) < 0) return RLOG(result);

    cfg_.tfm.start_x_distance = start_x;
    cfg_.tfm.end_x_distance = end_x;
    cfg_.tfm.start_x_time = cfg_.tfm.start_x_distance / material_velocity;
    cfg_.tfm.end_x_time = cfg_.tfm.end_x_distance / material_velocity;
    cfg_.tfm.angle = angle;
    cfg_.tfm.n_lines = n_lines;
    //cfg_.tfm.rec_elements.assign(elements.begin(), elements.end());

    //if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_elements)) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_LOAD_FMC_Forward(int start_element, int end_element, int step_element)
{
    int i, result, id_vch = 0;
    vector<int> elements;

    if (start_element < 0) start_element = 0;
    if (end_element < 0) end_element = 0;
    if (step_element < 1) step_element = 1;
    if (end_element < start_element) end_element = start_element;
    for (i = start_element; i <= end_element; i += step_element) elements.push_back(i);
    if ((result = TFM_LOAD_FMC_Forward(elements)) < 0) return RLOG(result);
    cfg_.tfm.emi_start_element = start_element;
    cfg_.tfm.emi_end_element = end_element;
    cfg_.tfm.emi_step_element = step_element;
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_LOAD_FMC_Forward(vector<int>& elements)
{
    int i, l, c, result;
    vector<float> v_ex_mm;
    vector<float> v_ez_mm;
    vector<float> v_pix_mm;
    vector<float> v_piz_mm;
    vector<float> v_sin_ang;
    vector<float> v_cos_ang;
    float angle_rad = cfg_.tfm.angle * M_PI / 180;
    vector<int> v_emi_disable;
    float material_velocity;
    int n_ch = GET_ChannelsNumber();
    int n_lines = cfg_.tfm.n_lines;
    float end_x = cfg_.tfm.end_x_distance;
    float start_x = cfg_.tfm.start_x_distance;
    float start_z = cfg_.acq.acq_start_distance;
    float end_z = cfg_.acq.acq_end_distance;

    if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
    material_velocity = cfg_.specimen->GetVelocity();
    if (material_velocity <= 0) ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    if (n_lines > 0)
    {
        v_emi_disable.assign(elements.size(), 0);
        start_z = cfg_.acq.acq_start_distance;
        end_z = cfg_.acq.acq_end_distance;

        for (i = 0; i < elements.size(); i++)
        {
            if (elements[i] >= cfg_.pa_probe->x_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
            v_ex_mm.push_back(cfg_.pa_probe->x_[elements[i]]);
            if (elements[i] >= cfg_.pa_probe->z_.size()) return ELOG(charEVCH_Parameter, EVCH_Parameter);
            v_ez_mm.push_back(cfg_.pa_probe->z_[elements[i]]);
        }
        for (float resolution = (end_x - start_x) / n_lines, i = 0; i < n_lines; i++) v_pix_mm.push_back(start_x + resolution * i);
        for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_piz_mm.push_back(start_z);
        for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_sin_ang.push_back(sin(angle_rad));
        for (float resolution = (end_z - start_z) / n_lines, i = 0; i < n_lines; i++) v_cos_ang.push_back(cos(angle_rad));

        if ((result = tfm_fmc_forward_parameters(elements, 0, v_ex_mm, v_ez_mm, v_pix_mm, v_piz_mm, v_sin_ang, v_cos_ang, material_velocity / 1000, GET_SamplingFrequency(), v_emi_disable, sys_.tfm_fmc_forward.beamformer)) < 0)
            return RLOG(result);
        cfg_.tfm.focal_laws_type = VCH_FP_FL_TYPE_FMC;

        // LEYES FOCALS EN EMISIÓN

        if (n_ch > 0)
        {
            cfg_.fp_focal_law.clear();
            cfg_.tfm_emission_focal_law.resize(elements.size());
            for (l = 0; l < cfg_.tfm_emission_focal_law.size(); l++)
            {
                cfg_.tfm_emission_focal_law[l].resize(n_ch);
                for (c = 0; c < n_ch; c++)
                {
                    cfg_.tfm_emission_focal_law[l][c].emitter_enabled = 0;
                    cfg_.tfm_emission_focal_law[l][c].emitter_delay = 0;
                    cfg_.tfm_emission_focal_law[l][c].emitter_delay_ticks = 0;
                }
                if (elements[l] >= 0 && elements[l] < cfg_.tfm_emission_focal_law[l].size())
                    cfg_.tfm_emission_focal_law[l][elements[l]].emitter_enabled = 1;
            }
            if (cfg_.acquisition_type == ACQUISITION_TYPE_TFM)
            {
                if ((result = LOAD_EmissionFocalLaw(cfg_.tfm_emission_focal_law)) < 0) return RLOG(result);
            }
            update_tfm_backward_ = true;
        }

        if ((result = TFM_FMC_Forward_HWMemory()) < 0) return RLOG(result);

        cfg_.tfm.emi_elements.assign(elements.begin(), elements.end());
    }
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LOAD_AcquisitionType(int data)
{
    int result;

#if !defined(_PA)
    if (data == 1) return ELOG(charEVCH_Parameter, EVCH_Parameter);
#endif
#if !defined(_TFM)
    if (data == 2) return ELOG(charEVCH_Parameter, EVCH_Parameter);
#endif

    cfg_.acquisition_type = data;
    if ((result = RELOAD_AcqDistance()) < 0) return RLOG(result);
    if ((result = RELOAD_FocalLaw()) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::RELOAD_AcquisitionType(void)
{
    int result;
    if ((result = LOAD_AcquisitionType(cfg_.acquisition_type)) < 0) return RLOG(result);
    return EVCH_NONE;
}
