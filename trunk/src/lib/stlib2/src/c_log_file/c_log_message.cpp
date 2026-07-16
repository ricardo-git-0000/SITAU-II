// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <cstdarg>
#include <iostream>


//#include <stdlib.h>
//#include <dos.h>
//#include <time.h>
//#include <io.h>
//#define NOMINMAX
#include <windows.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_log_file.h"
#include "c_log_message.h"
#include "st_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CLogMessage* gb_msg = NULL;
extern char gb_msg_progress_open_sys[255];

// _____________________________________________________________________________
// ------ CLogMessage class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CLogMessage::CLogMessage(int n_max_message)
{
    n_max_message_ = n_max_message;
    messages_.resize(n_max_message_);
    rd_ = wr_ = 0;
    delay_ = 0;
    delete_signal_ = false;
    plock_ = new CCriticalSection("CLogMessage");
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CLogMessage::~CLogMessage()
{
    if (plock_ != NULL && !plock_->Acquire(CS_WR))
    {
        ELOG(charES2_Acquire, ES2_Acquire);
    }
    messages_.clear();
    if (plock_ != NULL) plock_->Release();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogMessage::Write(char* msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);
    Write(msg, argptr);
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogMessage::Write(char* msg, va_list argptr)
{
    TLogMessage new_msg;
    time_t t;

    if (msg != nullptr)
    {
        time(&t);
        strftime(date_, sizeof(date_), "%d/%m/%Y %H:%M:%S", localtime(&t));
        _vsnprintf_s(msg_, sizeof(msg_), _TRUNCATE, msg, argptr);
        _snprintf_s(str_, sizeof(str_), _TRUNCATE, "[%s] %s", date_, msg_);
        last_message_ = msg_;
        new_msg.data = str_;
        new_msg.update = true;
        if (plock_ != NULL && !plock_->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return;
        }
        if (messages_[wr_].update)
            ::_InterlockedExchange((volatile unsigned __int64*)&rd_, (unsigned __int64)wr_);
        messages_[wr_] = new_msg;
        if (wr_ >= (unsigned __int64)messages_.size() - 1) ::_InterlockedExchange((volatile unsigned __int64*)&wr_, (unsigned __int64)0);
        else ::_InterlockedExchange((volatile unsigned __int64*)&wr_, (unsigned __int64)(wr_ + 1));
        if (plock_ != NULL) plock_->Release();
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CLogMessage::Read(char *data)
{
    int result = 0;

    if (messages_[rd_].update)
    {
        messages_[rd_].update = false;
        strcpy(data, messages_[rd_].data.c_str());
        if (plock_ != NULL && !plock_->Acquire(CS_WR)) return ELOG(charES2_Acquire, ES2_Acquire);
        if (rd_ >= (unsigned __int64)messages_.size() - 1) ::_InterlockedExchange((volatile unsigned __int64*)&rd_, (unsigned __int64)0);
        else ::_InterlockedExchange((volatile unsigned __int64*)&rd_, (unsigned __int64)(rd_ + 1));
        if (plock_ != NULL) plock_->Release();
        result = 1;
    }
    return result;
}





// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Message(char* msg, ...)
{
    va_list argptr;
    va_list argptr_copy;

    va_start(argptr, msg);
    va_copy(argptr_copy, argptr);
    _vsnprintf_s(gb_msg_progress_open_sys, 255, _TRUNCATE, msg, argptr);
    if (gb_msg != NULL)
    {
        gb_msg->Write(msg, argptr_copy);
    }
    va_end(argptr_copy);
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_StartDelay(int data)
{
    if (gb_msg != NULL) gb_msg->StartDelay(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_StopDelay(void)
{
    if (gb_msg != NULL) gb_msg->StopDelay();
}
