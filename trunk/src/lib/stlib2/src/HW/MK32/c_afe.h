#ifndef c_afeH
#define c_afeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdio.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define BASE_MAX_PULSES_NUMBER   0xFF
#define BASE_MAX_PULSE_MODE      0xF

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef union TFP_VCH_AFE_SPI_REG21
{
	unsigned short REG;
	struct
	{
		unsigned short DIGITAL_HPF_FILTER_ENABLE_1_4 : 1;	// Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 1 to 4 channels
		unsigned short DIGITAL_HPF_FILTER_K_CH_1_4 : 4;	// Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
		unsigned short RESERVED : 11;
	} BIT;
} TFP_VCH_AFE_SPI_REG21;
#define BASE_MIN_DIGITAL_HPF_FILTER_K_CH_1_4     2
#define BASE_MAX_DIGITAL_HPF_FILTER_K_CH_1_4     10

typedef union TFP_VCH_AFE_SPI_REG33
{
	unsigned short REG;
	struct
	{
		unsigned short DIGITAL_HPF_FILTER_ENABLE_5_8 : 1;	// Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 5 to 8 channels
		unsigned short DIGITAL_HPF_FILTER_K_CH_5_8 : 4;	// Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
		unsigned short RESERVED : 11;
	} BIT;
} TFP_VCH_AFE_SPI_REG33;
#define BASE_MIN_DIGITAL_HPF_FILTER_K_CH_5_8     2
#define BASE_MAX_DIGITAL_HPF_FILTER_K_CH_5_8     10

typedef union TFP_VCH_AFE_SPI_REG51
{
	unsigned short REG;
	struct
	{
		unsigned short RESERVED0 : 1;	// Bit[0]: no hace nada
		unsigned short LPF_PROGRAMMABILITY : 3;	// Bit[1:3]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz
		unsigned short PGA_INTEGRATOR_DISABLE : 1;	// Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.
		unsigned short PGA_CLAMP_LEVEL : 3;	// Bit[5:7]: In normal operation, clamp function can be set as 000 in the low noise mode. Para cualquier otra cosa, mirar datasheet.
		unsigned short RESERVED1 : 5;	// Bit[8:12]: no hace nada
		unsigned short PGA_GAIN_CONTROL : 1;	// Bit[13]: 0:24 dB; 1:30 dB
		unsigned short RESERVED2 : 2;	// Bit[14:15]: no hace nada
	} BIT;
} TFP_VCH_AFE_SPI_REG51;
#define BASE_MAX_LPF_PROGRAMMABILITY   7
#define BASE_MAX_PGA_CLAMP_LEVEL       7

#define PGA_GAIN_v24   24
#define PGA_GAIN_v30   30
#define PGA_GAIN_0   PGA_GAIN_v24
#define PGA_GAIN_1   PGA_GAIN_v30
#define LPF_PROGRAMMABILITY_v15   15
#define LPF_PROGRAMMABILITY_v20   20
#define LPF_PROGRAMMABILITY_v30   30
#define LPF_PROGRAMMABILITY_v10   10
#define LPF_PROGRAMMABILITY_0   LPF_PROGRAMMABILITY_v15
#define LPF_PROGRAMMABILITY_2   LPF_PROGRAMMABILITY_v20
#define LPF_PROGRAMMABILITY_3   LPF_PROGRAMMABILITY_v30
#define LPF_PROGRAMMABILITY_4   LPF_PROGRAMMABILITY_v10

typedef union TFP_VCH_AFE_SPI_REG52
{
	unsigned short REG;
	struct
	{
		unsigned short ACTIVE_TERMINATION : 5;	// Bit[4:0]: Control individual de la red de resistecias de terminación
		unsigned short ACT_TER_ENA_CTRL : 1;	// Bit[5]: 1: Enable internal active termination individual resistor control
		unsigned short PRESET_ACTIVE_TERMINATIONS : 2;	// Bit[7:6]: 00: 50 Ω; 01: 100 Ω; 10: 200 Ω; 11: 400 Ω	Note: The device adjusts resistor mapping (52[4:0]) automatically. 50-Ω active termination is not supported in 12-dB LNA setting. Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
		unsigned short ACTIVE_TERMINATION_ENABLE : 1;	// Bit[8]: Enable active termination
		unsigned short LNA_INPUT_CLAMP_SETTING : 2;	// Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp
		unsigned short RESERVED0 : 1;	// Bit[11]: Set to zero.
		unsigned short LNA_INTEGRATOR_DISABLE : 1;	// Bit[12]: Disable offset integrator for LNA. Mirar datasheet
		unsigned short LNA_GAIN : 2;	// Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved
		unsigned short LNA_INDIVIDUAL_CH_CNTL : 1;	// Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57. En principio no parece de utilidad.
	} BIT;
} TFP_VCH_AFE_SPI_REG52;
#define BASE_MAX_ACTIVE_TERMINATION          31
#define BASE_MAX_PRESET_ACTIVE_TERMINATIONS  3
#define BASE_MAX_LNA_INPUT_CLAMP_SETTING     3
#define BASE_MAX_LNA_GAIN                    3

#define LNA_GAIN_v12   12
#define LNA_GAIN_v18   18
#define LNA_GAIN_v24   24
#define LNA_GAIN_0   LNA_GAIN_v18
#define LNA_GAIN_1   LNA_GAIN_v24
#define LNA_GAIN_2   LNA_GAIN_v12
#define PRESET_ACTIVE_TERMINATIONS_v50    50
#define PRESET_ACTIVE_TERMINATIONS_v100   100
#define PRESET_ACTIVE_TERMINATIONS_v200   200
#define PRESET_ACTIVE_TERMINATIONS_v400   400
#define PRESET_ACTIVE_TERMINATIONS_0   PRESET_ACTIVE_TERMINATIONS_v50
#define PRESET_ACTIVE_TERMINATIONS_1   PRESET_ACTIVE_TERMINATIONS_v100
#define PRESET_ACTIVE_TERMINATIONS_2   PRESET_ACTIVE_TERMINATIONS_v200
#define PRESET_ACTIVE_TERMINATIONS_3   PRESET_ACTIVE_TERMINATIONS_v400
#define LNA_INPUT_CLAMP_SETTING_vAUTO  -1
#define LNA_INPUT_CLAMP_SETTING_v1_5   1.5
#define LNA_INPUT_CLAMP_SETTING_v1_15  1.15
#define LNA_INPUT_CLAMP_SETTING_v0_6   0.6
#define LNA_INPUT_CLAMP_SETTING_0   LNA_INPUT_CLAMP_SETTING_vAUTO
#define LNA_INPUT_CLAMP_SETTING_1   LNA_INPUT_CLAMP_SETTING_v1_5
#define LNA_INPUT_CLAMP_SETTING_2   LNA_INPUT_CLAMP_SETTING_v1_15
#define LNA_INPUT_CLAMP_SETTING_3   LNA_INPUT_CLAMP_SETTING_v0_6

typedef union TFP_VCH_AFE_SPI_REG59
{
	unsigned short REG;
	struct
	{
		unsigned short RESERVED0 : 2;	// Bit[1:0]:
		unsigned short HPF_LNA : 2;	// Bit[3:2]: 1: Enable internal active termination individual resistor control
		unsigned short DIG_TGC_ATT_GAIN : 3;	// Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1
		unsigned short DIG_TGC_ATT : 1;	// Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator
		unsigned short CW_SUM_AMP_PDN : 1;	// Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
		unsigned short PGA_TEST_MODE : 1;	// Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs.
		unsigned short RESERVED1 : 6;	// Bit[15:10]:
	} BIT;
} TFP_VCH_AFE_SPI_REG59;
#define BASE_MAX_DIG_TGC_ATT_GAIN   7
#define BASE_MAX_HPF_LNA            3

// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
typedef union TFP_VCH_AFE_SPI_REG61
{
	unsigned short REG;
	struct
	{
		unsigned short RESERVED0 : 13;	// Bit[12:0]:
		unsigned short V2I_CLAMP : 1;	// Bit[13]: 0: Clamp disabled
												//			1: Clamp enabled at the V2I input. An additional
												//			voltage clamp at the V2I input. This limits the
												//			amount of overload signal the PGA sees.
		unsigned short LPF_5MHz : 1;	// Bit[14]: 0: 5-MHz LPF disabled
												//			1: 5-MHz LPF enabled. Suppress signals >5 MHz
												//			or high-order harmonics. The LPF Register
												//			51[3:1] needs to be set as 100, that is, 10 MHz.
		unsigned short PGA_CLAMP_minus6dBFS : 1;	// Bit[15]: 0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp. PGA_CLAMP
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

	} BIT;
} TFP_VCH_AFE_SPI_REG61;

typedef struct TFP_VCH_AFE_SPI
{
	TFP_VCH_AFE_SPI_REG21 reg21;
	TFP_VCH_AFE_SPI_REG33 reg33;
	TFP_VCH_AFE_SPI_REG51 reg51;
	TFP_VCH_AFE_SPI_REG52 reg52;
	TFP_VCH_AFE_SPI_REG59 reg59;
	TFP_VCH_AFE_SPI_REG61 reg61;	// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
} TFP_VCH_AFE_SPI;

typedef struct TFP_VCH_AFE_BUSSAR
{
	unsigned char external_trigger;
	unsigned char decimation_ratio;
	unsigned long n_samples;
	unsigned long water_delay;
	unsigned char log2_average; // 0: no promedia, maximo 4 (16 promediados)
} TFP_VCH_AFE_BUSSAR;

typedef struct TFP_VCH_AFE
{
	unsigned long delay_offset;
	TFP_VCH_AFE_BUSSAR bussar;
	TFP_VCH_AFE_SPI spi;
} TFP_VCH_AFE;

// _____________________________________________________________________________
// ------ C_AFE class
// _____________________________________________________________________________
// =============================================================================

class C_AFE
{
private:

   bool update_register_;
   
protected:

public:

	TFP_VCH_AFE reg_;

   C_AFE();

   ~C_AFE();

   int ResetRegister(void);

	bool UpdateRegister(void) { return update_register_; }

	void PrintFileConfig(FILE* f);

	int CONFIG_Copy(C_AFE* src);

	int LoadRegisterAFE_DELAY_OFFSET(unsigned long data);

	int GetRegisterAFE_DELAY_OFFSET(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG21
	// =============================================================================

	int LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_1_4(unsigned short data);

	int LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_1_4(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG33
	// =============================================================================

	int LoadRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_5_8(unsigned short data);

	int LoadRegisterAFE_DIGITAL_HPF_FILTER_K_CH_5_8(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG51
	// =============================================================================

	int LoadValueAFE_LPF_PROGRAMMABILITY(unsigned short data);

	int LoadRegisterAFE_LPF_PROGRAMMABILITY(unsigned short data);

	int LoadRegisterAFE_PGA_INTEGRATOR_DISABLE(unsigned short data);

	int LoadRegisterAFE_PGA_CLAMP_LEVEL(unsigned short data);

	int LoadValueAFE_PGA_GAIN_CONTROL(unsigned short data);

	int LoadRegisterAFE_PGA_GAIN_CONTROL(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG52
	// =============================================================================

	int LoadRegisterAFE_ACTIVE_TERMINATION(unsigned short data);

	int LoadRegisterAFE_ACT_TER_ENA_CTRL(unsigned short data);

	int LoadValueAFE_PRESET_ACTIVE_TERMINATIONS(unsigned short data);

	int LoadRegisterAFE_PRESET_ACTIVE_TERMINATIONS(unsigned short data);

	int LoadRegisterAFE_ACTIVE_TERMINATION_ENABLE(unsigned short data);

	//int LoadValueAFE_LNA_INPUT_CLAMP_SETTING (float data);

	int LoadRegisterAFE_LNA_INPUT_CLAMP_SETTING(unsigned short data);

	int LoadRegisterAFE_LNA_INTEGRATOR_DISABLE(unsigned short data);

	int LoadValueAFE_LNA_GAIN(unsigned short data);

	int LoadRegisterAFE_LNA_GAIN(unsigned short data);

	int LoadRegisterAFE_LNA_INDIVIDUAL_CH_CNTL(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG59
	// =============================================================================

	int LoadRegisterAFE_HPF_LNA(unsigned short data);

	int LoadRegisterAFE_DIG_TGC_ATT_GAIN(unsigned short data);

	int LoadRegisterAFE_DIG_TGC_ATT(unsigned short data);

	int LoadRegisterAFE_CW_SUM_AMP_PDN(unsigned short data);

	int LoadRegisterAFE_PGA_TEST_MODE(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG61
	// =============================================================================

	int LoadRegisterAFE_V2I_CLAMP(unsigned short data);

	int LoadRegisterAFE_LPF_5MHz(unsigned short data);

	int LoadRegisterAFE_PGA_CLAMP_minus6dBFS(unsigned short data);

	// _____________________________________________________________________________
	// ------ AFE - BUSSAR
	// =============================================================================

	int LoadRegisterBUSSAR_EXTERNAL_TRIGGER(unsigned char data);

	int LoadRegisterBUSSAR_DECIMATION_RATIO(unsigned char data);

	int LoadRegisterBUSSAR_N_SAMPLES(unsigned long data);

	int LoadRegisterBUSSAR_WATER_DELAY(unsigned long data);

	int LoadRegisterBUSSAR_LOG2_AVERAGE(unsigned char data);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG21
	// =============================================================================

	int GetRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_1_4(void);

	int GetRegisterAFE_DIGITAL_HPF_FILTER_K_CH_1_4(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG33
	// =============================================================================

	int GetRegisterAFE_DIGITAL_HPF_FILTER_ENABLE_5_8(void);

	int GetRegisterAFE_DIGITAL_HPF_FILTER_K_CH_5_8(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG51
	// =============================================================================

	//int LoadValueAFE_LPF_PROGRAMMABILITY (void)

	int GetRegisterAFE_LPF_PROGRAMMABILITY(void);

	int GetRegisterAFE_PGA_INTEGRATOR_DISABLE(void);

	int GetRegisterAFE_PGA_CLAMP_LEVEL(void);

	//int LoadValueAFE_PGA_GAIN_CONTROL (void);

	int GetRegisterAFE_PGA_GAIN_CONTROL(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG52
	// =============================================================================

	int GetRegisterAFE_ACTIVE_TERMINATION(void);

	int GetRegisterAFE_ACT_TER_ENA_CTRL(void);

	//int LoadValueAFE_PRESET_ACTIVE_TERMINATIONS (void);

	int GetRegisterAFE_PRESET_ACTIVE_TERMINATIONS(void);

	int GetRegisterAFE_ACTIVE_TERMINATION_ENABLE(void);

	int GetRegisterAFE_LNA_INPUT_CLAMP_SETTING(void);

	int GetRegisterAFE_LNA_INTEGRATOR_DISABLE(void);

	//int GetValueAFE_LNA_GAIN (void);

	int GetRegisterAFE_LNA_GAIN(void);

	int GetRegisterAFE_LNA_INDIVIDUAL_CH_CNTL(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG59
	// =============================================================================

	int GetRegisterAFE_HPF_LNA(void);

	int GetRegisterAFE_DIG_TGC_ATT_GAIN(void);

	float GetValueAFE_DIG_TGC_ATT_GAIN(void);

	int GetRegisterAFE_DIG_TGC_ATT(void);

	float GetValueAFE_DIG_TGC_ATT(void);

	int GetRegisterAFE_CW_SUM_AMP_PDN(void);

	int GetRegisterAFE_PGA_TEST_MODE(void);

	// _____________________________________________________________________________
	// ------ AFE - SPI - REG61
	// =============================================================================

	int GetRegisterAFE_V2I_CLAMP(void);

	int GetRegisterAFE_LPF_5MHz(void);

	int GetRegisterAFE_PGA_CLAMP_minus6dBFS(void);

	// _____________________________________________________________________________
	// ------ AFE - BUSSAR
	// =============================================================================

	int GetRegisterBUSSAR_EXTERNAL_TRIGGER(void);

	int GetRegisterBUSSAR_DECIMATION_RATIO(void);

	int GetRegisterBUSSAR_N_SAMPLES(void);

	int GetRegisterBUSSAR_WATER_DELAY(void);

	int GetRegisterBUSSAR_LOG2_AVERAGE(void);

};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
