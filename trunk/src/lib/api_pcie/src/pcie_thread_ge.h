#ifndef pcie_thread_geH
#define pcie_thread_geH

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
#include "fuse.h"

// _____________________________________________________________________________
// ------ C O N S T A N T S
// =============================================================================

#define TIMEOUT_DMA_EVENTS_MS	5000
#define TIMEOUT_OLINK_EVENTS_MS	5000

#define GE_SOURCE_ACQ_DDR			0
#define GE_SOURCE_ACQ_BUFFER_DRV	1
#define GE_SOURCE_ACQ_BUFFER_USER	2

#define THRESHOLD_DMA_HIGH	200
#define THRESHOLD_DMA_LOW	100

#define FILE_LOG_THR_GE		"LOG_THR_GE.TXT"
#define FILE_LOG_THR_OLINK 	"LOG_THR_OLINK.TXT"
#define FILE_LOG_THR_DMA	"LOG_THR_DMA.TXT"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum ThrGE_Status
{
	ThrGE_Starting			= 0,
	ThrGE_Waiting           = 1,
	ThrGE_SITAU_Continue	= 2,
	ThrGE_SITAU_Pause		= 3,
	ThrGE_Stop				= 4,
	ThrGE_Stopping          = 5,
	ThrGE_Stopped			= 6,
  ThrGE_Terminate = 7,
} ThrGE_Status;

// _____________________________________________________________________________
// ------ TThread_GE class
// _____________________________________________________________________________
// =============================================================================

class TThread_GE
{
private:

	unsigned __int64 sitau_paused_ddr_;
	unsigned __int64 sitau_paused_buffer_drv_;
	unsigned __int64 sitau_paused_buffer_user_;
	unsigned __int64 request_continue_;
	unsigned __int64 request_pause_;

	ThrGE_Status status_;
	int error_;
	fuse *drv_handle_ = nullptr;
	HANDLE olink_event_hThread;
	HANDLE dma_event_hThread;
	HANDLE wake_up_;
	HANDLE olink_abort_event_;
  HANDLE dma_abort_event_;
  timeout_t m_max_timeout_;
	timeout_t timeout_olink_;
	timeout_t timeout_fuse_dma_;
	fuse_event_t HW_EVENT_ALIGNED m_olink_event_;
	fuse_event_t HW_EVENT_ALIGNED m_dma_event_;
	
	DWORD olink_event_thread(void);
	DWORD dma_event_thread(void);

protected:
    
public:

   HANDLE thread_handle_;
   uint32 thread_ge_id_;
   uint32 thread_olink_id_;
   uint32 thread_fuse_dma_id_;
   bool Terminated;
   TThreadPriority Priority;

   TThread_GE (fuse *drv_handle, bool thread_dma_enabled, bool thread_olink_enabled);
   ~TThread_GE(void);
   void ThreadFunction(void);
   void Terminate(void) {Terminated = true;}
   void WaitFor(DWORD timout_ms) {WaitForSingleObject(thread_handle_, timout_ms);}
   int Continue(int source);
   int Pause(int source);
   int Resume(void);
   int Stop(void);
   bool Running(void) {return status_ != ThrGE_Stopped;}
   ThrGE_Status GetStatus(void) {return status_;}
   int GetError(void) {return error_;};
};

int ThrGE_Create(fuse *drv_handle, int priority, bool thread_dma_enabled, bool thread_olink_enabled);
int ThrGE_Delete(void);
bool ThrGE_IsRunning(void);
bool ThrGE_IsStopped(void);
int ThrGE_Error(void);
int ThrGE_Continue(int source);
int ThrGE_Pause(int source);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern TThread_GE *gb_process_ge;

#endif
