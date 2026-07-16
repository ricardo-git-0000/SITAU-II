// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_log_file.h"
#include "utils.h"
#include "buffer_error_code.h"
#include "c_buffer_scan.h"
#include "t_base_tad.h"
#include "fileformat.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CBufferScan class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferScan::CBufferScan (unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packs, unsigned long n_data_pack, unsigned long data_size, bool lshift, int *error, AnsiString name, int image_format)
   : CBufferPtr(lshift, name, image_format)
{
int result;

    if ((result = CBufferPtr::Resize (n_lines_trg, n_lines_aux, n_blocks, n_data_block, n_packs, n_data_pack, data_size, lshift)) < 0)
        {RLOG(result);}
    if (error != NULL) *error = result;
    name_ = name;
    swap_ = false;
    Clear();
    Release();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long CBufferScan::Print(FILE *f)
{
   if (size_ > 0 && f != NULL)
   {
      fprintf(f,"\n\n---> '%s' ",name_.c_str());
      fprintf(f,"\n   Size:             %llu Bytes (%lu Bytes/Data)", size_, buffer_data_size_);

      fprintf(f,"\n   N. Acquisitions:  %lu Acquisitions (%lu Blocks/Acquisition)", buffer_n_acquisitions_, buffer_n_blocks_);
      fprintf(f,"\n   N. Signals/Block: %lu Signals (%lu Samples/Signal)", n_packs_, n_data_pack_);

      fprintf(f,"\n   Trigger Lines:    %lu", n_lines_trg_);
      fprintf(f,"\n   Auxiliar Lines:   %lu", n_lines_aux_);
   }
   return size_;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferScan::~CBufferScan (void)
{
	Clear();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::Clear (void)
{
    block_index_last_ = -1;
    trigger_index_last_ = -1;
    auxiliar_index_last_ = -1;
    block_index_last_read_ = -1;
    trigger_index_last_read_ = -1;
    auxiliar_index_last_read_ = -1;
    n_data_last_ = 0;
    ResetAcquisitionsCounter();
    CBufferPtr::Clear();
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferScan::ResetAcquisitionsCounter(bool availableData)
{
int i;

   if (n_lines_aux_ == 0)
         n_lines_aux_ = 1;
   if (n_lines_trg_ < 1)
         n_lines_trg_ = 100;
   available_data_.clear();
   available_data_.resize(n_lines_aux_, vector<unsigned char>(n_lines_trg_, availableData ? 1 : 0));
   for (i=0; i<available_data_.size(); i++)
      std::fill(available_data_[i].begin(), available_data_[i].end(), availableData ? 1 : 0);
   n_acquisitions_to_end_ = n_lines_trg_ - 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::Resize (unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packages, unsigned long n_data_package, unsigned long data_size, bool lshift, int image_format)
{
int result;

   image_format_ = image_format;
   if (!Acquire(CS_WR)) {ELOG(charEBUF_Acquire, EBUF_Acquire);}
   if (!MemSize(n_lines_trg * n_lines_aux, n_blocks, n_data_block, data_size, lshift))
   {
      if ((result = CBufferPtr::Resize (n_lines_trg, n_lines_aux, n_blocks, n_data_block, n_packages, n_data_package, data_size, lshift)) < 0)
         return RLOG(result);
      Clear();
   }
   this->data_from_file_ = false;
   Release();
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::Write (void *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux)
{
int result;
unsigned long i_lost_trg = 0;
CPtr *ptr_data=NULL;

    if (data == NULL) return EBUF_NONE;
	if (n_data > 0)
	{
        if ((result = CBufferPtr::Write ((unsigned char *)data, n_data, idx_block, idx_trg, idx_aux)) < 0)
            return RLOG(result);

      block_index_last_ = idx_block;
      if (idx_block >= buffer_n_blocks_ - 1) 
          trigger_index_last_ = idx_trg;
      auxiliar_index_last_ = idx_aux;
      n_data_last_ = n_data;
      if (idx_aux < available_data_.size() && idx_trg < available_data_[idx_aux].size())
      {
         if (available_data_[idx_aux][idx_trg] == 0 && n_acquisitions_to_end_ > 0) n_acquisitions_to_end_--;
         available_data_[idx_aux][idx_trg] = 1;
      }
   }
    return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPtr* CBufferScan::GetPtrBufferAcquisition(unsigned long idx_trg, unsigned long idx_aux, bool safe_access)
{
    CPtr* result = NULL;

    if ((result = CBufferPtr::GetPtr(0, idx_trg, idx_aux, safe_access)) != NULL)
    {
        if (result->n_data >= buffer_n_data_acquisition_) result->n_data = buffer_n_data_acquisition_;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::GetBufferAcquisition(unsigned char* data, unsigned long size_data, unsigned long* n_data, unsigned long* n_acquisitions_to_end, unsigned char* available_images, unsigned long index_trigger, unsigned long index_auxiliar)
{
    int result = EBUF_NONE;

    if (size_data >= size_) size_data = size_;
    if ((result = CBufferPtr::GetBlock(data, size_data, 0, index_trigger, index_auxiliar)) < 0) return RLOG(result);
    if (n_data != NULL) *n_data = result;
    if (available_images != NULL && index_auxiliar < available_data_.size() && available_data_.size() > 0)
        memcpy(available_images, &available_data_[index_auxiliar][0], available_data_[index_auxiliar].size() * sizeof(unsigned char));
    if (n_acquisitions_to_end != NULL) *n_acquisitions_to_end = n_acquisitions_to_end_;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPtr *CBufferScan::GetPtrBufferBlock (unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux, bool safe_access)
{
   return CBufferPtr::GetPtr(idx_block, idx_trg, idx_aux, safe_access);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::GetBufferBlock(unsigned char *data, unsigned long size_data, unsigned long *n_data, unsigned long *n_acquisitions_to_end, unsigned char *available_images, unsigned long index_focal_law, unsigned long index_trigger, unsigned long index_auxiliar)
{
int result = EBUF_NONE;

    if (available_data_[index_auxiliar][index_trigger] == 1)
    {
        if (size_data < buffer_n_data_block_) return ELOG(charEBUF_Parameter, EBUF_Parameter);
        if ((result = CBufferPtr::GetBlock(data, buffer_n_data_block_, index_focal_law, index_trigger, index_auxiliar)) < 0) return RLOG(result);
        if (n_data != NULL) *n_data = result;
        if (available_images != NULL && index_auxiliar < available_data_.size() && available_data_.size() > 0)
            memcpy(available_images, &available_data_[index_auxiliar][0], available_data_[index_auxiliar].size() * sizeof(unsigned char));
        if (n_acquisitions_to_end != NULL) *n_acquisitions_to_end = n_acquisitions_to_end_;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::Release (void)
{
   if (ptr_ != NULL) ptr_->Release();
	return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::GetAvailableAcquisitions (unsigned char *available_images, unsigned long index_auxiliar)
{
   if (available_images != NULL && index_auxiliar < available_data_.size() && available_data_.size() > 0)
      memcpy ( available_images,
               &available_data_[index_auxiliar][0],
               available_data_[index_auxiliar].size() * sizeof(unsigned char));
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferScan::WriteFile(fstream& f, vector<unsigned short> &pin_to_channel)
{
    int i, id_ascan, id_ascan_assign, id_sample, i_line_block, result = EBUF_NONE;
    unsigned long id_data,i_block, i_trg, i_aux;
    uint64 n_bytes, data_counter = 0;
    CPtr* ptr_data;
    char data_char;
    unsigned long data_size = sizeof(short);

    if ((ptr_data = GetPtr(0, 0, 0, false)) != NULL)
    {
        if (ptr_data->n_data > 0)
        {
            data_char = APPBIN_iCAMPO;
            n_bytes = sizeof(data_char);
            if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charEBUF_File, EBUF_File);

            n_bytes = sizeof(data_size);
            if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charEBUF_File, EBUF_File);

            n_bytes = sizeof(ptr_data->n_data);
            if (FS_Write(f, (char*)&ptr_data->n_data, n_bytes) != n_bytes) return ELOG(charEBUF_File, EBUF_File);

            if (data_from_file_)
            {
                if (FS_Write(f, (char*)ptr_data->data, ptr_data->n_data * data_size) != ptr_data->n_data * data_size) return ELOG(charEBUF_File, EBUF_File);
            }
            else
            {
                if (swap_fl_.size() != buffer_n_data_block_) swap_fl_.resize(buffer_n_data_block_);

                for (i_aux = 0; i_aux < n_lines_aux_; i_aux++)
                {
                    for (i_trg = 0; i_trg < n_lines_trg_; i_trg++)
                    {
                        for (i_block = 0; i_block < buffer_n_blocks_; i_block++)
                        {
                            if ((ptr_data = GetPtrBufferBlock(i_block, i_trg, i_aux, false)) != NULL && ptr_data->n_data > 0)
                            {
                                for (i = id_ascan = data_counter = 0; id_ascan < n_packs_ && data_counter < buffer_n_data_block_; id_ascan++)
                                {
                                    if (pin_to_channel.size() >= n_packs_ && id_ascan < pin_to_channel.size()) id_ascan_assign = pin_to_channel[id_ascan];
                                    else id_ascan_assign = id_ascan;
                                    id_ascan_assign = id_ascan_assign < n_packs_ ? id_ascan_assign : n_packs_ - 1;

                                    for (id_sample = 0; id_sample < n_data_pack_ && data_counter < buffer_n_data_block_; id_sample++, i++, data_counter++)
                                    {
                                        i_line_block = id_ascan_assign / MOD_MAX_CH;
                                        if (n_packs_ > MOD_MAX_CH) id_data = id_sample * MOD_MAX_CH + id_ascan_assign + i_line_block * MOD_MAX_CH * (n_data_pack_ - 1);
                                        else id_data = id_sample * n_packs_ + id_ascan_assign;
                                        if (id_data >= ptr_data->n_data) id_data = ptr_data->n_data - 1;
                                        swap_fl_[data_counter] = *((short*)ptr_data->data + id_data);
                                    }
                                }
                                if (FS_Write(f, (char*)&swap_fl_[0], swap_fl_.size() * data_size) != swap_fl_.size() * data_size) return ELOG(charEBUF_File, EBUF_File);
                            }
                        }
                    }
                }
            }
            data_char = APPBIN_fCAMPO;
            n_bytes = sizeof(data_char);
            if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charEBUF_File, EBUF_File);
        }
        else if ((result = WriteField(f, (uint64)0)) < 0) return RLOG(result);
    }
    else if ((result = WriteField(f, (uint64)0)) < 0) return RLOG(result);
    return result;
}

// _____________________________________________________________________________
// ------ 'CBufferScan' Access Functions
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferScan *CBufferScan_Resize (CBufferScan *buffer_scan, unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packages, unsigned long n_data_package, unsigned long data_size, bool lshift, int *error, AnsiString name, int image_format)
{
int result;

    if (buffer_scan == NULL)
      buffer_scan = new CBufferScan (n_lines_trg, n_lines_aux, n_blocks, n_data_block, n_packages, n_data_package, data_size, lshift, error, name, image_format);
   else
   {
      if ((result = buffer_scan->Resize (n_lines_trg, n_lines_aux, n_blocks, n_data_block, n_packages, n_data_package, data_size, lshift, image_format)) < 0)
         {RLOG(result);}
      if (error != NULL) *error = result;
   }
   return buffer_scan;
}

