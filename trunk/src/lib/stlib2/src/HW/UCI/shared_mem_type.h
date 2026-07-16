#ifndef shared_mem_typeH
#define shared_mem_typeH
// -----------------------------------------------------------------------------
/**
 @file shared_mem_typeH.h

 @brief Este archivo contiene la implementaci√≥n de las funciones de configuraci√≥n
 y acceso de los encoders
 
 @author (rg) Ricardo Gonz√°lez

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 13/03/2024 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include "xil_types.h"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I Û N   D E   C O N S T A N T E S
// =============================================================================

#define SHRD_FRAME_VERSION	0x00000001
#define SHRD_INFO			0x00000002

#define SHRD_SIZE_ENCODERS	4

#define SHRD_VERSION_DATE_SIZE           24
#define SHRD_VERSION_DESCRIPTION_SIZE    24

#define SHRD_CPU0_INFO_SIZE				5
#define SHRD_CPU1_INFO_SIZE				5

#define SHRD_LOG_ERROR_MSG_SIZE    100
#define SHRD_LOG_ERROR_FILE_SIZE   50
#define SHRD_LOG_ERROR_STACK_SIZE  5
#define SHRD_LOG_ERROR_SIZE        1

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct T_Version
{
	u8 branch_sw;
	u8 major_sw;
	u8 minor_sw;
	char date[SHRD_VERSION_DATE_SIZE];
	char description[SHRD_VERSION_DESCRIPTION_SIZE];
} T_Version;

typedef struct TSHRD_CPU1_LastAcquisitionInfo
{
	u64 time_stamp;
	u32 encoder_value[SHRD_SIZE_ENCODERS];
	u64 status_code;
	u32 rd_address;
	u32 wr_address;
	u32 n_overflows;
	u8 sitau_status;
	u8 trigger_source;
	u32 n_acquisitions;
	u32 ind_acquisitions;
	u32 size_buffer_command;

} TSHRD_CPU1_LastAcquisitionInfo;

typedef union TSHRD_CPU1_Flags
{
	u32 REG;
	struct
	{
		u32 CPU1_LOADED 		: 1;	// Bit[0]:
		u32 CPU1_STARTED 		: 1;	// Bit[1]:
		u32 MINI_BASE_LOADED 	: 1;	// Bit[2]:
		u32 ENABLED_CACHE		: 1;	// Bit[3]:
		u32 HIGH_VOLTAGE		: 1;	// Bit[4]:
		u32 FSM_STARTED			: 1;    // Bit[5]:
		u32 STATUS_ENABLED		: 1;    // Bit[6]:
		u32 RESERVED_BITS 		: 25;	// Bit[7:31]:
	} BIT;
} TSHRD_CPU1_Flags;

typedef struct TSHRD_CPU1
{
	u32 status;
	u32 info[SHRD_CPU1_INFO_SIZE];
	TSHRD_CPU1_Flags flags;
	TSHRD_CPU1_LastAcquisitionInfo acquisition;
} TSHRD_CPU1;

typedef union TSHRD_CPU0_Flags
{
	u32 REG;
	struct
	{
		u32 ERROR_GIC 			: 1;	// Bit[0]:
		u32 ERROR_MUTEX 		: 1;	// Bit[1]:
		u32 ERROR_PLATFORM 		: 1;	// Bit[2]:
		u32 ERROR_NETWORK 		: 1;	// Bit[3]:
		u32 ENABLED_CACHE 		: 1;	// Bit[4]:
		u32 ENABLED_IMAGE 		: 1;	// Bit[5]:
		u32 RESERVED_BITS 		: 28;	// Bit[6:31]:
	} BIT;
} TSHRD_CPU0_Flags;

typedef struct TSHRD_CPU0
{
	u32 status;
	u32 info[SHRD_CPU0_INFO_SIZE];
	TSHRD_CPU0_Flags flags;
} TSHRD_CPU0;

typedef struct TSHRD_LOG_Error
{
	int code;
	char file[SHRD_LOG_ERROR_FILE_SIZE];
	u16 line;
} TSHRD_LOG_Error;

typedef struct TSHRD_LOG_ErrorStack
{
	char msg[SHRD_LOG_ERROR_MSG_SIZE];
	int code;
	char file[SHRD_LOG_ERROR_FILE_SIZE];
	u16 line;
	u8 count;
	TSHRD_LOG_Error s[SHRD_LOG_ERROR_STACK_SIZE];
} TSHRD_LOG_ErrorStack;

typedef struct TSHRD_LOG_ErrorRegister
{
	u8 count;
	TSHRD_LOG_ErrorStack e[SHRD_LOG_ERROR_SIZE];
} TSHRD_LOG_ErrorRegister;

typedef struct TSHRD_GTX_Status
{
	union
	{
		u32 gtx_status_counters_reg_32;
		struct
		{
			u8 channel_not_ready;
			u8 soft_errors;
			u8 hard_errors;

			u8 reserved1 : 7;
			u8 reset_counters : 1;
		}BITS;
	};
} TSHRD_GTX_Status;

typedef struct TSHRD_0x00000002
{
	T_Version sw_version_cpu0;
	T_Version sw_version_cpu1;
	u32 hw_version;
	u32 mk32_version;
	TSHRD_CPU0 cpu0;
	TSHRD_CPU1 cpu1;

	u8 AFES_align_ok;
	u8 LVDS_deskew_ok;
	u32 errores_lvds;
	u32 errores_ddr;
	u32 codigo_pulser_leido;
	u8 n_bases_bcc;
	u8 retardo_extra_bcc;

	u8 qsfp_channel_rdy_up; // ConexiÛn interna
	u8 qsfp_channel_rdy_dn; // ConexiÛn externa

	TSHRD_GTX_Status qsfp_link_errors_dn;
	TSHRD_GTX_Status qsfp_link_errors_up;

	TSHRD_LOG_ErrorRegister log;
	u16 check_word;
	u8 check_byte;
} TSHRD_0x00000002;

#endif
