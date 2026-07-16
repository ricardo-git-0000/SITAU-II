// -----------------------------------------------------------------------------
/**
 @file c_virtual_channel.cpp

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

#define _USE_MATH_DEFINES
#define NOMINMAX

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <limits>
#include <math.h>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_virtual_channel.h"
#include "st_error_code.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "protocol_uci.h"
#include "st_reg.h"
#include "st_range.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "st_calc.h"
#include "st_load.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "sitau_hw.h"
#include "c_log_protocol.h"
#include "c_xml_sitau_hw.h"
#include "c_datalogger.h"
#include "st_thread_status.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

vector<CVirtualChannel *> gb_virtual_channel;
vector<CVirtualChannel *> gb_vch_load_config;
int gb_selected_virtual_channel = 0;

// _____________________________________________________________________________
// ------ CVirtualChannel class Methods
// _____________________________________________________________________________
// =============================================================================
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CVirtualChannel::CVirtualChannel (int id, int id_pa_sys, int id_connector, AnsiString name, bool hw_format)
{
   cfg_.id = id;
   previous_ = NULL;
   next_ = NULL;
   sys_.pulser = NULL;
   sys_.afe = NULL;
   cfg_.pa_probe = NULL;
   cfg_.specimen = NULL;
   cfg_.scan = NULL;
   buffer_mc_hwut_ = new CBufferMC_ScanHWUT(cfg_.id, &cfg_, &sys_, hw_format);
   buffer_pa_hwut_ = new CBufferPA_ScanHWUT(cfg_.id, &cfg_);
   buffer_tfm_hwut_ = new CBufferTFM_ScanHWUT(cfg_.id, &cfg_);
   memory_resize_enabled_ = false;
   cfg_.fir.enabled = 0;
   cfg_.fir.low_frequency = 0;
   cfg_.fir.high_frequency = 30;
   cfg_.fp_focal_law_file = "";
   cfg_.fp_processing_type = 0;
   //cfg_.fp_fir.enabled = 0;
   //cfg_.fp_fir.low_frequency = 0;
   //cfg_.fp_fir.high_frequency = 30;
   ResetRegister();
   Setup(id_pa_sys, id_connector, name);
   memory_resize_enabled_ = true;
   update_dsp_scan_data_ = false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CVirtualChannel::~CVirtualChannel()
{
int i;

   if (sys_.afe != NULL) delete sys_.afe;
   sys_.afe = NULL;

   if (sys_.pulser != NULL) delete sys_.pulser;
   sys_.pulser = NULL;

   for (i=0; i<sys_.base.size(); i++)
   {
      if (sys_.base[i] != NULL) delete sys_.base[i];
      sys_.base[i] = NULL;
   }
   sys_.base.clear();

   if (cfg_.scan != NULL) delete cfg_.scan;
   cfg_.scan = NULL;

   if (cfg_.pa_probe != NULL) delete cfg_.pa_probe;
   cfg_.pa_probe = NULL;

   if (cfg_.specimen != NULL) delete cfg_.specimen;
   cfg_.specimen = NULL;

   if (buffer_mc_hwut_ != NULL)
   {
      buffer_mc_hwut_->MemoryFree();
      delete buffer_mc_hwut_;
      buffer_mc_hwut_ = NULL;
   }
   if (buffer_pa_hwut_ != NULL)
   {
      buffer_pa_hwut_->MemoryFree();
      delete buffer_pa_hwut_;
      buffer_pa_hwut_ = NULL;
   }
   if (buffer_tfm_hwut_ != NULL)
   {
       buffer_tfm_hwut_->MemoryFree();
       delete buffer_tfm_hwut_;
       buffer_tfm_hwut_ = NULL;
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::ResetRegister(void)
{
      sys_.tgc.n_points = 0;
      sys_.tfm_fmc_forward.n_lword = 0;
      sys_.tfm_pwi_forward.n_lword = 0;
      if (sys_.pulser == NULL) sys_.pulser = new C_PULSER();
      if (sys_.afe == NULL) sys_.afe = new C_AFE();
      return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::HardwareSetup(int id_pa_sys, int id_connector)
{
int i, id_base, n_ch, id_first_base = 0, id_last_base = 0, n_bases = 0, n_total_modules=0;
bool multiplexed;

   sys_.id_pa_sys = 0;
   sys_.id_connector = 0;
   sys_.pa_sys = NULL;
   if ((sys_.pa_sys = STHW_GetPtrPASystem(id_pa_sys)) == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);

   sys_.id_pa_sys = id_pa_sys;
   sys_.id_connector = id_connector;

   cfg_.enabled = true;

   id_first_base = sys_.pa_sys->GetBASEFirst();
   id_last_base = sys_.pa_sys->GetBASELast();
   n_bases = sys_.pa_sys->GetBASENumber();
   multiplexed = sys_.pa_sys->Multiplexed();
   n_ch = sys_.pa_sys->GetChannelsNumber();

   n_total_modules = sys_.pa_sys->GetTotalMODULENumber();
   if (multiplexed && n_bases != 1) return ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

   if (sys_.pulser == NULL) sys_.pulser = new C_PULSER();
   if (sys_.afe == NULL) sys_.afe = new C_AFE();

   if (sys_.base.size() != n_bases)
   {
      for (i=0; i<sys_.base.size(); i++)
      {
         if (sys_.base[i] != NULL) delete sys_.base[i];
         sys_.base[i] = NULL;
      }
      sys_.base.resize(n_bases);
      for ( i = 0,
            id_base = id_first_base;
               id_base <= id_last_base &&
               i < sys_.base.size();
                  i++,
                  id_base++)
         sys_.base[i] = new CBASE_VirtualChannel(id_base, multiplexed);
   }
   else
   {
      for ( i = 0,
            id_base = id_first_base;
               id_base <= id_last_base &&
               i < sys_.base.size();
                  i++,
                  id_base++)
         sys_.base[i]->SetIdBase(id_base, multiplexed);
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CVirtualChannel::UpdateConfig(void)
{
   update_register_ = true;
   update_focal_law_fp_ = true;
   update_focal_law_pa_ = true;
   update_tfm_forward_ = true;
   update_tfm_backward_ = true;
   update_tgc_ = true;
   update_fir_filter_ = true;
   //update_signals_processing_ = true;
   update_next_vch_amplia_command_ = true;
   update_previous_vch_amplia_command_ = true;
   updating_amplia_command_ = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::Setup(int id_pa_sys, int id_connector, AnsiString name)
{
int result;

   cfg_.name = name;

   UpdateConfig();

   cfg_.auto_pulse_width = false;
   cfg_.acq_time_mode = 0;
   cfg_.acquisition_type = ACQUISITION_TYPE_MC;
   cfg_.signal_mode = 0;

   cfg_.fp_focal_laws_type = VCH_FP_FL_TYPE_FMC;

   cfg_.specimen = new CFL_SpecimenUI(1540);
   cfg_.pa_probe = new CFL_PAUI(128, 0.3, 0, 10, 0);
   cfg_.probe_aperture_samples = 0;

   cfg_.scan = new CFL_ScanUI(cfg_.pa_probe, cfg_.specimen);

   cfg_.fp_pwic_start_angle = -15;
   cfg_.fp_pwic_step_angle = 1;
   cfg_.fp_pwic_end_angle = 15;
   cfg_.fp_pwic_angles.clear();
   for (float angle = cfg_.fp_pwic_start_angle; angle <= cfg_.fp_pwic_end_angle; angle += cfg_.fp_pwic_step_angle) cfg_.fp_pwic_angles.push_back(angle);

   cfg_.acq_mode = ACQ_PulseEcho;

   //cfg_.gain_digital_db = 0;
   cfg_.gain_db = 0;

   cfg_.prf_time = (float)1. / (float)3000.;
   cfg_.prf_burst_time = (float)1. / (float)3000.;
   
   cfg_.prf_enabled = 1;
   cfg_.prf_burst_enabled = 1;

   cfg_.tfm.start_x_distance = 0;
   cfg_.tfm.end_x_distance = 0;
   cfg_.tfm.start_x_time = 0;
   cfg_.tfm.end_x_time = 0;
   cfg_.tfm.angle = 0;
   cfg_.tfm.image_size_user = 0;
   cfg_.tfm.image_size_hw = 0;
   cfg_.tfm.frame_size = 0;
   cfg_.tfm.n_lines = 0;
   cfg_.tfm.emi_start_element = 0;
   cfg_.tfm.emi_end_element = 127;
   cfg_.tfm.emi_step_element = 1;
   cfg_.tfm.pwic_start_angle = -15;
   cfg_.tfm.pwic_step_angle = 1;
   cfg_.tfm.pwic_end_angle = 15;
   cfg_.tfm.pwic_angles.clear();
   for (float angle = cfg_.tfm.pwic_start_angle; angle <= cfg_.tfm.pwic_end_angle; angle += cfg_.tfm.pwic_step_angle) cfg_.tfm.pwic_angles.push_back(angle);

   cfg_.fir.enabled = false;
   cfg_.fir.low_frequency = 0;
   cfg_.fir.high_frequency = 30000;
   RELOAD_RFFilter();

   cfg_.processing_type = 0;
   cfg_.wait_prf_signals_processing = 0;
   cfg_.n_signals_processing_avr = 8;
   cfg_.n_signals_processing_emi = 3;

   cfg_.sampling_frequency = (float)FCLK;
   cfg_.scan->SetSamplingFreq((float)FCLK);

   cfg_.tgc_enabled = 0;
   cfg_.digital_tgc_attenuator = 0;
   cfg_.tgc_attn_coefficient = 0;
   cfg_.tgc_type = 0;

   if ((result = HardwareSetup(id_pa_sys, id_connector)) < 0) return RLOG(result);

   if ((result = LOAD_DefaultConfig()) < 0) return RLOG(result);

   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_UpdateAcquisitionSize (void)
{
int i, j, n_bases=0, n_ch, n_fl, gtx_nfl_blocks;
unsigned long size_header_img_pa;

   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   if ((n_bases = STHW_GetHWBASENumber()) < 0) return RLOG(n_bases);
   if ((n_ch = GET_ChannelsNumber()) < 0) return RLOG(n_ch);
   n_fl = cfg_.mc_emission_focal_law.size();

   size_header_img_pa = 0;
   // Start Start Control Word (0xCEBX)
   size_header_img_pa += 2;
   // Id. Virtual Channel
   size_header_img_pa += 2;
   // Acquisition Size
   size_header_img_pa += 2;
   // Id. Focal Law
   size_header_img_pa += 2;
   // N. AScan Signals
   size_header_img_pa += 2;
   // N. AScan Samples
   size_header_img_pa += 2;
   // Acquisition Counter
   size_header_img_pa += 2;
   // Offset Encoder Trigger
   size_header_img_pa += 2;
   // Start End Control Word (0xC0C0)
   size_header_img_pa += 2;

   cfg_.fp_acquisition_size = 0; 
   if (n_fl > 0)
   {
       cfg_.fp_focal_law.resize(n_fl);
       for (i = 0; i < n_fl; i++)
       {
           cfg_.fp_focal_law[i].n_ascan = n_ch;
           cfg_.fp_focal_law[i].image_size = 0;
           for (j = 0; j < cfg_.fp_focal_law[i].n_ascan; j++)
           {
               if (cfg_.acq.n_acq_samples % 2 != 0) cfg_.acq.n_acq_samples++;
               cfg_.fp_focal_law[i].image_size += (unsigned long)cfg_.acq.n_acq_samples;
           }

           cfg_.fp_focal_law[i].frame_size = (cfg_.fp_focal_law[i].image_size + size_header_img_pa) / 2;
           cfg_.fp_acquisition_size += (unsigned long)cfg_.fp_focal_law[i].image_size;
       }
       cfg_.gtx_nfl_blocks = (GTX_SIZE_BLOCK - GTX_SIZE_HEADER) / (cfg_.acq.n_acq_samples * n_ch * 2);
       if (UCIP.n_images_burst * n_fl < cfg_.gtx_nfl_blocks) cfg_.gtx_nfl_blocks = UCIP.n_images_burst * n_fl;
       else if (UCIP.pause_after_burst > 0)
       {
           gtx_nfl_blocks = cfg_.gtx_nfl_blocks;
           while ((UCIP.n_images_burst * n_fl) % gtx_nfl_blocks) gtx_nfl_blocks--;
           cfg_.gtx_nfl_blocks = gtx_nfl_blocks;
       }
       cfg_.gtx_n_blocks_burst = UCIP.n_images_burst * n_fl / cfg_.gtx_nfl_blocks;
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PA_UpdateAcquisitionSize (void)
{
int n_parallel_lines, n_blocks_32b, n_blocks, n_fl, n_bases=0;
unsigned long size_header_img_pa;

#if !defined(_PA)
    return EVCH_NONE;
#endif

   n_fl = cfg_.pa_focal_law.beamformer.size();
   cfg_.pa_focal_law.image_size_user = 0;
   cfg_.pa_focal_law.image_size_hw = 0;
   cfg_.pa_focal_law.frame_size = 0;
   if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   if (cfg_.scan == NULL) return EVCH_NONE;
   
   n_bases = STHW_GetHWBASENumber();

   if (cfg_.acq.n_beamformed_samples % 2 != 0) cfg_.acq.n_beamformed_samples++;
   //if ((result = cfg_.scan->GetReceptionFocalLawNumber()) < 0) return RLOG(result);

   cfg_.pa_focal_law.image_size_user = n_fl * (unsigned long)cfg_.acq.n_beamformed_samples;

   n_parallel_lines = GET_PA_ParallelBeamformedLines();   
   n_blocks = Round32u((float)n_fl / (float)n_parallel_lines + (float)0.5);
   n_blocks_32b = Round32u((float)n_blocks / (float)4 + (float)0.5);

   size_header_img_pa = 0;
   // Start Start Control Word (0xCEBX)
   size_header_img_pa += 2;
   // Id. Virtual Channel
   size_header_img_pa += 2;
   // Acquisition Size
   size_header_img_pa += 2;
   // Id. Focal Law
   size_header_img_pa += 2;
   // N. AScan Signals
   size_header_img_pa += 2;
   // N. AScan Samples
   size_header_img_pa += 2;
   // N. Blocks
   size_header_img_pa += 1;
   // N. Blocks (32 bits)
   size_header_img_pa += 1;
   // N. Lines Blocks
   size_header_img_pa += (n_blocks_32b * 2);
   // Acquisition Counter
   size_header_img_pa += 2;
   // Offset Encoder Trigger
   size_header_img_pa += 2;
   // Start End Control Word (0xC0C0)
   size_header_img_pa += 2;
  
   //if ((data_int = cfg_.pa_focal_law.n_lines_user % n_parallel_lines) > 0)
   //   data_int = n_parallel_lines - data_int;
   //data_int = 0;
   //cfg_.pa_focal_law.n_lines_hw = cfg_.pa_focal_law.n_lines_user + data_int;
   cfg_.pa_focal_law.image_size_hw = n_fl * (unsigned long)cfg_.acq.n_beamformed_samples;

   cfg_.pa_focal_law.frame_size = (cfg_.pa_focal_law.image_size_hw + size_header_img_pa) / 2;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_UpdateAcquisitionSize(void)
{
    int n_parallel_lines, n_blocks_32b, n_blocks, n_fl, n_bases = 0;
    unsigned long size_header_img_pa;

#if !defined(_PA)
    return EVCH_NONE;
#endif

    n_fl = cfg_.tfm_backward.beamformer.size();
    cfg_.tfm.image_size_user = 0;
    cfg_.tfm.image_size_hw = 0;
    cfg_.tfm.frame_size = 0;
    if (sys_.pa_sys == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
    if (cfg_.scan == NULL) return EVCH_NONE;

    n_bases = STHW_GetHWBASENumber();

    if (cfg_.acq.n_beamformed_samples % 2 != 0) cfg_.acq.n_beamformed_samples++;
    cfg_.tfm.image_size_user = n_fl * (unsigned long)cfg_.acq.n_beamformed_samples;

    n_parallel_lines = GET_TFM_ParallelBeamformedLines();
    n_blocks = Round32u((float)n_fl / (float)n_parallel_lines + (float)0.5);
    n_blocks_32b = Round32u((float)n_blocks / (float)4 + (float)0.5);

    size_header_img_pa = 0;
    // Start Start Control Word (0xCEBX)
    size_header_img_pa += 2;
    // Id. Virtual Channel
    size_header_img_pa += 2;
    // Acquisition Size
    size_header_img_pa += 2;
    // Id. Focal Law
    size_header_img_pa += 2;
    // N. AScan Signals
    size_header_img_pa += 2;
    // N. AScan Samples
    size_header_img_pa += 2;
    // N. Blocks
    size_header_img_pa += 1;
    // N. Blocks (32 bits)
    size_header_img_pa += 1;
    // N. Lines Blocks
    size_header_img_pa += (n_blocks_32b * 2);
    // Acquisition Counter
    size_header_img_pa += 2;
    // Offset Encoder Trigger
    size_header_img_pa += 2;
    // Start End Control Word (0xC0C0)
    size_header_img_pa += 2;

    cfg_.tfm.image_size_hw = n_fl * (unsigned long)cfg_.acq.n_beamformed_samples;
    cfg_.tfm.frame_size = (cfg_.tfm.image_size_hw + size_header_img_pa) / 2;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::UpdateAcquisitionSize (void)
{
int result;

   if ((result = FP_UpdateAcquisitionSize()) < 0) return RLOG(result);
   if ((result = PA_UpdateAcquisitionSize()) < 0) return RLOG(result);
   if ((result = TFM_UpdateAcquisitionSize()) < 0) return RLOG(result);
   if ((result = UpdateAScanSamplesPosition(&cfg_.acq)) < 0) return RLOG(result);
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::Set(bool update)
{
int result;
int acquiring = ST_Acquiring();
bool stop = false;
bool locked = IsLockedStatusThread();

   if (!gb_load_config_enabled) return ES2_NONE;
   if ((result = UpdateAcquisitionSize()) < 0) return RLOG(result);
   if (update)
   {
      update_register_ = true;
      update_focal_law_fp_ = true;
      update_focal_law_pa_ = true;
      update_tgc_ = true;
      update_fir_filter_ = true;
      update_next_vch_amplia_command_ = true;
   }
   //stop = acquiring == 1 && (update_register_ || update_tgc_ || update_next_vch_amplia_command_);

   //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

   if (stop && (result = ST_Stop()) < 0) { RLOG(result); }

   if ((result = Set_Registers()) < 0) return RLOG(result);
   if ((result = SET_EmissionFocalLaws()) < 0) return RLOG(result);
#if defined(_PA)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_PA)
   {
       if ((result = SET_Beamformer()) < 0) return RLOG(result);
   }
#endif
   //if ((result = Set_TGC()) < 0) return RLOG(result);
   if ((result = Set_FIRFilter()) < 0) return RLOG(result);
#if defined(_TFM)
   if (cfg_.acquisition_type == ACQUISITION_TYPE_TFM)
   {
       switch (cfg_.tfm.focal_laws_type)
       {
       case VCH_FP_FL_TYPE_FMC:
           if ((result = TFM_SET_FMC_Forward()) < 0) return RLOG(result);
           break;
       case VCH_FP_FL_TYPE_PWI:
           if ((result = TFM_SET_PWI_Forward()) < 0) return RLOG(result);
           break;
       }
       if ((result = SET_TFMBackward()) < 0) return RLOG(result);
   }
#endif
   if (previous_ != NULL && update_previous_vch_amplia_command_)
   {
      updating_amplia_command_ = true;
      updating_amplia_command_ = false;
      update_previous_vch_amplia_command_ = false;
   }
    if (STDL_GetConfig())
    {
        FP_PRINT_EmissionFocalLawDelaysFile();
        if (log_protocol != NULL) log_protocol->Print("CONFIG_UCI.TXT");
        //FP_PRINT_EmissionFocalLawFile();
    }
    if (stop) ST_Start();

    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::Set_Registers(void)
{
int b, result;
int id_base_first = 0, id_base_last = 0;
TMSG_Config *msg = NULL;
int acquiring = ST_Acquiring();

try
{
   if (update_register_)
   {
      if ((msg = (TMSG_Config *)malloc(sizeof(TMSG_Config))) == NULL) return ELOG(charES2_NO_MEM, ES2_NO_MEM);

      msg->id_vch = cfg_.id;
      if (cfg_.enabled) msg->enabled = 1;
      else msg->enabled = 0;

      msg->n_bases = sys_.base.size();
      msg->acquisition_type = cfg_.acquisition_type;
      msg->signal_mode = cfg_.signal_mode;
      msg->gtx_n_focal_law = cfg_.gtx_nfl_blocks;
      msg->start_element = 0;

      if (cfg_.acq.n_acq_samples == 0 || cfg_.acq.n_beamformed_samples == 0)
      {
         if (cfg_.acq.acq_start_time < 0 || cfg_.acq.acq_end_time <= 0)
            return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);

         if ((result = LOAD_AcqTime (cfg_.acq.acq_start_time, cfg_.acq.acq_end_time)) < 0)
            return RLOG(result);
      }
      if (cfg_.acq.n_acq_samples % 2 != 0) cfg_.acq.n_acq_samples++;
      msg->n_acq_samples = cfg_.acq.n_acq_samples;
      if (cfg_.acq.n_beamformed_samples % 2 != 0) cfg_.acq.n_beamformed_samples++;
      msg->n_beamformed_samples = cfg_.acq.n_beamformed_samples;

      if (cfg_.prf_enabled == 1) msg->prf_time_line = cfg_.prf_time * 1000000;
      else msg->prf_time_line = 0;
      if (msg->prf_time_line < 10) msg->prf_time_line = 10;

      if (cfg_.prf_burst_enabled == 1) msg->prf_time_burst = cfg_.prf_burst_time * 1000000;
      else msg->prf_time_burst = 0;
      if (msg->prf_time_burst < 10) msg->prf_time_burst = 10;

      msg->gain_da = 0;
      msg->gain_hi = 0;

      msg->processing_type = cfg_.processing_type;
      msg->wait_prf_signals_processing = cfg_.wait_prf_signals_processing;
      msg->n_signals_processing_avr = cfg_.n_signals_processing_avr;
      msg->n_signals_processing_emi = cfg_.n_signals_processing_emi;

      msg->enabled_timestamp = 0;
      msg->enabled_buffer_dma = 0;
      msg->enabled_temperature = 0;
      msg->enabled_encoder_1 = 0;
      msg->enabled_encoder_2 = 0;
      msg->enabled_encoder_3 = 0;
      msg->enabled_encoder_4 = 0;

      if (sys_.afe == NULL) return ELOG(charEVCH_CONFIG_NULL, EVCH_CONFIG_NULL);
      msg->afe_spi.reg21.REG = sys_.afe->reg_.spi.reg21.REG;
      msg->afe_spi.reg33.REG = sys_.afe->reg_.spi.reg33.REG;
      msg->afe_spi.reg51.REG = sys_.afe->reg_.spi.reg51.REG;
      msg->afe_spi.reg52.REG = sys_.afe->reg_.spi.reg52.REG;
      msg->afe_spi.reg59.REG = sys_.afe->reg_.spi.reg59.REG;
      msg->afe_spi.reg61.REG = sys_.afe->reg_.spi.reg61.REG;

      // ------ AFE - BUSSAR
      msg->external_trigger = sys_.afe->reg_.bussar.external_trigger;
      msg->decimation_factor = sys_.afe->reg_.bussar.decimation_ratio;
      msg->n_acq_samples = sys_.afe->reg_.bussar.n_samples;
      msg->water_delay = sys_.afe->reg_.bussar.water_delay;
      msg->log2_average = sys_.afe->reg_.bussar.log2_average;

      // ------ PULSER
      if (sys_.pulser == NULL) return ELOG(charEVCH_CONFIG_NULL, EVCH_CONFIG_NULL);
      msg->pulser.enabled = sys_.pulser->reg_.enabled;
      msg->pulser.high_z = sys_.pulser->reg_.high_z;
      msg->pulser.current = sys_.pulser->reg_.current;
      msg->pulser.n_pulses = sys_.pulser->reg_.n_pulses;
      msg->pulser.pulse_width = sys_.pulser->reg_.pulse_width;
      msg->pulser.main_delay = sys_.pulser->reg_.main_delay;

      for (b=0; b<sys_.base.size() && b<MAX_N_BASES_MSG; b++)
      {
         if (sys_.base[b] != NULL)
         {
            msg->base_reg[b].enabled = 1;
            if ((result = sys_.base[b]->GetRegisters(&msg->base_reg[b])) < 0) return RLOG(result);
         }
         else msg->base_reg[b].enabled = 0;
      }
      for (; b<MAX_N_BASES_MSG; b++) msg->base_reg[b].enabled = 0;

      msg->n_mux_inc = 1;

      if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
      if ((result = TCPSend_MSG_VCH_Config(msg)) < 0) return RLOG(result);
      if (acquiring == 1) ST_Start();
      update_register_ = false;
      update_previous_vch_amplia_command_ = true;
   }
}
__finally
{
   if (msg != NULL) free(msg);
}
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::Set_FIRFilter(void)
{
   int i, n_bases, result;
   int id_base_first = 0, id_base_last = 0;// id_mod_first = 0, id_mod_last = FP_BASE_NUM_MOD;
   TMSG_FIR *msg = NULL;
   //int acquiring = ST_Acquiring();

   try
   {
      if (update_fir_filter_)
      {
         n_bases = id_base_last - id_base_first + 1;

         if ((msg = (TMSG_FIR*)malloc(sizeof(TMSG_FIR))) == NULL) return ELOG(charES2_NO_MEM, ES2_NO_MEM);

         msg->id_vch = cfg_.id;
         msg->n_coefficients = FIR_N_COEFFICIENTS;

         msg->enabled = cfg_.fir.enabled ? 1 : 0;
         for (i = 0; i < FIR_N_COEFFICIENTS && i < cfg_.fir.hw_coefficients.size(); i++)
             msg->coefficients[i] = cfg_.fir.hw_coefficients[i];
         //switch (cfg_.acquisition_type)
         //{
         //case ACQUISITION_TYPE_TFM:
         //    break;
         //case ACQUISITION_TYPE_PA:
         //    msg->enabled = cfg_.fir.enabled ? 1 : 0;
         //    for (i = 0; i < FIR_N_COEFFICIENTS && i < cfg_.fir.hw_coefficients.size(); i++)
         //        msg->coefficients[i] = cfg_.fir.hw_coefficients[i];
         //    break;
         //default:
         //    msg->enabled = cfg_.fp_fir.enabled ? 1 : 0;
         //    for (i = 0; i < FIR_N_COEFFICIENTS && i < cfg_.fp_fir.hw_coefficients.size(); i++)
         //        msg->coefficients[i] = cfg_.fp_fir.hw_coefficients[i];
         //    break;
         //}

         //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
         if ((result = TCPSend_MSG_VCH_FIR(msg)) < 0) return RLOG(result);
         //if (acquiring == 1) ST_Start();
         update_fir_filter_ = false;
         update_previous_vch_amplia_command_ = true;
      }
   }
   __finally
   {
      if (msg != NULL) free(msg);
   }
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_EmissionFocalLaws(void)
{
int b, c, idx, result, connector_start_channel;
int id_base_first = 0, id_base_last = 0;
TMSG_EmissionFocalLaws *msg = NULL;
TMSG_EmissionFocalLawsExtended* msg_ext = NULL;
int acquiring = ST_Acquiring();

try
{
   if (update_focal_law_fp_)
   {
      if (sys_.pa_sys != NULL)
      {
         if ((connector_start_channel = sys_.pa_sys->GetConnectorStartChannel(sys_.id_connector)) < 0)
            return RLOG(connector_start_channel);
      }
      else return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
      
      if ((msg = (TMSG_EmissionFocalLaws*)malloc(sizeof(TMSG_EmissionFocalLaws))) == NULL)
         return ELOG(charES2_NO_MEM, ES2_NO_MEM);

      msg->id_vch = cfg_.id;
      msg->n_bases = sys_.base.size();

      for (c=b=0; b<sys_.base.size() && b < MAX_N_BASES_MSG; b++)
      {
         if (sys_.base[b] != NULL)
         {
            for (idx=0; idx < FP_BASE_MAX_FOCAL_LAW * MOD_MAX_CH && idx<sys_.base[b]->reg_.fp_fl.efl.delay.size(); idx++)
                  msg->base[b].efl[idx] = sys_.base[b]->reg_.fp_fl.efl.delay[idx];
            msg->n_fl = sys_.base[b]->reg_.fp_fl.efl.n_fl;
         }
      }

      if (b < sys_.base.size())
      {
          if ((msg_ext = (TMSG_EmissionFocalLawsExtended*)malloc(sizeof(TMSG_EmissionFocalLawsExtended))) == NULL)
              return ELOG(charES2_NO_MEM, ES2_NO_MEM);

          msg_ext->id_vch = cfg_.id;
          msg_ext->id_first_base = b;
          msg_ext->n_bases = sys_.base.size();

          for (c = b = 0; b < sys_.base.size() && b < MAX_N_BASES_MSG; b++)
          {
              if (sys_.base[msg_ext->id_first_base + b] != NULL)
              {
                  for (idx = 0; idx < FP_BASE_MAX_FOCAL_LAW * MOD_MAX_CH && idx < sys_.base[msg_ext->id_first_base + b]->reg_.fp_fl.efl.delay.size(); idx++)
                      msg_ext->base[b].efl[idx] = sys_.base[msg_ext->id_first_base + b]->reg_.fp_fl.efl.delay[idx];
                  msg_ext->n_fl = sys_.base[msg_ext->id_first_base + b]->reg_.fp_fl.efl.n_fl;
              }
          }
      }

      if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
      if ((result = TCPSend_MSG_VCH_EmissionFocallaws(msg)) < 0) return RLOG(result);
      if (msg_ext != NULL && (result = TCPSend_MSG_VCH_EmissionFocallawsExtended(msg_ext)) < 0) return RLOG(result);
      if (acquiring == 1) ST_Start();
      update_focal_law_fp_ = false;
   }
}
__finally
{
   if (msg != NULL) free(msg);
}
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::Set_TGC(void)
{
int i, n_bases, result;
int id_base_first = 0, id_base_last = 0;
TMSG_TGC *msg = NULL;
//int acquiring = ST_Acquiring();

try
{
   if (update_tgc_)
   {
      n_bases = id_base_last - id_base_first + 1;

      if ((msg = (TMSG_TGC*)malloc(sizeof(TMSG_TGC))) == NULL) return ELOG(charES2_NO_MEM, ES2_NO_MEM);

      msg->id_vch = cfg_.id;
      msg->n_points = sys_.tgc.n_points;
      for (i = 0; i < sys_.tgc.n_points && i < TGC_MAX_SIZE; i++)
         msg->curve[i] = sys_.tgc.curve[i];

      //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
      if ((result = TCPSend_MSG_VirtualChannel_TGC(msg)) < 0) return RLOG(result);
      //if (acquiring == 1) ST_Start();
      update_register_ = false;
      update_previous_vch_amplia_command_ = true;
   }
}
__finally
{
   if (msg != NULL) free(msg);
}
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//Calculo los límites del rango de adquisición y de la velocidad sin tener en cuenta la restricción por suela,
//Sólo estoy considerando la restricción por número de líneas
float CVirtualChannel::AcquisitionDistanceMax (void)
{
float result, range_nmu;
int acq_mode;
float sampling_frequency;
float material_velocity;

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity() / (float)1000.;

   if ((acq_mode = GET_AcquisitionTimeMode()) < 0) return (float)RLOG(acq_mode);
   if ((sampling_frequency = GET_SamplingFrequency()) <= 0)
      return (float)ELOG(charEVCH_DataInconsistency, EVCH_DataInconsistency);

	// Comprobar que el número de muestras después del diezmado no es mayor de 20478
   // Aplicamos un factro de seguridad de 400 muestras
   range_nmu = material_velocity * ((float)MAX_NMU_OUT - 400) / sampling_frequency;

    result = range_nmu;
    if (result < 1) result = 1;
    if (acq_mode == ACQ_PulseEcho || acq_mode == ACQ_PitchCatch) result = result / (float)2.;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::MinDownSamplingFactor (float *fs_max)
{
int n_samples, result;
float start_range=10000000, end_range=0, range;
bool fin;
float material_velocity;

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity() / (float)1000.;

   start_range = GET_AcqStartDistance();
   end_range = GET_AcqEndDistance();
   range = end_range - start_range;

   result = ST_RangeMinDownSamplingFactor();
   if (fs_max != NULL) *fs_max = (float)FCLK / result;

	//Comprobar que el número de muestras después del diezmado no es mayor de 20478
   // Aplicamos un factro de seguridad de 400 muestras
	n_samples = Round32(range / material_velocity * (float)FCLK);
   fin = false;
   while (!fin)
   {
      if (RoundUp32((float)n_samples/(float)result) > (MAX_NMU_OUT - 400)) result++;
      else fin = true;
      if (result > ST_RangeMaxDownSamplingFactor())
      {
         result--;
         fin = true;
      }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::SamplesToTime (int data)
{
	return (float)(data * (float)PCLK);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CVirtualChannel::Release (void)
{
   if (buffer_mc_hwut_ != NULL) buffer_mc_hwut_->Release();
   if (buffer_pa_hwut_ != NULL) buffer_pa_hwut_->Release();
   if (buffer_tfm_hwut_ != NULL) buffer_tfm_hwut_->Release();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::UpdateAScanSamplesPosition(TFP_VCH_AcquisitionConfig *acq)
{
int i;
float acq_distance, acq_time, acq_start_distance, acq_start_time;
unsigned short n_samples;

   if (acq == NULL) return ELOG(charEVCH_Parameter, EVCH_Parameter);
   acq_distance = acq->acq_end_distance - acq->acq_start_distance;
   acq_time = acq->acq_end_time - acq->acq_start_time;
   n_samples = acq->n_acq_samples;
   acq_start_time = acq->acq_start_time;
   acq_start_distance = acq->acq_start_distance;

   if (n_samples <= 0) 
      return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);

   if (n_samples != acq->ascan_samples_position_time_double.size())
      acq->ascan_samples_position_time_double.resize(n_samples);

   if (n_samples != acq->ascan_samples_position_time_float.size())
      acq->ascan_samples_position_time_float.resize(n_samples);

   if (n_samples != acq->ascan_samples_position_distance_double.size())
      acq->ascan_samples_position_distance_double.resize(n_samples);

   if (n_samples != acq->ascan_samples_position_distance_float.size())
      acq->ascan_samples_position_distance_float.resize(n_samples);

   for (i=0; i<n_samples; i++)
   {
      if (i < acq->ascan_samples_position_time_double.size())
         acq->ascan_samples_position_time_double[i] = (double)i / (double)(n_samples - 1) * (double)acq_time + (double)acq_start_time;
      if (i < acq->ascan_samples_position_time_float.size())
         acq->ascan_samples_position_time_float[i] = (float)i / (float)(n_samples - 1)  * (float)acq_time + (float)acq_start_time;
      if (i < acq->ascan_samples_position_distance_double.size())
         acq->ascan_samples_position_distance_double[i] = (double)i / (double)(n_samples - 1)  * (double)acq_distance + (double)acq_start_distance;
      if (i < acq->ascan_samples_position_distance_float.size())
         acq->ascan_samples_position_distance_float[i] = (float)i / (float)(n_samples - 1)  * (float)acq_distance + (float)acq_start_distance;
   }
   GetSamplesIncrement();
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::TimeToDistance (float data)
{
float result;
float material_velocity;

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity();

    if (cfg_.acq_time_mode == ACQ_Transmission) result = data * (material_velocity / (float)1000.);
    else	result = data * (material_velocity / (float)2000.);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::DistanceToTime (float data)
{
float result;
float material_velocity;

   if (cfg_.specimen == NULL) return ELOG(charEVCH_SPECIMEN_NULL, EVCH_SPECIMEN_NULL);
   material_velocity = cfg_.specimen->GetVelocity();

    if (cfg_.acq_time_mode == ACQ_Transmission) result = data / (material_velocity / (float)1000.);
    else result = data / (material_velocity / (float)2000.);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TimeToSamples (float data)
{
	return Round32u(data * GET_SamplingFrequency());
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//unsigned long CVirtualChannel::TimeToSamples_HWGate (float data)
//{
//	return Round32u(data * GET_SamplingFrequency());
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::SamplesToDistance (int data)
{
	return TimeToDistance(SamplesToTime(data));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::PA_ADDataToFSH (short int data)
{
float result = 0;
int range_max_signal = ST_RangePA_MaxSignalAmplitude(cfg_.id);
int range_min_signal = ST_RangePA_MinSignalAmplitude(cfg_.id);
short int rf_data;

   switch (cfg_.acq_mode)
	{
   case 0: // Modo RF
      rf_data = (short int)data;
      if (data > 0) result = (float)rf_data * (float)50. / (float)range_max_signal;
      else result = (float)rf_data * (float)-50. / (float)range_min_signal;
      if (result > 50) result = (float)40;
      if (result < -50) result = (float)-40;
      break;
	default: // Modo VIDEO
      result = (float)data * (float)100. / (float)range_max_signal;
      if (result > 100) result = (float)90;
      if (result < 0) result = (float)10;
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CVirtualChannel::FP_ADDataToFSH (short int data)
{
float result = 0;
int range_max_signal = ST_RangeFP_MaxSignalAmplitude();
int range_min_signal = ST_RangeFP_MinSignalAmplitude();
short int rf_data;

   switch (cfg_.acq_mode)
	{
   case 0: // Modo RF
      rf_data = (short int)data;
      if (data > 0) result = (float)rf_data * (float)50. / (float)range_max_signal;
      else result = (float)rf_data * (float)-50. / (float)range_min_signal;
      if (result > 50) result = (float)40;
      if (result < -50) result = (float)-40;
      break;
	default: // Modo VIDEO
      result = (float)data * (float)100. / (float)range_max_signal;
      if (result > 100) result = (float)90;
      if (result < 0) result = (float)10;
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::PA_FSHToADData (float data)
{
int result = 0;
int max_range = ST_RangePA_MaxSignalAmplitude(cfg_.id);

   switch (cfg_.acq_mode)
	{
   // Modo RF
   case 0: result = Round32(data * max_range / (float)50.); break;
   // Modo VIDEO
	default: result = Round32(data * max_range / (float)100.); break;
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_FSHToADData (float data)
{
int result = 0;
int max_range = ST_RangeFP_MaxSignalAmplitude();

   switch (cfg_.acq_mode)
	{
   // Modo RF
   case 0: result = Round32(data * max_range / (float)50.); break;
   // Modo VIDEO
	default: result = Round32(data * max_range / (float)100.); break;
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::GetSamplesIncrement (void)
{
float inc_time = 0, inc_distance = 0;
int i;

   if (cfg_.acq.n_acq_samples > 0)
   {
      inc_time = (cfg_.acq.acq_end_time - cfg_.acq.acq_start_time) / cfg_.acq.n_acq_samples;
      inc_distance = (cfg_.acq.acq_end_distance - cfg_.acq.acq_start_distance) / cfg_.acq.n_acq_samples;
      if (cfg_.acq.n_acq_samples != cfg_.acq.acq_samples_time_.size()) cfg_.acq.acq_samples_time_.resize(cfg_.acq.n_acq_samples);
      if (cfg_.acq.n_acq_samples != cfg_.acq.acq_samples_distance_.size()) cfg_.acq.acq_samples_distance_.resize(cfg_.acq.n_acq_samples);
      for (i=0; i<cfg_.acq.n_acq_samples; i++)
      {
         cfg_.acq.acq_samples_time_[i] = cfg_.acq.acq_start_time + i * inc_time;
         cfg_.acq.acq_samples_distance_[i] = cfg_.acq.acq_start_distance + i * inc_distance;
      }
   }
   if (cfg_.acq.n_beamformed_samples > 0)
   {
      inc_time = (cfg_.acq.acq_end_time - cfg_.acq.acq_start_time) / cfg_.acq.n_beamformed_samples;
      inc_distance = (cfg_.acq.acq_end_distance - cfg_.acq.acq_start_distance) / cfg_.acq.n_beamformed_samples;
      if (cfg_.acq.n_beamformed_samples != cfg_.acq.beamformed_samples_time_.size()) 
         cfg_.acq.beamformed_samples_time_.resize(cfg_.acq.n_beamformed_samples);
      if (cfg_.acq.n_beamformed_samples != cfg_.acq.beamformed_samples_distance_.size()) 
         cfg_.acq.beamformed_samples_distance_.resize(cfg_.acq.n_beamformed_samples);
      for (i=0; i<cfg_.acq.n_beamformed_samples; i++)
      {
         cfg_.acq.beamformed_samples_time_[i] = cfg_.acq.acq_start_time + i * inc_time;
         cfg_.acq.beamformed_samples_distance_[i] = cfg_.acq.acq_start_distance + i * inc_distance;
      }
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FL_PWIEmission(vector <vector< T_VCH_EmitterFocalLaw > >& fl, float* angle, int n_angles, float pitch, float velocity, int n_ch)
{
    int i, c;
    float emitter_delay;

    fl.resize(n_angles);
    for (i = 0; i < n_angles; i++)
    {
        fl[i].resize(n_ch);
        for (c = 0; c < n_ch; c++)
        {
            if (angle[i] < 0) emitter_delay = sinf(angle[i] * (float)M_PI / (float)180.) * pitch / velocity * (c + 1 - n_ch);
            if (angle[i] >= 0) emitter_delay = sinf(angle[i] * (float)M_PI / (float)180.) * pitch / velocity * (c + 1 - 1);
            fl[i][c].emitter_delay_ticks = emitter_delay * FCLK4_MHz;
            fl[i][c].emitter_delay = emitter_delay;
            fl[i][c].emitter_enabled = 1;
        }
    }
    return EVCH_NONE;;
}




















// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannels_MemoryResize(int id_vch, int n_acquisitions)
{
int i, result;

   if (id_vch < 0)
   {
      for (i=0; i<gb_virtual_channel.size(); i++)
      {
         if (gb_virtual_channel[i] != NULL)
         {
            if (gb_virtual_channel[i]->GET_AcquisitionType() == ACQUISITION_TYPE_MC && gb_virtual_channel[i]->buffer_mc_hwut_ != NULL)
            {
               if ((result = gb_virtual_channel[i]->buffer_mc_hwut_->MemoryResize(n_acquisitions)) < 0)
                  return RLOG(result);
            }
            if (gb_virtual_channel[i]->GET_AcquisitionType() == ACQUISITION_TYPE_PA && gb_virtual_channel[i]->buffer_pa_hwut_ != NULL)
            {
               if ((result = gb_virtual_channel[i]->buffer_pa_hwut_->MemoryResize()) < 0)
                  return RLOG(result);
            }
            if (gb_virtual_channel[i]->GET_AcquisitionType() == ACQUISITION_TYPE_TFM && gb_virtual_channel[i]->buffer_tfm_hwut_ != NULL)
            {
                if ((result = gb_virtual_channel[i]->buffer_tfm_hwut_->MemoryResize()) < 0)
                    return RLOG(result);
            }
         }
      }
   }
   else
   {
      if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL)
      {
         if (gb_virtual_channel[id_vch]->buffer_mc_hwut_ != NULL)
         {
            if ((result = gb_virtual_channel[id_vch]->buffer_mc_hwut_->MemoryResize()) < 0)
               return RLOG(result);
         }
         if (gb_virtual_channel[id_vch]->buffer_pa_hwut_ != NULL)
         {
            if ((result = gb_virtual_channel[id_vch]->buffer_pa_hwut_->MemoryResize()) < 0)
               return RLOG(result);
         }
         if (gb_virtual_channel[id_vch]->buffer_tfm_hwut_ != NULL)
         {
             if ((result = gb_virtual_channel[id_vch]->buffer_tfm_hwut_->MemoryResize()) < 0)
                 return RLOG(result);
         }
      }
   }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannelsDestroy (void)
{
int i;

   for (i=0; i<gb_virtual_channel.size(); i++)
   {
		if (gb_virtual_channel[i] != NULL) delete gb_virtual_channel[i];
   	gb_virtual_channel[i] = NULL;
   }
   gb_virtual_channel.clear();
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannelsDelete (void)
{
vector<CVirtualChannel *>::iterator it;
int n_vch;

    ST_VirtualChannelsDestroy();
    gb_virtual_channel.resize(1);
    gb_virtual_channel[0] = new CVirtualChannel(0, 0, 0, "Default", gb_hw_format);
    //ST_BufferCreate();

    n_vch = (int)gb_virtual_channel.size();
    if (n_vch <= 0) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    if (gb_buffer_hw == NULL) gb_buffer_hw = new CBufferHW(SIZE_BUFFER_HW);
    if (gb_fp_buffer_acq == NULL) gb_fp_buffer_acq = new CBufferAcq();
    if (gb_pa_buffer_acq == NULL) gb_pa_buffer_acq = new CBufferAcq();
    if (gb_tfm_buffer_acq == NULL) gb_tfm_buffer_acq = new CBufferAcq();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannelDelete(int id_virtual_channel)
{
int i;

   if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
	if (gb_virtual_channel.size() <= id_virtual_channel || gb_virtual_channel[id_virtual_channel] == NULL)
		return ELOG(charEVCH_Parameter, EVCH_Parameter);

   if (gb_virtual_channel[id_virtual_channel] != NULL) delete gb_virtual_channel[id_virtual_channel];
   gb_virtual_channel[id_virtual_channel] = NULL;
   gb_virtual_channel.erase(gb_virtual_channel.begin() + id_virtual_channel);

   for (i=0; i<gb_virtual_channel.size(); i++) gb_virtual_channel[i]->cfg_.id = i;
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannelsAdd (char *name, int id_pa_sys, int id_connector)
{
int result;
CVirtualChannel *virtual_channel = NULL;
int acquiring = ST_Acquiring();

   if (STHW_GetPtrPASystem(id_pa_sys) == NULL) return ELOG(charEVCH_SYSTEM_NULL, EVCH_SYSTEM_NULL);
   virtual_channel = new CVirtualChannel((int)gb_virtual_channel.size(), id_pa_sys, id_connector, name, gb_hw_format);
   gb_virtual_channel.push_back(virtual_channel);
   if (acquiring == 1)
   {
       if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
       if ((result = ST_SetVirtualChannel(true)) < 0) return RLOG(result);
       if(acquiring == 1) ST_Start();
   }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannelsDefault (void)
{
   ST_VirtualChannelsDelete();
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el número de canales virtuales creados con la función \c UT_AddVirtualChannel().

 @return - Número de canales virtuales configurados.
			- Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_GetVirtualChannelsNumber (void)
{
int i, result = 0;

   for (i=0; i<(int)gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL) result++;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el canal virtual seleccinado previamente por la función \c ST_SetSelectedVirtualChannel()

 @return Canal virtual actualmente seleccionado.\n
         Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_GetSelectedVirtualChannel (void)
{
   return gb_selected_virtual_channel;
}
// -----------------------------------------------------------------------------
/**
 Selecciona el canal virtual del que se leeran los datos con la funciones:
 - \c ST_GetBufferLastAScan(),
 - \c ST_GetBufferTriggerLine(),
 - \c ST_GetBufferTriggerLine_HWGate_Alarm(),
 - \c ST_GetBufferTriggerLine_HWGate_PeakADValue(),
 - \c ST_GetBufferTriggerLine_HWGate_PositionSample(),
 - \c ST_GetBufferTriggerLine_HWGate_PositionTime(),
 - \c ST_GetAScanDataNumber(),
 - \c ST_GetAScanNumber(),
 - \c ST_GetImageDataNumber(),
 - \c ST_GetTriggerLineDataNumber(),

 @param[in]    data
               Canal virtual seleccionado.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_SetSelectedVirtualChannel (int id_vch)
{
   if (  id_vch < 0 ||
         id_vch >= gb_virtual_channel.size() ||
         gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   gb_selected_virtual_channel = id_vch;
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetVirtualChannel(bool update)
{
int i, result;

   for (i=0; i<gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL)
      {
         if ((result = gb_virtual_channel[i]->Set(update)) < 0) return RLOG(result);
      }
   }
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetVirtualChannel(int id_vch, bool update)
{
int result;

   if (  id_vch < 0 ||
         id_vch >= gb_virtual_channel.size() ||
         gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
   if ((result = gb_virtual_channel[id_vch]->Set(update)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetFP_AcquisitionSize(vector<unsigned long> &acq_size)
{
int i;
unsigned long data;

   acq_size.clear();
   for (i=0; i<gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL)
      {
         data = gb_virtual_channel[i]->GET_FP_AcquisitionSize();
         acq_size.push_back(data);
      }
   }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_PrintVirtualChannel_BASERegisters(int id_vch)
{
int result;

   if ((result = TCPSend_MSG_PrintVirtualChannel_BASERegisters(id_vch)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VirtualChannel_Set(void)
{
int i, result = EVCH_NONE;

   for (i=0; i<gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL)
      {
          //gb_virtual_channel[i]->RELOAD_AcquisitionType();
         if ((result = gb_virtual_channel[i]->Set()) < 0)
            return RLOG(result);
      }
   }
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_SamplesToTime (int data, int id_vch)
{
float result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return (float)ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SamplesToTime(data)) < 0)
      return (float)RLOG(Round32(result));
//   if ((result = gb_virtual_channel[id_vch]->GET_USER_AScanSamplePositionDistance_float(data)) < 0)
//      return RLOG(Round32(result));
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_SamplePositionToDistance_float (short int data, int id_vch)
{
float result = (float)ES2_IdVirtualChannel;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if ((result = gb_virtual_channel[id_vch]->SamplesToDistance(data)) < 0)
         return (float)RLOG(Round32(result));
   }
   return result;
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SamplePositionToDistance_vdouble (   short int *samples,
                                    unsigned long n_data,
                                    vector<double> &position_distance,
                                    int id_vch)
{
unsigned long i=0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if (position_distance.size() != n_data) position_distance.resize(n_data);
      for (i=0; i<n_data && i<(unsigned long)position_distance.size(); i++)
      {
         if ((position_distance[i] = gb_virtual_channel[id_vch]->SamplesToDistance(samples[i])) < 0)
            return RLOG(Round32((float)position_distance[i]));
      }
   }
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_VCH_SamplePositionToTime_float (short int samples, int id_vch)
{
float result = (float)ES2_IdVirtualChannel;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if ((result = gb_virtual_channel[id_vch]->SamplesToTime(samples)) < 0)
         return (float)RLOG(Round32(result));
   }
   return result;
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_VCH_SamplePositionToTime_vdouble (   short int *samples,
                                    unsigned long n_data,
                                    vector<double> &position_time,
                                    int id_vch)
{
unsigned long i=0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if (position_time.size() != n_data) position_time.resize(n_data);
      for (i=0; i<n_data && i<(unsigned long)position_time.size(); i++)
      {
         if ((position_time[i] = gb_virtual_channel[id_vch]->SamplesToTime(samples[i])) < 0)
            return RLOG(Round32((float)position_time[i]));
      }
   }
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float STPA_ADDataToFSH_float (short int data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
      return gb_virtual_channel[id_vch]->PA_ADDataToFSH(data);
   return 6;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float STFP_ADDataToFSH_float (short int data, int id_vch)
{
   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
      return gb_virtual_channel[id_vch]->FP_ADDataToFSH(data);
   return 6;
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STPA_ADDataToFSH_vdouble (short int *samples, unsigned long n_data, vector<double> &fsh, int id_vch)
{
unsigned long i=0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if (fsh.size() != n_data) fsh.resize(n_data);
      for (i=0; i<n_data && i<(unsigned long)fsh.size(); i++)
         fsh[i] = gb_virtual_channel[id_vch]->PA_ADDataToFSH(samples[i]);
   }
   return i;
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STFP_ADDataToFSH_vdouble (short int *samples, unsigned long n_data, vector<double> &fsh, int id_vch)
{
unsigned long i=0;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if (fsh.size() != n_data) fsh.resize(n_data);
      for (i=0; i<n_data && i<(unsigned long)fsh.size(); i++)
         fsh[i] = gb_virtual_channel[id_vch]->FP_ADDataToFSH(samples[i]);
   }
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STPA_VCH_FSHToADData (float data, int id_vch)
{
int result = ES2_NONE;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if ((result = gb_virtual_channel[id_vch]->PA_FSHToADData(data)) < 0)
         return RLOG(result);
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STFP_VCH_FSHToADData (float data, int id_vch)
{
int result = ES2_NONE;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (  id_vch < gb_virtual_channel.size() &&
         gb_virtual_channel[id_vch] != NULL)
   {
      if ((result = gb_virtual_channel[id_vch]->FP_FSHToADData(data)) < 0)
         return RLOG(result);
   }
   return result;
}









