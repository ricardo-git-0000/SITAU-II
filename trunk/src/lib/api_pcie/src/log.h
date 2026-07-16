#ifndef logH
#define logH

#pragma once

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <exception>
#include <string>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define SIZE_MSG_LOG	1024
// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern int gb_log_enabled;
extern char *gb_log_msg;
extern char *gb_log_thr_msg;
extern int gb_log_enabled;
extern string gb_log_pcie_thread_file_path;

// ____________________________________________________________________________
// ------ M A C R O S
// ============================================================================

#if defined(__FUNC__)
   // Crea un nuevo registro de error de aplicación y lo propaga
   #define ELOG(msg_err,code_parameter) log_err(msg_err,code_parameter,(unsigned long)__LINE__,__FILE__,__FUNC__);
#else
	// Crea un nuevo registro de error de aplicación y lo propaga
	#define ELOG(msg_err,code_parameter) log_err(msg_err,code_parameter,(unsigned long)__LINE__, __FILE__, NULL);
	#define ELOG_THR(file_log, msg_err, code_parameter) log_err(file_log, msg_err, code_parameter,(unsigned long)__LINE__, __FILE__, NULL);
#endif

#if defined(__FUNC__)
   // Registra el error de aplicación y lo propaga
	#define RLOG(code_parameter) log_err(NULL,code_parameter,(unsigned long)__LINE__,__FILE__,__FUNC__);
#else
	// Registra el error de aplicación y lo propaga
	#define RLOG(code_parameter) log_err(NULL,code_parameter,(unsigned long)__LINE__,__FILE__);
	#define RLOG_THR(file_log, code_parameter) log_err(file_log, NULL,code_parameter,(unsigned long)__LINE__,__FILE__);
#endif

// _____________________________________________________________________________
// ------ F U N C T I O N S   P R O T O T Y P E
// =============================================================================

double LOG(char *file_log, char *msg_err, double code_parameter, unsigned long line, char *file, char *func);
float log (char *msg_err=NULL, float code=0, unsigned long line=0, char *file=NULL, char *func=NULL);
float log_err (char *msg_err=NULL, float code=0, unsigned long line=0, char *file=NULL, char *func=NULL);
int log_err	(char *msg_err=NULL, int code=0, unsigned long line=0, char *file=NULL, char *func=NULL);
long log_err (char *msg_err=NULL, long code=0, unsigned long line=0, char *file=NULL, char *func=NULL);
float log_err(char *file_log = NULL, char *msg_err = NULL, float code = 0, unsigned long line = 0, char *file = NULL, char *func = NULL);
int log_err(char *file_log = NULL, char *msg_err = NULL, int code = 0, unsigned long line = 0, char *file = NULL, char *func = NULL);
long log_err(char *file_log = NULL, char *msg_err = NULL, long code = 0, unsigned long line = 0, char *file = NULL, char *func = NULL);
void LOG_Open(char *str);
void LOG_Open(char *file_log, char *str);
int LOG_Info(char *file_log, char* msg);
void LOG_Close(void);
void LOG_Close(char *file_log);
int LOG_GetEnabledFile(void);
void LOG_SetEnabledFile(int data);

#endif

