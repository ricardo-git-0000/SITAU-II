// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "pcie_thread_acq.h"
#include "pcie_thread_ge.h"
#include "pcie_error_code.h"
#include "c_timeout.h"
#include "c_image_handle.h"
#include "api_pcie.h"
#include "stlib2.h"
#include "log.h"
#include "pcie.h"
#include "stlib2.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TThread_Acq *gb_process_acq = NULL;

// _____________________________________________________________________________
// ------ Visual Studio Inferface - TThread_Acq class Methods
// _____________________________________________________________________________
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint32 WINAPI TThread_Acq_Execute(LPVOID lpParam)
{
uint32 result = 0;

   if (gb_process_acq != NULL)
   {
      gb_process_acq->ThreadFunction();
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
HANDLE TThread_Acq_CreateThread (bool CreateSuspended, LPDWORD thread_id)
{
HANDLE result = NULL;
uint32 dwCreationFlags = 0;

   if (CreateSuspended) dwCreationFlags = CREATE_SUSPENDED;
   if ((result = CreateThread (   
            NULL,                      // default security attributes
            0,                         // use default stack size
            TThread_Acq_Execute,   // thread function name
            NULL,                      // argument to thread function
            dwCreationFlags,           // use default creation flags
            thread_id)) == NULL)     // returns the thread identifier
   {
      ELOG(charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
   }
   return result;
}

// _____________________________________________________________________________
// ------ TThread_Acq class Methods
// _____________________________________________________________________________
// =============================================================================

//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
TThread_Acq::TThread_Acq (bool CreateSuspended, unsigned long n_blocks, unsigned long n_images_blocks, unsigned long time_sleep, int sorted)
{
  int n_fl;

	char str[255];
  float data_float = ST_GetPRF();
	LOG_Open(FILE_LOG_THR_ACQ, "Thread Acquisition");

	n_images_read_ = 0;
	sitau_drv_paused_ = false;
  sitau_user_paused_ = false;
  end_ = false;
   time_sleep_ = time_sleep;
   sorted_ = sorted;
   n_blocks_ = n_blocks;
   n_images_blocks_ = n_images_blocks;

   if ((n_fl = ST2_GetFocalLawNumber()) < 0) n_fl = 1;
  if (data_float > 0) prf_time_ = n_fl * 1000000 / data_float;
   else prf_time_ = 1;

   if (time_sleep_ > 1000) time_sleep_ = 1000;
   locked_ = false;
   status_ = ST_ThreadAcq_Stopped;
   error_ = 0;
   this->Terminated = false;
   this->error_ = EPCIE_NONE;
   this->Priority = tpNormal;
   this->thread_handle_ = TThread_Acq_CreateThread (CreateSuspended, &thread_id_);
   if (thread_handle_ == NULL) error_ = ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
   sprintf(str, "Thread Acquisition: Created (Id. %d)", thread_id_);
   LOG_Info(FILE_LOG_THR_ACQ, str);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
TThread_Acq::~TThread_Acq(void)
{
	focal_laws_pointers_.clear();
	LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: Deleted");
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
void TThread_Acq::ThreadFunction(void)
{
	int result;
	unsigned long n_fl, n_ch, n_samples;
	unsigned long long n_images;
	FILE *f=NULL;
	C_Timeout *timeout = NULL;
	string str;
	bool mode_continuous;
	int acq_mode;
	int n_burst_buffer = N_BURST_BUFFER, data_int;

	LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: Started");
	
	end_ = false;
	status_ = ST_ThreadAcq_Starting;
	if ((n_burst_buffer = ST_XML_GetBuffer_PCIeUserBufferBurstsNumber()) < 0) n_burst_buffer = N_BURST_BUFFER;

	if (gb_image_handle == nullptr)
	{
		error_ = ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_NoBuffer, EPCIE_NoBuffer);
		end_ = true;
		return;
	}

   status_ = ST_ThreadAcq_StartingBuffer;
   if ((result = ST2_GetFocalLawNumber()) < 0)
   {
	   error_ = RLOG_THR(FILE_LOG_THR_ACQ, result);
	   end_ = true;
	   return;
   }
   n_fl = result;
   if ((result = ST2_GetAScanDataNumber()) < 0)
   {
	   error_ = RLOG_THR(FILE_LOG_THR_ACQ, result);
	   end_ = true;
	   return;
   }
   n_samples = result;
   if ((result = ST_GetChannelsNumber()) < 0)
   {
	   error_ = RLOG_THR(FILE_LOG_THR_ACQ, result);
	   end_ = true;
	   return;
   }
   n_ch = result;

   if ((result = ST2_GetAcquisitionsNumber()) < 0)
   {
	   error_ = RLOG_THR(FILE_LOG_THR_ACQ, result);
	   end_ = true;
	   return;
   }
   n_images = result;
   if (focal_laws_pointers_.size() < n_fl) focal_laws_pointers_.resize(n_fl);

   if ((gb_buffer_data = CBufferScan_Resize (gb_buffer_data, n_burst_buffer, n_images_blocks_, n_fl, n_ch, n_samples, sizeof(short int), sorted_, &error_, "PCIE Buffer Data")) == NULL)
   {
	   error_ = ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_NoBuffer, EPCIE_NoBuffer);
	   end_ = true;
	   return;
   }
   gb_buffer_data->MovingAverage_KernelSize(gb_moving_average_kernel_size);
   gb_buffer_data->FileLogEnabled(false);
   if ((data_int = ST_XML_GetDataLogger_PCIeUserBuffer()) >= 0) gb_buffer_data->DataLogger(data_int);

	if (gb_log_enabled)
	{
		if ((f = fopen(FILE_LOG_THR_ACQ, "at")) != NULL)
		{
			gb_buffer_data->Print(f);
			fclose(f);
		}
	}

	status_ = ST_ThreadAcq_Running;

	acq_mode = ST_GetAcquisitionMode();
	mode_continuous = acq_mode == AQUISITION_MODE_CONTINOUS || acq_mode == AQUISITION_MODE_CONTINOUS_BURST;
	n_images_read_ = 0;
	sitau_drv_paused_ = false;
	sitau_user_paused_ = false;
	no_hw_image_index_ = 0;
  time_stamp_ = 0;
  idx_image_timestamped_ = 0;
	while (!Terminated && error_ >= 0 && ! end_)// && !gb_image_handle->LastImageRead())
	{
		if (!locked_)
		{
	        status_ = ST_ThreadAcq_Acquiring;
			if ((result = Acquire()) < 0)
			{
				error_ = RLOG_THR(FILE_LOG_THR_ACQ, result);
			}
			//last_image_read_ = gb_image_handle->LastImageRead();
			//if (!mode_continuous && result == 0 && n_images_read_ >= n_images)
			//	Terminate();
		}
		if (time_sleep_ > 0) Sleep(time_sleep_);
	}
	status_ = ST_ThreadAcq_Stopping;

   if (timeout != NULL) delete timeout;

   // Detiene la adquisición
   status_ = ST_ThreadAcq_StoppingThreadScan;

   LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: Terminated");

   status_ = ST_ThreadAcq_Stopped;
   end_ = true;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int TThread_Acq::Acquire(void)
{
int idx_burst_timestamped, result = 0;
unsigned long current_image_index = 0;
T_block_header *Block_header_p = NULL;

	if (gb_image_handle == nullptr) return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_IMAGE_HANDLE_NULL, EPCIE_IMAGE_HANDLE_NULL);
	if (gb_buffer_data == nullptr) return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_NoBuffer, EPCIE_NoBuffer);
	if (!gb_buffer_data->IsFull())
	{
		if (sitau_user_paused_ && !gb_image_handle->Paused() && gb_buffer_data->IsFillingRatio(0.5))
		{
			LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: 'Continue' Buffer Usuario");
			ThrGE_Continue(GE_SOURCE_ACQ_BUFFER_USER);
			sitau_user_paused_ = false;
		}

		if (gb_driver_enabled)
		{
			if ((result = gb_image_handle->GetOneImage_Pointers(&focal_laws_pointers_[0], &current_image_index, &Block_header_p)) < 0)
				return RLOG_THR(FILE_LOG_THR_ACQ, result);
		}
		else
		{
			for (int i = 0; i < focal_laws_pointers_.size(); i++) focal_laws_pointers_[i] = (short int *)(no_hw_image_index_ + i);
			no_hw_image_index_++;
			result = 1;
		}
		if (!sitau_drv_paused_ && gb_image_handle->Paused())
		{
			sitau_drv_paused_ = true;
			LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: 'Pause' Buffer Driver");
		}
		if (sitau_drv_paused_ && gb_image_handle->RequestContinue())
		{
			LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: 'Continue' Buffer Driver");
			ThrGE_Continue(GE_SOURCE_ACQ_BUFFER_DRV);
			gb_image_handle->ResetRequestContinue();
			sitau_drv_paused_ = false;
		}
		if (result == 1)
		{
			if (Block_header_p != NULL)
			{
				if (Block_header_p->header_struct.ch_id_last.bits.timestamp_present == 1)
				{
					time_stamped_ = true;
					time_stamp_ = Block_header_p->header_struct.tail * 1000;
					if (Block_header_p->header_struct.image_index % n_images_blocks_ == 0)
					{
						idx_burst_timestamped = floor(Block_header_p->header_struct.image_index / n_images_blocks_);
						idx_image_timestamped_ = Block_header_p->header_struct.image_index;
					}
					else
					{
						idx_burst_timestamped = floor(Block_header_p->header_struct.image_index / n_images_blocks_) + 1;
						idx_image_timestamped_ = idx_burst_timestamped * n_images_blocks_;
					}
					printf("\nTime Stamp: %d - Image: %d ", time_stamp_, idx_image_timestamped_);
				}
				else
				{
					time_stamp_ += prf_time_;
				}
				//if ((result = gb_buffer_data->Add(focal_laws_pointers_, current_image_index, Block_header_p->header_struct.ch_id_last.bits.timestamp_present == 1 ? Block_header_p->header_struct.tail : 0xFFFFFFFF)) < 0)
				if ((result = gb_buffer_data->Add(focal_laws_pointers_, current_image_index, time_stamp_)) < 0)
				{
					RLOG_THR(FILE_LOG_THR_ACQ, result);
				}
				if ((result = PCIe_FreeOneImage_Pointers(nullptr)) < 0)
				{
					RLOG_THR(FILE_LOG_THR_ACQ, result);
				}
				if (time_stamped_ && current_image_index == idx_image_timestamped_)
				{
					printf("\nImage TimeStamped: %d (%d)", current_image_index, time_stamp_);
					time_stamped_ = false;
				}
			}
			else
			{
				char str[255];
				sprintf(str, "no_hw_image_index_(%d)", no_hw_image_index_);
				LOG_Info(FILE_LOG_THR_ACQ,str);
				if ((result = gb_buffer_data->Add(focal_laws_pointers_, current_image_index, no_hw_image_index_)) < 0)
				{
					RLOG_THR(FILE_LOG_THR_ACQ, result);
				}
			}
			n_images_read_++;
			gb_buffer_data->LastImageRead(gb_image_handle->LastImageRead());
		}
	}
	else
	{
		if (!sitau_user_paused_)
		{
			LOG_Info(FILE_LOG_THR_ACQ, "Thread Acquisition: 'Pause' Buffer Usuario");
			ThrGE_Pause(GE_SOURCE_ACQ_BUFFER_USER);
			sitau_user_paused_ = true;
		}
	}

	//if (PCIe_DMAThreadIsRunning() <= 0)
	//{
	//	PCIe_InitBoard();
	//	Sleep(100);
	//	return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_NO_THREAD_DMA, EPCIE_NO_THREAD_DMA);
	//}
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_Acq::Resume(void)
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
   default: return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_PARAMETER, EPCIE_PARAMETER); break;
   }

   if (SetThreadPriority(thread_handle_, priority) == 0) 
      return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_ThreadResume, EPCIE_ThreadResume);

   if ((dword_value = ResumeThread(thread_handle_)) == (uint32)-1)
      return ELOG_THR(FILE_LOG_THR_ACQ, charEPCIE_ThreadResume, EPCIE_ThreadResume);
   
   return EPCIE_NONE;
}




// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrAcq_Create(int priority, unsigned long time_sleep, int sorted)
{
	if (gb_process_acq != NULL) return 0;// ThrAcq_Delete();
	if ((gb_process_acq = new TThread_Acq(true, gb_buffer_n_blocks, gb_buffer_n_images_block, time_sleep, sorted)) == NULL)
	{
		ThrAcq_Delete();
		return ELOG(charEPCIE_PtrAccess_NULL, EPCIE_PtrAccess_NULL);
	}
	switch (priority)
	{
	case 0:
		gb_process_acq->Priority = tpLower; // set the priority lower than normal
		break;
	case 2:
		gb_process_acq->Priority = tpHighest; // set the priority lower than normal
		break;
	case 3:
		gb_process_acq->Priority = tpTimeCritical; // set the priority lower than normal
		break;
	default:
		gb_process_acq->Priority = tpNormal; // set the priority lower than normal
		break;
	}
	gb_process_acq->Resume(); // now start the thread running
	return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrAcq_Delete(void)
{
C_Timeout *timeout = NULL;
bool flag;

	if (gb_process_acq != NULL)
	{
		gb_process_acq->Locked(true);
		gb_process_acq->Terminate();
		gb_process_acq->WaitFor();
		timeout = new C_Timeout(5000);
		timeout->Start();

		flag = false;
		while (!flag)
		{
			if (gb_process_acq->End()) flag = true;
			else if (timeout->Timeout()) 
				flag = true;
		}
		delete timeout;
		delete gb_process_acq;
		gb_process_acq = NULL;
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
bool ThrAcq_IsRunning(int *status)
{
  bool result = false;
  ThrAcq_Status rd_status = ST_ThreadAcq_Stopped;

  if (gb_process_acq != nullptr)
  {
    ThrAcq_Status rd_status = gb_process_acq->GetStatus();
    result = rd_status == ST_ThreadAcq_Acquiring;
  }
	if (status != nullptr) *status = rd_status;
	return result;
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
bool ThrAcq_IsStopped(void)
{
	return (gb_process_acq != NULL && gb_process_acq->GetStatus() == ST_ThreadAcq_Stopped);
}
// -----------------------------------------------------------------------------
/**

@param[in]
@param[out]
@return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ThrAcq_Error(void)
{
	return gb_process_acq != NULL ? gb_process_acq->GetError() : EPCIE_NO_THREAD_ACQ;
}
