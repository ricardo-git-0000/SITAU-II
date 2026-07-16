//---------------------------------------------------------------------------
#ifndef fp_t_sitau_hwH
#define fp_t_sitau_hwH

#include "cxml.h"

//---------------------------------------------------------------------------
#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

//---------------------------------------------------------------------------
//Ficheros de configuración
#define FILENAME_CONFIG_SYS      "SITAU.SC" 		//binario
#define FILENAME_CONFIG_SYS_XML  "SITAU.XML" 		//xml
#define FILENAME_CONFIG_SYS_TXT  "SITAU.TXT" //texto

//Identificadores de los distintos tipos de tarjeta del equipo SITAU.-
#define BOARD_BASE 		0
#define BOARD_APUL 		1
#define BOARD_ADCON 		2
#define BOARD_FRONTEND 	3
#define BOARD_UCI 		4
#define BOARD_USB 		5
#define BOARD_FUAT_FIX 	6
#define BOARD_FUAT_PRG 	7
#define BOARD_BATTERY 	8
#define BOARD_TARMA_IO 	9
#define BOARD_MEDTEMP 	10
#define BOARD_BETADYNE 	11
#define BOARD_NUMBER		12

#define POWER_SUPPLY_BATTERY	0
#define POWER_SUPPLY_LAMBDA	1

#define DEFAULT_NUM_BASES		1
#define DEFAULT_NUM_MODS_BASE	4

#define MOD_CHANNEL_NUMBER		8

#define RESERVED_BYTES			32
#define NUMBER_MAX_BOARDS	   20

#define APULModel_APUL4C "APUL4C"
#define APULModel_APUL8C "APUL8C"
#define APULVersion_1    "1"
#define APULVersion_2    "2"

#define MEDTEMPVersion_1 "1"
#define MEDTEMPVersion_2 "2"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TXMLHW_DELAY_MEM
{
	CXMLParameter<unsigned short> delay_mem_ini;
	CXMLParameter<unsigned short> delay_mem_end;
} TXMLHW_DELAY_MEM;

typedef struct TXMLHW_PULSER_CONTROL
{
	CXMLParameter<unsigned long> gnd0_hiz1;
	CXMLParameter<unsigned long> pulser_load;
	CXMLParameter<unsigned long> pulser_auto_load;
	CXMLParameter<unsigned long> pulser_current;
	CXMLParameter<unsigned long> pulser_status;
	CXMLParameter<unsigned long> stop;
	CXMLParameter<unsigned long> external_trigger;
	CXMLParameter<unsigned long> not_busy;
	CXMLParameter<unsigned long> software_trigger;
} TXMLHW_PULSER_CONTROL;

typedef struct TXMLHW_REG_PULSER
{
	CXMLParameter<unsigned char> high_z;
	CXMLParameter<unsigned long> main_delay;
	TXMLHW_DELAY_MEM delay_mem_ini_end;
	CXMLParameter<unsigned long> delay_mem_ptr;
	TXMLHW_PULSER_CONTROL control;
} TXMLHW_REG_PULSER;

typedef struct TXMLHW_AFE_SPI_REG21
{
	CXMLParameter<unsigned short> DIGITAL_HPF_FILTER_ENABLE_1_4; // Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 1 to 4 channels
	CXMLParameter<unsigned short> DIGITAL_HPF_FILTER_K_CH_1_4;   // Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
} TXMLHW_AFE_SPI_REG21;

typedef struct TXMLHW_AFE_SPI_REG33
{
	CXMLParameter<unsigned short> DIGITAL_HPF_FILTER_ENABLE_5_8; // Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 5 to 8 channels
	CXMLParameter<unsigned short> DIGITAL_HPF_FILTER_K_CH_5_8;   // Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
} TXMLHW_AFE_SPI_REG33;

typedef struct TXMLHW_AFE_SPI_REG51
{
	CXMLParameter<unsigned short> LPF_PROGRAMMABILITY;// 	: 2;	// Bit[3:1]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz
	CXMLParameter<unsigned short> PGA_INTEGRATOR_DISABLE;//	: 1;	// Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.
	CXMLParameter<unsigned short> PGA_CLAMP_LEVEL;//			: 3;	// Bit[7:5]: In normal operation, clamp function can be set as 000 in the low noise mode. Para cualquier otra cosa, mirar datasheet.
	//CXMLParameter<unsigned short> PGA_GAIN_CONTROL;//		: 1;	// Bit[13]: 0:24 dB; 1:30 dB
} TXMLHW_AFE_SPI_REG51;

typedef struct TXMLHW_AFE_SPI_REG52
{
	CXMLParameter<unsigned short> ACTIVE_TERMINATION;// 			: 5;	// Bit[4:0]:
	CXMLParameter<unsigned short> ACT_TER_ENA_CTRL;//			: 1;	// Bit[5]: 1: Enable internal active termination individual resistor control
	CXMLParameter<unsigned short> PRESET_ACTIVE_TERMINATIONS;//	: 2;	// Bit[7:6]: 00: 50 Ω; 01: 100 Ω; 10: 200 Ω; 11: 400 Ω	Note: The device adjusts resistor mapping (52[4:0]) automatically. 50-Ω active termination is not supported in 12-dB LNA setting. Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
	CXMLParameter<unsigned short> ACTIVE_TERMINATION_ENABLE;// 	: 1;	// Bit[8]: Enable active termination
	CXMLParameter<unsigned short> LNA_INPUT_CLAMP_SETTING;//		: 2;	// Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp
	CXMLParameter<unsigned short> LNA_INTEGRATOR_DISABLE;//		: 1;	// Bit[12]: Disable offset integrator for LNA. Mirar datasheet
	//CXMLParameter<unsigned short> LNA_GAIN;//					: 2;	// Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved
	//CXMLParameter<unsigned short> LNA_INDIVIDUAL_CH_CNTL;//		: 1;	// Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57. En principio no parece de utilidad.
} TXMLHW_AFE_SPI_REG52;

typedef struct TXMLHW_AFE_SPI_REG59
{
	CXMLParameter<unsigned short> HPF_LNA;//						: 2;	// Bit[3:2]: 1: Enable internal active termination individual resistor control
	//CXMLParameter<unsigned short> DIG_TGC_ATT_GAIN;//			: 3;	// Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1
	//CXMLParameter<unsigned short> DIG_TGC_ATT;//				 	: 1;	// Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator
	CXMLParameter<unsigned short> CW_SUM_AMP_PDN;//				: 1;	// Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
	CXMLParameter<unsigned short> PGA_TEST_MODE;//				: 1;	// Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs.
} TXMLHW_AFE_SPI_REG59;

// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
typedef struct TXMLHW_AFE_SPI_REG61
{
	CXMLParameter<unsigned short> V2I_CLAMP;//					: 1;	// Bit[13]: 0: Clamp disabled
	//			1: Clamp enabled at the V2I input. An additional
	//			voltage clamp at the V2I input. This limits the
	//			amount of overload signal the PGA sees.
	CXMLParameter<unsigned short> LPF_5MHz;//					: 1;	// Bit[14]: 0: 5-MHz LPF disabled
	//			1: 5-MHz LPF enabled. Suppress signals >5 MHz
	//			or high-order harmonics. The LPF Register
	//			51[3:1] needs to be set as 100, that is, 10 MHz.
	CXMLParameter<unsigned short> PGA_CLAMP_minus6dBFS;//		: 1;	// Bit[15]: 0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp. PGA_CLAMP
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
} TXMLHW_AFE_SPI_REG61;

typedef struct TXMLHW_AFE_SPI
{
	TXMLHW_AFE_SPI_REG21 reg21;
	TXMLHW_AFE_SPI_REG33 reg33;
	TXMLHW_AFE_SPI_REG51 reg51;
	TXMLHW_AFE_SPI_REG52 reg52;
	TXMLHW_AFE_SPI_REG59 reg59;
	TXMLHW_AFE_SPI_REG61 reg61;	// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
} TXMLHW_AFE_SPI;

typedef struct TXMLHW_AFE_BUSSAR
{
	CXMLParameter<unsigned char> external_trigger;
	//CXMLParameter<unsigned char> decimation_ratio;
	//CXMLParameter<unsigned long> n_samples;
	//CXMLParameter<unsigned long> water_delay;
	//CXMLParameter<unsigned char> log2_average;
} TXMLHW_AFE_BUSSAR;

typedef struct TXMLHW_AFE
{
	CXMLParameter<unsigned short> delay_offset_;
	TXMLHW_AFE_BUSSAR bussar;
	TXMLHW_AFE_SPI spi;
} TXMLHW_AFE;

typedef struct TXMLHW_Registers
{
	TXMLHW_REG_PULSER pulser;
	TXMLHW_AFE afe;
} TXMLHW_Registers; 

typedef struct TXMLHW_BASE
{
   AnsiString model;
   AnsiString version;
   AnsiString serial_number;
} TXMLHW_BASE;

typedef struct TXMLHW_CONNECTOR
{
   AnsiString name;
   bool pin_to_channel_assignment;
   int start_channel;
   int n_total_channels;
   vector<unsigned short> pin_to_channel;
   vector<unsigned short> pin_disabled;
} TXMLHW_CONNECTOR;

typedef struct TXMLHW_FRONTEND
{
   AnsiString model;
   AnsiString version;
   AnsiString serial_number;
   int n_active_channels;
   int n_multiplexed_channels;
   int n_channels;
   vector<unsigned short> id_base;
   vector<TXMLHW_CONNECTOR> connector;
   vector<unsigned short> pin_disabled;
} TXMLHW_FRONTEND;

typedef struct TXMLHW_Encoder
{
   int id;
   AnsiString name;
} TXMLHW_Encoder;

typedef struct TXMLHW_UCI
{
   bool enabled;
   AnsiString model;
   AnsiString version;
   AnsiString serial_number;
   vector<TXMLHW_Encoder> encoder;
} TXMLHW_UCI;

typedef struct TXMLHW_BEAMFORMER
{
   int number;
} TXMLHW_BEAMFORMER;

typedef struct TXMLHW_SITAUHardware
{
	int bases_number;
   vector<TXMLHW_BASE> base;
   vector<TXMLHW_FRONTEND> frontend;
   TXMLHW_UCI uci;
   TXMLHW_BEAMFORMER beamformer;
} TXMLHW_SITAUHardware;

typedef struct TXMLHW_PASystem
{
   AnsiString name;
   vector<unsigned short> id_base;
   int id_frontend;
   int n_active_channels;
   int n_multiplexed_channels;
   int id_connector;
   int id_base_first;
} TXMLHW_PASystem;

typedef struct TXMLHW_PowerSupply
{
   bool enabled;
   AnsiString serial_number;
   AnsiString model;
   int type;
} TXMLHW_PowerSupply;

//typedef struct TXMLHW_EquipmentInformation
//{
//   AnsiString serial_number;
//   AnsiString model;
//   AnsiString manufacture_date;
//   AnsiString calibration_date;
//} TXMLHW_EquipmentInformation;

typedef struct TXMLHW_DataLogger
{
	int enabled;
	int config;
	int pcie_user_buffer;
	int pa_beamformer;
	int tfm_beamformer;
} TXMLHW_DataLogger;

typedef struct TXMLHW_Buffer
{
	int pcie_user_buffer_burst_number;
	int pcie_fifo_pointers_number;
	int pcie_dma_blocks_number;
} TXMLHW_Buffer;

typedef struct TXMLHW_UserConfig
{
	int status_enabled;
	int status_period_ms;
} TXMLHW_UserConfig;

typedef struct TXMLHW_Data
{
   vector<TXMLHW_PASystem> pa_system;
   TXMLHW_SITAUHardware sitau_hardware;
   //TXMLHW_EquipmentInformation equipment_information;
   //TXMLHW_PowerSupply power_supply;
   TXMLHW_Registers registers;
   TXMLHW_DataLogger data_logger;
   TXMLHW_Buffer buffer;
   TXMLHW_UserConfig user_config;
} TXMLHW_Data;

typedef struct THW_Encoder
{
   int number;
   vector<AnsiString> id_2_name;
} THW_Encoder;

#endif
