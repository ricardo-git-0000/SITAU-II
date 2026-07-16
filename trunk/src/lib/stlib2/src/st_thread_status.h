#ifndef st_thread_statusH
#define st_thread_statusH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

//#include <Classes.hpp>
//#include <winsock2.h>   //prueba
//
//#define NOMINMAX
#include <windows.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "thread_vsapi.h"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ TThread_Status class
// _____________________________________________________________________________
// =============================================================================

class TThread_Status
{
private:

   unsigned long period_ms_;
   bool locked_;
   bool locked_ok_;
   bool end_;
   //TThread_Acq_Status status_;
   int error_;
   //int acq_counter_;

protected:
    
public:

   HANDLE thread_handle_;
   DWORD thread_id_;
   bool Terminated;
   TThreadPriority Priority;

   TThread_Status (bool CreateSuspended, unsigned long timeout=500);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(void) {WaitForSingleObject(thread_handle_, 3000);}
   int Resume(void);
   bool End(void) {return end_;}
   void Locked(bool data) {locked_ = data;}
   bool Locked(void) {return locked_ok_;}
   int GetError(void) {return error_;};
   bool IsRunning(void) { return !end_ && !locked_; }
};

int CreateStatusThread(unsigned long wait_time_ms);
void DeleteStatusThread(void);
bool IsRunningStatusThread(void);
bool LockStatusThread(void);
bool UnLockStatusThread(void);
bool IsLockedStatusThread(void);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_Status * gb_process_status;

#endif
