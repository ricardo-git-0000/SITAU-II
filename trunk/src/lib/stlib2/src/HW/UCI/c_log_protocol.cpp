// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L  C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_log_protocol.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CLOG_PROTOCOL* log_protocol = NULL;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CLOG_PROTOCOL::CLOG_PROTOCOL()
{
    sent_MSG_PulseAmplitude = false;
    sent_MSG_ACQUIRE = false;
    sent_MSG_SITAUConfig = false;
    sent_MSG_TriggerSource = false;
    sent_MSG_VCH_Config = false;
    sent_MSG_VCH_FIR = false;
    sent_MSG_VCH_EmissionFocallaws = false;
    sent_MSG_VCH_EmissionFocallawsExtended = false;
    sent_MSG_VirtualChannel_Beamformer = false;
    sent_MSG_VirtualChannel_BeamformerExtended = false;
    sent_MSG_VirtualChannel_TGC = false;
    sent_MSG_PASystemConfig = false;
    sent_MSG_TERMINAL_LOG = false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CLOG_PROTOCOL::~CLOG_PROTOCOL()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage (unsigned char data)
{
   last_MSG_PulseAmplitude = data;
   sent_MSG_PulseAmplitude = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(unsigned short data)
{
   last_MSG_ACQUIRE = data;
   sent_MSG_ACQUIRE = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TFP_MSG_SITAUConfig *data)
{   
   memcpy(&last_MSG_SITAUConfig, data, sizeof(TFP_MSG_SITAUConfig));
   sent_MSG_SITAUConfig = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_StatusConfig* data)
{
    memcpy(&last_MSG_StatusConfig, data, sizeof(TMSG_StatusConfig));
    sent_MSG_SITAUConfig = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_TriggerSource* data)
{
   memcpy(&last_MSG_TriggerSource, data, sizeof(TMSG_TriggerSource));
   sent_MSG_TriggerSource = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_Config* data)
{
   memcpy(&last_MSG_VCH_Config, data, sizeof(TMSG_Config));
   sent_MSG_VCH_Config = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_FIR* data)
{
   memcpy(&last_MSG_VCH_FIR, data, sizeof(TMSG_FIR));
   sent_MSG_VCH_FIR = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_EmissionFocalLaws* data)
{
   memcpy(&last_MSG_VCH_EmissionFocallaws, data, sizeof(TMSG_EmissionFocalLaws));
   sent_MSG_VCH_EmissionFocallaws = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_EmissionFocalLawsExtended* data)
{
    memcpy(&last_MSG_VCH_EmissionFocallawsExtended, data, sizeof(TMSG_EmissionFocalLawsExtended));
    sent_MSG_VCH_EmissionFocallawsExtended = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_Beamformer* data)
{
   memcpy(&last_MSG_VirtualChannel_Beamformer, data, sizeof(TMSG_Beamformer));
   sent_MSG_VirtualChannel_Beamformer = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_BeamformerExtended* data)
{
    memcpy(&last_MSG_VirtualChannel_BeamformerExtended, data, sizeof(TMSG_BeamformerExtended));
    sent_MSG_VirtualChannel_BeamformerExtended = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TMSG_TGC* data)
{
   memcpy(&last_MSG_VirtualChannel_TGC, data, sizeof(TMSG_TGC));
   sent_MSG_VirtualChannel_TGC = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::LastMessage(TFP_MSG_PASystemConfig* data)
{
   memcpy(&last_MSG_PASystemConfig, data, sizeof(TFP_MSG_PASystemConfig));
   sent_MSG_PASystemConfig = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLOG_PROTOCOL::Print(char *filename)
{
   FILE* f = NULL;
   int idx, j, k, r;

   if ((f = fopen(filename, "wt")) != NULL)
   {
      if (sent_MSG_PulseAmplitude)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_PulseAmplitude: %d V", last_MSG_PulseAmplitude);
      }
      if (sent_MSG_ACQUIRE)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_ACQUIRE: %d Acquisitions", last_MSG_ACQUIRE);
      }
      if (sent_MSG_SITAUConfig)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_SITAUConfig: %d Bases", last_MSG_SITAUConfig.n_bases);
      }
      if (sent_MSG_StatusConfig)
      {
          fprintf(f, "\n__________________________________________________________________________________");
          fprintf(f, "\nMSG_StatusConfig.-");
          fprintf(f, "\n   Status Enabled: %d", last_MSG_StatusConfig.enabled);
          fprintf(f, "\n   Status PCIe: %d", last_MSG_StatusConfig.pcie);
          fprintf(f, "\n   Status Period: %d ms", last_MSG_StatusConfig.period_ms);
      }
      if (sent_MSG_TriggerSource)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_TriggerSource.-");
         fprintf(f, "\n   Trigger Source:");
         if (last_MSG_TriggerSource.trigger_source.BIT.SW2HW == 1) fprintf(f, "\n      - Software UCI");
         if (last_MSG_TriggerSource.trigger_source.BIT.EXT == 1) fprintf(f, "\n      - External Signal");
         if (last_MSG_TriggerSource.trigger_source.BIT.PRF_1 == 1) fprintf(f, "\n      - PRF Timer 1");
         if (last_MSG_TriggerSource.trigger_source.BIT.PRF_2 == 1) fprintf(f, "\n      - PRF Timer 2");
         if (last_MSG_TriggerSource.trigger_source.BIT.TIME_STAMP == 1) fprintf(f, "\n      - Time Stamp");
         if (last_MSG_TriggerSource.trigger_source.BIT.ENC_1 == 1) fprintf(f, "\n      - Encoder 1");
         if (last_MSG_TriggerSource.trigger_source.BIT.ENC_2 == 1) fprintf(f, "\n      - Encoder 2");
         if (last_MSG_TriggerSource.trigger_source.BIT.ENC_3 == 1) fprintf(f, "\n      - Encoder 3");
         if (last_MSG_TriggerSource.trigger_source.BIT.ENC_4 == 1) fprintf(f, "\n      - Encoder 4");
         fprintf(f, "\n   PRF Time: %f", last_MSG_TriggerSource.prf_time);
         fprintf(f, "\n   Encoder Steps: %d", last_MSG_TriggerSource.encoder_steps);
         fprintf(f, "\n   N. Acquisitions: %d", last_MSG_TriggerSource.n_acquisitions);
      }
      if (sent_MSG_VCH_Config)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_VCH_Config (%d VCH).-", last_MSG_VCH_Config.id_vch);
         fprintf(f, "\n   Enabled: %d", last_MSG_VCH_Config.enabled);
         fprintf(f, "\n   N. Bases: %d", last_MSG_VCH_Config.n_bases);
         fprintf(f, "\n   Start Element: %d", last_MSG_VCH_Config.start_element);
         fprintf(f, "\n   N. Acquired Samples: %d", last_MSG_VCH_Config.n_acq_samples);
         fprintf(f, "\n   N. Beamformed Samples: %d", last_MSG_VCH_Config.n_beamformed_samples);
         fprintf(f, "\n   PRF Time Lines: %f", last_MSG_VCH_Config.prf_time_line);
         fprintf(f, "\n   Processing Type: %d", last_MSG_VCH_Config.processing_type);
         fprintf(f, "\n   Processing Wait PRF: %d", last_MSG_VCH_Config.wait_prf_signals_processing);
         fprintf(f, "\n   Processing AVR Signals: %d", last_MSG_VCH_Config.n_signals_processing_avr);
         fprintf(f, "\n   Processing EMI Signals: %d", last_MSG_VCH_Config.n_signals_processing_emi);
         fprintf(f, "\n   external_trigger: %d", last_MSG_VCH_Config.external_trigger);
         fprintf(f, "\n   decimation_factor: %d", last_MSG_VCH_Config.decimation_factor);
         fprintf(f, "\n   water_delay: %d", last_MSG_VCH_Config.water_delay);
         fprintf(f, "\n   log2_average: %d", last_MSG_VCH_Config.log2_average);
         fprintf(f, "\n   AFE SPI.-");
         fprintf(f, "\n      reg21.DIGITAL_HPF_FILTER_ENABLE_1_4: %d", last_MSG_VCH_Config.afe_spi.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4);
         fprintf(f, "\n      reg21.DIGITAL_HPF_FILTER_K_CH_1_4: %d", last_MSG_VCH_Config.afe_spi.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4);
         fprintf(f, "\n      reg33.DIGITAL_HPF_FILTER_ENABLE_5_8: %d", last_MSG_VCH_Config.afe_spi.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8);
         fprintf(f, "\n      reg33.DIGITAL_HPF_FILTER_K_CH_5_8: %d", last_MSG_VCH_Config.afe_spi.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8);
         fprintf(f, "\n      reg51.LPF_PROGRAMMABILITY: %d", last_MSG_VCH_Config.afe_spi.reg51.BIT.LPF_PROGRAMMABILITY);
         fprintf(f, "\n      reg51.PGA_INTEGRATOR_DISABLE: %d", last_MSG_VCH_Config.afe_spi.reg51.BIT.PGA_INTEGRATOR_DISABLE);
         fprintf(f, "\n      reg51.PGA_CLAMP_LEVEL: %d", last_MSG_VCH_Config.afe_spi.reg51.BIT.PGA_CLAMP_LEVEL);
         fprintf(f, "\n      reg51.PGA_GAIN_CONTROL: %d", last_MSG_VCH_Config.afe_spi.reg51.BIT.PGA_GAIN_CONTROL);
         fprintf(f, "\n      reg52.ACTIVE_TERMINATION: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.ACTIVE_TERMINATION);
         fprintf(f, "\n      reg52.ACT_TER_ENA_CTRL: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.ACT_TER_ENA_CTRL);
         fprintf(f, "\n      reg52.PRESET_ACTIVE_TERMINATIONS: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
         fprintf(f, "\n      reg52.ACTIVE_TERMINATION_ENABLE: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE);
         fprintf(f, "\n      reg52.LNA_INPUT_CLAMP_SETTING: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING);
         fprintf(f, "\n      reg52.LNA_INTEGRATOR_DISABLE: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.LNA_INTEGRATOR_DISABLE);
         fprintf(f, "\n      reg52.LNA_GAIN: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.LNA_GAIN);
         fprintf(f, "\n      reg52.LNA_INDIVIDUAL_CH_CNTL: %d", last_MSG_VCH_Config.afe_spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL);
         fprintf(f, "\n      reg59.HPF_LNA: %d", last_MSG_VCH_Config.afe_spi.reg59.BIT.HPF_LNA);
         fprintf(f, "\n      reg59.DIG_TGC_ATT_GAIN: %d", last_MSG_VCH_Config.afe_spi.reg59.BIT.DIG_TGC_ATT_GAIN);
         fprintf(f, "\n      reg59.DIG_TGC_ATT: %d", last_MSG_VCH_Config.afe_spi.reg59.BIT.DIG_TGC_ATT);
         fprintf(f, "\n      reg59.CW_SUM_AMP_PDN: %d", last_MSG_VCH_Config.afe_spi.reg59.BIT.CW_SUM_AMP_PDN);
         fprintf(f, "\n      reg59.PGA_TEST_MODE: %d", last_MSG_VCH_Config.afe_spi.reg59.BIT.PGA_TEST_MODE);
         fprintf(f, "\n   PULSER.-");
         fprintf(f, "\n      enabled: %d", last_MSG_VCH_Config.pulser.enabled);
         fprintf(f, "\n      high_z: %d", last_MSG_VCH_Config.pulser.high_z);
         fprintf(f, "\n      mode: %d", last_MSG_VCH_Config.pulser.current);
         fprintf(f, "\n      n_pulses: %d", last_MSG_VCH_Config.pulser.n_pulses);
         fprintf(f, "\n      pulse_width: %d", last_MSG_VCH_Config.pulser.pulse_width);
         fprintf(f, "\n      main_delay: %d", last_MSG_VCH_Config.pulser.main_delay);
         for (j = 0; j < last_MSG_VCH_Config.n_bases; j++)
         {
            fprintf(f, "\n   BASE(%d).-", j);
            fprintf(f, "\n      enabled: %d", last_MSG_VCH_Config.base_reg[j].enabled);
            fprintf(f, "\n      id_base: %d", last_MSG_VCH_Config.base_reg[j].id_base);
         }
      }
      if (sent_MSG_VCH_FIR)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_VCH_FIR (%d VCH).-", last_MSG_VCH_FIR.id_vch);
         fprintf(f, "\n   Enabled: %d", last_MSG_VCH_FIR.enabled);
         fprintf(f, "\n   N. Coefficients: %d", last_MSG_VCH_FIR.n_coefficients);
         fprintf(f, "\n   Coefficients: ");
         for (j = 0; j < last_MSG_VCH_FIR.n_coefficients; j++) fprintf(f, " %d", last_MSG_VCH_FIR.coefficients[j]);
      }
      if (sent_MSG_VCH_EmissionFocallaws)
      {
         fprintf(f, "\n__________________________________________________________________________________");
         fprintf(f, "\nMSG_VCH_EmissionFocallaws (%d VCH).-", last_MSG_VCH_EmissionFocallaws.id_vch);
         fprintf(f, "\n   N. Bases: %d", last_MSG_VCH_EmissionFocallaws.n_bases);
         fprintf(f, "\n   N. Focal Laws: %d", last_MSG_VCH_EmissionFocallaws.n_fl);
         for (j = 0; j < last_MSG_VCH_EmissionFocallaws.n_bases && j< MAX_N_BASES_MSG; j++)
         {
            fprintf(f, "\n   BASE(%d).-", j);
            for (k = 0; k < last_MSG_VCH_EmissionFocallaws.n_fl; k++)
            {
               fprintf(f, "\n      FL(%03d): ", k);
               idx = k * MOD_MAX_CH;               
               for (r = 0; r < MOD_MAX_CH; r++)
               {
                  if (last_MSG_VCH_EmissionFocallaws.base[j].efl[idx + r] == 0xFFFF) fprintf(f, " -----");
                  else fprintf(f, " %05d", last_MSG_VCH_EmissionFocallaws.base[j].efl[idx + r]);
               }
            }
         }
      }
      if (sent_MSG_VCH_EmissionFocallawsExtended)
      {
          fprintf(f, "\n__________________________________________________________________________________");
          fprintf(f, "\nMSG_VCH_EmissionFocallawsExtended (%d VCH).-", last_MSG_VCH_EmissionFocallawsExtended.id_vch);
          fprintf(f, "\n   Id. First Base: %d", last_MSG_VCH_EmissionFocallawsExtended.id_first_base);
          fprintf(f, "\n   N. Bases: %d", last_MSG_VCH_EmissionFocallawsExtended.n_bases);
          fprintf(f, "\n   N. Focal Laws: %d", last_MSG_VCH_EmissionFocallawsExtended.n_fl);
          for (j = 0; j < last_MSG_VCH_EmissionFocallawsExtended.n_bases && j < MAX_N_BASES_MSG; j++)
          {
              fprintf(f, "\n   BASE(%d).-", last_MSG_VCH_EmissionFocallawsExtended.id_first_base + j);
              for (k = 0; k < last_MSG_VCH_EmissionFocallawsExtended.n_fl; k++)
              {
                  fprintf(f, "\n      FL(%03d): ", k);
                  idx = k * MOD_MAX_CH;
                  for (r = 0; r < MOD_MAX_CH; r++)
                  {
                      if (last_MSG_VCH_EmissionFocallawsExtended.base[j].efl[idx + r] == 0xFFFF) fprintf(f, " -----");
                      else fprintf(f, " %05d", last_MSG_VCH_EmissionFocallawsExtended.base[j].efl[idx + r]);
                  }
              }
          }
      }

      fclose(f);
   }
}
