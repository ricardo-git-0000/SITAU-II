#ifndef c_log_fileH
#define c_log_fileH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <stdlib.h>
#include <dos.h>
#include <time.h>
#include <string>
#include <io.h>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"

// -----------------------------------------------------------------------------
// USER INFORMATION ============================================================

#define LOG_DEFAULT_FILE_NAME   "LOG_STLIB2.TXT"
#define LOG_DEFAULT_NAME		"STLIB2 Library"
#define LOG_DEFAULT_ID			0xFFFF

#define LOG_STATUS_FILE_NAME	"LOG_STATUS.TXT"
#define LOG_STATUS_NAME			"Status Thread"

#define LOG_STATUS_FILE_NAME	"LOG_STATUS.TXT"
#define LOG_STATUS_NAME			"Status Thread"

extern int LOG_STATUS;

// USER INFORMATION ============================================================
// -----------------------------------------------------------------------------

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define LOG_EXCEPTION_CODE		-1
#define LOG_TRACE_SIZE			1024
#define LOG_ERROR_MSG_SIZE		255
#define LOG_ERROR_FILE_SIZE		100
#define LOG_ERROR_STACK_SIZE	10
#define LOG_ERROR_SIZE			5

#define ELOG(msg, code) LOG_Error(-1, msg, code, -1, __LINE__, __FILE__, __FUNCTION__);
#define RLOG(code) LOG_Error(-1, nullptr, code, -1, __LINE__, __FILE__, __FUNCTION__);
#define ELOGIT(msg,code,iteration) LOG_Error(-1, msg, code, iteration, __LINE__, __FILE__, __FUNCTION__);
#define NEELOG() LOG_Error(-1, "(EXCEPTION: NO CATCHING)", LOG_EXCEPTION_CODE, -1, __LINE__, __FILE__, __FUNCTION__);
#define NEELOGIT(iteration) LOG_Error(-1, "(EXCEPTION: NO CATCHING)", LOG_EXCEPTION_CODE, iteration, __LINE__, __FILE__, __FUNCTION__);

#define ELOGf(dev, msg, code) LOG_Error(id, msg, code, -1, __LINE__, __FILE__, __FUNCTION__);
#define RLOGf(dev, code) LOG_Error(didev, nullptr, code, -1, __LINE__, __FILE__, __FUNCTION__);
#define ELOGITf(dev, msg,code,iteration) LOG_Error(id, msg, code, iteration, __LINE__, __FILE__, __FUNCTION__);
#define NEELOGf(dev) LOG_Error(id, "(EXCEPTION: NO CATCHING)", LOG_EXCEPTION_CODE, -1, __LINE__, __FILE__, __FUNCTION__);
#define NEELOGITf(dev, iteration) LOG_Error(id, "(EXCEPTION: NO CATCHING)", LOG_EXCEPTION_CODE, iteration, __LINE__, __FILE__, __FUNCTION__);

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

//typedef struct T_LOG_Error
//{
//	int code;
//	char file[LOG_ERROR_FILE_SIZE];
//	int line;
//} T_LOG_Error;
//
//typedef struct T_LOG_ErrorStack
//{
//	char msg[LOG_ERROR_MSG_SIZE];
//	int  s_idx;
//	T_LOG_Error s[LOG_ERROR_STACK_SIZE];
//} T_LOG_ErrorStack;
//
//typedef struct T_LOG_ErrorRegister
//{
//	int e_idx;
//	T_LOG_ErrorStack e[LOG_ERROR_SIZE];
//} T_LOG_ErrorRegister;

// _____________________________________________________________________________
// ------ CLogFile class
// _____________________________________________________________________________
// =============================================================================
class CLogFile
{
public:

	CLogFile() {};
	CLogFile(char* file, char* name, char* version, bool file_opened=false, bool lock_enaled=false);
	~CLogFile();
	void Default(void);
	void Close(void);
	void Info(char* msg, ...);
	void Info(char* msg, va_list argptr);
	template <typename T_Generic>
	T_Generic Error(char* msg_err, T_Generic code_parameter, int iteration, unsigned long line, char* file, char* function);
	//void Error(T_LOG_ErrorRegister* error);
	int Create(char* file, char* name, char* version, bool file_opened = false, bool lock_enabled = false);
	int FileOpened(bool data);
	string FileName(void) { return file_name_; }
	string LastErrorMsg(void) { return last_error_msg_; }
	int LastErrorCode(void) { return last_error_code_; }
	void LastErrorEnabled(bool data) { last_error_enabled_ = data; }
	void IDThread(int data) { id_thread_ = data; }
	int IDThread(void) { return id_thread_; }

protected:

private:

	bool created_;
   bool locked_;
   int error_;
   bool file_opened_;
   bool carriage_return_;
   bool last_error_enabled_;
   string last_error_msg_;
   int last_error_code_;
   CCriticalSection * plock_;
   string file_name_;
   string log_file_path_;
   string name_;
   FILE* fs_;
   int id_thread_;
};

int LOG_Error(int id, char* msg_err, int code_parameter, int iteration, unsigned long line, char* file, char* function);
float LOG_Error(int id, char* msg_err, float code_parameter, int iteration, unsigned long line, char* file, char* function);
double LOG_Error(int id, char* msg_err, double code_parameter, int iteration, unsigned long line, char* file, char* function);
unsigned long LOG_Error(int id, char* msg_err, unsigned long code_parameter, int iteration, unsigned long line, char* file, char* function);
unsigned long long LOG_Error(int id, char* msg_err, unsigned long long code_parameter, int iteration, unsigned long line, char* file, char* function);
long long LOG_Error(int id, char* msg_err, long long code_parameter, int iteration, unsigned long line, char* file, char* function);
int LOG_LastError(char* msg, int id = -1);
//void LOG_Register(T_LOG_ErrorRegister* data, int id = -1);
void LOG_SetLastErrorEnabled(bool data);
int LOG_Create(char* file, char* name, char* version);
int LOG_IDThread(char* file, int id);
void LOG_Open(char* version);
void LOG_Close(void);
void LOG_Delete(void);
void LOG_Info(char* msg, ...);
void LOG_Info(int id, char* msg_err, ...);

extern int gb_log_enabled;
extern string gb_log_file_path;
extern int gb_last_error_code;
extern string gb_last_error_msg;

#endif
