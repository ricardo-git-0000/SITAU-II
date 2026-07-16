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
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_acq.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "utils.h"
#include "st_calc.h"
#include "st_vch_get.h"
#include "c_virtual_channel.h"
#include "sitau_hw.h"
#include "fileformat.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

//bool gb_buffer_fp_hwut_lock = false;

// _____________________________________________________________________________
// ------ CBufferMC_ScanHWUT class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferMC_ScanHWUT::CBufferMC_ScanHWUT (int ch, TFP_VCH_ConfigData *vch, TFP_VCH_SysConfig *sys, bool hw_format)
{
   this->id_last_fl_ = 0;
   this->id_vch_ = ch;
   this->get_data_ = true;
   this->enabled_counter_ = true;
   this->n_lines_trg_ = 0;
   this->lshift_ = true;
   this->buffer_data_ = NULL;
   this->vch_ = vch;
   this->sys_ = sys;
   this->acquisition_counter_ = 0;
   this->last_focal_law_acquired_ = 0;
   this->last_mc_read_ = -1;
   this->buffer_counter_ = NULL;
   this->hw_format_ = hw_format;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferMC_ScanHWUT::~CBufferMC_ScanHWUT (void)
{
   MemoryFree();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferMC_ScanHWUT::Print(FILE *f)
{
unsigned long size_mem = 0;

    if (f != NULL) fprintf(f, "\n\n       GEThernet - MC Buffer.-");
    if (buffer_data_ != NULL) size_mem += buffer_data_->Print(f);
    if (buffer_counter_ != NULL) size_mem += buffer_counter_->Print(f);
    return size_mem;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long long CBufferMC_ScanHWUT::GetMemory(void)
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
int CBufferMC_ScanHWUT::MemoryResize (int n_acquisitions)
{
int error, n_fl;
char str[255];

   if (vch_ == NULL) return EBUF_NONE;
   if (vch_->fp_focal_law.size() == 0) return ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency);

   n_fl = (int)vch_->mc_emission_focal_law.size();

   index_auxiliar_ = 0;
   n_lines_aux_ = 1;
    if ((n_lines_aux_ = ST_GetScanLinesAuxiliar()) < 0) return RLOG(n_lines_aux_);
    if (n_acquisitions == 0)
    {
        if ((n_lines_trg_ = ST_GetScanLinesTrigger()) < 0) return RLOG(n_lines_trg_);
    }
    else n_lines_trg_ = n_acquisitions / n_lines_aux_;

try
{
    if (get_data_)
    {
        sprintf(str, "(vch %d) Original Data", id_vch_);
        buffer_data_ = CBufferScan_Resize(buffer_data_, n_lines_trg_, n_lines_aux_, vch_->fp_focal_law.size(), vch_->fp_focal_law[0].image_size, vch_->fp_focal_law[0].n_ascan, vch_->acq.n_acq_samples, sizeof(short int), lshift_, &error, AnsiString(str));
        if (error < 0) return RLOG(error);
    }
    else MemoryFree();

    // Memory Resize --> Acq. Counter Scan
    if (enabled_counter_)
    {
        buffer_counter_ = CBufferScan_Resize(buffer_counter_, n_lines_trg_, n_lines_aux_, vch_->fp_focal_law.size(), 1, 1, 1, sizeof(unsigned long), lshift_, &error, "Index Acquisition");
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
int CBufferMC_ScanHWUT::MemoryFree(void)
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
int CBufferMC_ScanHWUT::WriteData (short int *data, unsigned long n_data, unsigned long idx_fl, unsigned long idx_trg, unsigned long idx_aux, unsigned long *n_acquisitions_to_end)
{
int result = EBUF_NONE;
CPtr *cptr_data = NULL;

    if (vch_ == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
    if (data == NULL || n_data == 0) return EBUF_NONE;
    if (buffer_data_ != NULL)
    {
        if ((result = buffer_data_->Write(data, n_data, idx_fl, idx_trg, idx_aux)) < 0) return RLOG(result);
    }
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferMC_ScanHWUT::WriteCounter(unsigned long data, unsigned long index_block, unsigned long index_trg, unsigned long index_aux, unsigned long* n_acquisitions_to_end)
{
   int result;

   if (buffer_counter_ != NULL)
   {
      if ((result = buffer_counter_->Write(&data, 1, index_block, index_trg, index_aux)) < 0)
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
int CBufferMC_ScanHWUT::GetBuffer_LastMCData(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long* index_trigger, unsigned long* index_auxiliar)
{
    int i, n_ascan, n_samples, n_blocks, i_block, n_data_block, id_ascan, id_ascan_assign, id_sample, i_line_block, result = EBUF_NONE;
    unsigned long id_data;
    CPtr* ptr_data = NULL;

    if (buffer_data_ != NULL)
    {
        if (size_data < buffer_data_->GetAcquisitionDataNumber()) return ELOG(charEBUF_Parameter, EBUF_Parameter)
        if ((ptr_data = buffer_data_->GetPtrLastTrigger(index_trigger, index_auxiliar, false)) != NULL)
        {
            if (ptr_data->n_data > 0)
            {
                if (hw_format_)
                {
                    result = ptr_data->n_data > size_data ? size_data : ptr_data->n_data;
                    memcpy((short int*)data, (short int*)ptr_data->data, result * 2);
                    DecAcquisitionCounter();
                    if (n_data != NULL) *n_data = result;
                }
                else
                {
                    if (vch_ == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
                    if (vch_->pa_probe == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
                    n_data_block = buffer_data_->GetBlockDataNumber();
                    n_blocks = buffer_data_->GetBlocksNumber();
                    n_samples = buffer_data_->GetPackDataNumber();
                    n_ascan = buffer_data_->GetPacksNumber();

                    for (i = i_block = 0; i_block < n_blocks; i_block++)
                    {
                        for (id_ascan = 0; id_ascan < n_ascan; id_ascan++)
                        {
                            if (vch_->pa_probe->pin_to_channel_.size() >= n_ascan && id_ascan < vch_->pa_probe->pin_to_channel_.size())
                                id_ascan_assign = vch_->pa_probe->pin_to_channel_[id_ascan];
                            else id_ascan_assign = id_ascan;
                            id_ascan_assign = id_ascan_assign < n_ascan ? id_ascan_assign : n_ascan - 1;

                            for (id_sample = 0; id_sample < n_samples; id_sample++, i++)
                            {
                                i_line_block = id_ascan_assign / MOD_MAX_CH;
                                if (n_ascan > MOD_MAX_CH) id_data = i_block * n_data_block + id_sample * MOD_MAX_CH + id_ascan_assign + i_line_block * MOD_MAX_CH * (n_samples - 1);
                                else id_data = i_block * n_data_block + id_sample * n_ascan + id_ascan_assign;
                                if (id_data >= ptr_data->n_data) id_data = ptr_data->n_data - 1;
                                *(data + i) = *((short int*)ptr_data->data + id_data);
                            }
                        }
                    }
                    result = i;
                    if (n_data != NULL) *n_data = i;
                    DecAcquisitionCounter();
                }
            }
            else ELOG("WARNING GetBuffer_LastMCData() n_data = 0", EBUF_DataInconsistency);
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferMC_ScanHWUT::GetBuffer_MCData(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trigger, unsigned long index_auxiliar)
{
    int i, n_ascan, n_samples, n_blocks, i_block, n_data_block, id_ascan, id_ascan_assign, id_sample, i_line_block, result = EBUF_NONE;
    unsigned long id_data;
    CPtr* ptr_data = NULL;

    if (buffer_data_ != NULL)
    {
        if (size_data < buffer_data_->GetAcquisitionDataNumber()) return ELOG(charEBUF_Parameter, EBUF_Parameter)
        if ((ptr_data = buffer_data_->GetPtrTrigger(index_trigger, index_auxiliar, false)) != NULL)
        {
            if (ptr_data->n_data > 0)
            {
                if (hw_format_)
                {
                    result = ptr_data->n_data > size_data ? size_data : ptr_data->n_data;
                    memcpy((short int*)data, (short int*)ptr_data->data, result * 2);
                    DecAcquisitionCounter();
                    if (n_data != NULL) *n_data = result;
                }
                else
                {
                    if (vch_ == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
                    if (vch_->pa_probe == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
                    n_data_block = buffer_data_->GetBlockDataNumber();
                    n_blocks = buffer_data_->GetBlocksNumber();
                    n_samples = buffer_data_->GetPackDataNumber();
                    n_ascan = buffer_data_->GetPacksNumber();

                    for (i = i_block = 0; i_block < n_blocks; i_block++)
                    {
                        for (id_ascan = 0; id_ascan < n_ascan; id_ascan++)
                        {
                            if (vch_->pa_probe->pin_to_channel_.size() >= n_ascan && id_ascan < vch_->pa_probe->pin_to_channel_.size())
                                id_ascan_assign = vch_->pa_probe->pin_to_channel_[id_ascan];
                            else id_ascan_assign = id_ascan;
                            id_ascan_assign = id_ascan_assign < n_ascan ? id_ascan_assign : n_ascan - 1;

                            for (id_sample = 0; id_sample < n_samples; id_sample++, i++)
                            {
                                i_line_block = id_ascan_assign / MOD_MAX_CH;
                                if (n_ascan > MOD_MAX_CH) id_data = i_block * n_data_block + id_sample * MOD_MAX_CH + id_ascan_assign + i_line_block * MOD_MAX_CH * (n_samples - 1);
                                else id_data = i_block * n_data_block + id_sample * n_ascan + id_ascan_assign;
                                if (id_data >= ptr_data->n_data) id_data = ptr_data->n_data - 1;
                                *(data + i) = *((short int*)ptr_data->data + id_data);
                            }
                        }
                    }
                    result = i;
                    if (n_data != NULL) *n_data = i;
                    DecAcquisitionCounter();
                }
            }
            else ELOG("WARNING GetBuffer_LastMCData() n_data = 0", EBUF_DataInconsistency);
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferMC_ScanHWUT::GetBuffer_FocalLawData(short int *data, unsigned long size_data, unsigned long *n_data, unsigned long index_focal_law, unsigned long index_trigger, unsigned long index_auxiliar)
{
int i, n_ascan, n_samples, id_ascan, id_ascan_assign, id_sample, i_line_block, result = EBUF_NONE;
unsigned long block_size, id_data;
CPtr* ptr_data;

    if (n_data != NULL) *n_data = 0;
    if (n_lines_trg_ == 0) return EBUF_NONE;
    if (index_trigger >= n_lines_trg_) return (charEBUF_Parameter, EBUF_Parameter);
    if (buffer_data_ != NULL)
    {
        if (index_focal_law >= buffer_data_->GetBlocksNumber()) return (charEBUF_Parameter, EBUF_Parameter);
        block_size = buffer_data_->GetBlockSize();
        if (size_data < block_size) return EBUF_NONE;
        if (hw_format_)
        {
            if ((result = buffer_data_->GetBufferBlock((unsigned char*)data, block_size, n_data, NULL, NULL, index_focal_law, index_trigger, index_auxiliar)) < 0)
                return RLOG(result);
        }
        else
        {
            if (vch_ == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
            if (vch_->pa_probe == NULL) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
            if (index_focal_law >= vch_->mc_emission_focal_law.size()) return ELOG(charEBUF_CONFIG_NULL, EBUF_CONFIG_NULL);
            n_samples = vch_->acq.n_acq_samples;
            n_ascan = vch_->fp_focal_law[index_focal_law].n_ascan;

            if ((ptr_data = buffer_data_->GetPtrBufferBlock(index_focal_law, index_trigger, index_auxiliar, false)) != NULL && ptr_data->n_data > 0)
            {
                for (i = id_ascan = 0; id_ascan < n_ascan; id_ascan++)
                {
                    if (vch_->pa_probe->pin_to_channel_.size() >= n_ascan && id_ascan < vch_->pa_probe->pin_to_channel_.size()) id_ascan_assign = vch_->pa_probe->pin_to_channel_[id_ascan];
                    else id_ascan_assign = id_ascan;
                    id_ascan_assign = id_ascan_assign < n_ascan ? id_ascan_assign : n_ascan - 1;

                    for (id_sample = 0; id_sample < n_samples; id_sample++, i++)
                    {
                        i_line_block = id_ascan_assign / MOD_MAX_CH;
                        if (n_ascan > MOD_MAX_CH) id_data = id_sample * MOD_MAX_CH + id_ascan_assign + i_line_block * MOD_MAX_CH * (n_samples - 1);
                        else id_data = id_sample * n_ascan + id_ascan_assign;
                        if (id_data >= ptr_data->n_data) id_data = ptr_data->n_data - 1;
                        *(data + i) = *((short int*)ptr_data->data + id_data);
                        
                    }
                }
                result = i;
                if (n_data != NULL) *n_data = i;
            }
        }
    }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferMC_ScanHWUT::Release (void)
{
    if (buffer_data_ != NULL) buffer_data_->Release();
    if (buffer_counter_ != NULL) buffer_counter_->Release();
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferMC_ScanHWUT::Clear (void)
{
   acquisition_counter_ = 0;
   last_mc_read_ = -1;
   if (buffer_data_ != NULL) buffer_data_->Clear();
   if (buffer_counter_ != NULL) buffer_counter_->Clear();
}





// _____________________________________________________________________________
// ------ 'gb_buffer_fp_scan_hwut' Access Functions
// _____________________________________________________________________________
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void BufferMC_HWUT_Clear (void)
{
    int i;

    for (i = 0; i < gb_virtual_channel.size(); i++)
    {
        if (gb_virtual_channel[i] != NULL && gb_virtual_channel[i]->buffer_mc_hwut_ != NULL)
        {
            if (gb_virtual_channel[i]->cfg_.acquisition_type == ACQUISITION_TYPE_MC)
                gb_virtual_channel[i]->buffer_mc_hwut_->Clear();
        }
    }
}





