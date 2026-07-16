// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <string>  
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_vch_config.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "fileformat.h"
#include "st_get.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "st_calc.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_range.h"
#include "utils.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "c_xml_config.h"
#include "c_xml_sitau_hw.h"
#include "sitau_hw.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::CONFIG_Copy (CVirtualChannel *vch)
{
int i, b, id_base, result = 0, n_bases, id_first_base = 0;
bool multiplexed;
CPASystem *pa_sys = NULL;

   if (vch == NULL) return ELOG(charEVCH_Parameter, EVCH_Parameter);
   //if (sitau_hw == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   if ((pa_sys = STHW_GetPtrPASystem(vch->sys_.id_pa_sys)) == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);

   sys_.id_pa_sys = vch->sys_.id_pa_sys;
   sys_.pa_sys = pa_sys;

   if ((n_bases = sys_.pa_sys->GetBASENumber()) <= 0) return ELOG(charEVCH_BASE_NULL, EVCH_BASE_NULL);
   if (n_bases != vch->sys_.base.size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

   cfg_.enabled = vch->cfg_.enabled;
   cfg_.name = vch->cfg_.name;
   sys_.id_connector = vch->sys_.id_connector;

   sys_.tgc = vch->sys_.tgc;
   if (sys_.pulser == NULL) sys_.pulser = new C_PULSER();
   if (sys_.afe == NULL) sys_.afe = new C_AFE();
   sys_.pulser->CONFIG_Copy(vch->sys_.pulser);
   sys_.afe->CONFIG_Copy(vch->sys_.afe);

   if (sys_.base.size() != vch->sys_.base.size())
   {
      if ((id_first_base = sys_.pa_sys->GetBASEFirst()) < 0) return ELOG(charEVCH_IdBASE, EVCH_IdBASE);
      multiplexed = sys_.pa_sys->Multiplexed();
      sys_.base.resize(n_bases);
      for (b=0; b<sys_.base.size(); b++)
      {
         for (i = 0, id_base = id_first_base; i < n_bases; i++, id_base++) sys_.base[i] = new CBASE_VirtualChannel(id_base, multiplexed);
      }
   }

   for (b=0; b<sys_.base.size() && b<vch->sys_.base.size(); b++)
   {
      if (sys_.base[b] != NULL && (result = sys_.base[b]->CopyConfig(vch->sys_.base[b])) < 0) return RLOG(result);
   }

   // Comprueba si los datos del sub-systema son correctos
   if (sys_.base.size() == 0) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (sys_.base[0] == NULL) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
   if (sys_.pa_sys->GetBASEFirst() != sys_.base[0]->sys_.id_base_sitau || sys_.pa_sys->GetBASENumber() != sys_.base.size())
   {
      if ((pa_sys = STHW_GetPtrPASystem_BASEFirst(sys_.base[0]->sys_.id_base_sitau)) == NULL) return ELOG(charEVCH_IdBASE, EVCH_IdBASE);
      sys_.pa_sys = pa_sys;
      sys_.id_pa_sys = pa_sys->GetId();
   }

   if (cfg_.specimen != NULL)
   {
      if ((result = cfg_.specimen->CONFIG_Copy (vch->cfg_.specimen)) < 0) {RLOG(result);}
   }
   if (cfg_.pa_probe != NULL)
   {
      if ((result = cfg_.pa_probe->CONFIG_Copy (vch->cfg_.pa_probe)) < 0) {RLOG(result);}
   }
   if (cfg_.scan != NULL)
   {
      if ((result = cfg_.scan->CONFIG_Copy (vch->cfg_.scan)) < 0) {RLOG(result);}
   }

   // Parámetros de adquisición - Pulser
   cfg_.auto_pulse_width = vch->cfg_.auto_pulse_width;

   // Parámetros de adquisición
   cfg_.acq_mode = vch->cfg_.acq_mode;
   cfg_.acq_time_mode = vch->cfg_.acq_time_mode;
   cfg_.prf_time = vch->cfg_.prf_time;
   cfg_.prf_burst_time = vch->cfg_.prf_burst_time;
   cfg_.prf_enabled = vch->cfg_.prf_enabled;
   cfg_.acquisition_type = vch->cfg_.acquisition_type;
#if !defined(_PA)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_PA) cfg_.acquisition_type = ACQUISITION_TYPE_MC;
#endif
#if !defined(_TFM)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_TFM) cfg_.acquisition_type = ACQUISITION_TYPE_MC;
#endif
   cfg_.signal_mode = vch->cfg_.signal_mode;
   
   // Parámetros de adquisición - DSP
   cfg_.fir.enabled = vch->cfg_.fir.enabled;
   cfg_.fir.low_frequency = vch->cfg_.fir.low_frequency;
   cfg_.fir.high_frequency = vch->cfg_.fir.high_frequency;
   //cfg_.fp_fir.enabled = vch->cfg_.fp_fir.enabled;
   //cfg_.fp_fir.low_frequency = vch->cfg_.fp_fir.low_frequency;
   //cfg_.fp_fir.high_frequency = vch->cfg_.fp_fir.high_frequency;
   RELOAD_RFFilter();
   cfg_.sampling_frequency = vch->cfg_.sampling_frequency;
   cfg_.n_signals_processing_emi = vch->cfg_.n_signals_processing_emi;
   cfg_.n_signals_processing_avr = vch->cfg_.n_signals_processing_avr;
   cfg_.parameter_processing_avr = vch->cfg_.parameter_processing_avr;
   if (sys_.afe->reg_.bussar.log2_average > 0) cfg_.processing_type = 3;
   else cfg_.processing_type = vch->cfg_.processing_type;;

   // Parámetros de adquisición - Ganancia

   //cfg_.gain_digital_db = vch->cfg_.gain_digital_db;
   cfg_.gain_db = vch->cfg_.gain_db;

   //cfg_.material_velocity = vch->cfg_.material_velocity;

   //cfg_.probe_pitch = vch->cfg_.probe_pitch;
   //cfg_.probe_n_elements = vch->cfg_.probe_n_elements;
   //cfg_.probe_frequency = vch->cfg_.probe_frequency;
   //cfg_.probe_aperture_samples = vch->cfg_.probe_aperture_samples;
   //if ((result = LOAD_PAConfig (vch->cfg_.probe_n_elements, vch->cfg_.probe_pitch, vch->cfg_.probe_frequency)) < 0)
   //   return RLOG(result);

   if (vch->cfg_.fp_pwic_start_angle + vch->cfg_.fp_pwic_step_angle >= vch->cfg_.fp_pwic_end_angle)
   {
      cfg_.fp_pwic_start_angle = vch->cfg_.fp_pwic_start_angle;
      cfg_.fp_pwic_step_angle = vch->cfg_.fp_pwic_step_angle;
      cfg_.fp_pwic_end_angle = cfg_.fp_pwic_start_angle + cfg_.fp_pwic_step_angle;
   }
   else
   {
      cfg_.fp_pwic_start_angle = vch->cfg_.fp_pwic_start_angle;
      cfg_.fp_pwic_step_angle = vch->cfg_.fp_pwic_step_angle;
      cfg_.fp_pwic_end_angle = vch->cfg_.fp_pwic_end_angle;
   }

   cfg_.fp_focal_law_file = vch->cfg_.fp_focal_law_file;
    switch (vch->cfg_.fp_focal_laws_type)
    {
    case VCH_FP_FL_TYPE_PWI:
        if ((result = LOAD_PWICFocalLaw(vch->cfg_.fp_pwic_start_angle, vch->cfg_.fp_pwic_step_angle, vch->cfg_.fp_pwic_end_angle)) < 0)
        {
            RLOG(result);
        }
        break;
    case VCH_FP_FL_TYPE_FMC:
        if ((result = LOAD_FMCFocalLaw()) < 0) { RLOG(result); }
        break;
    case VCH_FP_FL_TYPE_USER:
        if (cfg_.fp_focal_law_file.c_str() == "")
        {
            if ((result = LOAD_FMCFocalLaw()) < 0) { RLOG(result); }
        }
        if ((result = FP_LOAD_EmissionFocalLawFile((char*)cfg_.fp_focal_law_file.c_str())) < 0)
        { 
            RLOG(result); 
            if ((result = LOAD_FMCFocalLaw()) < 0) { RLOG(result); }
        }
        break;
    }

   // Parámetros de adquisición - Rango
   cfg_.acq.increased_acq_samples = vch->cfg_.acq.increased_acq_samples;
   cfg_.fp_acquisition_size = vch->cfg_.fp_acquisition_size;
   if ((result = LOAD_AcqDistance(vch->cfg_.acq.acq_start_distance, vch->cfg_.acq.acq_end_distance, vch->cfg_.acq_time_mode, true)) < 0)
      return RLOG(result);

   cfg_.tfm_backward.parallel_lines = vch->cfg_.tfm_backward.parallel_lines;

   cfg_.tfm.start_x_distance = vch->cfg_.tfm.start_x_distance;
   cfg_.tfm.end_x_distance = vch->cfg_.tfm.end_x_distance;
   cfg_.tfm.start_x_time = vch->cfg_.tfm.start_x_time;
   cfg_.tfm.end_x_time = vch->cfg_.tfm.end_x_time;
   cfg_.tfm.angle = vch->cfg_.tfm.angle;
   cfg_.tfm.image_size_user = vch->cfg_.tfm.image_size_user;
   cfg_.tfm.image_size_hw = vch->cfg_.tfm.image_size_hw;
   cfg_.tfm.frame_size = vch->cfg_.tfm.frame_size;
   cfg_.tfm.n_lines = vch->cfg_.tfm.n_lines;
   cfg_.tfm.emi_start_element = vch->cfg_.tfm.emi_start_element;
   cfg_.tfm.emi_end_element = vch->cfg_.tfm.emi_end_element;
   cfg_.tfm.emi_step_element = vch->cfg_.tfm.emi_step_element;
   cfg_.tfm.emi_elements.assign(vch->cfg_.tfm.emi_elements.begin(), vch->cfg_.tfm.emi_elements.end());
   cfg_.tfm.rec_elements.assign(vch->cfg_.tfm.rec_elements.begin(), vch->cfg_.tfm.rec_elements.end());

   //if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0)
   //    return RLOG(result);
   if ((result = TFM_LOAD_Backward(cfg_.tfm.start_x_distance, cfg_.tfm.end_x_distance, cfg_.tfm.n_lines, cfg_.tfm.angle)) < 0)
       return RLOG(result);
   switch (cfg_.tfm.focal_laws_type)
   {
   case VCH_FP_FL_TYPE_FMC:
       if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0) return RLOG(result);
       break;
   case VCH_FP_FL_TYPE_PWI:
       if ((result = TFM_LOAD_PWI_Forward(cfg_.tfm.pwic_start_angle, cfg_.tfm.pwic_step_angle, cfg_.tfm.pwic_end_angle)) < 0) return RLOG(result);
       break;
   }
   if ((result = RELOAD_AcquisitionType()) < 0)
       return RLOG(result);

   update_register_ = true;
   update_focal_law_fp_ = true;
   update_focal_law_pa_ = true;
   update_tgc_ = true;
   update_tfm_forward_ = true;
   update_tfm_backward_ = true;
	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::DecoFileVersion ( AnsiString version_file,
                                       unsigned short *field_1,
                                       unsigned short *field_2,
                                       unsigned short *field_3,
                                       unsigned short *field_4)
{
int idx;
AnsiString str;
#if defined(_MSC_VER)
int n_data = (int)version_file.length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#else
int n_data = version_file.Length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#endif
   return EVCH_NONE;
}
// ----------------------------------------------------------------------------
/**
	ST_LoadConfig
	==============
Descripcion:
	Carga la configuracion desde un archivo. El archivo debe haber sido creado
	previamente con la funcion 'USC_SaveConfig'.

Parametros:
	(*file): Nombre del archivo.
	(*date): Devuelve la fecha y hora en la que la configuracion fue salvada.
				El formato es una cadena de caracteres (dd/mm/YYYY HH:MM:SS).
				El tamaño de esta cadena debe ser de 20 caracteres.

Devuelve:
	- La última posición leida en el archivo.
	- (Ver codigos de error en 'return.h')

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LoadXMLConfigData (TXMLCFG_VirtualChannel *xml, int n_bursts, int n_images_per_burst, int id_vch)
{
int result;
float data_float;

   if (xml == NULL) return ELOG(charES2_FILE,ES2_FILE);

#if !defined(_PA)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_PA) xml->ut_parameters.acquisition_type.Set(ACQUISITION_TYPE_MC);
#endif
#if !defined(_TFM)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_TFM) xml->ut_parameters.acquisition_type.Set(ACQUISITION_TYPE_MC);
#endif

   cfg_.id = id_vch;
   cfg_.acq.increased_acq_samples = xml->pa_focal_law.pa_increased_acquired_samples.Value();

   cfg_.acquisition_type = xml->ut_parameters.acquisition_type.Value();
   cfg_.signal_mode = xml->ut_parameters.signal_mode.Value();


// ..... Virtual Channel Pulser Parameters .....................................

    if ((result = LOAD_Gain(xml->ut_parameters.gain.Value())) < 0) { RLOG(result); }
    data_float = xml->ut_parameters.prf_frequency.Value();
    if (data_float <= 0) data_float = 1000.;
    if ((result = LOAD_PRFTime(1. / data_float)) < 0) { RLOG(result); }
    data_float = xml->ut_parameters.prf_burst_frequency.Value();
    if (data_float <= 0) data_float = 1000.;
    if ((result = LOAD_PRFBurstTime(1. / data_float)) < 0) { RLOG(result); }
    if ((result = LOAD_MaterialVelocity(xml->ut_parameters.material_velocity.Value())) < 0) { RLOG(result); }
    if (xml->ut_parameters.acq_end_distance.Update())
    {
        if ((result = LOAD_AcqDistance(xml->ut_parameters.acq_start_distance.Value(), xml->ut_parameters.acq_end_distance.Value(), xml->ut_parameters.acq_time_mode.Value(), true)) < 0) 
            { RLOG(result); }
    }
    else
    {
        if ((result = LOAD_AcqSamples(xml->ut_parameters.acq_start_samples.Value(), xml->ut_parameters.acq_n_samples.Value())) < 0) { RLOG(result); }
    }

   // .............................................................................
   // .............................................................................
   if ((result = LOAD_Pulser(xml->pulser.pulses_number.Value(), xml->pulser.pulse_width.Value())) < 0) { RLOG(result); }
   if ((result = LOAD_PulserEnabled(xml->pulser.pulser_enabled.Value())) < 0) { RLOG(result); }
   // .............................................................................
   // .............................................................................
   if ((result = LOAD_PAConfig(xml->probe.probe_elements_number.Value(), xml->probe.probe_pitch.Value(), xml->probe.probe_frequency.Value())) < 0)
   {
       RLOG(result);
   }
   if (cfg_.pa_probe != NULL)
   {
       if (xml->probe.pin_to_channel.Value().size() > 0)
       {
           if (xml->probe.pin_to_channel.Value().size() != xml->probe.probe_elements_number.Value()) { ELOG(charES2_RANGO, ES2_RANGO); }
           else xml->probe.pin_to_channel.Assign(cfg_.pa_probe->pin_to_channel_);
       }
       cfg_.pa_probe->pin_to_channel_assignment_ = xml->probe.pin_to_channel_assignment.Value();
       //xml->probe.pin_to_channel_assignment.Set(cfg_.pa_probe->pin_to_channel_assignment_);
   }
   ST_SetHighVoltageMaximum(xml->probe.high_voltage_maximum.Value());
   // .............................................................................
   // .............................................................................
   if ((result = ST_SetAcquisitionsNumber(n_bursts, n_images_per_burst)) < 0)
   {
       RLOG(result);
   }

   cfg_.fp_focal_law_file = xml->fp_focal_law.fp_focal_law_file.Value();
   switch (xml->fp_focal_law.fp_focal_law_type.Value())
   {
   case VCH_FP_FL_TYPE_PWI:
       if (xml->fp_focal_law.fp_pwic_angles.Update())
       {
           if ((result = LOAD_PWICFocalLaw_Angles(&xml->fp_focal_law.fp_pwic_angles.Value()[0], xml->fp_focal_law.fp_pwic_angles.Value().size())) < 0)
           {
               RLOG(result);
           }
       }
       else
       {
           if ((result = LOAD_PWICFocalLaw(xml->fp_focal_law.fp_pwic_start_angle.Value(), xml->fp_focal_law.fp_pwic_step_angle.Value(), xml->fp_focal_law.fp_pwic_end_angle.Value())) < 0)
           {
               RLOG(result);
           }
       }
       break;
   case VCH_FP_FL_TYPE_FMC:
       if ((result = LOAD_FMCFocalLaw()) < 0)
       {
           RLOG(result);
       }
       break;
   case VCH_FP_FL_TYPE_USER:
       if (cfg_.fp_focal_law_file.c_str() == "")
       {
           if ((result = LOAD_FMCFocalLaw()) < 0)
           {
               RLOG(result);
           }
       }
       if ((FP_LOAD_EmissionFocalLawFile((char*)cfg_.fp_focal_law_file.c_str()) < 0))
       { 
           RLOG(result); 
           if ((result = LOAD_FMCFocalLaw()) < 0)
           {
               RLOG(result);
           }
       }
       break;
   }

   //cfg_.fp_focal_law_file = xml->fp_focal_law.fp_focal_law_file.Value();
   //if (ST_MC_SetEmissionFocalLawFile((char*)cfg_.fp_focal_law_file.c_str()) < 0)
   //{
   //    if (xml->fp_focal_law.fp_pwic_angles.Update())
   //    {
   //        if ((result = LOAD_PWICFocalLaw_Angles(&xml->fp_focal_law.fp_pwic_angles.Value()[0], xml->fp_focal_law.fp_pwic_angles.Value().size())) < 0)
   //        {
   //            RLOG(result);
   //        }
   //    }
   //    else
   //    {
   //        if ((result = LOAD_PWICFocalLaw(xml->fp_focal_law.fp_pwic_start_angle.Value(), xml->fp_focal_law.fp_pwic_step_angle.Value(), xml->fp_focal_law.fp_pwic_end_angle.Value())) < 0)
   //        {
   //            RLOG(result);
   //        }
   //    }
   //    if (xml->fp_focal_law.fp_focal_law_type.Value() != 2)
   //    {
   //        if ((result = LOAD_FMCFocalLaw()) < 0)
   //        {
   //            RLOG(result);
   //        }
   //    }
   //}
   // .............................................................................
   // .............................................................................
   cfg_.fp_processing_type = xml->fp_dsp.mc_processing_type.Value();

   cfg_.fir.enabled = xml->dsp.fir_enabled.Value();
   cfg_.fir.low_frequency = xml->dsp.fir_low_frequency.Value();
   cfg_.fir.high_frequency = xml->dsp.fir_high_frequency.Value();
   //cfg_.fp_fir.enabled = xml->fp_dsp.mc_fir_enabled.Value();
   //cfg_.fp_fir.low_frequency = xml->fp_dsp.mc_fir_low_frequency.Value();
   //cfg_.fp_fir.high_frequency = xml->fp_dsp.mc_fir_high_frequency.Value();
   // .............................................................................
   // .............................................................................
   if (cfg_.scan != NULL)
   {
       if ((result = cfg_.scan->LoadXMLConfigData(&xml->pa_focal_law)) < 0)
       {
           RLOG(result);
       }
   }
   // .............................................................................
   // .............................................................................
   if ((result = LOAD_RFFilter(xml->dsp.fir_low_frequency.Value(), xml->dsp.fir_high_frequency.Value())) < 0) { RLOG(result); }
   if ((result = LOAD_RFFilterEnabled(xml->dsp.fir_enabled.Value())) < 0) { RLOG(result); }
   // .............................................................................
   // .............................................................................
   // .............................................................................
   // .............................................................................

   if ((result = LOAD_TGC_Enabled(xml->ut_parameters.tgc_enabled.Value())) < 0) return RLOG(result);
   // .............................................................................
   // .............................................................................

   cfg_.tfm_backward.parallel_lines = xml->tfm_focal_law.tfm_global_parallel_lines.Value();

   cfg_.tfm.emi_start_element = xml->tfm_focal_law.tfm_emission_start_element.Value();
   cfg_.tfm.emi_end_element = xml->tfm_focal_law.tfm_emission_end_element.Value();
   cfg_.tfm.emi_step_element = xml->tfm_focal_law.tfm_emission_step_element.Value();

   cfg_.tfm.angle = xml->tfm_focal_law.tfm_receiver_angle.Value();
   cfg_.tfm.start_x_distance = xml->tfm_focal_law.tfm_receiver_start_distance.Value();
   cfg_.tfm.end_x_distance = xml->tfm_focal_law.tfm_receiver_end_distance.Value();
   cfg_.tfm.n_lines = xml->tfm_focal_law.tfm_receiver_lines_number.Value();

   //if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0)
   //    return RLOG(result);
   if ((result = TFM_LOAD_Backward(cfg_.tfm.start_x_distance, cfg_.tfm.end_x_distance, cfg_.tfm.n_lines, cfg_.tfm.angle)) < 0)
       return RLOG(result);
   switch (cfg_.tfm.focal_laws_type)
   {
   case VCH_FP_FL_TYPE_FMC:
       if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0) return RLOG(result);
       break;
   case VCH_FP_FL_TYPE_PWI:
       if ((result = TFM_LOAD_PWI_Forward(cfg_.tfm.pwic_start_angle, cfg_.tfm.pwic_step_angle, cfg_.tfm.pwic_end_angle)) < 0) return RLOG(result);
       break;
   }   
   // .............................................................................
   // .............................................................................
   //if ((result = LOAD_AcquisitionType(xml->ut_parameters.acquisition_type.Value())) < 0) return RLOG(result);
   // .............................................................................
   // .............................................................................

   switch (cfg_.acquisition_type)
   {
   case ACQUISITION_TYPE_TFM:
       break;
   case ACQUISITION_TYPE_PA:
       //if ((result = LOAD_RFFilter(xml->pa_dsp.pa_fir_low_frequency.Value(), xml->pa_dsp.pa_fir_high_frequency.Value())) < 0) { RLOG(result); }
       //if ((result = LOAD_RFFilterEnabled(xml->pa_dsp.pa_fir_enabled.Value())) < 0) { RLOG(result); }
       switch (xml->pa_dsp.pa_processing_type.Value())
       {
       case 1:
           if ((result = LOAD_Processing(1, 0, 0)) < 0) { RLOG(result); }
           break;
       case 2:
           if ((result = LOAD_Processing(2, xml->pa_dsp.pa_emi_signals_number.Value(), 0)) < 0) { RLOG(result); }
           break;
       case 3:
           if ((result = LOAD_Processing(3, xml->pa_dsp.pa_average_signals_number.Value(), 0)) < 0) { RLOG(result); }
           break;
       default:
           if ((result = LOAD_Processing(0, 0, 0)) < 0) { RLOG(result); }
           break;
       }
       break;
   default:
       //if ((result = LOAD_RFFilter(cfg_.fp_fir.low_frequency, cfg_.fp_fir.high_frequency)) < 0) { RLOG(result); }
       //if ((result = LOAD_RFFilterEnabled(cfg_.fp_fir.enabled)) < 0) { RLOG(result); }
       if (xml->fp_dsp.mc_processing_type.Value() > 0)
       {
           if ((result = LOAD_Processing(3, xml->fp_dsp.mc_average_signals_number.Value(), 0)) < 0) { RLOG(result); }
           //if ((result = LOAD_LOG2_AVERAGE(xml->fp_dsp.mc_average_signals_number.Value())) < 0) { RLOG(result); }
       }
       else
       {
           if ((result = LOAD_Processing(0, 0, 0)) < 0) { RLOG(result); }
           //if ((result = LOAD_LOG2_AVERAGE(0)) < 0) { RLOG(result); }
       }
       break;
   }

   if ((result = LoadXMLHardwareData()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// ----------------------------------------------------------------------------
/**
    ST_LoadConfig
    ==============
Descripcion:
    Carga la configuracion desde un archivo. El archivo debe haber sido creado
    previamente con la funcion 'USC_SaveConfig'.

Parametros:
    (*file): Nombre del archivo.
    (*date): Devuelve la fecha y hora en la que la configuracion fue salvada.
                El formato es una cadena de caracteres (dd/mm/YYYY HH:MM:SS).
                El tamaño de esta cadena debe ser de 20 caracteres.

Devuelve:
    - La última posición leida en el archivo.
    - (Ver codigos de error en 'return.h')

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::LoadXMLHardwareData(void)
{
    int result;
    TXMLHW_Data* xml = NULL;

    if (gb_xml_sitau_hw == NULL || (xml = gb_xml_sitau_hw->GetData()) == NULL) return EVCH_NONE;
    if (sys_.afe == NULL) return ELOG(charEVCH_CONFIG_NULL, EVCH_CONFIG_NULL);
    if (sys_.pulser == NULL) return ELOG(charEVCH_CONFIG_NULL, EVCH_CONFIG_NULL);

    if ((result = sys_.afe->LoadRegisterAFE_DELAY_OFFSET(xml->registers.afe.delay_offset_.Value())) < 0) { RLOG(result); }     
       
    // AFE - SPI - REG21
    if ((result = sys_.afe->LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_1_4(xml->registers.afe.spi.reg21.DIGITAL_HPF_FILTER_ENABLE_1_4.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_1_4(xml->registers.afe.spi.reg21.DIGITAL_HPF_FILTER_K_CH_1_4.Value())) < 0) { RLOG(result); }

    // AFE - SPI - REG33
    if ((result = sys_.afe->LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_5_8(xml->registers.afe.spi.reg33.DIGITAL_HPF_FILTER_ENABLE_5_8.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_5_8(xml->registers.afe.spi.reg33.DIGITAL_HPF_FILTER_K_CH_5_8.Value())) < 0) { RLOG(result); }

    // AFE - SPI - REG51
    if ((result = sys_.afe->LoadRegisterAFE_LPF_PROGRAMMABILITY(xml->registers.afe.spi.reg51.LPF_PROGRAMMABILITY.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_PGA_INTEGRATOR_DISABLE(xml->registers.afe.spi.reg51.PGA_INTEGRATOR_DISABLE.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_PGA_CLAMP_LEVEL(xml->registers.afe.spi.reg51.PGA_CLAMP_LEVEL.Value())) < 0) { RLOG(result); }

    // AFE - SPI - REG52
    if ((result = sys_.afe->LoadRegisterAFE_ACTIVE_TERMINATION(xml->registers.afe.spi.reg52.ACTIVE_TERMINATION.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_ACT_TER_ENA_CTRL(xml->registers.afe.spi.reg52.ACT_TER_ENA_CTRL.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_PRESET_ACTIVE_TERMINATIONS(xml->registers.afe.spi.reg52.PRESET_ACTIVE_TERMINATIONS.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_ACTIVE_TERMINATION_ENABLE(xml->registers.afe.spi.reg52.ACTIVE_TERMINATION_ENABLE.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_LNA_INPUT_CLAMP_SETTING(xml->registers.afe.spi.reg52.LNA_INPUT_CLAMP_SETTING.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_LNA_INTEGRATOR_DISABLE(xml->registers.afe.spi.reg52.LNA_INTEGRATOR_DISABLE.Value())) < 0) { RLOG(result); }

    // AFE - SPI - REG59
    if ((result = sys_.afe->LoadRegisterAFE_HPF_LNA(xml->registers.afe.spi.reg59.HPF_LNA.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_CW_SUM_AMP_PDN(xml->registers.afe.spi.reg59.CW_SUM_AMP_PDN.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_PGA_TEST_MODE(xml->registers.afe.spi.reg59.PGA_TEST_MODE.Value())) < 0) { RLOG(result); }

    // AFE - SPI - REG61
    if ((result = sys_.afe->LoadRegisterAFE_V2I_CLAMP(xml->registers.afe.spi.reg61.V2I_CLAMP.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_LPF_5MHz(xml->registers.afe.spi.reg61.LPF_5MHz.Value())) < 0) { RLOG(result); }
    if ((result = sys_.afe->LoadRegisterAFE_PGA_CLAMP_minus6dBFS(xml->registers.afe.spi.reg61.PGA_CLAMP_minus6dBFS.Value())) < 0) { RLOG(result); }

    // AFE - BUSSAR
    if ((result = sys_.afe->LoadRegisterBUSSAR_EXTERNAL_TRIGGER(xml->registers.afe.bussar.external_trigger.Value())) < 0) { RLOG(result); }

    // PULSER
    if ((result = sys_.pulser->LoadRegisterPULSER_HIGH_Z(xml->registers.pulser.high_z.Value())) < 0) { RLOG(result); }
    if ((result = sys_.pulser->LoadRegisterPULSER_CURRENT(xml->registers.pulser.control.pulser_current.Value())) < 0) { RLOG(result); }
    if ((result = sys_.pulser->LoadRegisterPULSER_MAIN_DELAY(xml->registers.pulser.main_delay.Value())) < 0) { RLOG(result); }
    return EVCH_NONE;
}
// ----------------------------------------------------------------------------
/**
	ST_LoadConfig
	==============
Descripcion:
	Carga la configuracion desde un archivo. El archivo debe haber sido creado
	previamente con la funcion 'USC_SaveConfig'.

Parametros:
	(*file): Nombre del archivo.
	(*date): Devuelve la fecha y hora en la que la configuracion fue salvada.
				El formato es una cadena de caracteres (dd/mm/YYYY HH:MM:SS).
				El tamaño de esta cadena debe ser de 20 caracteres.

Devuelve:
	- La última posición leida en el archivo.
	- (Ver codigos de error en 'return.h')

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GetXMLConfigData (TXMLCFG_VirtualChannel *xml)
{
vector<unsigned char> tgc_curve;

   if (xml == NULL) return ELOG(charES2_FILE,ES2_FILE);
   XMLCFG_ResetData(xml);

// _____________________________________________________________________________
// ----- Virtual Channel Config

// .............................................................................
   xml->ut_parameters.gain.Set(GET_Gain());
   xml->ut_parameters.tgc_enabled.Set(GET_TGC_Enabled());
   xml->ut_parameters.tgc_type.Set(ST_GetTGCConfigType());
   xml->ut_parameters.tgc_attenuation.Set(GET_TGC_AttenuationCoefficient());
   xml->ut_parameters.prf_frequency.Set(cfg_.prf_time > 0 ? 1. / cfg_.prf_time : ST_MAX_PRFLIN);
   xml->ut_parameters.prf_burst_frequency.Set(cfg_.prf_burst_time > 0 ? 1. / cfg_.prf_burst_time : ST_MAX_PRFLIN);
   xml->ut_parameters.material_velocity.Set(GET_MaterialVelocity());
   xml->ut_parameters.acq_start_distance.Set(GET_AcqStartDistance());
   xml->ut_parameters.acq_end_distance.Set(GET_AcqEndDistance());
   xml->ut_parameters.acq_time_mode.Set(GET_AcquisitionTimeMode());
   xml->ut_parameters.acq_start_samples.Set(GET_AcqStartSamples());
   xml->ut_parameters.acq_n_samples.Set(GET_FP_AScanDataNumber());
   xml->ut_parameters.sampling_freq_decimation.Set(GET_DownSamplingFactor());
   xml->ut_parameters.acquisition_type.Set(GET_AcquisitionType());
   xml->ut_parameters.signal_mode.Set(GET_SignalMode());
// .............................................................................
// .............................................................................
   xml->pulser.pulser_enabled.Set(GET_PulserEnabled() ? 1 : 0);
   xml->pulser.pulses_number.Set(GET_PulsesNumber());
   xml->pulser.pulse_width.Set((int)GET_PulseWidth());
   xml->pulser.pulse_amplitude.Set(ST_GetPulseAmplitude());
// .............................................................................
// .............................................................................
   xml->probe.high_voltage_maximum.Set(ST_GetHighVoltageMaximum());
   xml->probe.probe_pitch.Set(GET_Probe_Pitch());
   xml->probe.probe_elements_number.Set(GET_Probe_ElementNumber());
   xml->probe.probe_frequency.Set(GET_Probe_Frequency());
   if (cfg_.pa_probe != NULL)
   {
       xml->probe.pin_to_channel_assignment.Set(cfg_.pa_probe->pin_to_channel_assignment_);
       if (cfg_.pa_probe->pin_to_channel_.size() != xml->probe.probe_elements_number.Value())
       {
           xml->probe.pin_to_channel_assignment.Set(1);
       }
       else xml->probe.pin_to_channel.Set(cfg_.pa_probe->pin_to_channel_);
   }
   else xml->probe.pin_to_channel_assignment.Set(1);
// .............................................................................
// .............................................................................
   xml->fp_focal_law.fp_focal_law_file.Set(GET_FP_EmissionFocalLawFile());
   xml->fp_focal_law.fp_focal_law_type.Set(GET_MC_FocalLawType());
   xml->fp_focal_law.fp_pwic_start_angle.Set(GET_PWIC_StartAngle());
   xml->fp_focal_law.fp_pwic_end_angle.Set(GET_PWIC_EndAngle());
   xml->fp_focal_law.fp_pwic_step_angle.Set(GET_PWIC_StepAngle());
   xml->fp_focal_law.fp_pwic_angles.Set(GET_PWIC_Angles());
// .............................................................................
// .............................................................................
   xml->pa_focal_law.pa_scan_type.Set(GET_PA_ScanType());
   xml->pa_focal_law.pa_aperture_size.Set(GET_PA_ReceptionAperture());
   xml->pa_focal_law.pa_start_element.Set(GET_PA_ReceptionStartElement());
   xml->pa_focal_law.pa_linear_step.Set(GET_PA_LinearStep());
   xml->pa_focal_law.pa_linear_end_element.Set(GET_PA_LinearEndElement());
   xml->pa_focal_law.pa_linear_angle.Set(GET_PA_LinearAngle());
   xml->pa_focal_law.pa_angular_start_angle.Set(GET_PA_AngularStartAngle());
   xml->pa_focal_law.pa_angular_end_angle.Set(GET_PA_AngularEndAngle());
   xml->pa_focal_law.pa_angular_step.Set(GET_PA_AngularStep());
   xml->pa_focal_law.pa_emission_focal_point.Set(GET_PA_EmissionFocalPoint());
   xml->pa_focal_law.pa_reception_focal_point.Set(GET_PA_ReceptionFocalPoint());
   xml->pa_focal_law.pa_focus_enabled.Set(GET_PA_FocusingEnabled());
   xml->pa_focal_law.pa_reception_auto_focus.Set(1);
   xml->pa_focal_law.pa_global_parallel_lines.Set(GET_PA_ParallelBeamformedLines());
   xml->pa_focal_law.pa_increased_acquired_samples.Set(GET_PA_IncreasedAcquiredSamples());
// .............................................................................
// .............................................................................
   xml->dsp.fir_enabled.Set(cfg_.fir.enabled);
   xml->dsp.fir_low_frequency.Set(cfg_.fir.low_frequency);
   xml->dsp.fir_high_frequency.Set(cfg_.fir.high_frequency);
// .............................................................................
// .............................................................................
   xml->fp_dsp.mc_processing_type.Set(cfg_.fp_processing_type);
   xml->fp_dsp.mc_average_signals_number.Set(GET_LOG2_AVERAGE());
   xml->pa_dsp.pa_processing_type.Set(GET_ProcessingType());
   xml->pa_dsp.pa_average_signals_number.Set(GET_ProcessingSignalsNumber_AVR());
   xml->pa_dsp.pa_emi_signals_number.Set(GET_ProcessingSignalsNumber_EMI());
// .............................................................................
// .............................................................................
   xml->tfm_focal_law.tfm_global_parallel_lines.Value(GET_TFM_ParallelBeamformedLines());
   xml->tfm_focal_law.tfm_emission_start_element.Value(GET_TFM_ForwardStartElement());
   xml->tfm_focal_law.tfm_emission_end_element.Value(GET_TFM_ForwardEndElement());
   xml->tfm_focal_law.tfm_emission_step_element.Value(GET_TFM_ForwardStepElement());
   xml->tfm_focal_law.tfm_receiver_angle.Value(GET_TFM_BackwardAngle());
   xml->tfm_focal_law.tfm_receiver_start_distance.Value(GET_TFM_BackwardStartDistance());
   xml->tfm_focal_law.tfm_receiver_end_distance.Value(GET_TFM_BackwardEndDistance());
   xml->tfm_focal_law.tfm_receiver_lines_number.Value(GET_TFM_BackwardLinesNumber());

	return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_PRINT_EmissionFocalLawDelaysFile(void)
{
    int i, j, c, n_ch = 0, n_fl = 0, n_delay = 0;
    FILE* f = NULL;
    AnsiString str;

    n_fl = cfg_.mc_emission_focal_law.size();
    for (c = i = 0; i < cfg_.mc_emission_focal_law.size(); i++)
    {
        if (i == 0) n_ch = cfg_.mc_emission_focal_law[i].size();
        else if (n_ch != cfg_.mc_emission_focal_law[i].size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
    }

    if ((f = fopen("EFL.TXT", "wt")) == NULL) return ELOG(charES2_FILE, ES2_FILE);
    for (i = 0; i < cfg_.mc_emission_focal_law.size(); i++)
    {
        for (j = 0; j < cfg_.mc_emission_focal_law[i].size(); j++)
        {
            if (cfg_.mc_emission_focal_law[i][j].emitter_enabled == 1) fprintf(f, "%.4f ", (float)cfg_.mc_emission_focal_law[i][j].emitter_delay);
            else fprintf(f, "%.4f ", (float)-1);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PrintFileConfig(FILE *f, bool sitau_info)
{
int n_fl, n_ch, i, l, b, c, n_encoders, data_int;
float prf_time;

    if (f == NULL) return ELOG(charEVCH_File, EVCH_File);

    n_encoders = ST_RangeEncodersNumber();
    n_ch = ST_GetChannelsNumber();

    fprintf (f, "\n____________________________________________________________________________________________________");
    fprintf (f, "\n--> Virtual Channel %d (%s)", cfg_.id, cfg_.name.c_str());
    fprintf (f, "\n    ------------------------------------------------------------------------------------------------");
    fprintf (f, "\n    Gain:                  %.2f dB", cfg_.gain_db);
    if (cfg_.specimen != NULL) fprintf (f, "\n    Material Velocity:     %.0f m/s", cfg_.specimen->GetVelocity());
    else fprintf (f, "\n    Material Velocity:     ERROR");
    switch (cfg_.acq_mode)
    {
    case 0: fprintf (f, "\n    Acq. Mode:             RF"); break;
    case 1: fprintf (f, "\n    Acq. Mode:             VIDEO"); break;
    case 2: fprintf (f, "\n    Acq. Mode:             Full Rectified"); break;
    case 3: fprintf (f, "\n    Acq. Mode:             Half Positive Rectified"); break;
    case 4: fprintf (f, "\n    Acq. Mode:             Half Negative Rectified"); break;
    default: fprintf (f, "\n    Acq. Mod:             ERROR"); break;
    }

    prf_time = GET_PRFTime();
    if (prf_time > 0) fprintf (f, "\n    A-Scan PRF             %d Hz", Round32((float)1. / prf_time));
    else fprintf (f, "\n    A-Scan PRF             0 Hz");

    fprintf (f, "\n    Sampling Frequency:    %.2f MHz", GET_SamplingFrequency());

    if (GET_PulserEnabled()) fprintf (f, "\n    Pulser:                %d V, %.0f ns, %d pulses (High Voltage Maximum: %d V)", ST_GetPulseAmplitude(), GET_PulseWidth(), GET_PulsesNumber(), ST_GetHighVoltageMaximum());
    else fprintf (f, "\n    Pulser:                DISABLED");

    fprintf(f, "\n\n    Start Acquisition Time:     %.2f us", cfg_.acq.acq_start_time);
    fprintf(f, "\n    Start Acquisition Distance: %.2f mm", cfg_.acq.acq_start_distance);
    fprintf(f, "\n    End Acquisition Time:       %.2f us", cfg_.acq.acq_end_time);
    fprintf(f, "\n    End Acquisition Distance:   %.2f mm", cfg_.acq.acq_end_distance);
    fprintf(f, "\n    Start Acquisition Samples:  %d", cfg_.acq.n_acq_start_samples);
    fprintf(f, "\n    Acquisition Samples:        %d", cfg_.acq.n_acq_samples);

    fprintf(f, "\n\n    Operation Mode:   (%d) ", UCIP.acquisition_mode);
    switch (UCIP.acquisition_mode)
    {
    case 0: fprintf(f, "Burst Mode"); break;
    case 1: fprintf(f, "Continuous Mode"); break;
    default: fprintf(f, "Unknown Mode"); break;
    }
    fprintf(f, "\n    Delay Sync Time:  %.1f sec - %d acquisitions", UCIP.delay_sync_time, UCIP.delay_sync_n_acq);

    if (cfg_.pa_probe != NULL)
    {
        fprintf(f, "\n\n    Probe Frequency:        %.2f MHz", cfg_.pa_probe->GetFrequency());
        fprintf(f, "\n    Probe Elements Number:  %d elements", cfg_.pa_probe->GetElementNumber());
        fprintf(f, "\n    Probe Pitch:            %.2f mm", cfg_.pa_probe->GetPitch());
    }

    switch (cfg_.acquisition_type)
    {
    case ACQUISITION_TYPE_PA:
        fprintf(f, "\n\n    ------------------------------------------------------------------------------------------------");
        fprintf(f, "\n    --> PHASED ARRAY MODE");
        fprintf(f, "\n    ---------------------");
        if (cfg_.fir.enabled) fprintf(f, "\n\n    FIR Filter:  Lower Freq.(%.3f MHz) - Higher Freq.(%.3f MHz)", cfg_.fir.low_frequency, cfg_.fir.low_frequency);
        else fprintf(f, "\n\n    FIR Filter:  DISABLED");

        switch (cfg_.processing_type)
        {
        case 0: fprintf(f, "\n    DSP Processing:  NONE"); break;
        case 1: fprintf(f, "\n    DSP Processing:  Average(%d)", cfg_.n_signals_processing_avr); break;
        case 2: fprintf(f, "\n    DSP Processing:  EMI(%d)", cfg_.n_signals_processing_emi); break;
        }

        if (cfg_.scan == NULL)
        {
            fprintf(f, "\n\n    ____________________________________________");
            fprintf(f, "\n    (Phased Array) FOCAL LAWS.- [Null Pointer].-");
        }
        else
        {
            fprintf(f, "\n\n    ___________________________");
            fprintf(f, "\n    (Phased Array) FOCAL LAWS.-");
            fprintf(f, "\n    Aperture: %d", GET_PA_ReceptionAperture());
            switch (GET_PA_ScanType())
            {
            case VCH_PA_FL_TYPE_LINEAR:
                fprintf(f, "\n    Type: Linear Scan");
                fprintf(f, "\n    Linear Scan: %d to %d (%d step)", GET_PA_ReceptionStartElement(), GET_PA_LinearEndElement(), GET_PA_LinearStep());
                break;
            case VCH_PA_FL_TYPE_ANGULAR:
                fprintf(f, "\n    Type: Angular Scan");
                fprintf(f, "\n    Start_Element: %d", GET_PA_ReceptionStartElement());
                fprintf(f, "\n    Angular Scan: %.2f deg to %.2f deg (%.2f deg step)", GET_PA_AngularStartAngle(), GET_PA_AngularStep(), GET_PA_AngularEndAngle());
                break;
            default: fprintf(f, "\n    Type: Unknown"); break;
            }

            fprintf(f, "\n    Emission Focus: %.2f mm", GET_PA_EmissionFocalPoint());
            fprintf(f, "\n    Reception Focus: %.2f mm", GET_PA_ReceptionFocalPoint());
        }

        fprintf(f, "\n\n    ________________________________________________________________________________________________");
        fprintf(f, "\n    ================================================================================================");
        fprintf(f, "\n    ____________________");
        fprintf(f, "\n    Emitter Delay (us).-\n");

        n_fl = cfg_.pa_emission_focal_law.size();
        for (c = i = 0; i < cfg_.pa_emission_focal_law.size(); i++)
        {
            if (i == 0) n_ch = cfg_.pa_emission_focal_law[i].size();
            else if (n_ch != cfg_.pa_emission_focal_law[i].size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        }
        fprintf(f, "\n          ");
        for (c = 0; c < n_ch; c++) fprintf(f, "(%7d) ", c);
        for (l = 0; l < n_fl; l++)
        {
            fprintf(f, "\n    (%3d)  ", l + 1);
            for (c = 0; c < n_ch; c++)
            {
                if (cfg_.pa_emission_focal_law[l][c].emitter_enabled == 1) fprintf(f, "%7.4f   ", cfg_.pa_emission_focal_law[l][c].emitter_delay);
                else fprintf(f, "-------   ");
            }
        }
        break;
    case ACQUISITION_TYPE_TFM:
        fprintf(f, "\n\n    ------------------------------------------------------------------------------------------------");
        fprintf(f, "\n    --> TFM MODE");
        fprintf(f, "\n    ------------");
        if (cfg_.fir.enabled) fprintf(f, "\n\n    FIR Filter:  Lower Freq.(%.3f MHz) - Higher Freq.(%.3f MHz)", cfg_.fir.low_frequency, cfg_.fir.low_frequency);
        else fprintf(f, "\n\n    FIR Filter:  DISABLED");

        switch (cfg_.processing_type)
        {
        case 0: fprintf(f, "\n    DSP Processing:  NONE"); break;
        case 1: fprintf(f, "\n    DSP Processing:  Average(%d)", cfg_.n_signals_processing_avr); break;
        case 2: fprintf(f, "\n    DSP Processing:  EMI(%d)", cfg_.n_signals_processing_emi); break;
        }

        fprintf(f, "\n    Start Distance: %.2f mm", cfg_.tfm.start_x_distance);
        fprintf(f, "\n    End Distance: %.2f mm", cfg_.tfm.end_x_distance);
        fprintf(f, "\n    Start Time: %.2f us", cfg_.tfm.start_x_time);
        fprintf(f, "\n    End Time: %.2f us", cfg_.tfm.end_x_time);
        fprintf(f, "\n    Angle: %.2fº", cfg_.tfm.angle);
        fprintf(f, "\n    N. Lines: %d", cfg_.tfm.n_lines);
        fprintf(f, "\n    Receiver Elements: [");
        for (i=0; i<cfg_.tfm.rec_elements.size(); i++) fprintf(f, " %d", cfg_.tfm.rec_elements[i]);
        fprintf(f, "]");
        fprintf(f, "\n    Emission Start Element: %d", cfg_.tfm.emi_start_element);
        fprintf(f, "\n    Emission End Element: %d", cfg_.tfm.emi_end_element);
        fprintf(f, "\n    Emission Step Element: %d", cfg_.tfm.emi_step_element);
        fprintf(f, "\n    Emitter Elements: [");
        for (i = 0; i < cfg_.tfm.emi_elements.size(); i++) fprintf(f, " %d", cfg_.tfm.emi_elements[i]);
        fprintf(f, "]");

        switch (cfg_.tfm.focal_laws_type)
        {
        case VCH_FP_FL_TYPE_FMC: fprintf(f, "\n    Type: FMC"); break;
        case VCH_FP_FL_TYPE_PWI: fprintf(f, "\n    Type: PWIC"); break;
        case VCH_FP_FL_TYPE_USER: fprintf(f, "\n    Type: User File"); break;
        default: fprintf(f, "\n    Type: Unknown"); break;
        }

        if (cfg_.tfm.focal_laws_type == VCH_FP_FL_TYPE_PWI)
        {
            if (cfg_.tfm.pwic_angles.size() > 0)
            {
                fprintf(f, "\n    PWIC Angles Config (º): [");
                for (i = 0; i < cfg_.tfm.pwic_angles.size(); i++)
                {
                    if (i < cfg_.tfm.pwic_angles.size() - 1) fprintf(f, "%.1f,", cfg_.tfm.pwic_angles[i]);
                    else fprintf(f, "%.1f]", cfg_.tfm.pwic_angles[i]);
                }
            }
            else fprintf(f, "\n    PWIC Angles Config (º): %.2f deg to %.2f deg (%.2f deg step)", cfg_.tfm.pwic_start_angle, cfg_.tfm.pwic_step_angle, cfg_.tfm.pwic_end_angle);
        }

        fprintf(f, "\n\n    ________________________________________________________________________________________________");
        fprintf(f, "\n    ================================================================================================");
        fprintf(f, "\n    ____________________");
        fprintf(f, "\n    Emitter Delay (us).-\n");

        n_fl = cfg_.tfm_emission_focal_law.size();
        for (c = i = 0; i < cfg_.tfm_emission_focal_law.size(); i++)
        {
            if (i == 0) n_ch = cfg_.tfm_emission_focal_law[i].size();
            else if (n_ch != cfg_.tfm_emission_focal_law[i].size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        }
        fprintf(f, "\n          ");
        for (c = 0; c < n_ch; c++) fprintf(f, "(%7d) ", c);
        for (l = 0; l < n_fl; l++)
        {
            fprintf(f, "\n    (%3d)  ", l + 1);
            for (c = 0; c < n_ch; c++)
            {
                if (cfg_.tfm_emission_focal_law[l][c].emitter_enabled == 1) fprintf(f, "%7.4f   ", cfg_.tfm_emission_focal_law[l][c].emitter_delay);
                else fprintf(f, "-------   ");
            }
        }
        break;
    case ACQUISITION_TYPE_MC:
        fprintf(f, "\n\n    ------------------------------------------------------------------------------------------------");
        fprintf(f, "\n    --> (MC) MATRIX CAPTURE MODE");
        fprintf(f, "\n    ----------------------------");
        data_int = GET_LOG2_AVERAGE();
        fprintf(f, "\n    Average:  2^%d = %d signals", data_int, (int)pow(2, data_int));

        fprintf(f, "\n\n    ___________________________________________________________________________");
        fprintf(f, "\n    (MC - Matrix Cature) %d FocalLaws/Image (%d Acquisitions = %d Burst x %d Images/Burst).-", GET_FocalLawNumber(), UCIP.trigger_acq_number, UCIP.n_bursts, UCIP.n_images_burst);
        switch (cfg_.fp_focal_laws_type)
        {
        case VCH_FP_FL_TYPE_FMC: fprintf(f, "\n    Type: FMC"); break;
        case VCH_FP_FL_TYPE_PWI: fprintf(f, "\n    Type: PWIC"); break;
        case VCH_FP_FL_TYPE_USER: fprintf(f, "\n    Type: User File"); break;
        default: fprintf(f, "\n    Type: Unknown"); break;
        }

        if (cfg_.fp_focal_laws_type == VCH_FP_FL_TYPE_PWI)
        {
            if (cfg_.fp_pwic_angles.size() > 0)
            {
                fprintf(f, "\n    PWIC Angles Config (º): [");
                for (i = 0; i < cfg_.fp_pwic_angles.size(); i++)
                {
                    if (i < cfg_.fp_pwic_angles.size() - 1) fprintf(f, "%.1f,", cfg_.fp_pwic_angles[i]);
                    else fprintf(f, "%.1f]", cfg_.fp_pwic_angles[i]);
                }
            }
            else fprintf(f, "\n    PWIC Angles Config (º): %.2f deg to %.2f deg (%.2f deg step)", cfg_.fp_pwic_start_angle, cfg_.fp_pwic_step_angle, cfg_.fp_pwic_end_angle);
        }

        fprintf(f, "\n\n    ________________________________________________________________________________________________");
        fprintf(f, "\n    ================================================================================================");
        fprintf(f, "\n    ____________________");
        fprintf(f, "\n    Emitter Delay (us).-\n");

        n_fl = cfg_.mc_emission_focal_law.size();
        for (c = i = 0; i < cfg_.mc_emission_focal_law.size(); i++)
        {
            if (i == 0) n_ch = cfg_.mc_emission_focal_law[i].size();
            else if (n_ch != cfg_.mc_emission_focal_law[i].size()) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);
        }
        fprintf(f, "\n          ");
        for (c = 0; c < n_ch; c++) fprintf(f, "(%7d) ", c);
        for (l = 0; l < n_fl; l++)
        {
            fprintf(f, "\n    (%3d)  ", l + 1);
            for (c = 0; c < n_ch; c++)
            {
                if (cfg_.mc_emission_focal_law[l][c].emitter_enabled == 1) fprintf(f, "%7.4f   ", cfg_.mc_emission_focal_law[l][c].emitter_delay);
                else fprintf(f, "-------   ");
            }
        }
        break;
    default:
        fprintf(f, "\n    (Unsupported Acquisition Mode %d).-", cfg_.acquisition_type);
        break;
    }

    if (sitau_info)
    {
        fprintf(f, "\n    ________________________________________________________________________________________________");
        fprintf(f, "\n    ================================================================================================");
        fprintf(f, "\n    ___________________________");
        fprintf(f, "\n    Pin To Element Assignment.-\n");
        if (cfg_.pa_probe != NULL && !cfg_.pa_probe->pin_to_channel_assignment_ && cfg_.pa_probe->pin_to_channel_.size() == cfg_.pa_probe->GetElementNumber())
        {
            for (i = 0; i < cfg_.pa_probe->pin_to_channel_.size(); i++)
            {
                fprintf(f, "\n    Pin (%3d) --> Element (%3d)", i, cfg_.pa_probe->pin_to_channel_[i]);
            }
        }
        else
        {
            for (i = 0; i < cfg_.pa_probe->GetElementNumber(); i++)
            {
                fprintf(f, "\n    Pin (%3d) --> Element (%3d)", i, i);
            }
        }
        fprintf(f, "\n    ________________________________________________________________________________________________");
        fprintf(f, "\n    ================================================================================================");

        if (sys_.pulser != NULL) sys_.pulser->PrintFileConfig(f);
        if (sys_.afe != NULL) sys_.afe->PrintFileConfig(f);

        for (b = 0; b < sys_.base.size(); b++)
        {
            if (sys_.base[b] != NULL) sys_.base[b]->PrintFileConfig(f);
        }
    }   
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::CONFIG_PrintFilePASystem(FILE *f, int id_pa_sys)
{
int n_fl = 0;

   if (f == NULL) return ELOG(charEVCH_File, EVCH_File);
   if (sys_.pa_sys != NULL && id_pa_sys == sys_.pa_sys->sys_.id)
   {
      fprintf (f, "\n\n   Virtual Channel %d (%s)",
         cfg_.id,
         cfg_.name.c_str());

      fprintf (f, "\n");
   }
   return n_fl;
}

