#ifndef fp_protocol_uciH
#define fp_protocol_uciH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include "protocol_define.h"
#include "protocol_base.h"
#include "c_afe.h"
#include "c_log_file.h"
#include "shared_mem.h"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define VERSION_DATE_SIZE           24
#define VERSION_DESCRIPTION_SIZE    24

#define MAX_N_BASES_MSG	4
#define MAX_N_BASES_MSG_256	8 //ESTO NO SE USA Y PUEDE AFECTAR AL FUNCIONAMIENTO DE 256 CANALES, TENER EN CUENTA

#define TFM_FMC_FORWARD_MEMORY_SIZE_MSG (BEAMFORMER_SIZE_FMC_FWD_WORD32*MAX_LINE*MAX_PROCESSING_ELEM*MAX_N_BASES_MSG)
#define TFM_PWI_FORWARD_MEMORY_SIZE_MSG	(BEAMFORMER_SIZE_PWI_FWD_WORD32*MAX_PWI_FL*MAX_LINE)
#define TFM_PWI_COS_ANG_MEMORY_SIZE_MSG (MAX_PWI_FL)

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef union T_AMPLIACommand {
   unsigned long CMD;          // 32Bits Access
   struct {                    // BIT  Access
      unsigned short REG : 6;  // Bit[0:5]
      unsigned short TAC : 2;  // Bit[6:7]
      unsigned short DIR : 8;  // Bit[8:15]
      unsigned short DAT : 16; // Bit[16:31]
   } BIT;
} T_AMPLIACommand; 

typedef enum T_CMD_Type
{
    CMD_NOP = 0,
    CMD_STOP = 1,
    CMD_TERMINAL_LOG = 2,
    CMD_STATUS_v0 = 3,
    CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 = 4,
    CMD_FP_VIRTUAL_CHANNEL_TGC = 5,
    CMD_FP_VIRTUAL_CHANNEL_FIR = 6,
    CMD_FP_PULSE_AMPLITUDE = 7,
    CMD_FP_PRINT_VCH_CONFIG = 8,
    CMD_FP_ACQUIRE = 9,
    CMD_VCH_EMISSION_FOCAL_LAW = 10,
    CMD_FP_TRIGGER_SOURCE_v0 = 11,
    CMD_FP_LOAD_FIRMWARE = 12,
    CMD_VIRTUAL_CHANNEL_BEAMFORMER = 13,
    CMD_VERSION = 14,
    CMD_RESET_ACQ_COUNTER = 15,
    CMD_RESET_ENCODER = 16,
    CMD_FP_TRIGGER_SOURCE_v1 = 17,
    CMD_FP_TRIGGER_SOURCE_v2 = 18,
    CMD_STATUS = 19,
    CMD_FP_VIRTUAL_CHANNEL_CONFIG_v1 = 20,
    CMD_FP_TRIGGER_SOURCE_v3 = 21,
    CMD_PAUSE = 22,
    CMD_CONTINUE = 23,
    CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 = 24,
    CMD_TFM_FMC_FORWARD = 25,
    CMD_HW_SITAU_ENABLED = 26,
    CMD_FP_TRIGGER_SOURCE = 27,
    CMD_STATUS_CONFIG = 28,
    CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 = 29,
    CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT = 30,
    CMD_VCH_EMISSION_FOCAL_LAW_EXT = 31,
    CMD_TFM_PWI_FORWARD = 32,
    CMD_FP_VIRTUAL_CHANNEL_CONFIG = 33,
} T_CMD_Type;


typedef struct TFP_CMD_PASystemConfig  {
   unsigned char exists;
   unsigned char enabled;
   unsigned char multiplexed;
   unsigned char n_bases;
   unsigned char id_base_first;
   unsigned char id_base_last;
   unsigned short id_fl_first;
   unsigned short id_fl_last;
   unsigned short actual_start_element;
   unsigned short actual_focal_law;
   unsigned short n_active_channels;
   unsigned short n_multiplexed_channels;
   unsigned short n_total_channels;
} TFP_CMD_PASystemConfig;

typedef struct TFP_MSG_PASystemConfig {
   TFP_CMD_PASystemConfig pa_sys[UCI_N_MAX_SUBSYSTEM];
} TFP_MSG_PASystemConfig;

typedef struct TFP_MSG_SITAUConfig {
   unsigned char n_bases;
} TFP_MSG_SITAUConfig;

typedef struct TFP_MSG_SITAUTemperature {
   unsigned char temp_1;
   unsigned char temp_2;
   unsigned char hyst;
} TFP_MSG_SITAUTemperature;

typedef struct TFP_MSG_PulseAmplitude {
   unsigned char data;
} TFP_MSG_PulseAmplitude;

typedef struct TMSG_Config {
    int id_vch;
    unsigned char enabled;
    unsigned char n_bases;
    unsigned char acquisition_type;
    unsigned long gtx_n_focal_law;
    unsigned short start_element;
    unsigned short n_acq_samples;
    unsigned short n_beamformed_samples;
    unsigned short gain_da;
    unsigned short gain_hi;
    float prf_time_line;
    float prf_time_burst;
    unsigned char n_mux_inc;
    unsigned char signal_mode;

    unsigned char processing_type;
    unsigned char wait_prf_signals_processing;
    unsigned short n_signals_processing_avr;
    unsigned short n_signals_processing_emi;

    unsigned char enabled_timestamp;
    unsigned char enabled_buffer_dma;
    unsigned char enabled_temperature;
    unsigned char enabled_encoder_1;
    unsigned char enabled_encoder_2;
    unsigned char enabled_encoder_3;
    unsigned char enabled_encoder_4;

    u8 external_trigger;
    u8 decimation_factor;
    u32 water_delay;
    u8 log2_average;

    TFP_VCH_AFE_SPI afe_spi;
    TMSG_STRUCT_Pulser pulser;

    TMSG_BASE base_reg[MAX_N_BASES];	//!< Estructura con los registros de las Bases
} TMSG_Config;

typedef struct TMSG_TGC {
   unsigned short id_vch;
   unsigned short n_points;
   unsigned char curve[TGC_MAX_SIZE];
} TMSG_TGC;

typedef struct TMSG_FIR {
   unsigned short id_vch;
   unsigned char enabled;
   unsigned short n_coefficients;
   short int coefficients[FIR_N_COEFFICIENTS];
} TMSG_FIR;

typedef struct TMSG_Beamformer {
   int id_vch;
   unsigned long n_acq_samples; // (PA) n_afe_samples = n_samples_beamformed + aperture_length
                                       // (TFM) n_afe_samples = n_samples_diagonal_ida_vuelta;
   unsigned long n_beamformed_samples;  
   unsigned short n_fl;
   unsigned short n_fl_parallel; // 1, 8, 16, 32
   unsigned short t0_max;
   int n_lword;
   int shift_bits;
   int beamforming_type;
   TMSG_STRUCT_Beamformer base[MAX_N_BASES_MSG];
} TMSG_Beamformer;

typedef struct TMSG_BeamformerExtended {
    unsigned short id_vch;
    unsigned short id_first_base;
    unsigned long n_acq_samples; // (PA) n_afe_samples = n_samples_beamformed + aperture_length
    // // (TFM) n_afe_samples = n_samples_diagonal_ida_vuelta;
    unsigned long n_beamformed_samples;
    unsigned short n_fl;
    unsigned short n_fl_parallel; // 1, 8, 16, 32
    unsigned short t0_max;
    int n_lword;
    int shift_bits;
    int beamforming_type;
    TMSG_STRUCT_Beamformer base[MAX_N_BASES_MSG];
} TMSG_BeamformerExtended;

typedef struct TMSG_TFM_FMC_Forward {
    int id_vch;
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
    unsigned long memory[TFM_FMC_FORWARD_MEMORY_SIZE_MSG];
} TMSG_TFM_FMC_Forward;

typedef struct TMSG_TFM_PWI_Forward {
    int id_vch;
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
    unsigned long forwardMemory[TFM_PWI_FORWARD_MEMORY_SIZE_MSG];
    unsigned short anglesMemory[MAX_PWI_FL];
} TMSG_TFM_PWI_Forward;

typedef struct TMSG_EmissionFocalLaws {
   unsigned short id_vch;
   unsigned short n_bases;
   unsigned short n_fl;
   TMSG_STRUCT_EmissionFocalLaw base[MAX_N_BASES_MSG];
} TMSG_EmissionFocalLaws;

typedef struct TMSG_EmissionFocalLawsExtended {
    unsigned short id_vch;
    unsigned short id_first_base;
    unsigned short n_bases;
    unsigned short n_fl;
    TMSG_STRUCT_EmissionFocalLaw base[MAX_N_BASES_MSG];
} TMSG_EmissionFocalLawsExtended;

typedef union TMSG_STRUCT_TriggerSource {
   unsigned long REG;
   struct {
      unsigned long SW2HW : 1;  // Bit[0]: Software UCI
      unsigned long EXT : 1;  // Bit[1]: External Signal
      unsigned long PRF_1 : 1;  // Bit[2]: PRF Timer 1
      unsigned long PRF_2 : 1;  // Bit[3]: PRF Timer 2
      unsigned long TIME_STAMP : 1;  // Bit[4]: Time Stamp
      unsigned long ENC_1 : 1;  // Bit[5]: Encoder 1
      unsigned long ENC_2 : 1;  // Bit[6]: Encoder 2
      unsigned long ENC_3 : 1;  // Bit[7]: Encoder 3
      unsigned long ENC_4 : 1;  // Bit[8]: Encoder 4
      unsigned long NOP : 23; // Bit[9:31]: Reserved
   } BIT;
} TMSG_STRUCT_TriggerSource;

typedef struct TMSG_TriggerSource {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned long n_images_burst;
   unsigned short delay_sync_n_acq;
   unsigned short acquisition_mode;
   unsigned short data_link_mode;
   unsigned short pause_after_burst;
} TMSG_TriggerSource;

typedef struct TMSG_StatusConfig {
    unsigned char enabled;
    unsigned char pcie;
    unsigned long period_ms;
} TMSG_StatusConfig;

typedef struct TMSG_Pause {
    unsigned long n_acquisitions;
} TMSG_Pause;

typedef struct TMSG_LoadFirmware {
   unsigned long size_file;
} TMSG_LoadFirmware;

#define MSG_VERSION_DATE_SIZE   24
#define MSG_VERSION_DEVICE_SIZE 24
typedef struct T_CMD_Version {
   unsigned char branch;
   unsigned char major;
   unsigned char minor;
   char date[MSG_VERSION_DATE_SIZE];
   char device[MSG_VERSION_DEVICE_SIZE];
} T_CMD_Version;

typedef struct TMSG_Version {
   unsigned short checksum;
   T_CMD_Version cpu0;
   T_CMD_Version cpu1;
   unsigned long hw_uci;
   unsigned long hw_mk32;
} TMSG_Version;

typedef struct TMSG_SITAUStatus {
   unsigned short checksum;
   unsigned __int64 status_code;
   unsigned long rd_address;
   unsigned long wr_address;
   unsigned long n_overflows;
   unsigned char sitau_status;
   unsigned char trigger_source;
   unsigned long n_acquisitions;
   unsigned long ind_acquisitions;
   unsigned long size_buffer_command;
   TSHRD_LOG_ErrorRegister log_error;
} TMSG_SITAUStatus;

typedef union TMSG_TerminalLog {
   unsigned short REG;          // 32Bits Access
   struct {                     // BIT  Access
      unsigned short PRG_BASE_WMPAR          : 1;  // Bit[0]
      unsigned short PRG_MOD_WMPAR           : 1;  // Bit[1]
      unsigned short AMPLIA_SEND_COMMAND     : 1;  // Bit[2]
      unsigned short PRG_REG                 : 1;  // Bit[3]
      unsigned short ACQUIRING               : 1;  // Bit[4]
      unsigned short ACQUIRING_FRAME         : 1;  // Bit[5]
      unsigned short PROTOCOL                : 1;  // Bit[6]
      unsigned short BEAMFORMING             : 1;  // Bit[7]
      unsigned short NOP                     : 8;  // Bit[8:15]
   } BIT;
} TMSG_TerminalLog;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern bool gb_load_config_enabled;
extern int gb_log_protocol_enabled;
extern TMSG_TerminalLog gb_terminal_log;

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

//int TCPSend_MSG_VERSION (TMSG_Version *version);

int TCPSend_MSG_STOP(void);

int TCPSend_MSG_PAUSE(TMSG_Pause* msg, bool wait=false);

int TCPSend_MSG_CONTINUE(void);

int TCPSend_MSG_PulseAmplitude (unsigned char data);

int TCPSend_MSG_RESET_ACQ_COUNTER(void);

int TCPSend_MSG_RESET_ENCODER(int id_encoder);

int TCPSend_MSG_ACQUIRE(unsigned short n_acq);

int TCPSend_MSG_PrintPASystem(void);

int TCPSend_MSG_TriggerSource(TMSG_TriggerSource *msg);

int TCPSend_MSG_StatusConfig(TMSG_StatusConfig* msg);

int TCPSend_MSG_VCH_Config(TMSG_Config *msg);

int TCPSend_MSG_VCH_FIR(TMSG_FIR * msg);

int TCPSend_MSG_VCH_EmissionFocallaws(TMSG_EmissionFocalLaws *msg);

int TCPSend_MSG_VCH_EmissionFocallawsExtended(TMSG_EmissionFocalLawsExtended* msg);

int TCPSend_MSG_VirtualChannel_Beamformer(TMSG_Beamformer *msg);

int TCPSend_MSG_VirtualChannel_BeamformerExtended(TMSG_BeamformerExtended* msg);

int TCPSend_MSG_TFM_FMC_Forward(TMSG_TFM_FMC_Forward* msg);

int TCPSend_MSG_TFM_PWI_Forward(TMSG_TFM_PWI_Forward* msg);

int TCPSend_MSG_VirtualChannel_TGC(TMSG_TGC *msg);

int TCPSend_MSG_PrintVirtualChannel_Config(int id_vch);

int TCPSend_MSG_PrintVirtualChannel_BASERegisters(int id_vch);

int TCPSend_MSG_TERMINAL_LOG(unsigned short data);

int TCPSend_MSG_SITAUHardwareEnabled(int data);

int TCPSend_MSG_PrintConfig(void);

int TCPSend_MSG_CACHE_ENABLED(void);

int TCPSend_MSG_CACHE_DISABLED(void);

int TCPSend_MSG_IMAGE_ENABLED(void);

int TCPSend_MSG_IMAGE_DISABLED(void);

int TCPSend_MSG_LoadFirmware(char *buffer, unsigned long n_data);

bool TCPSend_MSG_SHARED_MEM(void);

#endif

