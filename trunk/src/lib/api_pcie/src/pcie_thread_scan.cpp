// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================


// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pcie_tad.h"
#include "pcie_thread_scan.h"
#include "pcie_tad.h"
//#include "stlib2.h"
//#include "stlib_dev.h"
//#include "st_reg.h"
#include "pcie_error_code.h"
#include "log.h"
//#include "c_buffer_acq.h"
//#include "c_buffer_hw.h"
//#include "buffer_error_code.h"
//#include "c_buffer_fp_scan_hw_ut.h"
//#include "c_buffer_pa_scan_hw_ut.h"
//#include "buffer_fp_data.h"
//#include "buffer_pa_data.h"
//#include "c_virtual_channel.h"
//#include "st_vch_set.h"

#if !defined(_MSC_VER)
   #pragma hdrstop
   #pragma package(smart_init)
#endif

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TThread_AcqScan *gb_process_acq_scan = NULL;

// _____________________________________________________________________________
// ------ Visual Studio Inferface - TThread_AcqScan class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint32 WINAPI TThread_AcqScan_Execute(LPVOID lpParam)
{
uint32 result = 0;

   if (gb_process_acq_scan != NULL)
   {
      gb_process_acq_scan->ThreadFunction();
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
HANDLE TThread_AcqScan_CreateThread (bool CreateSuspended, LPDWORD thread_id, int focal_law_mode)
{
HANDLE result = NULL;
uint32 dwCreationFlags = 0;

   if (CreateSuspended) dwCreationFlags = CREATE_SUSPENDED;
   if ((result = CreateThread (   
            NULL,                      // default security attributes
            0,                         // use default stack size
            TThread_AcqScan_Execute,   // thread function name
            NULL,                      // argument to thread function
            dwCreationFlags,           // use default creation flags
            thread_id)) == NULL)     // returns the thread identifier
   {
      ELOG(charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
   }
   return result;
}

// _____________________________________________________________________________
// ------ TThread_AcqScan class Methods
// _____________________________________________________________________________
// =============================================================================

//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TThread_AdqImg::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TThread_AcqScan::TThread_AcqScan (bool CreateSuspended, int focal_law_mode)
{
   end_ = false;
   focal_law_mode_ = focal_law_mode;
   fp_index_trigger_ = 0;
   fp_index_auxiliar_ = 0;
   pa_index_trigger_ = 0;
   pa_index_auxiliar_ = 0;
   fp_id_vch_ = 0;
   fp_id_fl_ = 0;
   pa_id_vch_ = 0;
   pa_id_fl_ = 0;
   locked_ = false;
   status_ = ST_ThreadAcqScan_Stopped;

   this->Terminated = false;
   this->error_ = EPCIE_NONE;
   this->wait_timeout_ = 3000;
   this->Priority = tpNormal;
   this->thread_handle_ = TThread_AcqScan_CreateThread (CreateSuspended, &this->thread_id_, focal_law_mode_);
   if (thread_handle_ == NULL)  // returns the thread identifier
   {
      //Microsoft::VisualStudio::ErrorHandler(TEXT("CreateThread"));
      error_ = ELOG(charEPCIE_IdThreadNULL, EPCIE_IdThreadNULL);
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::GetData(void)
{
int new_acq, result, id_vch, id_fl;
short int *ptr_acq_data = NULL;
vector<TPtrBufferGate> ptrbuffer_hw_gate, ptrbuffer_sw_gate;
vector<CPtr *> ptrbuffer_encoder;
FILE *f=NULL;
CAcqData *ptr_data = NULL;
unsigned long ant_index_trigger = 0;

   if ((result = gb_buffer_hw->FP_DecoDataBuffer(&id_vch, &id_fl)) < 0)
      return RLOG(result);

   if (  result > 0 && 
         fp_id_vch_ >= 0 &&
         fp_id_vch_ < gb_virtual_channel.size() && 
         gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_ != NULL && 
         fp_id_fl_ >= 0 && 
         fp_id_fl_ < gb_virtual_channel[fp_id_vch_]->GET_FP_FocalLaw_Number())
   {
      fp_id_vch_ = id_vch;
      fp_id_fl_ = id_fl;
      if ((ptr_data = &gb_virtual_channel[fp_id_vch_]->acq_data_) != NULL)
      {
         if ((new_acq = gb_fp_buffer_acq->GetNoEncoder_AcqIndex (ptr_data->acq_counter, fp_id_vch_, fp_id_fl_, &fp_index_trigger_, &fp_index_auxiliar_)) < 0)
            return RLOG(new_acq);

         if (new_acq > 0)
         {
            // _________________________________________________________________
            // SCAN DATA
            if (gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->SelectAcquisitionData_AScan())
            {
               // Get Scan Data from acquisition buffer
               if (ptr_data->data.size() > 0)
               {
                  // Load Scan Data to scan buffer
                  if ((result = gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->WriteData (fp_id_fl_, &ptr_data->data[0], (unsigned long)ptr_data->data.size(), fp_index_trigger_, fp_index_auxiliar_)) < 0)
                     return RLOG(result);
               }
            }

            // _________________________________________________________________
            // SCAN Counter
            if (gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->Enabled_Counter())
            {
               if ((result = gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->WriteCounter(ptr_data->acq_counter, fp_index_trigger_, fp_index_auxiliar_)) < 0)
                  return RLOG(result);
            }

            gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->IndexAuxiliar(fp_index_auxiliar_);
            gb_virtual_channel[fp_id_vch_]->buffer_fp_hwut_->IncAcquisitionCounter(fp_id_fl_);
         }
      }
   }
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void TThread_AcqScan::ThreadFunction(void)
{
int result;
short int *ptr_acq_data = NULL;
vector<TPtrBufferGate> ptrbuffer_hw_gate, ptrbuffer_sw_gate;
vector<CPtr *> ptrbuffer_encoder;
FILE *f=NULL;
CAcqData *ptr_data = NULL;
time_t t;

   end_ = false;
   status_ = ST_ThreadAcqScan_StartingBuffer;
   error_ = 0;

   if (gb_log_enabled && gb_log_thread_file_path != "")
   {
      if ((f = fopen (gb_log_thread_file_path.c_str(), "at")) != NULL)
      {
         time(&t);
         fprintf(f, "\r\n%s TThread_Scan (id: %d) -> Start", ctime(&t), GetCurrentThreadId());
      }
      fclose(f);
   }

   if (gb_fp_buffer_acq == NULL) 
   {
      error_ = ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   if (gb_pa_buffer_acq == NULL)
   {
      error_ = ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }

   if ((n_vch_ = ST_GetVirtualChannelsNumber()) <= 0)
   {
      error_ = RLOG(n_vch_);
      end_ = true;
      ST_Release();
      return;
   }
   else if (n_vch_ == 0)
   {
      error_ = ELOG(charEBUF_IdScan, EBUF_IdScan);
      end_ = true;
      ST_Release();
      return;
   }

   ST_BufferPrint();

   if (n_vch_ != gb_virtual_channel.size())
   {
      error_ = RLOG(error_);
      end_ = true;
      ST_Release();
      return;
   }
	if ((result = ST_GetScanLinesTrigger()) <= 0)
   {
      error_ = ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   fp_n_trg_lines_ = result;
   pa_n_trg_lines_ = result;
   if ((fp_n_aux_lines_ = ST_GetScanLinesAuxiliar()) <= 0)
   {
      result = ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   pa_n_aux_lines_ = fp_n_aux_lines_;
   if ((n_encoder_ = ST_RangeEncodersNumber()) <= 0)
   {
      error_ = ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }

   index_auxiliar_saved_ = -1;
   gb_fp_buffer_acq->UpdateConfig();
   gb_fp_buffer_acq->ResetLastAcqCounter();
   gb_fp_buffer_acq->DBG_FileOpen();
   gb_pa_buffer_acq->UpdateConfig();
   gb_pa_buffer_acq->ResetLastAcqCounter();
   gb_pa_buffer_acq->DBG_FileOpen();

   ST_VCH_RESET_AcquisitionCounter();
   status_ = ST_ThreadAcqScan_Running;

   while (!Terminated && error_ >= 0)
   {
      if (!locked_)
      {
         if ((result = FP_GetData()) < 0)
         {
            error_ = RLOG(result);
            end_ = true;
            ST_Release();
            return;
         }
         if ((result = PA_GetData()) < 0)
         {
            error_ = RLOG(result);
            end_ = true;
            ST_Release();
            return;
         }
         //if (no_data_received) Sleep(50);
      }
   }
   status_ = ST_ThreadAcqScan_Stopping;
   if (gb_fp_buffer_acq != NULL) gb_fp_buffer_acq->DBG_FileClose();
   if (gb_pa_buffer_acq != NULL) gb_pa_buffer_acq->DBG_FileClose();
   gb_buffer_hw->Clear_AcquisitionBuffer();
   ST_Release();

   status_ = ST_ThreadAcqScan_Stopped;
   end_ = true;

   if (gb_log_enabled && gb_log_thread_file_path != "")
   {
      if ((f = fopen (gb_log_thread_file_path.c_str(), "at")) != NULL)
      {
         time(&t);
         fprintf(f, "\r\n%s TThread_Scan (id: %d) -> End", ctime(&t), GetCurrentThreadId());
      }
      fclose(f);
   }

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::Resume(void)
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
   default: return ELOG(charEPCIE_RANGO, EPCIE_RANGO); break;
   }

   if (SetThreadPriority(thread_handle_, priority) == 0) 
      return ELOG(charEPCIE_ThreadResume, EPCIE_ThreadResume);

   if ((dword_value = ResumeThread(thread_handle_)) == (uint32)-1)
      return ELOG(charEPCIE_ThreadResume, EPCIE_ThreadResume);
   
   return EPCIE_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool TThread_AcqScan::FP_EndAcquisition(void)
{
   return fp_id_vch_ >= gb_virtual_channel.size()-1 && 
      fp_id_fl_ >= gb_virtual_channel[fp_id_vch_]->GET_FP_FocalLaw_Number()-1 && 
      fp_index_trigger_ >= fp_n_trg_lines_ - 1 && 
      fp_index_auxiliar_ >= fp_n_aux_lines_ - 1;
}

