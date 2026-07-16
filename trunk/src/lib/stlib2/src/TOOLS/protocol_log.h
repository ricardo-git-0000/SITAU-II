#ifndef protocol_logH
#define protocol_logH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define LOG_ERROR_MSG_SIZE    255
#define LOG_ERROR_FILE_SIZE   100
#define LOG_ERROR_STACK_SIZE  10
#define LOG_ERROR_SIZE        5

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct T_LOG_Error
{
   int code;
   char file[LOG_ERROR_FILE_SIZE];
   int line;   
} T_LOG_Error;

typedef struct T_LOG_ErrorStack
{
   char msg[LOG_ERROR_MSG_SIZE];
   int  s_idx;
   T_LOG_Error s[LOG_ERROR_STACK_SIZE];
} T_LOG_ErrorStack;

typedef struct T_LOG_ErrorRegister
{
   int e_idx;
   T_LOG_ErrorStack e[LOG_ERROR_SIZE];
} T_LOG_ErrorRegister;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#endif
