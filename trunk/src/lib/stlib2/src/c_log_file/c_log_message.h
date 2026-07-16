#ifndef c_log_messageH
#define c_log_messageH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

//#include <stdlib.h>
//#include <dos.h>
//#include <time.h>
#include <string>
//#include <io.h>
#include <vector>
//#define NOMINMAX
//#include <windows.h>
//
using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TLogMessage
{
	bool update;
	string data;
} TLogMessage;

// _____________________________________________________________________________
// ------ CLogMessage class
// _____________________________________________________________________________
// =============================================================================
class CLogMessage
{
public:

	CLogMessage::CLogMessage(int n_max_message = 100);
	CLogMessage::~CLogMessage();
	void Write(char* msg, ...);
	void Write(char* msg, va_list argptr);
	int Read(char* data);
	void Readlast(char* data) { strcpy(data, last_message_.c_str()); }
	void StartDelay(int data) { delay_ = data; }
	void StopDelay(void) { delay_ = 0; }
	int Delay(void) { return delay_; }
	void Clear(void) { messages_.clear(); }
	void SetDeleteSignal(void) { delete_signal_ = true; }
	bool GetDeleteSignal(void) { return delete_signal_; }

protected:

private:

	unsigned __int64 rd_;
	unsigned __int64 wr_;
	int delay_;
	int n_max_message_;
	bool delete_signal_;
	char str_[255];
	char date_[255];
	char msg_[255];
	vector<TLogMessage> messages_;
	string last_message_;   
	CCriticalSection* plock_;
};


void LOG_Message(char* msg, ...);
void LOG_StartDelay(int data);
void LOG_StopDelay(void);

extern CLogMessage* gb_msg;

#endif
