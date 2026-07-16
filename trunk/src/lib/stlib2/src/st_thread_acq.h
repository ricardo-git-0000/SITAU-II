#ifndef st_thread_acqH
#define st_thread_acqH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

//#include <Classes.hpp>
////#include <winsock2.h>   //prueba
//
//#define NOMINMAX
//#include <windows.h>
//#include <thread_vsapi.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

//#if defined(_MSC_VER)
   #include "thread_vsapi.h"
//#endif

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum TThread_Acq_Status
{
	ST_ThreadAcq_Stopped             = 0,
	ST_ThreadAcq_Starting            = 1,
	ST_ThreadAcq_StartingBuffer      = 2,
	ST_ThreadAcq_Running             = 3,
	ST_ThreadAcq_Waiting             = 4,
	ST_ThreadAcq_Acquiring           = 5,
	ST_ThreadAcq_Stopping            = 6,
   ST_ThreadAcq_StoppingThreadScan  = 7,
} TThread_Acq_Status;

// _____________________________________________________________________________
// ------ TThread_Acq class
// _____________________________________________________________________________
// =============================================================================

class TThread_Acq
{
private:

   unsigned long time_;
   bool locked_;
   bool end_;
   TThread_Acq_Status status_;
   int error_;
   //int acq_counter_;

protected:
    
public:

   HANDLE thread_handle_;
   DWORD thread_id_;
   bool Terminated;
   DWORD wait_timeout_;
   TThreadPriority Priority;

   TThread_Acq (bool CreateSuspended, unsigned long wait_time=0);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(void) {WaitForSingleObject(thread_handle_, wait_timeout_);}
   int Resume(void);
   int Acquire(void);
   bool End(void) {return end_;}
   TThread_Acq_Status GetStatus(void) {return status_;}
   void Locked(bool data) {locked_ = data;}
   bool Locked(void) {return locked_;}
   int GetError(void) {return error_;};
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_Acq *gb_process_acq;

#endif
