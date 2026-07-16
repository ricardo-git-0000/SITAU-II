#ifndef c_critical_sectionH
#define c_critical_sectionH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "c_timeout.h"

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

	CRITICAL_SECTION plock_;
	DWORD id_thread_acquired_;
	DWORD id_thread_released_;
	int timeout_value_;
	C_Timeout *timer_;

public:
	CCriticalSection();
	~CCriticalSection();
	BOOL Acquire(void);
	void Release(void);
};

#endif
