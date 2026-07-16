// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if !defined(_MSC_VER)
   #include <vcl.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"
#include "st_thread_scan.h"
#include "tad.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_reg.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "c_buffer_acq.h"
#include "c_buffer_hw.h"
#include "buffer_error_code.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "c_virtual_channel.h"
#include "st_vch_set.h"
#include "st_thread_acq.h"
#include "st_get.h"

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
#if defined(_MSC_VER)
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
DWORD WINAPI TThread_AcqScan_Execute(LPVOID lpParam)
{
DWORD result = 0;

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
HANDLE TThread_AcqScan_CreateThread (bool CreateSuspended, LPDWORD thread_id)
{
HANDLE result = NULL;
DWORD dwCreationFlags = 0;

   //if (gb_process_acq_scan == NULL) return ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);

   if (CreateSuspended) dwCreationFlags = CREATE_SUSPENDED;
//SECURITY_ATTRIBUTES security_attributes;
//
//   security_attributes.bInheritHandle = TRUE;
//   security_attributes.nLength = sizeof(security_attributes);
   if ((result = CreateThread (   
            NULL,                      // default security attributes
            0,                         // use default stack size
            TThread_AcqScan_Execute,   // thread function name
            NULL,                      // argument to thread function
            dwCreationFlags,           // use default creation flags
            thread_id)) == NULL)     // returns the thread identifier
   {
      //Microsoft::VisualStudio::ErrorHandler(TEXT("CreateThread"));
      ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
   return result;
}

#endif

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
TThread_AcqScan::TThread_AcqScan (bool CreateSuspended)
{
   end_ = false;
   mc_index_trigger_ = 0;
   mc_index_auxiliar_ = 0;
   pa_index_trigger_ = 0;
   pa_index_auxiliar_ = 0;
   tfm_index_trigger_ = 0;
   tfm_index_auxiliar_ = 0;
   mc_id_vch_ = 0;
   mc_id_fl_ = 0;
   pa_id_vch_ = 0;
   pa_id_fl_ = 0;
   mc_index_focal_law_ = 0;
   pa_index_focal_law_ = 0;
   tfm_index_focal_law_ = 0;
   locked_ = false;
   status_ = ST_ThreadAcqScan_Stopped;

#if defined(_MSC_VER)
   this->Terminated = false;
   this->error_ = ES2_NONE;
   this->wait_timeout_ = 3000;
   this->Priority = tpNormal;
   this->thread_handle_ = TThread_AcqScan_CreateThread (CreateSuspended, &this->thread_id_);
   if (thread_handle_ == NULL)  // returns the thread identifier
   {
      //Microsoft::VisualStudio::ErrorHandler(TEXT("CreateThread"));
      error_ = ELOG(charES2_IdThreadNULL, ES2_IdThreadNULL);
   }
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::MC_GetData(void)
{
int new_acq, result = 0, id_vch = -1, id_fl;
short int *ptr_acq_data = NULL;
vector<CPtr *> ptrbuffer_encoder;
FILE *f=NULL;
CAcqData *ptr_data = NULL;
unsigned long ant_index_trigger = 0;
bool cmd_continue = gb_process_acq != NULL && !gb_process_acq->End() && UCIP.pause_after_burst == 1 && UCIP.auto_continue_mode == 1;

    if (gb_buffer_hw != NULL && (result = gb_buffer_hw->MC_DecoDataBuffer(&id_vch, &id_fl)) < 0) return RLOG(result);
    if (result > 0 && id_vch >= 0 && id_vch < gb_virtual_channel.size() &&  gb_virtual_channel[id_vch]->buffer_mc_hwut_ != NULL && id_fl >= 0 && id_fl < gb_virtual_channel[id_vch]->GET_FocalLawNumber())
    {
        mc_id_vch_ = id_vch;
        mc_id_fl_ = id_fl;
        if ((ptr_data = &gb_virtual_channel[mc_id_vch_]->acq_data_) != NULL)
        {
            if ((new_acq = gb_fp_buffer_acq->GetNoEncoder_AcqIndex (ptr_data->acq_counter, mc_id_vch_, mc_id_fl_, &mc_index_trigger_, &mc_index_auxiliar_)) < 0)    return RLOG(new_acq);

            if (new_acq > 0)
            {
                // _________________________________________________________________
                // SCAN DATA
                if (gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->SelectAcquisitionData_AScan())
                {
                    // Get Scan Data from acquisition buffer
                    if (ptr_data->data.size() > 0)
                    {
                        // Load Scan Data to scan buffer
                        //LOG_Info("Scan --> TRG(%04d) - FL(%04d)", mc_index_trigger_, mc_id_fl_);
                        if ((result = gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->WriteData (&ptr_data->data[0], (unsigned long)ptr_data->data.size(), mc_id_fl_, mc_index_trigger_, mc_index_auxiliar_)) < 0)
                            return RLOG(result);
                    }
                }

                // _________________________________________________________________
                // SCAN Counter
                if (gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->Enabled_Counter())
                {
                    if ((result = gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->WriteCounter(ptr_data->acq_counter, mc_id_fl_, mc_index_trigger_, mc_index_auxiliar_)) < 0)
                        return RLOG(result);
                }

                gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->IndexAuxiliar(mc_index_auxiliar_);
                gb_virtual_channel[mc_id_vch_]->buffer_mc_hwut_->IncAcquisitionCounter(mc_id_fl_);
                mc_index_focal_law_++;
            }
        }
    }
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::PA_GetData(void)
{
int new_acq, result = 0, id_vch = -1;
short int *ptr_acq_data = NULL;
vector<CPtr *> ptrbuffer_encoder;
FILE *f=NULL;
CAcqData *ptr_data = NULL;

   if (gb_buffer_hw != NULL && (result = gb_buffer_hw->PA_DecoDataBuffer(&id_vch)) < 0) return RLOG(result);
   if (result > 0 &&  id_vch >= 0 &&  id_vch < gb_virtual_channel.size() &&  gb_virtual_channel[id_vch]->buffer_pa_hwut_ != NULL)
   {
      if ((ptr_data = &gb_virtual_channel[id_vch]->acq_data_) != NULL)
      {
         if ((new_acq = gb_pa_buffer_acq->GetNoEncoder_AcqIndex(ptr_data->acq_counter, id_vch, 0, &pa_index_trigger_, &pa_index_auxiliar_)) < 0)
            return RLOG(new_acq);

         if (new_acq > 0)
         {
            // _________________________________________________________________
            // SCAN DATA
            if (gb_virtual_channel[id_vch]->buffer_pa_hwut_->SelectAcquisitionData_AScan())
            {
               // Get Scan Data from acquisition buffer
               if (ptr_data->data.size() > 0)
               {
                  // Load Scan Data to scan buffer
                  if ((result = gb_virtual_channel[id_vch]->buffer_pa_hwut_->WriteData(&ptr_data->data[0], (unsigned long)ptr_data->data.size(), pa_index_trigger_, pa_index_auxiliar_)) < 0)
                     return RLOG(result);
               }
            }

            // _________________________________________________________________
            // SCAN Counter
            if (gb_virtual_channel[id_vch]->buffer_pa_hwut_->Enabled_Counter())
            {
               if ((result = gb_virtual_channel[id_vch]->buffer_pa_hwut_->WriteCounter(ptr_data->acq_counter, pa_index_trigger_, pa_index_auxiliar_)) < 0)
                  return RLOG(result);
            }

            gb_virtual_channel[id_vch]->buffer_pa_hwut_->IndexAuxiliar(pa_index_auxiliar_);
            gb_virtual_channel[id_vch]->buffer_pa_hwut_->IncAcquisitionCounter(0);
            pa_index_focal_law_++;
         }
      }
   }
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::TFM_GetData(void)
{
    int new_acq, result = ES2_NONE, id_vch = -1;
    short int* ptr_acq_data = NULL;
    vector<CPtr*> ptrbuffer_encoder;
    FILE* f = NULL;
    CAcqData* ptr_data = NULL;

    if (gb_buffer_hw != NULL && (result = gb_buffer_hw->TFM_DecoDataBuffer(&id_vch)) < 0) return RLOG(result);

    if (result > 0 && id_vch >= 0 && id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch]->buffer_tfm_hwut_ != NULL)
    {
        if ((ptr_data = &gb_virtual_channel[id_vch]->acq_data_) != NULL)
        {
            if ((new_acq = gb_tfm_buffer_acq->GetNoEncoder_AcqIndex(ptr_data->acq_counter, id_vch, 0, &tfm_index_trigger_, &tfm_index_auxiliar_)) < 0)
                return RLOG(new_acq);

            if (new_acq > 0)
            {
                // _________________________________________________________________
                // SCAN DATA
                if (gb_virtual_channel[id_vch]->buffer_tfm_hwut_->SelectAcquisitionData_AScan())
                {
                    // Get Scan Data from acquisition buffer
                    if (ptr_data->data.size() > 0)
                    {
                        // Load Scan Data to scan buffer
                        if ((result = gb_virtual_channel[id_vch]->buffer_tfm_hwut_->WriteData(&ptr_data->data[0], (unsigned long)ptr_data->data.size(), tfm_index_trigger_, tfm_index_auxiliar_)) < 0)
                            return RLOG(result);
                    }
                }

                // _________________________________________________________________
                // SCAN Counter
                if (gb_virtual_channel[id_vch]->buffer_tfm_hwut_->Enabled_Counter())
                {
                    if ((result = gb_virtual_channel[id_vch]->buffer_tfm_hwut_->WriteCounter(ptr_data->acq_counter, tfm_index_trigger_, tfm_index_auxiliar_)) < 0)
                        return RLOG(result);
                }

                gb_virtual_channel[id_vch]->buffer_tfm_hwut_->IndexAuxiliar(tfm_index_auxiliar_);
                gb_virtual_channel[id_vch]->buffer_tfm_hwut_->IncAcquisitionCounter(0);
                tfm_index_focal_law_++;
            }
        }
    }
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void TThread_AcqScan::Stop(void)
{
    if (gb_process_acq != NULL)
    {
        gb_process_acq->Terminate();
    }
    Terminated = true;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void TThread_AcqScan::ThreadFunction(void)
{
int result;
short int *ptr_acq_data = NULL;
vector<CPtr *> ptrbuffer_encoder;
FILE *f=NULL;
CAcqData *ptr_data = NULL;

   end_ = false;
   status_ = ST_ThreadAcqScan_StartingBuffer;
   error_ = 0;

   if (gb_fp_buffer_acq == NULL) 
   {
      error_ = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   if (gb_pa_buffer_acq == NULL)
   {
      error_ = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   if (gb_tfm_buffer_acq == NULL)
   {
       error_ = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
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
    if ((result = ST_GetScanLinesTrigger()) < 0)
    {
        error_ = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
        end_ = true;
        ST_Release();
        return;
    }
    n_trg_lines_ = result;

   if ((mc_n_aux_lines_ = ST_GetScanLinesAuxiliar()) <= 0)
   {
      result = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   pa_n_aux_lines_ = mc_n_aux_lines_;
   if ((n_encoder_ = ST_RangeEncodersNumber()) <= 0)
   {
      error_ = ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
      end_ = true;
      ST_Release();
      return;
   }
   
   trigger_focal_laws_ = ST_GetTriggerFocalLawsNumber();
   mc_index_focal_law_ = 0;
   pa_index_focal_law_ = 0;
   tfm_index_focal_law_ = 0;
   index_auxiliar_saved_ = -1;
   gb_fp_buffer_acq->UpdateConfig();
   gb_fp_buffer_acq->ResetLastAcqCounter();
   gb_fp_buffer_acq->DBG_FileOpen();
   gb_pa_buffer_acq->UpdateConfig();
   gb_pa_buffer_acq->ResetLastAcqCounter();
   gb_pa_buffer_acq->DBG_FileOpen();
   gb_tfm_buffer_acq->UpdateConfig();
   gb_tfm_buffer_acq->ResetLastAcqCounter();
   gb_tfm_buffer_acq->DBG_FileOpen();

   ST_VCH_RESET_AcquisitionCounter();
   status_ = ST_ThreadAcqScan_Running;

   while (!Terminated && error_ >= 0)
   {
      if (!locked_)
      {
         if ((result = MC_GetData()) < 0)
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
         if ((result = TFM_GetData()) < 0)
         {
             error_ = RLOG(result);
             end_ = true;
             ST_Release();
             return;
         }
      }
   }
   status_ = ST_ThreadAcqScan_Stopping;
   if (gb_fp_buffer_acq != NULL) gb_fp_buffer_acq->DBG_FileClose();
   if (gb_pa_buffer_acq != NULL) gb_pa_buffer_acq->DBG_FileClose();
   if (gb_tfm_buffer_acq != NULL) gb_tfm_buffer_acq->DBG_FileClose();
   gb_buffer_hw->Clear_AcquisitionBuffer();
   ST_Release();

   status_ = ST_ThreadAcqScan_Stopped;
   end_ = true;
}

#if defined(_MSC_VER)

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TThread_AcqScan::Resume(void)
{
DWORD dword_value;
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
   default: return ELOG(charES2_RANGO, ES2_RANGO); break;
   }

   if (SetThreadPriority(thread_handle_, priority) == 0) 
      return ELOG(charES2_ThreadResume, ES2_ThreadResume);

   if ((dword_value = ResumeThread(thread_handle_)) == (DWORD)-1)
      return ELOG(charES2_ThreadResume, ES2_ThreadResume);
   
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool TThread_AcqScan::EndAcquisition(void)
{
    if (UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS) return false;
    if (n_trg_lines_ > 0)
        return pa_index_trigger_ >= n_trg_lines_ - 1 || mc_index_trigger_ >= n_trg_lines_ - 1 || tfm_index_trigger_ >= n_trg_lines_ - 1;
    return false;
}


#endif

