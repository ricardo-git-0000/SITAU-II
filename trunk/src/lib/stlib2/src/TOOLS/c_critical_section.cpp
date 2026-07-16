// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"
#include "c_log_file.h"
#include "common_error_code.h"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

vector<int> gb_non_blocking_thread;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ResetGlobalVariables_c_critical_section (void)
{
   if (gb_non_blocking_thread.size() > 0) ELOG(charECOM_DataInconsistency, ECOM_DataInconsistency);
   gb_non_blocking_thread.clear();

}


// _____________________________________________________________________________
// ------ CCriticalSection class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CCriticalSection::CCriticalSection(AnsiString name)
{
   this->timeout_value_ = 500;
   this->timeout_unit_ = MILISECONDS;
   this->id_thread_acquired_ = -1;
   this->id_thread_released_ = -1;
   this->ctrl_id_thread_acquired_ = -1;
   this->ctrl_id_thread_released_ = -1;
#if defined(_MSC_VER)
   InitializeCriticalSection(&this->plock_);
   InitializeCriticalSection(&this->plock_control_);
   //this->plock_ = new CRITICAL_SECTION();
   //this->plock_control_ = new CRITICAL_SECTION();
#else
   this->plock_ = new TCriticalSection();
   this->plock_control_ = new TCriticalSection();
#endif
   this->timer_ = new CTimeout(timeout_value_, timeout_unit_);
   this->lock_count_ = 0;
   this->lock_try_ = 0;
   this->ctrl_lock_count_ = 0;
   this->ctrl_lock_try_ = 0;
   this->lock_type_ = CS_NONE;
   this->name_ = name;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CCriticalSection::~CCriticalSection()
{
   ControlAcquire();
#if defined(_MSC_VER)
   DeleteCriticalSection(&this->plock_);
   //this->plock_ = NULL;
   DeleteCriticalSection(&this->plock_control_);
   //this->plock_control_ = NULL;
#else
   if (this->plock_ != NULL) delete this->plock_;
   this->plock_ = NULL;
   if (this->plock_control_ != NULL) delete this->plock_control_;
   this->plock_control_ = NULL;
#endif
   if (this->timer_ != NULL) delete this->timer_;
   this->timer_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
BOOL CCriticalSection::ControlAcquire(void)
{
BOOL result = false;
int id_actual_thread = GetCurrentThreadId();

   return true;

#if defined(_MSC_VER)

//if (this->plock_control_ != NULL)
//{
   if (id_actual_thread == ctrl_id_thread_acquired_)
      return true;

   result = false;
   this->ctrl_lock_try_ = 0;
   if (this->timer_ != NULL)
   {
      this->timer_->Start();
      while (!result && !this->timer_->Timeout())
      {
         result = TryEnterCriticalSection(&this->plock_control_);
         this->ctrl_lock_try_++;
      }
   }
   else result = TryEnterCriticalSection(&this->plock_control_);

   if (result) // Acquire sucessfully
   {
      this->ctrl_lock_count_++;
      if (this->ctrl_lock_count_ > 1)
      {
         char str[255];
         sprintf(str,"(Control Acquired: %s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Count: %d, Try: %d",
            name_.c_str(),
            id_actual_thread,
            ctrl_id_thread_acquired_,
            ctrl_id_thread_released_,
            ctrl_lock_count_,
            ctrl_lock_try_);
         ELOG(str, ECOM_RecursiveRelease);
      }
      this->ctrl_id_thread_acquired_ = id_actual_thread;
   }
   else // Acquire fail
   {
      char str[255];
      sprintf(str,"(Control Acquired: %s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Count: %d, Try: %d",
         name_.c_str(),
         id_actual_thread,
         ctrl_id_thread_acquired_,
         ctrl_id_thread_released_,
         ctrl_lock_count_,
         ctrl_lock_try_);
      ELOG(str, ECOM_RecursiveRelease);
   }
//}
   return result;

#else

try
{
   if (this->plock_control_ != NULL)
   {
      if (id_actual_thread == ctrl_id_thread_acquired_)
         return true;

      this->timer_->Start();
      result = false;
      this->ctrl_lock_try_ = 0;
      while (!result && !this->timer_->Timeout())
      {
         result = this->plock_control_->TryEnter();
         this->ctrl_lock_try_++;
      }
      if (result) // Acquire sucessfully
      {
         this->ctrl_lock_count_++;
         if (this->ctrl_lock_count_ > 1)
         {
            char str[255];
            sprintf(str,"(Control Acquired: %s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Count: %d, Try: %d",
               name_.c_str(),
               id_actual_thread,
               ctrl_id_thread_acquired_,
               ctrl_id_thread_released_,
               ctrl_lock_count_,
               ctrl_lock_try_);
            ELOG(str, ECOM_RecursiveRelease);
         }
         this->ctrl_id_thread_acquired_ = id_actual_thread;
      }
      else // Acquire fail
      {
         char str[255];
         sprintf(str,"(Control Acquired: %s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Count: %d, Try: %d",
            name_.c_str(),
            id_actual_thread,
            ctrl_id_thread_acquired_,
            ctrl_id_thread_released_,
            ctrl_lock_count_,
            ctrl_lock_try_);
         ELOG(str, ECOM_RecursiveRelease);
      }
   }
}
catch (Exception &E)
{
  EELOG(&E);
  throw Exception(E);
}
catch (exception &e)
{
  EeLOG(&e);
  throw Exception(e.what());
}
catch (...)
{
  NEELOG();
  throw;
}
   return result;
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CCriticalSection::ControlRelease(void)
{
int id_actual_thread = GetCurrentThreadId();

   return;

   if (id_actual_thread == 0)
      {ELOG(charECOM_IdThreadNULL, ECOM_IdThreadNULL);}

#if defined(_MSC_VER)
   if (id_actual_thread == this->ctrl_id_thread_acquired_)
   {
      this->ctrl_lock_count_--;
      this->ctrl_id_thread_acquired_ = 0;
      this->ctrl_id_thread_released_ = id_actual_thread;                        
      LeaveCriticalSection(&this->plock_control_);
   }
#else
   if (this->plock_control_ != NULL && id_actual_thread == this->ctrl_id_thread_acquired_)
   {
      this->ctrl_lock_count_--;
      this->ctrl_id_thread_acquired_ = 0;
      this->ctrl_id_thread_released_ = id_actual_thread;
      this->plock_control_->Release();
   }
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CCriticalSection::Acquire(void)
{
int id_actual_thread = GetCurrentThreadId();

   if (id_actual_thread == id_thread_acquired_) return;
#if defined(_MSC_VER)
   EnterCriticalSection(&this->plock_);
#else
   this->plock_->Acquire();
#endif
   this->lock_count_++;
   if (this->lock_count_ > 1)
   {
      char str[255];
      sprintf(str,"(%s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Type: %d, Count: %d, Try: %d",
         name_.c_str(),
         id_actual_thread,
         id_thread_acquired_,
         id_thread_released_,
         lock_type_,
         lock_count_,
         lock_try_);
      ELOG(str, ECOM_RecursiveRelease);
   }
   this->id_thread_acquired_ = id_actual_thread;
   this->lock_type_ = CS_Blocking;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
BOOL CCriticalSection::Acquire(int type, bool safe_access, bool *non_blocking)
{
BOOL result = false;
int i, id_actual_thread = GetCurrentThreadId();
bool non_blocking_thread = (type == CS_NonBlocking || NonBlockingThread(id_actual_thread));

   if (non_blocking != NULL) *non_blocking = non_blocking_thread;
   if (id_actual_thread == 0)
   {
      ELOG(charECOM_IdThreadNULL, ECOM_IdThreadNULL);
      return false;
   }

   if (id_actual_thread == id_thread_acquired_)
      return true;

   if (safe_access || this->lock_type_ == CS_WR)
   {
      if (!ControlAcquire()) return false;
      if (non_blocking_thread)
      {
         result = TryEnterCriticalSection(&this->plock_);
         if (result) // Acquire sucessfully
         {
            this->lock_count_++;
            if (this->lock_count_ > 1)
            {
               char str[255];
               sprintf(str,"(%s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Type: %d, Count: %d, Try: %d",
                  name_.c_str(),
                  id_actual_thread,
                  id_thread_acquired_,
                  id_thread_released_,
                  lock_type_,
                  lock_count_,
                  lock_try_);
               ELOG(str, ECOM_RecursiveRelease);
            }
            this->id_thread_acquired_ = id_actual_thread;
            this->lock_type_ = type;
         }
      }
      else
      {
         result = false;
         if (type == CS_TRY)
         {
            for (i=0; !result && i<CS_N_TRY; i++)
            {
               this->timer_->Start();
               this->lock_try_ = 0;
               while (!result && !this->timer_->Timeout())
               {
                  result = TryEnterCriticalSection(&this->plock_);
                  this->lock_try_++;
               }
               if (!result)
               {
                  char str[255];
                  sprintf(str,"(%s) -> WARNNING TRY (%d) - Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Type: %d, Count: %d, Try: %d",
                     name_.c_str(),
                     i,
                     id_actual_thread,
                     id_thread_acquired_,
                     id_thread_released_,
                     lock_type_,
                     lock_count_,
                     lock_try_);
                  ELOG(str, ECOM_RecursiveRelease);
               }
            }
         }
         else
         {
            this->timer_->Start();
            this->lock_try_ = 0;
            while (!result && !this->timer_->Timeout())
            {
               result = TryEnterCriticalSection(&this->plock_);
               this->lock_try_++;
            }
         }
         if (result) // Acquire sucessfully
         {
            this->lock_count_++;
            if (this->lock_count_ > 1)
            {
               char str[255];
               sprintf(str,"(%s) -> Actual Thread: %d, Acquired Thread: %d, Release Thread: %d, Type: %d, Count: %d, Try: %d",
                  name_.c_str(),
                  id_actual_thread,
                  id_thread_acquired_,
                  id_thread_released_,
                  lock_type_,
                  lock_count_,
                  lock_try_);
               ELOG(str, ECOM_RecursiveRelease);
            }
            this->id_thread_acquired_ = id_actual_thread;
            this->lock_type_ = type;
         }
         else // Acquire fail
         {
            {
               char str[255];
               sprintf(str,"(%s) -> Actual Thread: %d, Acquired Thread: %d, Released Thread: %d, Type: %d, Count: %d, Try: %d",
                  name_.c_str(),
                  id_actual_thread,
                  id_thread_acquired_,
                  id_thread_released_,
                  lock_type_,
                  lock_count_,
                  lock_try_);
               ELOG(str, ECOM_RecursiveRelease);
            }
         }
      }
      ControlRelease();
   }
   else result = true;
   return result;

}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CCriticalSection::Release(void)
{
int id_actual_thread = GetCurrentThreadId();

   if (id_actual_thread == 0)
      {ELOG(charECOM_IdThreadNULL, ECOM_IdThreadNULL);}

#if defined(_MSC_VER)
   if (id_actual_thread == this->id_thread_acquired_)
#else
   if (this->plock_ != NULL && id_actual_thread == this->id_thread_acquired_)
#endif
   {
      {
         this->lock_count_--;
         this->id_thread_acquired_ = 0;
         this->id_thread_released_ = id_actual_thread;
         this->lock_type_ = CS_NONE;
      }
#if defined(_MSC_VER)
      LeaveCriticalSection(&this->plock_);
#else
      this->plock_->Release();
#endif
   }
}




// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void AddNonBlockingThread(int id_thread)
{
   gb_non_blocking_thread.push_back(id_thread);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ResetNonBlockingThread(void)
{
   gb_non_blocking_thread.clear();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void DeleteNonBlockingThread(int id_thread)
{
int i;
bool end_flag = false;

   for (i=0; !end_flag && i<(int)gb_non_blocking_thread.size(); i++)
   {
      if (id_thread == gb_non_blocking_thread[i])
      {
         //gb_non_blocking_thread.erase(&gb_non_blocking_thread[i]);
         gb_non_blocking_thread.erase(gb_non_blocking_thread.begin() + i);
         end_flag = true;
      }
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool NonBlockingThread(int id_thread)
{
int i;

   for (i=0; i<(int)gb_non_blocking_thread.size(); i++)
   {
      if (id_thread == gb_non_blocking_thread[i])
         return true;
   }
   return false;
}

