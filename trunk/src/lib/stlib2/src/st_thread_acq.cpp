// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <winsock2.h>   //prueba

//#define NOMINMAX
//#include <windows.h>
//#include <thread_vsapi.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"
#include "st_thread_acq.h"
#include "st_error_code.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_reg.h"
#include "c_log_file.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "c_buffer_hw.h"
#include "c_virtual_channel.h"
#include "winsock_api.h"
#include "api_tcp.h"

#if defined(_MSC_VER)
   #include "thread_vsapi.h"
#else
   #pragma package(smart_init)
#endif

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#if !defined(_MSC_VER)
TShape *gb_getting_data = NULL;
#endif

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TThread_Acq *gb_process_acq = NULL;

// _____________________________________________________________________________
// ------ Visual Studio Inferface - TThread_Acq class Methods
// _____________________________________________________________________________
// =============================================================================
#if defined(_MSC_VER)
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
DWORD WINAPI TThread_Acq_Execute(LPVOID lpParam)
{
DWORD result = 0;

   if (gb_process_acq != NULL)
   {
      gb_process_acq->ThreadFunction();
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
HANDLE TThread_Acq_CreateThread (bool CreateSuspended, LPDWORD thread_id)
{
HANDLE result = NULL;
DWORD dwCreationFlags = 0;

   //if (gb_process_acq == NULL) return ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   if (CreateSuspended) dwCreationFlags = CREATE_SUSPENDED;
   if ((result = CreateThread (   
            NULL,                      // default security attributes
            0,                         // use default stack size
            TThread_Acq_Execute,   // thread function name
            NULL,                      // argument to thread function
            dwCreationFlags,           // use default creation flags
            thread_id)) == NULL)     // returns the thread identifier
   {
      //Microsoft::VisualStudio::ErrorHandler(TEXT("CreateThread"));
      ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
   return result;
}

#endif

// _____________________________________________________________________________
// ------ TThread_Acq class Methods
// _____________________________________________________________________________
// =============================================================================

//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
#if defined(_MSC_VER)
TThread_Acq::TThread_Acq ( bool CreateSuspended,
   								//unsigned long n_acq,
                           unsigned long wait_time)
#else
__fastcall TThread_Acq::TThread_Acq (	bool CreateSuspended,
													unsigned long wait_time)
    : TThread(CreateSuspended)
#endif
{
   end_ = false;
   time_ = wait_time;

   if (!gb_no_hw)
   {
      if (time_ > 100) time_ = 1000;
   }

   locked_ = false;
#if !defined(_MSC_VER)
   wake_ = NULL;
#endif

   //status_file_name_ = gb_st_file_path + UT_THREAD_STATUS_FILE;
   status_ = ST_ThreadAcq_Stopped;
//   if ((f = fopen (status_file_name_.c_str(), "wt")) != NULL)
//   {
//      time(&t);
////      fseek(f, 0, SEEK_END);
//      fprintf(f, "\r\n%s TThread_Acq -> Created", ctime(&t));
//   }
//   fclose(f);
#if defined(_MSC_VER)
   this->Terminated = false;
   this->error_ = ES2_NONE;
   this->wait_timeout_ = 3000;
   this->Priority = tpNormal;
   this->thread_handle_ = TThread_Acq_CreateThread (CreateSuspended, &this->thread_id_);
   if (thread_handle_ == NULL)  // returns the thread identifier
   {
      //Microsoft::VisualStudio::ErrorHandler(TEXT("CreateThread"));
      ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
#endif
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
void TThread_Acq::ThreadFunction(void)
{
int i, result;
FILE *f=NULL;
CTimeout *timeout = NULL;

    //LOG_Message("Frame Acquisition Thread Started");
    //LOG_Info(gb_msg_progress_open_sys);

   end_ = false;
   status_ = ST_ThreadAcq_Starting;

   gb_buffer_hw->Clear_AcquisitionBuffer();
   gb_buffer_hw->DBG_FileOpen();

   //acq_counter_ = 0;
   gb_no_hardware_waiting_continue = false;
   gb_no_hardware_waiting_continue_previous = true;
   status_ = ST_ThreadAcq_Running;
   while (!Terminated && error_ >= 0 && ! end_)
   {
      if (!locked_)
      {
         if (gb_no_hw)
         {
            //if (timeout != NULL && timeout->Timeout())
            {
               if ((result = ST_Run(1)) < 0)  error_ = RLOG(result);
               status_ = ST_ThreadAcq_Acquiring;
               if ((result = Acquire()) < 0) error_ = RLOG(result);

               if (UCIP.acquisition_mode == ACQUISITION_MODE_FIXED)// && UCIP.focallaw_mode == 0)
               {
                  //acq_counter_++;
                  end_ = gb_buffer_hw->AcquiredImages() >= UCIP.n_bursts * UCIP.n_images_burst;
               }
            }
         }
         else
         {
            status_ = ST_ThreadAcq_Acquiring;
            if ((result = Acquire()) < 0)
               error_ = RLOG(result);
         }
      }
	}
   status_ = ST_ThreadAcq_Stopping;
   gb_buffer_hw->DBG_FileClose();

   for (i=0; i<(int)gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL) gb_virtual_channel[i]->Release();
   }
   if (timeout != NULL) delete timeout;

#if !defined(_MSC_VER)
   delete wake_;
#endif

   status_ = ST_ThreadAcq_StoppingThreadScan;

   //if (gb_log_enabled && gb_log_thread_file_path != "")
   //{
   //   if ((f = fopen (gb_log_thread_file_path.c_str(), "at")) != NULL)
   //   {
   //      time(&t);
   //      fprintf(f, "\r\n%s TThread_Acq (id: %d) -> End", ctime(&t), GetCurrentThreadId());
   //   }
   //   fclose(f);
   //}
   status_ = ST_ThreadAcq_Stopped;
   end_ = true;
   //LOG_Message("Frame Acquisition Thread Closed");
   //LOG_Info(gb_msg_progress_open_sys);

}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int TThread_Acq::Acquire(void)
{
int result;

   if ((result = ST_Acquire()) < 0)
      return RLOG(result);

   return result;
}

#if defined(_MSC_VER)
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_Acq::Resume(void)
{
DWORD dword_value;
int priority;
   
   switch(Priority)
   {
   case tpIdle: priority = THREAD_PRIORITY_IDLE; break;
   case tpLowest: priority = THREAD_PRIORITY_LOWEST; break;
   case tpLower: priority = THREAD_PRIORITY_BELOW_NORMAL; break;
   case tpNormal: priority = THREAD_PRIORITY_NORMAL; break;
   case tpHigher: priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
   case tpHighest: priority = THREAD_PRIORITY_HIGHEST; break;
   case tpTimeCritical: priority = THREAD_PRIORITY_TIME_CRITICAL; break;
   default: return ELOG(charES2_RANGO, ES2_RANGO); break;
   }

   if (SetThreadPriority(thread_handle_, priority) == 0) 
      return ELOG(charES2_ThreadResume, ES2_ThreadResume);

   if ((dword_value = ResumeThread(thread_handle_)) == (DWORD)-1)
      return ELOG(charES2_ThreadResume, ES2_ThreadResume);
   
   return ES2_NONE;
}
#endif