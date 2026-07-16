// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "pcie_thread_ge.h"
#include "pcie_error_code.h"
#include "c_timeout.h"
#include "c_image_handle.h"
#include "api_pcie.h"
#include "pcie.h"
#include "stlib2.h"
#include "log.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TThread_GE *gb_process_ge = nullptr;

// _____________________________________________________________________________
// ------ Visual Studio Inferface - TThread_GE class Methods
// _____________________________________________________________________________
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint32 WINAPI TThread_GE_Execute(LPVOID lpParam)
{
uint32 result = 0;

   if (gb_process_ge != nullptr)
   {
      gb_process_ge->ThreadFunction();
   }
   return result;
}

// _____________________________________________________________________________
// ------ TThread_GE class Methods
// _____________________________________________________________________________
// =============================================================================

//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
TThread_GE::TThread_GE (fuse *drv_handle, bool thread_dma_enabled, bool thread_olink_enabled)
{
	int result;
	unsigned long mask;
	char str[255];

	LOG_Open(FILE_LOG_THR_GE, "Thread SITAU GEthernet");
	LOG_Open(FILE_LOG_THR_OLINK, "Thread OLink Event");
	LOG_Open(FILE_LOG_THR_DMA, "Thread FUSE DMA Event");

	sitau_paused_ddr_ = 0;
	sitau_paused_buffer_drv_ = 0;
	sitau_paused_buffer_user_ = 0;
	request_continue_ = 0;
	request_pause_ = 0;
	status_ = ThrGE_Stopped;
	Terminated = false;
	error_ = EPCIE_NONE;
	Priority = tpNormal;
	thread_ge_id_ = 0;
	thread_olink_id_ = 0;
	m_max_timeout_ = 0;
	timeout_olink_ = 0;
	drv_handle_ = drv_handle;
	olink_event_hThread = nullptr;
	dma_event_hThread = nullptr;
	//if (drv_handle_ == nullptr) error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_DRV_NULL, EPCIE_DRV_NULL);

	thread_handle_ = CreateThread(
		nullptr,              // default security attributes
		0,                    // use default stack size
		TThread_GE_Execute,   // thread function name
		nullptr,              // argument to thread function
		CREATE_SUSPENDED,     // use default creation flags
		&thread_ge_id_);      // returns the thread identifier
	if (thread_handle_ == nullptr) error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
	
	wake_up_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	olink_abort_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	dma_abort_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (drv_handle_ != nullptr)
	{
		if (thread_olink_enabled)
		{
			mask = CH_ERR_EVT_INT_MASK;
			if ((result = drv_handle_->enable_event_olink(mask)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			if ((result = drv_handle_->set_event_olink_timeout(TIMEOUT_OLINK_EVENTS_MS)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			if ((result = drv_handle_->get_event_olink_timeout(timeout_olink_)) < 0) { error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL); }
			else m_max_timeout_ = (timeout_olink_ > m_max_timeout_) ? timeout_olink_ : m_max_timeout_;

			olink_event_hThread = ::CreateThread(
				nullptr,                                  // lpThreadAttributes 
				0,                                        // dwStackSize
				[](auto param) {                          // lpStartAddress
				return ((TThread_GE *)param)->olink_event_thread();
				},
				this,                                     // lpParameter 
				0,                                        // dwCreationFlags 
				&thread_olink_id_);                                 // lpThreadId
			if (olink_event_hThread == nullptr) error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
			sprintf(str, "Thread OLink Event Created (Id. %d)", thread_olink_id_);
			LOG_Info(FILE_LOG_THR_GE, str);
		}
		else
		{
			mask = 0;
			if ((result = drv_handle_->enable_event_olink(mask)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			LOG_Info(FILE_LOG_THR_GE, "Thread OLink Event Disabled");
		}

		if (thread_dma_enabled)
		{
			mask = DMA_RX_OVF_INT_MASK | DMA_IMG_ERR_INT_MASK | DMA_DDR_OVF_INT_MASK | DMA_DDR_AFULL_INT_MASK | DMA_DDR_AEMPTY_INT_MASK;
			if ((result = drv_handle_->enable_event_dma(mask)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			if ((result = drv_handle_->set_event_fuse_dma_timeout(TIMEOUT_DMA_EVENTS_MS)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);

			if ((result = drv_handle_->get_event_fuse_dma_timeout(timeout_fuse_dma_)) < 0) { error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL); }
			else m_max_timeout_ = (timeout_fuse_dma_ > m_max_timeout_) ? timeout_fuse_dma_ : m_max_timeout_;

			drv_handle_->dma_hw_set_filling_thrs(THRESHOLD_DMA_LOW, THRESHOLD_DMA_HIGH);
			//drv_handle_->dma_hw_get_filling_thrs(rd_thr_l, rd_thr_h);

			dma_event_hThread = ::CreateThread(
				nullptr,                                  // lpThreadAttributes 
				0,                                        // dwStackSize
				[](auto param) {                          // lpStartAddress
				return ((TThread_GE *)param)->dma_event_thread();
				},
				this,                                     // lpParameter 
				0,                                        // dwCreationFlags 
				&thread_fuse_dma_id_);                                 // lpThreadId
			if (dma_event_hThread == nullptr) error_ = ELOG_THR(FILE_LOG_THR_GE, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
			sprintf(str, "Thread FUSE DMA Event Created (Id. %d)", thread_fuse_dma_id_);
			LOG_Info(FILE_LOG_THR_GE, str);
		}
		else
		{
			mask = 0;
			if ((result = drv_handle_->enable_event_dma(mask)) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			LOG_Info(FILE_LOG_THR_GE, "Thread FUSE DMA Event Disabled");
		}
	}
	sprintf(str, "Thread SITAU GEthernet: Created (Id. %d)", thread_ge_id_);
	LOG_Info(FILE_LOG_THR_GE, str);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
TThread_GE::~TThread_GE(void)
{
	Stop();
	if (wake_up_ != nullptr) CloseHandle(wake_up_);
	LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Deleted");
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
void TThread_GE::ThreadFunction(void)
{
	int result;
	FILE *f=nullptr;
  char str[255];
  unsigned __int64 sitau_paused_ddr;
  unsigned __int64 sitau_paused_buffer_drv;
  unsigned __int64 sitau_paused_buffer_user;
  bool paused = false;

	
	status_ = ThrGE_Starting;
	error_ = 0;

	LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Started");
	try
	{
		sitau_paused_ddr_ = 0;
		sitau_paused_buffer_drv_ = 0;
		sitau_paused_buffer_user_ = 0;
    sitau_paused_ddr = sitau_paused_ddr_;
    sitau_paused_buffer_drv = sitau_paused_buffer_drv_;
    sitau_paused_buffer_user = sitau_paused_buffer_user_;
		while (!Terminated)
		{
			if (request_continue_ == (unsigned __int64)0 && request_pause_ == (unsigned __int64)0)
			{
				status_ = ThrGE_Waiting;
				::WaitForSingleObject(wake_up_, INFINITE);
			}

			if (request_pause_ == (unsigned __int64)1)
			{
				if (!paused && (sitau_paused_ddr_ == 1 || sitau_paused_buffer_drv_ == 1 || sitau_paused_buffer_user_ == 1))
				{
					sprintf(str, "Thread SITAU GEthernet : Pause (pause_ddr(%d) - pause_buffer_drv(%d) - pause_buffer_user(%d))", (int)sitau_paused_ddr_, (int)sitau_paused_buffer_drv_, (int)sitau_paused_buffer_user_);
					LOG_Info(FILE_LOG_THR_GE, str);
          //printf("\nPAUSE");
					if ((result = ST_Pause()) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
					::_InterlockedExchange((volatile unsigned __int64 *)&request_pause_, (unsigned __int64)0);
          paused = true;
				}
			}
			if (request_continue_ == (unsigned __int64)1)
			{
				if (sitau_paused_ddr_ == 0 && sitau_paused_buffer_drv_ == 0 && sitau_paused_buffer_user_ == 0)
				{
					LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Continue");
          //printf("\nCONTINUE");
          if ((result = ST_Continue()) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
					::_InterlockedExchange((volatile unsigned __int64 *)&request_continue_, (unsigned __int64)0);
          paused = false;
        }
				else if (sitau_paused_ddr != sitau_paused_ddr_ || sitau_paused_buffer_drv != sitau_paused_buffer_drv_ || sitau_paused_buffer_user != sitau_paused_buffer_user_)
				{
					sprintf(str, "Thread SITAU GEthernet : No Continue (pause_ddr(%d) - pause_buffer_drv(%d) - pause_buffer_user(%d))", (int)sitau_paused_ddr_, (int)sitau_paused_buffer_drv_, (int)sitau_paused_buffer_user_);
					LOG_Info(FILE_LOG_THR_GE, str);
          sitau_paused_ddr = sitau_paused_ddr_;
          sitau_paused_buffer_drv = sitau_paused_buffer_drv_;
          sitau_paused_buffer_user = sitau_paused_buffer_user_;
				}
			}
			//LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Wake up");

			switch (status_)
			{
			//case ThrGE_SITAU_Continue:
			//	if (sitau_paused_ddr_ == 0 && sitau_paused_buffer_drv_ == 0 && sitau_paused_buffer_user_ == 0)
			//	{
			//		LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Continue");
			//		if ((result = ST_Continue()) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			//		::_InterlockedExchange((volatile unsigned __int64 *)&request_continue_, (unsigned __int64)0);
			//	}
			//	else
			//	{
			//		sprintf(str, "Thread SITAU GEthernet : No Continue (pause_ddr(%d) - pause_buffer_drv(%d) - pause_buffer_user(%d))", sitau_paused_ddr_, sitau_paused_buffer_drv_, sitau_paused_buffer_user_);
			//		LOG_Info(FILE_LOG_THR_GE, str);
			//	}
			//	break;
			//case ThrGE_SITAU_Pause:
			//	if (sitau_paused_ddr_ == 1 || sitau_paused_buffer_drv_ == 1 || sitau_paused_buffer_user_ == 1)
			//	{
			//		sprintf(str, "Thread SITAU GEthernet : Pause (pause_ddr(%d) - pause_buffer_drv(%d) - pause_buffer_user(%d))", sitau_paused_ddr_, sitau_paused_buffer_drv_, sitau_paused_buffer_user_);
			//		LOG_Info(FILE_LOG_THR_GE, str);
			//		if ((result = ST_Pause()) < 0) error_ = RLOG_THR(FILE_LOG_THR_GE, result);
			//		::_InterlockedExchange((volatile unsigned __int64 *)&request_pause_, (unsigned __int64)0);
			//	}
			//	break;
			case ThrGE_Stop:
			//	LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Stopping...");
			//	status_ = ThrGE_Stopping;
			//	break;
			//case ThrGE_Terminate:
				LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: ThrGE_Terminating...");
				Terminated = true;
				break;
			}
			::ResetEvent(wake_up_);
		}
	}
	__finally
	{
    ST_Continue();
		LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: Terminated");
		status_ = ThrGE_Stopped;
	}
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
DWORD TThread_GE::olink_event_thread(void)
{
	fuse_event_t event = 0x00000000;
	FILE *f = nullptr;
	int result;
	char str[255];

	LOG_Info(FILE_LOG_THR_OLINK, "Thread OLink Event: Started");
	while (1) 
	{
		if ((::WaitForSingleObject(olink_abort_event_, 0)) == WAIT_OBJECT_0)
		{
      LOG_Info(FILE_LOG_THR_OLINK, "Thread OLink Event: Aborting...");
      break;
		}
		if (drv_handle_->get_event_olink(event) < 0)
		{
			sprintf(str, "[%s] get_event_olink() Error!!!\n", __FUNCTION__);
			ELOG_THR(FILE_LOG_THR_OLINK, str, EPCIE_USER);
		}
		else 
		{
			if (event != 0x00000000 && !::SetEvent(wake_up_)) return ELOG_THR(FILE_LOG_THR_OLINK, charEPCIE_SET_EVENT, EPCIE_SET_EVENT);
			InterlockedOr((volatile LONG *)&m_olink_event_, event);

			// 0. Channel up falling edge (event interrupt)
			if (event & CH_ERR_EVT_INT_MASK) {
				int err;
				sprintf(str, "[%s] Channel up falling edge!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
				result = drv_handle_->olink_get_channel_err(err);
				sprintf(str, "[%s] Channel err = %d\n", __FUNCTION__, err);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 1. Aurora soft error rising edge (event interrupt)
			if (event & SOFT_ERR_EVT_INT_MASK) {
				sprintf(str, "[%s] Aurora soft error activation!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 2. Aurora hard error rising edge (event interrupt)
			if (event & HARD_ERR_EVT_INT_MASK) {
				sprintf(str, "[%s] Aurora hard error activation!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 3. Not channel up (level interrupt)
			if (event & CHANNEL_DOWN_INT_MASK) {
				sprintf(str, "[%s] Channel down !!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 4. Aurora soft error (level interrupt)
			if (event & SOFT_ERR_INT_MASK) {
				sprintf(str, "[%s] Aurora soft error!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 5. Aurora hard error (level interrupt)
			if (event & HARD_ERR_INT_MASK) {
				sprintf(str, "[%s] Aurora hard error!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}

			// 6. Aurora PLL loss (level interrupt)
			if (event & GT_PLL_LOSS_INT_MASK) {
				sprintf(str, "[%s] Aurora PLL loss!!!\n", __FUNCTION__);
				LOG_Info(FILE_LOG_THR_OLINK, str);
			}
		}
	}
	LOG_Info(FILE_LOG_THR_OLINK, "Thread OLink Event: Terminated");
	return EPCIE_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
DWORD TThread_GE::dma_event_thread(void)
{
fuse_event_t event;
FILE *f = nullptr;
unsigned int n_buffers;
char str[255];
int result;
bool paused = false;

  LOG_Info(FILE_LOG_THR_DMA, "Thread FUSE DMA Event: Started");
	while (1) 
	{
		if (::WaitForSingleObject(dma_abort_event_, 0) == WAIT_OBJECT_0)
		{
      LOG_Info(FILE_LOG_THR_DMA, "Thread FUSE DMA Event: Aborting...");
			break;
		}
		//if (0)
		{
			if (drv_handle_->get_event_dma(event) < 0)
			{
				sprintf(str, "[%s] get_event_dma() Error!!!\n", __FUNCTION__);
				ELOG_THR(FILE_LOG_THR_DMA, str, EPCIE_USER);
			}
			else
			{
				//if (event != 0x00000000 && !::SetEvent(wake_up_)) return ELOG_THR(FILE_LOG_THR_DMA, charEPCIE_SET_EVENT, EPCIE_SET_EVENT);
				//::InterlockedOr((LONG *)&m_dma_event_, (LONG)event);

				// 0. DMA buffer ready
				if (event & DMA_BUF_RDY_INT_MASK) {
					sprintf(str, "[%s] There are available hw images\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 1. New buffer received
				if (event & DMA_NEW_BUF_INT_MASK) {
					sprintf(str, "[%s] New hw_image received\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 2. DMA Rx overflow: the input FIFO has overflowed and there is data loss. 
				if (event & DMA_RX_OVF_INT_MASK) {
					sprintf(str, "[%s] Rx module overflow, unrecoverable data loss!!!\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 3. Image ID counter missmatch.
				if (event & DMA_IMG_ID_ERR_INT_MASK) {
					sprintf(str, "[%s] Image ID counter missmatch\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 4. Image error (wrong format) 
				if (event & DMA_IMG_ERR_INT_MASK) {
					sprintf(str, "[%s] Image error (wrong format), hw image discarded!!!\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 5. DDR FIFO overflow 
				if (event & DMA_DDR_OVF_INT_MASK) {
					sprintf(str, "[%s] FIFO DDR overflow, hw_images loss!!!\n", __FUNCTION__);
					LOG_Info(FILE_LOG_THR_DMA, str);
				}

				// 6. DDR FIFO almost FULL 
				if (event & DMA_DDR_AFULL_INT_MASK) {
          if (!paused && Running())
          {
            if ((result = Pause(GE_SOURCE_ACQ_DDR)) < 0) return RLOG(result);
            paused = true;
          }
          drv_handle_->dma_hw_get_filling_level(n_buffers);
					sprintf(str, "[%s] n_buffers = %3d FIFO DDR is almost FULL, risk of hw_images loss\n", __FUNCTION__, n_buffers);
					LOG_Info(FILE_LOG_THR_DMA, str);
					//if (rmt_cntx) {
					//	ret = generation_set_pause(rmt_cntx, 1);
					//}
				}

				// 7. DDR FIFO almost EMPTY
				if (event & DMA_DDR_AEMPTY_INT_MASK) 
        {
          if (Running())
          {
            if ((result = Continue(GE_SOURCE_ACQ_DDR)) < 0) return RLOG(result);
            paused = false;
          }          
					drv_handle_->dma_hw_get_filling_level(n_buffers);
					sprintf(str, "[%s] n_buffers = %3d FIFO DDR is almost EMPTY, transmission can be restarted\n", __FUNCTION__, n_buffers);
					LOG_Info(FILE_LOG_THR_DMA, str);
					//if (rmt_cntx) {
					//	ret = generation_set_pause(rmt_cntx, 0);
					//}
				}
			}
		}
	}
	LOG_Info(FILE_LOG_THR_DMA, "Thread FUSE DMA Event: Terminated");
	return EPCIE_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int TThread_GE::Pause(int source)
{
  bool change = false;

	switch (source)
	{
	case GE_SOURCE_ACQ_DDR:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Pause' Buffer DDR");
    //if (sitau_paused_ddr_ != (unsigned __int64)1)
		{
		::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_ddr_, (unsigned __int64)1);
		change = true;
		}
		break;
	case GE_SOURCE_ACQ_BUFFER_DRV:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Pause' Buffer DRV");
    //if (sitau_paused_buffer_drv_ != (unsigned __int64)1)
		{
		::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_buffer_drv_, (unsigned __int64)1);
		change = true;
		}
		break;
	case GE_SOURCE_ACQ_BUFFER_USER:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Pause' Buffer User");
    //if (sitau_paused_buffer_user_ != (unsigned __int64)1)
		{
			::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_buffer_user_, (unsigned __int64)1);
			change = true;
		}
		break;
	}
	//if (change && (sitau_paused_ddr_ == 1 || sitau_paused_buffer_drv_ == 1 || sitau_paused_buffer_user_ == 1))
	{
		//::_InterlockedExchange((volatile unsigned __int64 *)&status_, (unsigned __int64)ThrGE_SITAU_Pause);
		::_InterlockedExchange((volatile unsigned __int64 *)&request_pause_, (unsigned __int64)1);
		if (!::SetEvent(wake_up_)) return ELOG_THR(FILE_LOG_THR_GE, charEPCIE_SET_EVENT, EPCIE_SET_EVENT);
	}
	return EPCIE_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int TThread_GE::Continue(int source)
{
  bool change = false;

  switch (source)
	{
	case GE_SOURCE_ACQ_DDR:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Continue' Buffer DDR");
    //if (sitau_paused_ddr_ != (unsigned __int64)0)
    {
      ::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_ddr_, (unsigned __int64)0);
      change = true;
    }
		break;
	case GE_SOURCE_ACQ_BUFFER_DRV:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Continue' Buffer DRV");
    //if (sitau_paused_buffer_drv_ != (unsigned __int64)0)
    {
      ::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_buffer_drv_, (unsigned __int64)0);
      change = true;
    }
		break;
	case GE_SOURCE_ACQ_BUFFER_USER:
    LOG_Info(FILE_LOG_THR_GE, "Thread SITAU GEthernet: 'Continue' Buffer User");
    //if (sitau_paused_buffer_user_ != (unsigned __int64)0)
    {
      ::_InterlockedExchange((volatile unsigned __int64 *)&sitau_paused_buffer_user_, (unsigned __int64)0);
      change = true;
    }
		break;
	}
	//if (change && sitau_paused_ddr_ == 0 && sitau_paused_buffer_drv_ == 0 && sitau_paused_buffer_user_ == 0)
	{
	  ::_InterlockedExchange((volatile unsigned __int64 *)&request_continue_, (unsigned __int64)1);
	  if (!::SetEvent(wake_up_)) return ELOG_THR(FILE_LOG_THR_GE, charEPCIE_SET_EVENT, EPCIE_SET_EVENT);
	}
	return EPCIE_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int TThread_GE::Stop(void)
{
	::_InterlockedExchange((volatile unsigned __int64 *)&status_, (unsigned __int64)ThrGE_Stop);
	if (!::SetEvent(olink_abort_event_)) { ELOG_THR(FILE_LOG_THR_GE, charEPCIE_SET_EVENT, EPCIE_SET_EVENT); }
	if (!::SetEvent(dma_abort_event_)) { ELOG_THR(FILE_LOG_THR_GE, charEPCIE_SET_EVENT, EPCIE_SET_EVENT); }
	if (olink_event_hThread != nullptr) ::WaitForSingleObject(olink_event_hThread, 5000);
	if (dma_event_hThread != nullptr) ::WaitForSingleObject(dma_event_hThread, 5000);
	if (!::SetEvent(wake_up_)) { ELOG_THR(FILE_LOG_THR_GE, charEPCIE_SET_EVENT, EPCIE_SET_EVENT); }
	if (thread_handle_ != nullptr) ::WaitForSingleObject(thread_handle_, 3000);
  return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_GE::Resume(void)
{
uint32 dword_value;
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
   default: return ELOG_THR(FILE_LOG_THR_GE, charEPCIE_PARAMETER, EPCIE_PARAMETER); break;
   }
   if (SetThreadPriority(thread_handle_, priority) == 0) return ELOG_THR(FILE_LOG_THR_GE, charEPCIE_ThreadResume, EPCIE_ThreadResume);
   if ((dword_value = ResumeThread(thread_handle_)) == (uint32)-1) return ELOG_THR(FILE_LOG_THR_GE, charEPCIE_ThreadResume, EPCIE_ThreadResume);
   return EPCIE_NONE;
}




// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrGE_Create(fuse *drv_handle, int priority, bool thread_dma_enabled, bool thread_olink_enabled)
{
	if (gb_process_ge != nullptr) //ThrGE_Delete();
		return 0;
	if ((gb_process_ge = new TThread_GE(drv_handle, thread_dma_enabled, thread_olink_enabled)) == nullptr)
	{
		ThrGE_Delete();
		return ELOG(charEPCIE_PtrAccess_NULL, EPCIE_PtrAccess_NULL);
	}
	switch (priority)
	{
	case 0:
		gb_process_ge->Priority = tpLower; // set the priority lower than normal
		break;
	case 2:
		gb_process_ge->Priority = tpHighest; // set the priority lower than normal
		break;
	case 3:
		gb_process_ge->Priority = tpTimeCritical; // set the priority lower than normal
		break;
	default:
		gb_process_ge->Priority = tpNormal; // set the priority lower than normal
		break;
	}
	gb_process_ge->Resume(); // now start the thread running
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrGE_Delete(void)
{
C_Timeout *timeout = nullptr;

	if (gb_process_ge != nullptr)
	{
		//gb_process_ge->Stop();
		//gb_process_ge->WaitFor(3000);
		//timeout = new C_Timeout(2000);
		//timeout->Start();
		//flag = false;
		//while (!flag)
		//{
		//	if (!gb_process_ge->Running()) flag = true;
		//	else if (timeout->Timeout()) flag = true;
		//}
		//delete timeout;
		delete gb_process_ge;
		gb_process_ge = nullptr;
	}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
bool ThrGE_IsRunning(void)
{
	return (gb_process_ge != nullptr && gb_process_ge->Running());
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
bool ThrGE_IsStopped(void)
{
	return !ThrGE_IsRunning();
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrGE_Error(void)
{
	return gb_process_ge != nullptr ? gb_process_ge->GetError() : EPCIE_NO_THREAD_GE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrGE_Continue(int source)
{
	int result;

	if (gb_driver_enabled && !ThrGE_IsRunning()) return ELOG(charEPCIE_NO_THREAD_GE, EPCIE_NO_THREAD_GE);
	if (gb_process_ge != nullptr && (result = gb_process_ge->Continue(source)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrGE_Pause(int source)
{
	int result;

	if (!ThrGE_IsRunning()) return ELOG(charEPCIE_NO_THREAD_GE, EPCIE_NO_THREAD_GE);
	if (gb_process_ge != nullptr && (result = gb_process_ge->Pause(source)) < 0) return RLOG(result);
	return EPCIE_NONE;
}
