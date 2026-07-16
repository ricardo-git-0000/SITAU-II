#ifndef protocol_baseH
#define protocol_baseH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include "protocol_define.h"
#include "t_base_tad.h"

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

// ____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ============================================================================

#define CMD_BASE_MAX_REG_COMMAND    50 + FP_BASE_MEM_GAN

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

/**
Registros de la BASE
*/
typedef struct TMSG_STRUCT_Pulser
{  
   unsigned char enabled;
   unsigned char n_pulses;
   unsigned char high_z;
   unsigned char current;
   unsigned long pulse_width;
	unsigned long main_delay;
} TMSG_STRUCT_Pulser;

typedef struct TMSG_STRUCT_EmissionFocalLaw {
   unsigned short efl[MOD_MAX_CH * FP_BASE_MAX_FOCAL_LAW];
} TMSG_STRUCT_EmissionFocalLaw;

typedef struct TMSG_BASE {
   unsigned char enabled;
   unsigned char id_base;
} TMSG_BASE;

typedef struct TFP_CMD_Channel_FocalLaw {
   unsigned short emitter_delay[MOD_MAX_CH];
   unsigned char receiver_enabled[MOD_MAX_CH];   
} TFP_CMD_Channel_FocalLaw;

typedef struct TMSG_BASE_FP_ReceptionFocalLaw {
   unsigned short n_ascan;
   unsigned char ch[MOD_MAX_CH];   
} TMSG_BASE_FP_ReceptionFocalLaw;

typedef struct TMSG_STRUCT_Beamformer {
   u32 bf_memory[FP_BASE_BEAMFORMER_MEMORY_SIZE]; // size = n_fl * 32
} TMSG_STRUCT_Beamformer;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================
                        
#endif

