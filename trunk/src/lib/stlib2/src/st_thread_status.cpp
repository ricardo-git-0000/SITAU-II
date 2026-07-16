// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

//#include "tad.h"
#include "st_thread_status.h"
#include "shared_mem.h"
#include "protocol_uci.h"
#include "st_error_code.h"
#include "ctimer.h"
#include "stlib2.h"
#include "c_log_file.h"
//#include "stlib_dev.h"
//#include "st_reg.h"
//#include "c_log_file.h"
//#include "c_buffer_mc_scan_hw_ut.h"
//#include "c_buffer_pa_scan_hw_ut.h"
//#include "c_buffer_hw.h"
//#include "c_virtual_channel.h"
//#include "winsock_api.h"
//#include "api_tcp.h"
//#include "thread_vsapi.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TThread_Status *gb_process_status = NULL;

// _____________________________________________________________________________
// ------ Visual Studio Inferface - TThread_Status class Methods
// _____________________________________________________________________________
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
DWORD WINAPI TThread_Status_Execute(LPVOID lpParam)
{
DWORD result = 0;

   if (gb_process_status != NULL)
   {
      gb_process_status->ThreadFunction();
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
HANDLE TThread_Status_CreateThread (bool CreateSuspended, LPDWORD thread_id)
{
HANDLE result = NULL;
DWORD dwCreationFlags = 0;

   //if (gb_process_status == NULL) return ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   if (CreateSuspended) dwCreationFlags = CREATE_SUSPENDED;
   if ((result = CreateThread (   
            NULL,                      // default security attributes
            0,                         // use default stack size
            TThread_Status_Execute,   // thread function name
            NULL,                      // argument to thread function
            dwCreationFlags,           // use default creation flags
            thread_id)) == NULL)     // returns the thread identifier
   {
      ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
   return result;
}

// _____________________________________________________________________________
// ------ TThread_Status class Methods
// _____________________________________________________________________________
// =============================================================================

//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
TThread_Status::TThread_Status (bool CreateSuspended, unsigned long period_ms)
{
   end_ = false;
   period_ms_ = period_ms < 0 ? 0 : period_ms;
   locked_ = false;
   this->locked_ok_ = true;
   //status_ = ST_ThreadAcq_Stopped;
   this->Terminated = false;
   this->error_ = ES2_NONE;
   this->Priority = tpNormal;
   this->thread_handle_ = TThread_Status_CreateThread (CreateSuspended, &this->thread_id_);
   if (thread_handle_ == NULL)  // returns the thread identifier
   {
      ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
void TThread_Status::ThreadFunction(void)
{
char str[255];
DWORD thread_id = GetCurrentThreadId();
u8 LVDS_deskew_ok = 0xFF, AFES_align_ok = 0xFF, qsfp_channel_rdy_dn = 0xFF, qsfp_channel_rdy_up = 0xFF;
u8 channel_dn_not_ready = 0xFF, channel_dn_hard_errors = 0xFF, channel_dn_soft_errors = 0xFF;
u8 channel_up_not_ready = 0xFF, channel_up_hard_errors = 0xFF, channel_up_soft_errors = 0xFF;

    if (LOG_IDThread(LOG_STATUS_FILE_NAME, thread_id) < 0)
    {
        ST_InfoLibraryVersion(str);
        LOG_STATUS = LOG_Create(LOG_STATUS_FILE_NAME, LOG_STATUS_NAME, str);
        LOG_IDThread(LOG_STATUS_FILE_NAME, thread_id);
    }
    LOG_Info("%s: Created (Id. %d) - TimeOut: %d ms", LOG_STATUS_NAME, thread_id, period_ms_);

    end_ = false;
    while (!Terminated && error_ >= 0 && ! end_)
    {
        if (!locked_)
        {
            if (locked_ok_) locked_ok_ = false;
            if (TCPSend_MSG_SHARED_MEM())
            {
                if (AFES_align_ok != gb_shared.AFES_align_ok && gb_shared.AFES_align_ok == 0)
                    LOG_Info("Error AFES Align");
                if (LVDS_deskew_ok != gb_shared.LVDS_deskew_ok && gb_shared.LVDS_deskew_ok == 0)
                    LOG_Info("Error LVDS Deskew");
                if (qsfp_channel_rdy_dn != gb_shared.qsfp_channel_rdy_dn && gb_shared.qsfp_channel_rdy_dn == 0)
                    LOG_Info("Error QSFP Channel Down");
                if (channel_dn_not_ready != gb_shared.qsfp_link_errors_dn.BITS.channel_not_ready && gb_shared.qsfp_link_errors_dn.BITS.channel_not_ready == 1)
                    LOG_Info("Error QSFP Channel Down: Not Ready");
                if (channel_dn_hard_errors != gb_shared.qsfp_link_errors_dn.BITS.hard_errors && gb_shared.qsfp_link_errors_dn.BITS.hard_errors > 0)
                    LOG_Info("Error QSFP Channel Down: Hard Errors (%d)", gb_shared.qsfp_link_errors_dn.BITS.hard_errors);
                if (channel_dn_soft_errors != gb_shared.qsfp_link_errors_dn.BITS.soft_errors && gb_shared.qsfp_link_errors_dn.BITS.soft_errors > 0)
                    LOG_Info("Error QSFP Channel Down: Soft Errors (%d)", gb_shared.qsfp_link_errors_dn.BITS.soft_errors);
                if (qsfp_channel_rdy_up != gb_shared.qsfp_channel_rdy_up && gb_shared.qsfp_channel_rdy_up == 0)
                    LOG_Info("Error QSFP Channel Up");
                if (channel_up_not_ready != gb_shared.qsfp_link_errors_up.BITS.channel_not_ready && gb_shared.qsfp_link_errors_up.BITS.channel_not_ready == 1)
                    LOG_Info("Error QSFP Channel Up: Not Ready");
                if (channel_up_hard_errors != gb_shared.qsfp_link_errors_up.BITS.hard_errors && gb_shared.qsfp_link_errors_up.BITS.hard_errors > 0)
                    LOG_Info("Error QSFP Channel Up: Hard Errors (%d)", gb_shared.qsfp_link_errors_up.BITS.hard_errors);
                if (channel_up_soft_errors != gb_shared.qsfp_link_errors_up.BITS.soft_errors && gb_shared.qsfp_link_errors_up.BITS.soft_errors > 0)
                    LOG_Info("Error QSFP Channel Up: Soft Errors (%d)", gb_shared.qsfp_link_errors_up.BITS.soft_errors);
                AFES_align_ok = gb_shared.AFES_align_ok;
                LVDS_deskew_ok = gb_shared.LVDS_deskew_ok;
                qsfp_channel_rdy_dn = gb_shared.qsfp_channel_rdy_dn;
                channel_dn_not_ready = gb_shared.qsfp_link_errors_dn.BITS.channel_not_ready;
                channel_dn_hard_errors = gb_shared.qsfp_link_errors_dn.BITS.hard_errors;
                channel_dn_soft_errors = gb_shared.qsfp_link_errors_dn.BITS.soft_errors;
                qsfp_channel_rdy_up = gb_shared.qsfp_channel_rdy_up;
                channel_up_not_ready = gb_shared.qsfp_link_errors_up.BITS.channel_not_ready;
                channel_up_hard_errors = gb_shared.qsfp_link_errors_up.BITS.hard_errors;
                channel_up_soft_errors = gb_shared.qsfp_link_errors_up.BITS.soft_errors;
            }
        }
        else if (!locked_ok_) locked_ok_ = true;
        if (period_ms_ > 0) Sleep(period_ms_);
	}
    end_ = true;
    LOG_Info("%s: Deleted (Id. %d) - TimeOut: %d ms", LOG_STATUS_NAME, thread_id, period_ms_);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_Status::Resume(void)
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





// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CreateStatusThread(unsigned long wait_time_ms)
{
    DeleteStatusThread();
    if ((gb_process_status = new TThread_Status(true, wait_time_ms)) == NULL)
    {
        return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
    }
    gb_process_status->Resume();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void DeleteStatusThread(void)
{
    CTimeout* timeout = NULL;
    bool fin;

    if (gb_process_status != NULL)
    {
        gb_process_status->Locked(true);
        gb_process_status->Terminate();
        gb_process_status->WaitFor();
        timeout = new CTimeout(3000, MILISECONDS);
        timeout->Start();

        fin = false;
        while (!fin)
        {
            if (gb_process_status->End()) fin = true;
            else if (timeout->Timeout())
                fin = true;
        }
        delete timeout;
        delete gb_process_status;
        gb_process_status = NULL;
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool IsRunningStatusThread(void)
{
    if (gb_process_status != NULL) return gb_process_status->IsRunning();
    else return false;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool IsLockedStatusThread(void)
{
    if (gb_process_status != NULL) return gb_process_status->Locked();
    else return true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool LockStatusThread(void)
{
    CTimeout *timeout = NULL;
    bool result = true;

    try
    {
        if (gb_process_status != NULL)
        {
            result = gb_process_status->Locked();
            if (!result)
            {
                gb_process_status->Locked(true);
                if ((timeout = new CTimeout(3000, MILISECONDS)) != NULL)
                {
                    timeout->Start();
                    while (!gb_process_status->Locked() && !timeout->Timeout());
                }
                result = gb_process_status->Locked();
            }
        }
    }
    __finally
    {
        if (timeout != NULL) delete timeout;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool UnLockStatusThread(void)
{
    CTimeout* timeout = NULL;
    bool result = true;

    try
    {
        if (gb_process_status != NULL)
        {
            result = !gb_process_status->Locked();
            if (!result)
            {
                gb_process_status->Locked(false);
                if ((timeout = new CTimeout(3000, MILISECONDS)) != NULL)
                {
                    timeout->Start();
                    while (gb_process_status->Locked() && !timeout->Timeout());
                }
                result = !gb_process_status->Locked();
            }
        }
    }
    __finally
    {
        if (timeout != NULL) delete timeout;
    }
    return result;
}
