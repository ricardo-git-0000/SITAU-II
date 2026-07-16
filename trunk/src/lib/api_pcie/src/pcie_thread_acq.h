#ifndef pcie_thread_acqH
#define pcie_thread_acqH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#define NOMINMAX
#include <windows.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "c_pcie_buffer.h"

// _____________________________________________________________________________
// ------ C O N S T A N T S
// =============================================================================

#define FILE_LOG_THR_ACQ		"LOG_THR_ACQ.TXT"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum ThrAcq_Status
{
	ST_ThreadAcq_Stopped             = 0,
	ST_ThreadAcq_Starting            = 1,
	ST_ThreadAcq_StartingBuffer      = 2,
	ST_ThreadAcq_Running             = 3,
	ST_ThreadAcq_Waiting             = 4,
	ST_ThreadAcq_Acquiring           = 5,
	ST_ThreadAcq_Stopping            = 6,
   ST_ThreadAcq_StoppingThreadScan  = 7,
} ThrAcq_Status;

// _____________________________________________________________________________
// ------ TThread_Acq class
// _____________________________________________________________________________
// =============================================================================

class TThread_Acq
{
private:

	unsigned long long n_images_read_;
	bool sitau_drv_paused_;
	bool sitau_user_paused_;
	unsigned long time_sleep_;
	bool locked_;
	bool end_;
	ThrAcq_Status status_;
	int error_;
	vector<short int *> focal_laws_pointers_;
	bool sorted_;
	unsigned long n_blocks_;
	unsigned long n_images_blocks_;
  unsigned long idx_image_timestamped_;
  unsigned long time_stamp_;
  bool time_stamped_;
  float prf_time_;

protected:
    
public:

   HANDLE thread_handle_;
   uint32 thread_id_;
   bool Terminated;
   TThreadPriority Priority;
   unsigned long no_hw_image_index_;

   TThread_Acq (bool CreateSuspended, unsigned long n_blocks, unsigned long n_images_blocks, unsigned long time_sleep, int sorted);
   ~TThread_Acq(void);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(void) {WaitForSingleObject(thread_handle_, time_sleep_);}
   int Resume(void);
   int Acquire(void);
   bool End(void) {return end_;}
   ThrAcq_Status GetStatus(void) {return status_;}
   void Locked(bool data) {locked_ = data;}
   bool Locked(void) {return locked_;}
   int GetError(void) {return error_;};
   void SetSorted(bool data) { sorted_ = data; }
   bool GetSorted(void) { return sorted_; }
   //int LastImageRead(void) { return last_image_read_; }
};

int ThrAcq_Create(int priority, unsigned long time_sleep = 0, int sorted = 0);
int ThrAcq_Delete(void);
bool ThrAcq_IsRunning(int *status=nullptr);
bool ThrAcq_IsStopped(void);
int ThrAcq_Error(void);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_Acq *gb_process_acq;

#endif
