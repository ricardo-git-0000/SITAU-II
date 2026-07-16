// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "stlib2.h"
#include "stlib_dev.h"
#include "st_reg.h"
#include "buffer_error_code.h"
#include "c_log_file.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "c_buffer_acq.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "utils.h"
#include "st_calc.h"
#include "st_vch_get.h"
#include "c_virtual_channel.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CBufferPA_ScanHWUT class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferPA_ScanHWUT::CBufferPA_ScanHWUT (int ch, TFP_VCH_ConfigData *vch)
{
   this->id_last_fl_ = 0;
   this->id_vch_ = ch;
   this->get_data_ = true;
   this->enabled_counter_ = true;
   this->n_lines_trg_ = 0;
   this->lshift_ = false;
   this->vch_ = vch;
   this->acquisition_counter_ = 0;
   this->last_focal_law_acquired_ = 0;
   this->buffer_data_ = NULL;
   this->buffer_counter_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferPA_ScanHWUT::~CBufferPA_ScanHWUT (void)
{
   MemoryFree();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferPA_ScanHWUT::Print(FILE *f)
{
unsigned long size_mem = 0;

    if (f != NULL) fprintf(f, "\n\n       GEThernet - PA Buffer.-");
    if (buffer_data_ != NULL) size_mem += buffer_data_->Print(f);
    if (buffer_counter_ != NULL) size_mem += buffer_counter_->Print(f);
    return size_mem;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long long CBufferPA_ScanHWUT::GetMemory(void)
{
unsigned long long result = 0;

   if (buffer_data_ != NULL) result += buffer_data_->GetMemory();
   if (buffer_counter_ != NULL) result += buffer_counter_->GetMemory();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::MemoryResize (void)
{
int error;
char str[255];

   if (vch_ == NULL) return EBUF_NONE;

   index_auxiliar_ = 0;
   if ((n_lines_trg_ = ST_GetScanLinesTrigger()) < 0) return RLOG(n_lines_trg_);
   if ((n_lines_aux_ = ST_GetScanLinesAuxiliar()) < 0) return RLOG(n_lines_aux_);
   trigger_source_ = ST_GetTriggerSource();
try
{
   if (get_data_)
   {
      sprintf(str, "(vch %d) Original Data", id_vch_);
      switch (vch_->acquisition_type)
      {
      case ACQUISITION_TYPE_PA:
          buffer_data_ = CBufferScan_Resize(buffer_data_, n_lines_trg_, 1, 1, vch_->pa_focal_law.image_size_user, vch_->pa_focal_law.beamformer.size(), vch_->acq.n_beamformed_samples, sizeof(short int), lshift_, &error, AnsiString(str));
          break;
      case ACQUISITION_TYPE_TFM:
          buffer_data_ = CBufferScan_Resize(buffer_data_, n_lines_trg_, 1, 1, vch_->tfm.image_size_user, vch_->tfm_backward.beamformer.size(), vch_->acq.n_beamformed_samples, sizeof(short int), lshift_, &error, AnsiString(str));
          break;
      default: return ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency); break;
      }
      if (error < 0) return RLOG(error);
   }
   else MemoryFree();

   // Memory Resize --> Acq. Counter Scan
   if (enabled_counter_)
   {
      buffer_counter_ = CBufferScan_Resize(buffer_counter_, n_lines_trg_, n_lines_aux_, 1, 1, 1, 1, sizeof(unsigned long), lshift_, &error, "Index Acquisition");
      if (error < 0) return RLOG(error);
   }
   else
   {
      if (buffer_counter_ != NULL) delete buffer_counter_;
      buffer_counter_ = NULL;
   }
}
__finally
{
   if (error < 0) MemoryFree();
}
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::MemoryFree(void)
{
   if (buffer_data_ != NULL) delete buffer_data_;
   buffer_data_ = NULL;
   if (buffer_counter_ != NULL) delete buffer_counter_;
   buffer_counter_ = NULL;
   n_lines_trg_ = 0;
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::WriteData ( short int *data,
                                 unsigned long n_data,                                 
                                 unsigned long index_trg,
                                 unsigned long index_aux,
                                 unsigned long *n_acquisitions_to_end)
{
int result = EBUF_NONE;
CPtr *cptr_data = NULL;

   if (vch_ == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
   if (data == NULL || n_data == 0) return EBUF_NONE;
   if (buffer_data_ != NULL)
   {
      if (n_data > buffer_data_->GetBlockSize()) n_data = buffer_data_->GetBlockSize();
      if ((result = buffer_data_->Write(data, n_data, 0, index_trg, index_aux)) < 0) return RLOG(result);
      if (n_acquisitions_to_end != NULL)
         *n_acquisitions_to_end = buffer_data_->GetAcquisitionsToEndLine();
   }
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::WriteCounter(unsigned long data, unsigned long index_trg, unsigned long index_aux, unsigned long* n_acquisitions_to_end)
{
   int result;

   if (buffer_counter_ != NULL)
   {
      if ((result = buffer_counter_->Write(&data, 1, 0, index_trg, index_aux)) < 0)
         return RLOG(result);
      if (n_acquisitions_to_end != NULL)
         *n_acquisitions_to_end = buffer_counter_->GetAcquisitionsToEndLine();
   }
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::GetBuffer_LastImageData (short int *data, unsigned long size_data, unsigned long *n_data, unsigned long *index_trigger, unsigned long *index_auxiliar)
{
    int result = EBUF_NONE;
    CPtr* ptr_data = NULL;

    if (buffer_data_ != NULL)
    {
        if (size_data < buffer_data_->GetBlockSize()) return 0;
        if ((ptr_data = buffer_data_->GetPtrLastTrigger(index_trigger, index_auxiliar, false)) != NULL)
        {
            if (ptr_data->n_data > 0)
            {
                    result = ptr_data->n_data > size_data ? size_data : ptr_data->n_data;
                    memcpy((short int*)data, (short int*)ptr_data->data, result * 2);
                    DecAcquisitionCounter();
                    if (n_data != NULL) *n_data = result;
            }
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::GetBuffer_ImageData (short int *data, unsigned long size_data, unsigned long *n_data, unsigned long index_trigger, unsigned long index_auxiliar)
{
int result = EBUF_NONE;
unsigned long block_size;

   if (n_data != NULL) *n_data = 0;
   if (buffer_data_ != NULL)
   {
      block_size = buffer_data_->GetBlockSize();
      if (size_data < block_size) return ELOG(charEBUF_Parameter, EBUF_Parameter);
      if ((result = buffer_data_->GetBufferBlock ((unsigned char *)data, block_size, n_data, NULL, NULL, 0, index_trigger, index_auxiliar)) < 0)
         return RLOG(result);
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferPA_ScanHWUT::Release (void)
{
   if (buffer_data_ != NULL) buffer_data_->Release();
   if (buffer_counter_ != NULL) buffer_counter_->Release();
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferPA_ScanHWUT::Clear (void)
{
   acquisition_counter_ = 0;   
   if (buffer_data_ != NULL) buffer_data_->Clear();
   if (buffer_counter_ != NULL) buffer_counter_->Clear();
}




// _____________________________________________________________________________
// ------ 'gb_buffer_pa_scan_hwut' Access Functions
// _____________________________________________________________________________
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void BufferPA_HWUT_Clear (void)
{
int i;

    for (i=0; i<gb_virtual_channel.size(); i++)
    {
        if (gb_virtual_channel[i] != NULL && gb_virtual_channel[i]->buffer_pa_hwut_ != NULL)
        {
            if (gb_virtual_channel[i]->cfg_.acquisition_type == ACQUISITION_TYPE_PA) 
                gb_virtual_channel[i]->buffer_pa_hwut_->Clear();
        }
    }
}





