#ifndef c_critical_sectionH
#define c_critical_sectionH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"
#include "ctimer.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================

#define CS_WR           0
#define CS_RD           1
#define CS_NonBlocking  2
#define CS_NONE         3
#define CS_TRY          4
#define CS_Blocking     5
#define CS_N_TRY        20

// _____________________________________________________________________________
// ------ CCriticalSection class
// _____________________________________________________________________________
// =============================================================================
class CCriticalSection
{
private:
#if defined(_MSC_VER)
   CRITICAL_SECTION plock_control_;
   CRITICAL_SECTION plock_;
#else
   TCriticalSection *plock_control_;
   TCriticalSection *plock_;
#endif
   DWORD id_thread_acquired_;
   DWORD id_thread_released_;
   int lock_type_;
   int lock_count_;
   int timeout_value_;
   int timeout_unit_;
   unsigned long lock_try_;
   CTimeout *timer_;
   AnsiString name_;

   DWORD ctrl_id_thread_acquired_;
   DWORD ctrl_id_thread_released_;
   int ctrl_lock_count_;
   unsigned long ctrl_lock_try_;

   BOOL ControlAcquire(void);
   void ControlRelease(void);

public:
	CCriticalSection(AnsiString id);
	~CCriticalSection();
   void Acquire(void);
   BOOL Acquire(int type, bool safe_access=true, bool *non_blocking=NULL);
   void Release(void);
   void Name(AnsiString data) {name_ = data;};
};

void AddNonBlockingThread(int id_thread);

void ResetNonBlockingThread(void);

void DeleteNonBlockingThread(int id_thread);

bool NonBlockingThread(int id_thread);

void ResetGlobalVariables_c_critical_section (void);

#endif
