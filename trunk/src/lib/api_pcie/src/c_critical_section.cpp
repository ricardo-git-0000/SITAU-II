// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"
#include "log.h"
#include "pcie_error_code.h"

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

// _____________________________________________________________________________
// ------ CCriticalSection class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CCriticalSection::CCriticalSection()
{
   this->timeout_value_ = 500;
   this->id_thread_acquired_ = -1;
   this->id_thread_released_ = -1;
   InitializeCriticalSection(&this->plock_);
   this->timer_ = new C_Timeout(timeout_value_);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CCriticalSection::~CCriticalSection()
{
   DeleteCriticalSection(&this->plock_);
   if (this->timer_ != NULL) delete this->timer_;
   this->timer_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
BOOL CCriticalSection::Acquire()
{
BOOL result = false;
int id_actual_thread = GetCurrentThreadId();

	if (id_actual_thread == 0)
	{
		ELOG(charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
		return false;
	}

	if (id_actual_thread == id_thread_acquired_) return true;
	result = false;
	this->timer_->Start();
	while (!result && !this->timer_->Timeout())
	{
		result = TryEnterCriticalSection(&this->plock_);
	}
		
	if (result) // Acquire sucessfully
	{
		this->id_thread_acquired_ = id_actual_thread;
	}
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CCriticalSection::Release(void)
{
int id_actual_thread = GetCurrentThreadId();

	if (id_actual_thread == 0) {ELOG(charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);}
	if (id_actual_thread == this->id_thread_acquired_)
	{
        this->id_thread_acquired_ = 0;
        this->id_thread_released_ = id_actual_thread;
		LeaveCriticalSection(&this->plock_);
	}
}


