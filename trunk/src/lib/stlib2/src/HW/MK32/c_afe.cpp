// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O ---
// ======================================================================

#include "c_afe.h"
#include "base_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "st_reg.h"

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ C_AFE class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_AFE::C_AFE()
{
   update_register_ = false;
   ResetRegister();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_AFE::~C_AFE()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::ResetRegister(void)
{
   reg_.delay_offset = 0;

   reg_.bussar.external_trigger = 1;
   reg_.bussar.decimation_ratio = 0;
   reg_.bussar.n_samples = 1;
   reg_.bussar.water_delay = 0;
   reg_.bussar.log2_average = 0;

   reg_.spi.reg21.REG = 0;
   reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4 = 0; // Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 1 to 4 channels
   reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4 = 2;   // Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.

   reg_.spi.reg33.REG = 0;
   reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8 = 0; // Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 5 to 8 channels
   reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8 = 2;   // Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.

   reg_.spi.reg51.REG = 0;
   reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY = 3;// 	   : 2;	// Bit[3:1]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz
   reg_.spi.reg51.BIT.PGA_INTEGRATOR_DISABLE = 0;//	: 1;	// Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.
   reg_.spi.reg51.BIT.PGA_CLAMP_LEVEL = 0;//			: 3;	// Bit[7:5]: In normal operation, clamp function can be set as 000 in the low noise mode. Para cualquier otra cosa, mirar datasheet.
   reg_.spi.reg51.BIT.PGA_GAIN_CONTROL = 1;//		   : 1;	// Bit[13]: 0:24 dB; 1:30 dB

   reg_.spi.reg52.REG = 0;
   reg_.spi.reg52.BIT.ACTIVE_TERMINATION = 0;// 			: 5;	// Bit[4:0]:
   reg_.spi.reg52.BIT.ACT_TER_ENA_CTRL = 0;//			   : 1;	// Bit[5]: 1: Enable internal active termination individual resistor control
   //reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS = 0;//	: 2;	// Bit[7:6]: 00: 50 Ω; 01: 100 Ω; 10: 200 Ω; 11: 400 Ω	Note: The device adjusts resistor mapping (52[4:0]) automatically. 50-Ω active termination is not supported in 12-dB LNA setting. Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
   //reg_.spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE 	: 1;	// Bit[8]: Enable active termination
   //reg_.spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING		: 2;	// Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp
   //reg_.spi.reg52.BIT.LNA_INTEGRATOR_DISABLE		: 1;	// Bit[12]: Disable offset integrator for LNA. Mirar datasheet
   reg_.spi.reg52.BIT.LNA_GAIN = 1;//					      : 2;	// Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved
   //reg_.spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL		: 1;	// Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57. En principio no parece de utilidad.

   reg_.spi.reg59.REG = 0;
   //reg_.spi.reg59.BIT.HPF_LNA						: 2;	// Bit[3:2]: 1: Enable internal active termination individual resistor control
   reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN = 0;//			: 3;	// Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1
   reg_.spi.reg59.BIT.DIG_TGC_ATT = 0;//				 	: 1;	// Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator
   //reg_.spi.reg59.BIT.CW_SUM_AMP_PDN				: 1;	// Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
   //reg_.spi.reg59.BIT.PGA_TEST_MODE				: 1;	// Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs.

   reg_.spi.reg61.REG = 0;
   //reg_.spi.reg61.V2I_CLAMP					: 1;	// Bit[13]: 0: Clamp disabled
       //									//			1: Clamp enabled at the V2I input. An additional
       //									//			voltage clamp at the V2I input. This limits the
       //									//			amount of overload signal the PGA sees.
   //reg_.spi.reg61.LPF_5MHz					: 1;	// Bit[14]: 0: 5-MHz LPF disabled
       //									//			1: 5-MHz LPF enabled. Suppress signals >5 MHz
       //									//			or high-order harmonics. The LPF Register
       //									//			51[3:1] needs to be set as 100, that is, 10 MHz.
   //reg_.spi.reg61.PGA_CLAMP_minus6dBFS		: 1;	// Bit[15]: 0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp. PGA_CLAMP
                                  //			Reg51[7:5] should be set as 000 in the lownoise
                                  //			mode or 100 in the low-power/mediumpower
                                  //			mode. In this setting, PGA output HD3 will
                                  //			be worsen by 3 dB at –6-dBFS ADC input. The
                                  //			actual PGA output is reduced to approximately 1.5
                                  //			Vpp, about 2.5 dB below the ADC full-scale input
                                  //			2 Vpp . As a result, AFE5809’s LPF is not
                                  //			saturated, and it can suppress harmonic signals
                                  //			better at PGA output. Due to PGA output
                                  //			reduction, the ADC output dynamic range is
                                  //			impacted.
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void C_AFE::PrintFileConfig(FILE* f)
{
   if (f == NULL) return;
   fprintf(f, "\n    _______________");
   fprintf(f, "\n    AFE Registers.-\n");

   fprintf(f, "\n        AFE.spi.reg51 = (%d) LPF_PROGRAMMABILITY", reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY);
   fprintf(f, "\n                        (%d) PGA_INTEGRATOR_DISABLE", reg_.spi.reg51.BIT.PGA_INTEGRATOR_DISABLE);
   fprintf(f, "\n                        (%d) PGA_CLAMP_LEVEL", reg_.spi.reg51.BIT.PGA_CLAMP_LEVEL);
   fprintf(f, "\n                        (%d) PGA_GAIN_CONTROL", reg_.spi.reg51.BIT.PGA_GAIN_CONTROL);
   fprintf(f, "\n");

   fprintf(f, "\n        AFE.spi.reg52 = (%d) ACTIVE_TERMINATION", reg_.spi.reg52.BIT.ACTIVE_TERMINATION);
   fprintf(f, "\n                        (%d) ACT_TER_ENA_CTRL", reg_.spi.reg52.BIT.ACT_TER_ENA_CTRL);
   fprintf(f, "\n                        (%d) PRESET_ACTIVE_TERMINATIONS", reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
   fprintf(f, "\n                        (%d) ACTIVE_TERMINATION_ENABLE", reg_.spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE);
   fprintf(f, "\n                        (%d) LNA_INPUT_CLAMP_SETTING", reg_.spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING);
   fprintf(f, "\n                        (%d) LNA_INTEGRATOR_DISABLE", reg_.spi.reg52.BIT.LNA_INTEGRATOR_DISABLE);
   fprintf(f, "\n                        (%d) LNA_GAIN", reg_.spi.reg52.BIT.LNA_GAIN);
   fprintf(f, "\n                        (%d) LNA_INDIVIDUAL_CH_CNTL", reg_.spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL);
   fprintf(f, "\n");

   fprintf(f, "\n        AFE.spi.reg59 = (%d) HPF_LNA", reg_.spi.reg59.BIT.HPF_LNA);
   fprintf(f, "\n                        (%d) DIG_TGC_ATT_GAIN", reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN);
   fprintf(f, "\n                        (%d) DIG_TGC_ATT", reg_.spi.reg59.BIT.DIG_TGC_ATT);
   fprintf(f, "\n                        (%d) CW_SUM_AMP_PDN", reg_.spi.reg59.BIT.CW_SUM_AMP_PDN);
   fprintf(f, "\n                        (%d) PGA_TEST_MODE", reg_.spi.reg59.BIT.PGA_TEST_MODE);
   fprintf(f, "\n");

   fprintf(f, "\n        AFE.spi.reg61 = (%d) V2I_CLAMP", reg_.spi.reg61.BIT.V2I_CLAMP);
   fprintf(f, "\n                        (%d) LPF_5MHz", reg_.spi.reg61.BIT.LPF_5MHz);
   fprintf(f, "\n                        (%d) PGA_CLAMP_minus6dBFS", reg_.spi.reg61.BIT.PGA_CLAMP_minus6dBFS);
   fprintf(f, "\n");

   fprintf(f, "\n        AFE.bussar = (%d) external_trigger", reg_.bussar.external_trigger);
   fprintf(f, "\n                     (%d) decimation_ratio", reg_.bussar.decimation_ratio);
   fprintf(f, "\n                     (%d) n_samples", reg_.bussar.n_samples);
   fprintf(f, "\n                     (%d) water_delay", reg_.bussar.water_delay);
   fprintf(f, "\n                     (%d) log2_average", reg_.bussar.log2_average);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::CONFIG_Copy(C_AFE* src)
{
   if (src == NULL) return ELOG(charEBASE_PtrAccess_NULL, EBASE_PtrAccess_NULL);
   reg_.delay_offset = src->reg_.delay_offset;

   reg_.bussar.external_trigger = src->reg_.bussar.external_trigger;
   reg_.bussar.decimation_ratio = src->reg_.bussar.decimation_ratio;
   reg_.bussar.n_samples = src->reg_.bussar.n_samples;
   reg_.bussar.water_delay = src->reg_.bussar.water_delay;
   reg_.bussar.log2_average = src->reg_.bussar.log2_average;

   reg_.spi.reg21.REG = src->reg_.spi.reg21.REG;
   reg_.spi.reg33.REG = src->reg_.spi.reg33.REG;
   reg_.spi.reg51.REG = src->reg_.spi.reg51.REG;
   reg_.spi.reg52.REG = src->reg_.spi.reg52.REG;
   reg_.spi.reg59.REG = src->reg_.spi.reg59.REG;
   reg_.spi.reg61.REG = src->reg_.spi.reg61.REG;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DELAY_OFFSET(unsigned long data)
{
   update_register_ = update_register_ ? true : (data != reg_.delay_offset);
   reg_.delay_offset = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DELAY_OFFSET(void)
{
   return reg_.delay_offset;
}


// _____________________________________________________________________________
// ------ AFE - SPI - REG21
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_1_4(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4);
   reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4 = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_1_4(unsigned short data)
{
   data = (data < BASE_MIN_DIGITAL_HPF_FILTER_K_CH_1_4) ? BASE_MIN_DIGITAL_HPF_FILTER_K_CH_1_4 : data;
   data = (data > BASE_MAX_DIGITAL_HPF_FILTER_K_CH_1_4) ? BASE_MAX_DIGITAL_HPF_FILTER_K_CH_1_4 : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4);
   reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4 = data;
   return EBASE_NONE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG33
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_5_8(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8);
   reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8 = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_5_8(unsigned short data)
{
   data = (data < BASE_MIN_DIGITAL_HPF_FILTER_K_CH_5_8) ? BASE_MIN_DIGITAL_HPF_FILTER_K_CH_5_8 : data;
   data = (data > BASE_MAX_DIGITAL_HPF_FILTER_K_CH_5_8) ? BASE_MAX_DIGITAL_HPF_FILTER_K_CH_5_8 : data;
   if (update_register_ != NULL)
      update_register_ = update_register_ ? true : (data != reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8);
   reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8 = data;
   return EBASE_NONE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG51
// =============================================================================

// -----------------------------------------------------------------------------
/**
000: 15 MHz
010: 20 MHz
011: 30 MHz
100: 10 MHz
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadValueAFE_LPF_PROGRAMMABILITY(unsigned short data)
{
   unsigned short reg_value = 0;

   switch (data)
   {
   case LPF_PROGRAMMABILITY_0: reg_value = 0; break;
   case LPF_PROGRAMMABILITY_2: reg_value = 2; break;
   case LPF_PROGRAMMABILITY_3: reg_value = 3; break;
   case LPF_PROGRAMMABILITY_4: reg_value = 4; break;
   default: return ELOG(charEBASE_Parameter, EBASE_Parameter); break;
   }
   update_register_ = update_register_ ? true : (reg_value != reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY);
   reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY = reg_value;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
000: 15 MHz
010: 20 MHz
011: 30 MHz
100: 10 MHz
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LPF_PROGRAMMABILITY(unsigned short data)
{
   data = (data > BASE_MAX_LPF_PROGRAMMABILITY) ? BASE_MAX_LPF_PROGRAMMABILITY : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY);
   reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
1: Disables offset integrator for PGA.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PGA_INTEGRATOR_DISABLE(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg51.BIT.PGA_INTEGRATOR_DISABLE);
   reg_.spi.reg51.BIT.PGA_INTEGRATOR_DISABLE = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
In normal operation, clamp function can be set as 000 in the low noise mode.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PGA_CLAMP_LEVEL(unsigned short data)
{
   data = (data > BASE_MAX_PGA_CLAMP_LEVEL) ? BASE_MAX_PGA_CLAMP_LEVEL : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg51.BIT.PGA_CLAMP_LEVEL);
   reg_.spi.reg51.BIT.PGA_CLAMP_LEVEL = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0:24 dB
1:30 dB
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadValueAFE_PGA_GAIN_CONTROL(unsigned short data)
{
   unsigned short reg_value = 0;

   switch (data)
   {
   case PGA_GAIN_0: reg_value = 0; break;
   case PGA_GAIN_1: reg_value = 1; break;
   default: return ELOG(charEBASE_Parameter, EBASE_Parameter); break;
   }
   update_register_ = update_register_ ? true : (reg_value != reg_.spi.reg51.BIT.PGA_GAIN_CONTROL);
   reg_.spi.reg51.BIT.PGA_GAIN_CONTROL = reg_value;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PGA_GAIN_CONTROL(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg51.BIT.PGA_GAIN_CONTROL);
   reg_.spi.reg51.BIT.PGA_GAIN_CONTROL = data;
   return EBASE_NONE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG52
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_ACTIVE_TERMINATION(unsigned short data)
{
   data = (data > BASE_MAX_ACTIVE_TERMINATION) ? BASE_MAX_ACTIVE_TERMINATION : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.ACTIVE_TERMINATION);
   reg_.spi.reg52.BIT.ACTIVE_TERMINATION = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
1: Enable internal active termination individual resistor control
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_ACT_TER_ENA_CTRL(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.ACT_TER_ENA_CTRL);
   reg_.spi.reg52.BIT.ACT_TER_ENA_CTRL = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
00: 50 Ω
01: 100 Ω
10: 200 Ω
11: 400 Ω
Note: The device adjusts resistor mapping (52[4:0]) automatically.
50-Ω active termination is not supported in 12-dB LNA setting.
Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadValueAFE_PRESET_ACTIVE_TERMINATIONS(unsigned short data)
{
   unsigned short reg_value = 0;

   switch (data)
   {
   case PRESET_ACTIVE_TERMINATIONS_0: reg_value = 0; break;
   case PRESET_ACTIVE_TERMINATIONS_1: reg_value = 1; break;
   case PRESET_ACTIVE_TERMINATIONS_2: reg_value = 2; break;
   case PRESET_ACTIVE_TERMINATIONS_3: reg_value = 3; break;
   default: return ELOG(charEBASE_Parameter, EBASE_Parameter); break;
   }
   update_register_ = update_register_ ? true : (reg_value != reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
   reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS = reg_value;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
00: 50 Ω
01: 100 Ω
10: 200 Ω
11: 400 Ω
Note: The device adjusts resistor mapping (52[4:0]) automatically.
50-Ω active termination is not supported in 12-dB LNA setting.
Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PRESET_ACTIVE_TERMINATIONS(unsigned short data)
{
   data = (data > BASE_MAX_PRESET_ACTIVE_TERMINATIONS) ? BASE_MAX_PRESET_ACTIVE_TERMINATIONS : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
   reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
1: Enable internal active termination individual resistor control
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_ACTIVE_TERMINATION_ENABLE(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE);
   reg_.spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
00: Auto setting
01: 1.5 Vpp
10: 1.15 Vpp
11: 0.6 Vpp
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LNA_INPUT_CLAMP_SETTING(unsigned short data)
{
   data = (data > BASE_MAX_LNA_INPUT_CLAMP_SETTING) ? BASE_MAX_LNA_INPUT_CLAMP_SETTING : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING);
   reg_.spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
Disable offset integrator for LNA. Mirar datasheet
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LNA_INTEGRATOR_DISABLE(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.LNA_INTEGRATOR_DISABLE);
   reg_.spi.reg52.BIT.LNA_INTEGRATOR_DISABLE = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadValueAFE_LNA_GAIN(unsigned short data)
{
   unsigned short reg_value = 0;

   switch (data)
   {
   case LNA_GAIN_0: reg_value = 0; break;
   case LNA_GAIN_1: reg_value = 1; break;
   case LNA_GAIN_2: reg_value = 2; break;
   default: return ELOG(charEBASE_Parameter, EBASE_Parameter); break;
   }
   update_register_ = update_register_ ? true : (reg_value != reg_.spi.reg52.BIT.LNA_GAIN);
   reg_.spi.reg52.BIT.LNA_GAIN = reg_value;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LNA_GAIN(unsigned short data)
{
   data = (data > BASE_MAX_LNA_GAIN) ? BASE_MAX_LNA_GAIN : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.LNA_GAIN);
   reg_.spi.reg52.BIT.LNA_GAIN = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
1: Activa control individual de la ganancia LNA con el registro 57. En principio no parece de utilidad.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LNA_INDIVIDUAL_CH_CNTL(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL);
   reg_.spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL = data;
   return EBASE_NONE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG59
// =============================================================================

// -----------------------------------------------------------------------------
/**
1: Enable internal active termination individual resistor control
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_HPF_LNA(unsigned short data)
{
   data = (data > BASE_MAX_HPF_LNA) ? BASE_MAX_HPF_LNA : data;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg59.BIT.HPF_LNA);
   reg_.spi.reg59.BIT.HPF_LNA = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIG_TGC_ATT_GAIN(unsigned short data)
{
   data = (data > BASE_MAX_DIG_TGC_ATT_GAIN) ? BASE_MAX_DIG_TGC_ATT_GAIN : data;
   update_register_ = update_register_ ? update_register_ : (data != reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN);
   reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_DIG_TGC_ATT(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg59.BIT.DIG_TGC_ATT);
   reg_.spi.reg59.BIT.DIG_TGC_ATT = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_CW_SUM_AMP_PDN(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg59.BIT.CW_SUM_AMP_PDN);
   reg_.spi.reg59.BIT.CW_SUM_AMP_PDN = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PGA_TEST_MODE(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg59.BIT.PGA_TEST_MODE);
   reg_.spi.reg59.BIT.PGA_TEST_MODE = data;
   return EBASE_NONE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG61
// =============================================================================

// -----------------------------------------------------------------------------
/**
0: Clamp disabled
1: Clamp enabled at the V2I input.
An additional voltage clamp at the V2I input.
This limits the amount of overload signal the PGA sees.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_V2I_CLAMP(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg61.BIT.V2I_CLAMP);
   reg_.spi.reg61.BIT.V2I_CLAMP = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: 5-MHz LPF disabled
1: 5-MHz LPF enabled. Suppress signals >5 MHz or high-order harmonics. The LPF Register 51[3:1] needs to be set as 100, that is, 10 MHz.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_LPF_5MHz(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg61.BIT.LPF_5MHz);
   reg_.spi.reg61.BIT.LPF_5MHz = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp.
PGA_CLAMP Reg51[7:5] should be set as 000 in the lownoise mode or 100 in the low-power/mediumpower mode.
In this setting, PGA output HD3 will be worsen by 3 dB at –6-dBFS ADC input.
The actual PGA output is reduced to approximately 1.5 Vpp, about 2.5 dB below the ADC full-scale input 2 Vpp.
As a result, AFE5809’s LPF is not saturated, and it can suppress harmonic signals better at PGA output.
Due to PGA output reduction, the ADC output dynamic range is impacted.
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterAFE_PGA_CLAMP_minus6dBFS(unsigned short data)
{
   data = (data == 1) ? 1 : 0;
   update_register_ = update_register_ ? true : (data != reg_.spi.reg61.BIT.PGA_CLAMP_minus6dBFS);
   reg_.spi.reg61.BIT.PGA_CLAMP_minus6dBFS = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterBUSSAR_EXTERNAL_TRIGGER(unsigned char data)
{
   update_register_ = update_register_ ? true : (data != reg_.bussar.external_trigger);
   reg_.bussar.external_trigger = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterBUSSAR_DECIMATION_RATIO(unsigned char data)
{
   update_register_ = update_register_ ? true : (data != reg_.bussar.decimation_ratio);
   reg_.bussar.decimation_ratio = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterBUSSAR_N_SAMPLES(unsigned long data)
{
   update_register_ = update_register_ ? true : (data != reg_.bussar.n_samples);
   reg_.bussar.n_samples = data;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterBUSSAR_WATER_DELAY(unsigned long data)
{
   update_register_ = update_register_ ? true : (data != reg_.bussar.water_delay);
   reg_.bussar.water_delay = data + reg_.delay_offset;
   //reg_.bussar.water_delay = reg_.delay_offset;
   return EBASE_NONE;
}
// -----------------------------------------------------------------------------
/**
0: no promedia, maximo 4 (16 promediados)
*/
// -----------------------------------------------------------------------------
int C_AFE::LoadRegisterBUSSAR_LOG2_AVERAGE(unsigned char data)
{
   update_register_ = update_register_ ? true : (data != reg_.bussar.log2_average);
   reg_.bussar.log2_average = data;
   return EBASE_NONE;
}
// _____________________________________________________________________________
// ------ AFE - SPI - REG21
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_1_4(void)
{
   return reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIGITAL_HPF_FILTER_K_CH_1_4(void)
{
   return reg_.spi.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG33
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_5_8(void)
{
   return reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIGITAL_HPF_FILTER_K_CH_5_8(void)
{
   return reg_.spi.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG51
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LPF_PROGRAMMABILITY(void)
{
   return reg_.spi.reg51.BIT.LPF_PROGRAMMABILITY;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PGA_INTEGRATOR_DISABLE(void)
{
   return reg_.spi.reg51.BIT.PGA_INTEGRATOR_DISABLE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PGA_CLAMP_LEVEL(void)
{
   return reg_.spi.reg51.BIT.PGA_CLAMP_LEVEL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PGA_GAIN_CONTROL(void)
{
   return reg_.spi.reg51.BIT.PGA_GAIN_CONTROL;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG52
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_ACTIVE_TERMINATION(void)
{
   return reg_.spi.reg52.BIT.ACTIVE_TERMINATION;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_ACT_TER_ENA_CTRL(void)
{
   return reg_.spi.reg52.BIT.ACT_TER_ENA_CTRL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PRESET_ACTIVE_TERMINATIONS(void)
{
   return reg_.spi.reg52.BIT.PRESET_ACTIVE_TERMINATIONS;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_ACTIVE_TERMINATION_ENABLE(void)
{
   return reg_.spi.reg52.BIT.ACTIVE_TERMINATION_ENABLE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LNA_INPUT_CLAMP_SETTING(void)
{
   return reg_.spi.reg52.BIT.LNA_INPUT_CLAMP_SETTING;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LNA_INTEGRATOR_DISABLE(void)
{
   return reg_.spi.reg52.BIT.LNA_INTEGRATOR_DISABLE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LNA_GAIN(void)
{
   return reg_.spi.reg52.BIT.LNA_GAIN;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LNA_INDIVIDUAL_CH_CNTL(void)
{
   return reg_.spi.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG59
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_HPF_LNA(void)
{
   return reg_.spi.reg59.BIT.HPF_LNA;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIG_TGC_ATT_GAIN(void)
{
   return reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float C_AFE::GetValueAFE_DIG_TGC_ATT_GAIN(void)
{
   return reg_.spi.reg59.BIT.DIG_TGC_ATT_GAIN * (float)6.;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_DIG_TGC_ATT(void)
{
   return reg_.spi.reg59.BIT.DIG_TGC_ATT;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_CW_SUM_AMP_PDN(void)
{
   return reg_.spi.reg59.BIT.CW_SUM_AMP_PDN;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PGA_TEST_MODE(void)
{
   return reg_.spi.reg59.BIT.PGA_TEST_MODE;
}

// _____________________________________________________________________________
// ------ AFE - SPI - REG61
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_V2I_CLAMP(void)
{
   return reg_.spi.reg61.BIT.V2I_CLAMP;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_LPF_5MHz(void)
{
   return reg_.spi.reg61.BIT.LPF_5MHz;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterAFE_PGA_CLAMP_minus6dBFS(void)
{
   return reg_.spi.reg61.BIT.PGA_CLAMP_minus6dBFS;
}

// _____________________________________________________________________________
// ------ AFE - BUSSAR
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterBUSSAR_EXTERNAL_TRIGGER(void)
{
   return reg_.bussar.external_trigger;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterBUSSAR_DECIMATION_RATIO(void)
{
   return reg_.bussar.decimation_ratio;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterBUSSAR_N_SAMPLES(void)
{
   return reg_.bussar.n_samples;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterBUSSAR_WATER_DELAY(void)
{
   return reg_.bussar.water_delay;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_AFE::GetRegisterBUSSAR_LOG2_AVERAGE(void)
{
   return reg_.bussar.log2_average;
}
