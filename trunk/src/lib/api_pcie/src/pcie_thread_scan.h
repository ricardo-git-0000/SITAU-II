//---------------------------------------------------------------------------

#ifndef pcie_thread_scanH
#define pcie_thread_scanH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#define NOMINMAX
#include <windows.h>
#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define Thread_AcqScan_TRG_SW					0
#define Thread_AcqScan_TRG_SCAN_ENCODER	3
#define Thread_AcqScan_TRG_SCAN_SW			4

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum TThread_AcqScan_Status
{
	ST_ThreadAcqScan_Stopped            = 0,
	ST_ThreadAcqScan_Starting           = 1,
	ST_ThreadAcqScan_StartingBuffer     = 2,
	ST_ThreadAcqScan_Running            = 3,
	ST_ThreadAcqScan_Stopping           = 4,
   ST_ThreadAcqScan_StoppingThreadAcq  = 5,
} TThread_AcqScan_Status;

// _____________________________________________________________________________
// ------ TThread_AcqScan class
// _____________________________________________________________________________
// =============================================================================

class TThread_AcqScan
{
private:
   int n_encoder_;
   int n_vch_;
   bool locked_;
   bool end_;
   TThread_AcqScan_Status status_;
   int error_;
   unsigned long fp_index_trigger_, fp_index_auxiliar_, fp_acq_counter_trigger_;
   unsigned long  fp_n_trg_lines_, fp_n_aux_lines_, fp_lines_to_end_line_;
   unsigned long pa_index_trigger_, pa_index_auxiliar_, pa_acq_counter_trigger_;
   unsigned long  pa_n_trg_lines_, pa_n_aux_lines_, pa_lines_to_end_line_;
   long index_auxiliar_saved_;
   unsigned short acq_free_memory_;
   int fp_id_vch_;
   int fp_id_fl_;
   int pa_id_vch_;
   int pa_id_fl_;

protected:

public:
   int focal_law_mode_;
   HANDLE thread_handle_;
   uint32 thread_id_;
   bool Terminated;
   uint32 wait_timeout_;
   TThreadPriority Priority;

   TThread_AcqScan (bool CreateSuspended, int focal_law_mode);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(void) {WaitForSingleObject(thread_handle_, wait_timeout_);}
   int Resume(void);
   int GetData(void);
   int GetError(void) {return error_;}
   bool End(void) {return end_;}
   unsigned short GetAcqFreeMemory(void) {return acq_free_memory_;}
   TThread_AcqScan_Status GetStatus(void) {return status_;}
   void Locked(bool data) {locked_ = data;}
   bool Locked(void) {return locked_;}
   bool FP_EndAcquisition(void);
};

uint32 Execute(void);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_AcqScan *gb_process_acq_scan;

#endif
