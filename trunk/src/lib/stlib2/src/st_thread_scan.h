//---------------------------------------------------------------------------

#ifndef st_thread_scanH
#define st_thread_scanH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif
//#include <winsock2.h>   //prueba

#include <windows.h>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#if defined(_MSC_VER)
   #include "thread_vsapi.h"
#endif

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

#if defined(_MSC_VER)
class TThread_AcqScan
#else
class TThread_AcqScan : public TThread
#endif
{
private:
   int n_encoder_;
   int n_vch_;
   bool locked_;
   bool end_;
   TThread_AcqScan_Status status_;
   int error_;
   unsigned long  n_trg_lines_;
   unsigned long mc_index_trigger_, mc_index_auxiliar_, mc_acq_counter_trigger_;
   unsigned long  mc_n_aux_lines_, mc_lines_to_end_line_;
   unsigned long pa_index_trigger_, pa_index_auxiliar_, pa_acq_counter_trigger_;
   unsigned long  pa_n_aux_lines_, pa_lines_to_end_line_;
   unsigned long tfm_index_trigger_, tfm_index_auxiliar_, tfm_acq_counter_trigger_;
   unsigned long  tfm_n_aux_lines_, tfm_lines_to_end_line_;
   unsigned long mc_index_focal_law_, pa_index_focal_law_, tfm_index_focal_law_;
   unsigned long trigger_focal_laws_;
   long index_auxiliar_saved_;
   unsigned short acq_free_memory_;
   int mc_id_vch_;
   int mc_id_fl_;
   int pa_id_vch_;
   int pa_id_fl_;
   //bool mc_end_acquisition_;
   //bool pa_end_acquisition_;
   //bool tfm_end_acquisition_;

   void Stop(void);

protected:

#if !defined(_MSC_VER)
   void __fastcall Execute();
#endif

public:
   HANDLE thread_handle_;
   DWORD thread_id_;
   bool Terminated;
   DWORD wait_timeout_;
   TThreadPriority Priority;

   TThread_AcqScan (bool CreateSuspended);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(void) {WaitForSingleObject(thread_handle_, wait_timeout_);}
   int Resume(void);
   int MC_GetData(void);
   int PA_GetData(void);
   int TFM_GetData(void);
   int GetError(void) {return error_;}
   bool End(void) {return end_;}
   unsigned short GetAcqFreeMemory(void) {return acq_free_memory_;}
   TThread_AcqScan_Status GetStatus(void) {return status_;}
   void Locked(bool data) {locked_ = data;}
   bool Locked(void) {return locked_;}
   bool EndAcquisition(void);
   //unsigned long CurrentIndexTrigger(void) { return pa_index_trigger_ < mc_index_trigger_ ? pa_index_trigger_ : mc_index_trigger_; }
   unsigned long MC_CurrentIndexFocalLaw(void) { return mc_index_focal_law_; }   
   unsigned long PA_CurrentIndexFocalLaw(void) { return pa_index_focal_law_; }
   unsigned long TFM_CurrentIndexFocalLaw(void) { return tfm_index_focal_law_; }
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_AcqScan *gb_process_acq_scan;

#endif
