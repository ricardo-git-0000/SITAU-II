// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_log_file.h"
#include "buffer_error_code.h"
#include "c_buffer_ptr.h"
#include "ctimer.h"
#include "fileformat.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CPtr class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CPtr::CPtr (AnsiString name)
{
   this->data = NULL;
   this->n_data = 0;
   this->locked_ = false;
   this->name_ = name;
   this->plock_data_ = new CCriticalSection(name_);
   this->plock_access_ = new CCriticalSection(name_);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPtr::~CPtr()
{
   if (this->plock_access_ != NULL && !this->plock_access_->Acquire(CS_NONE))
      {ELOG(charEBUF_Acquire, EBUF_Acquire);}
   if (this->plock_data_ != NULL) delete this->plock_data_;
   this->plock_data_ = NULL;
   if (this->plock_access_ != NULL)
   {
      plock_access_->Release();
      delete this->plock_access_;
   }
   this->plock_access_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
BOOL CPtr::Acquire (int type, bool safe_access, bool *non_blocking)
{
BOOL result = false;

   if (!safe_access) return true;
   if (this->plock_access_ != NULL && !this->plock_access_->Acquire(CS_NONE, true, non_blocking))
   {
      if (this->plock_access_ != NULL) plock_access_->Release();
      if (non_blocking != NULL && !(*non_blocking))
         {ELOG(charEBUF_Acquire, EBUF_Acquire);}
      return false;
   }
   if (this->plock_data_ != NULL)
      result = this->plock_data_->Acquire(type, safe_access, non_blocking);
   if (this->plock_access_ != NULL) plock_access_->Release();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CPtr::Release (void)
{
   if (this->plock_data_ != NULL)
      this->plock_data_->Release();
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
BOOL CPtr::WaitFor (void)
{
BOOL result = false;
int i, dead_count = 5;

   for (i=0; !result && i<dead_count; i++)
      result = plock_data_->Acquire(CS_WR);
   return result;
}

// _____________________________________________________________________________
// ------ CBufferPtr class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CBufferPtr::CBufferPtr (bool lshift, AnsiString name, int image_format)
{
    this->data_from_file_ = false;
	this->n_lines_trg_ = 0;
    this->n_lines_aux_ = 0;
    this->size_ = 0;
    this->idx_last_aux_ = -1;
    this->idx_last_trg_ = -1;
    this->idx_last_block_ = -1;
    this->idx_last_aux_read_ = -2;
    this->idx_last_trg_read_ = -2;
    this->idx_last_block_read_ = 0;
    this->idx_last_acquisition_written_ = -1;
    this->idx_actual_acquisition_writing_ = -1;
    this->counter_blocks_ = 0;
    this->lshift_ = lshift;
    this->buffer_lshift_flag_ = false;
    this->buffer_n_acquisitions_ = 0;
    this->buffer_n_data_acquisition_ = 0;
    this->buffer_n_blocks_ = 0;
    this->buffer_n_data_block_ = 0;
    this->n_packs_ = 0;
    this->n_data_pack_ = 0;
    this->buffer_data_size_ = 0;
    this->buffer_capacity_ = 0;
    this->buffer_last_block_ = -1;
    this->buffer_last_acquisition_ = -1;
    this->buffer_ = NULL;
    this->buffer_lshift_ = NULL;
    this->buffer_lshift_select_ = false;
    this->buffer_lshift_start_ = false;
    this->lshift_ = lshift;
    this->image_format_ = image_format;
    this->name_ = name;
    this->ptr_ = new CPtr(name_);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferPtr::~CBufferPtr()
{
   if (!ptr_->WaitFor())
      {ELOG(charEBUF_Acquire, EBUF_Acquire);}
   if (this->buffer_ != NULL) free(this->buffer_);
   this->buffer_ = NULL;
   if (this->buffer_lshift_ != NULL) free(this->buffer_lshift_);
   this->buffer_lshift_ = NULL;
   if (this->ptr_ != NULL) delete this->ptr_;
   this->ptr_ = NULL;
   this->size_ = 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
BOOL CBufferPtr::Acquire (int type, bool safe_access)
{
BOOL result = true;

   if (this->ptr_ != NULL) result = this->ptr_->Acquire(type, safe_access);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferPtr::Release (void)
{
   if (this->ptr_ != NULL) this->ptr_->Release();
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
void CBufferPtr::Clear (void)
{
   if (this->buffer_ != NULL)
      memset(this->buffer_, 0, this->size_);
   this->buffer_lshift_select_ = false;
   this->buffer_lshift_start_ = false;
   this->buffer_lshift_flag_ = false;
   this->buffer_last_block_ = -1;
   this->idx_last_aux_ = -1; 
   this->idx_last_trg_ = -1;
   this->idx_last_block_ = -1;
   this->idx_last_aux_read_ = -2;
   this->idx_last_trg_read_ = -2;
   this->idx_last_block_read_ = 0;
   this->idx_last_acquisition_written_ = -1;
   this->idx_actual_acquisition_writing_ = -1;
   this->buffer_last_acquisition_ = -1;
   this->counter_blocks_ = 0;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::Resize (unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packs, unsigned long n_data_pack, unsigned long data_size, bool lshift)
{
    unsigned long buffer_n_data_block, buffer_n_acquisitions, buffer_data_size;
    unsigned long long buffer_capacity, size;
    unsigned char* resized_buffer;

try
{
   if (n_data_block > 0) buffer_n_data_block = n_data_block;
   else buffer_n_data_block = 1;

   buffer_n_acquisitions = n_lines_trg * n_lines_aux;

   if (data_size > 0) buffer_data_size = data_size;
   else buffer_data_size = 1;

   buffer_capacity = buffer_n_acquisitions * buffer_n_data_block * n_blocks;

   size = buffer_capacity * buffer_data_size;

   if (this->buffer_ == NULL || size_ != size)
   {
      resized_buffer = (unsigned char *)realloc(this->buffer_, size + 100);
      if (resized_buffer == NULL)
	   	return ELOG(charEBUF_NoMemory,EBUF_NoMemory);
      this->buffer_ = resized_buffer;

      if (lshift)
      {
         resized_buffer = (unsigned char *)realloc(this->buffer_lshift_, size + 100);
         if (resized_buffer == NULL)
            return ELOG(charEBUF_NoMemory,EBUF_NoMemory);
         this->buffer_lshift_ = resized_buffer;
      }
      else if (this->buffer_lshift_ != NULL)
      {
         free(this->buffer_lshift_);
         this->buffer_lshift_ = NULL;
      }
   }
   else if (lshift != lshift_)
   {
      if (lshift)
      {
         resized_buffer = (unsigned char *)realloc(this->buffer_lshift_, size + 100);
         if (resized_buffer == NULL)
            return ELOG(charEBUF_NoMemory,EBUF_NoMemory);
         this->buffer_lshift_ = resized_buffer;
      }
      else if (this->buffer_lshift_ != NULL)
      {
         free(this->buffer_lshift_);
         this->buffer_lshift_ = NULL;
      }
   }
   this->lshift_ = lshift;
   this->buffer_n_blocks_ = n_blocks;
   this->buffer_n_data_block_ = buffer_n_data_block;
   this->buffer_n_data_acquisition_ = buffer_n_data_block_ * buffer_n_blocks_;
   this->buffer_n_acquisitions_ = buffer_n_acquisitions;
   this->buffer_data_size_ = buffer_data_size;
   this->buffer_capacity_ = buffer_capacity;
   this->n_lines_trg_ = n_lines_trg;
   this->n_lines_aux_ = n_lines_aux;
   this->buffer_n_data_trigger_ = n_lines_trg_ * buffer_n_acquisitions_;
   this->size_ = size;
   this->n_packs_ = n_packs;
   this->n_data_pack_ = n_data_pack;
   this->data_from_file_ = false;
}
__finally
{
   ptr_->Release();
}
   return EBUF_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CPtr *CBufferPtr::GetPtr (unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux, bool safe_access)
{
    unsigned long buffer_index, offset;

    if (buffer_ == NULL || size_ == 0 || ptr_ == NULL) return NULL;

    if (n_lines_trg_ <= 1) idx_trg = 0;
    else if (idx_trg >= n_lines_trg_) idx_trg = n_lines_trg_ - 1;

    if (n_lines_aux_ <= 1) idx_aux = 0;
    else if (idx_aux >= n_lines_aux_) idx_aux = n_lines_aux_ - 1;

    if (buffer_n_blocks_ <= 1) idx_block = 0;
    else if (idx_block >= buffer_n_blocks_) idx_block = buffer_n_blocks_ - 1;

    if (lshift_ && n_lines_aux_ == 1 && buffer_lshift_flag_ && idx_last_trg_ >= 0)
    {
        if (idx_last_trg_ < n_lines_trg_ - 1) idx_trg += (idx_last_trg_ + 1);
        if (idx_trg >= n_lines_trg_) idx_trg = idx_trg - n_lines_trg_;
        offset = idx_trg * buffer_n_data_acquisition_;
        buffer_index = offset * buffer_data_size_;
    }
    else
    {
        offset = idx_trg * buffer_n_data_acquisition_ + idx_aux * n_lines_trg_ * buffer_n_data_acquisition_ + idx_block * buffer_n_data_block_;
        buffer_index = offset * buffer_data_size_;
    }
    if (buffer_index > size_)
    {
        ELOG(charEBUF_Overflow, EBUF_Overflow);
        return NULL;
    }
    this->ptr_->data = &buffer_[buffer_index];
    this->ptr_->n_data = buffer_capacity_ - offset;
    this->ptr_->data_size = buffer_data_size_;
    return this->ptr_;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CPtr* CBufferPtr::GetPtrLastTrigger(unsigned long* idx_trg, unsigned long* idx_aux, bool safe_access)
{
    uint64 n_buffer_data, buffer_index, offset = 0;;
    CPtr* result = NULL;
    long idx_last_acquisition_written = idx_last_acquisition_written_;

    if (buffer_ != NULL && ptr_ != NULL && idx_last_acquisition_written >= 0 &&
        (idx_last_trg_read_ != idx_last_acquisition_written))
    {
        n_buffer_data = buffer_n_data_acquisition_ * buffer_data_size_;
        offset = idx_last_acquisition_written * buffer_n_data_acquisition_;
        buffer_index = offset * buffer_data_size_;
        if (buffer_index + n_buffer_data > size_)
        {
            ELOG(charEBUF_Overflow, EBUF_Overflow);
            return NULL;
        }
        this->ptr_->data = &buffer_[buffer_index];
        this->ptr_->n_data = buffer_capacity_ - offset;
        this->ptr_->data_size = buffer_data_size_;

        idx_last_aux_read_ = idx_last_aux_;
        idx_last_trg_read_ = idx_last_acquisition_written;
        
        if (idx_trg != NULL) *idx_trg = idx_last_acquisition_written;
        if (idx_aux != NULL) *idx_aux = idx_last_aux_;
        result = this->ptr_;
    }
    return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CPtr* CBufferPtr::GetPtrTrigger(unsigned long idx_trg, unsigned long idx_aux, bool safe_access)
{
    uint64 n_buffer_data, buffer_index, offset = 0;;
    CPtr* result = NULL;
    long idx_trg_read = idx_trg; // Con lineas auxiliares deberia ser: idx_trg + (idx_aux + 1) * (n_lines_trg_);
    long idx_last_acquisition_written = idx_last_acquisition_written_;

    if (idx_trg_read >= this->buffer_n_acquisitions_)
    {
        ELOG(charEBUF_Parameter, EBUF_Parameter);
        return NULL;
    }
    if (buffer_ != NULL && ptr_ != NULL && idx_last_acquisition_written >= 0 &&
        (idx_trg_read <= idx_last_acquisition_written))
    {
        n_buffer_data = buffer_n_data_acquisition_ * buffer_data_size_;
        offset = idx_trg_read * buffer_n_data_acquisition_;
        buffer_index = offset * buffer_data_size_;
        if (buffer_index + n_buffer_data > size_)
        {
            ELOG(charEBUF_Overflow, EBUF_Overflow);
            return NULL;
        }
        this->ptr_->data = &buffer_[buffer_index];
        this->ptr_->n_data = buffer_capacity_ - offset;
        this->ptr_->data_size = buffer_data_size_;
        result = this->ptr_;
    }
    return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
unsigned long CBufferPtr::GetDataNumber (void)
{
   if (buffer_data_size_ > 0) return size_ / buffer_data_size_;
   else return 0;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::GetBlock (unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux)
{
    uint64 n_buffer_data, buffer_index, offset_aux = 0, offset_trg = 0, offset_line = 0, offset = 0;;
    int result = EBUF_NONE;

    if (data != NULL && n_data > 0 && buffer_ != NULL)
    {
        if ((n_buffer_data = n_data * buffer_data_size_) > size_) n_buffer_data = size_;

        offset = idx_trg * buffer_n_data_acquisition_ + idx_aux * n_lines_trg_ * buffer_n_data_acquisition_ + idx_block * buffer_n_data_block_;
        buffer_index = offset * buffer_data_size_;
        if (buffer_index + n_buffer_data > size_)
            return ELOG(charEBUF_Parameter, EBUF_Parameter);
        memcpy(data, buffer_ + buffer_index, n_buffer_data);
        result = n_buffer_data / buffer_data_size_;
    }
    return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::SetData(unsigned char* data, unsigned long n_data, long index)
{
    unsigned long n_buffer_data, buffer_index;
    int result = EBUF_NONE;

    if (data != NULL && n_data > 0)
    {
        if ((n_buffer_data = n_data * buffer_data_size_) > size_)
            n_buffer_data = size_;

        if (index < 0) index = 0;
        buffer_index = index * buffer_n_data_block_ * buffer_data_size_;
        if (buffer_ != NULL)
        {
            if (buffer_index + n_buffer_data > size_)
                return ELOG(charEBUF_Overflow, EBUF_Overflow);
            memcpy(&buffer_[buffer_index], data, n_buffer_data);
            result = n_buffer_data / buffer_data_size_;
        }
    }
    return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::SetFileData(fstream &f, __int64 position, unsigned long n_data, int data_size, unsigned long offset)
{
    int result = EBUF_NONE;

    if (offset + n_data > buffer_capacity_) return ELOG(charEBUF_Overflow, EBUF_Overflow);

    if (position >= 0)
    {
        if (FS_Seek(f, position, FS_SeekBegin) != position) { ELOG(charEBUF_File, EBUF_File); }
    }
    if (data_size == sizeof(char))
    {
        if ((result = ReadField(f, (char*)buffer_ + offset, n_data)) < 0) return RLOG(result);
    }
    else if (data_size == sizeof(short int))
    {
        if ((result = ReadField(f, (short int*)buffer_ + offset, n_data)) < 0) return RLOG(result);
    }
    else if (data_size == sizeof(long))
    {
        if ((result = ReadField(f, (long*)buffer_ + offset, n_data)) < 0) return RLOG(result);
    }
    else if (data_size == sizeof(double))
    {
        if ((result = ReadField(f, (double*)buffer_ + offset, n_data)) < 0) return RLOG(result);
    }
    else return ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency);
    return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::Write (unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux)
{
int result = EBUF_NONE;
bool non_blocking;

   if (ptr_ != NULL && !ptr_->Acquire(CS_WR, true, &non_blocking))
   {
      if (!non_blocking) {ELOG(charEBUF_Acquire, EBUF_Acquire);}
      return NULL;
   }
try
{
    if ((result = MemCpy(data, n_data, idx_block, idx_trg, idx_aux)) < 0)
        return RLOG(result);
   buffer_last_block_++;
   if (buffer_last_block_ != idx_block)
   {
       char str[255];
       sprintf(str, "buffer_last_block_(%d) != idx_block(%d)", buffer_last_block_, idx_block);
       ELOG(str, -1);
       buffer_last_block_ = idx_block;
   }
   if (buffer_last_block_ >= buffer_n_blocks_ - 1)
   {
       buffer_last_block_ = -1;
       buffer_last_acquisition_++;
   }
}
__finally
{
   if (ptr_ != NULL) ptr_->Release();
}
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::MemCpy (unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux)
{
unsigned long buffer_n_data, buffer_index, offset = 0;

    if (buffer_ == NULL || data == NULL) return ELOG(charEBUF_Parameter, EBUF_Parameter);
    buffer_n_data = buffer_data_size_ * n_data;

    if (idx_block == 0)
    {
        if (counter_blocks_ != buffer_n_blocks_ - 1 && idx_last_acquisition_written_ >= 0)
        {
            char str[255];
            sprintf(str, "Wrinting first block of acquisition %d -> Number of blocks of the previous acquisition %d", idx_trg, counter_blocks_);
            ELOG(str, EBUF_DataInconsistency);
        }
        counter_blocks_ = 0;
        if (idx_actual_acquisition_writing_ < 0)
        {
            idx_actual_acquisition_writing_ = 0;
        }
        else
        {
            idx_actual_acquisition_writing_++;
            if (idx_actual_acquisition_writing_ >= buffer_n_acquisitions_) 
                idx_actual_acquisition_writing_ = 0;
        }
    }
    else if (idx_block == buffer_n_blocks_ - 1)
    {
        counter_blocks_++;
    }
    else if (idx_block >= buffer_n_blocks_)
    {
        char str[255];
        sprintf(str, "Block received %d is out of the range %d", idx_block, buffer_n_blocks_);
        return ELOG(str, EBUF_DataInconsistency);
    }
    else
    {
        counter_blocks_++;
    }

    offset = idx_actual_acquisition_writing_ * buffer_n_data_acquisition_ + idx_block * buffer_n_data_block_;
    buffer_index = offset * buffer_data_size_;
    if (buffer_index + buffer_n_data > size_)
        return ELOG(charEBUF_Parameter, EBUF_Parameter);
    memcpy(buffer_ + buffer_index, data, buffer_n_data);

    if (idx_block == buffer_n_blocks_ - 1)
    {
        idx_last_acquisition_written_ = idx_actual_acquisition_writing_;
    }

   return EBUF_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::MemCpyBlock(unsigned char* data, unsigned long n_data, unsigned long idx_trg, unsigned long idx_aux)
{
    unsigned long buffer_n_data, index, buffer_index, i, idx;
    unsigned long i_trg = 0, i_aux = 0;

    if (buffer_ == NULL || data == NULL) return ELOG(charEBUF_Parameter, EBUF_Parameter);
    if (buffer_n_data_block_ != n_data) return ELOG(charEBUF_Parameter, EBUF_Parameter);
    buffer_n_data = buffer_data_size_ * n_data;

    if (n_lines_aux_ <= 1) i_aux = 0;
    else if (idx_aux >= n_lines_aux_) i_aux = n_lines_aux_ - 1;
    else i_aux = idx_aux;

    if (n_lines_trg_ <= 1) i_trg = 0;
    else if (idx_trg >= n_lines_trg_) i_trg = n_lines_trg_ - 1;
    else i_trg = idx_trg;

    switch (image_format_)
    {
    case BUFFER_FORMAT_ORTHOGONAL:
        for (i = 0; i < n_data; i++)
        {
            if (buffer_n_data_block_ > 1)
            {
                idx = (i_trg + i * n_lines_trg_) * buffer_data_size_;
                memcpy(buffer_ + idx, data + i * buffer_data_size_, buffer_data_size_);
            }
            else
            {
                idx = i_trg + i * n_lines_trg_;
                buffer_[idx] = data[i];
            }
        }
        break;
    default:
        index = i_aux * buffer_n_data_trigger_ + i_trg * buffer_n_data_acquisition_;
        buffer_index = index * buffer_data_size_;
        if (buffer_index + n_data > size_) return ELOG(charEBUF_Parameter, EBUF_Parameter);
        memcpy(buffer_ + buffer_index, data, buffer_n_data);
        break;
    }
    return EBUF_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::MemLeftShift (unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux)
{
int result = EBUF_NONE;
unsigned long buffer_n_data, buffer_index;

   if (idx_trg < buffer_n_acquisitions_ - 1)
   {
      if ((result = MemCpy (data, n_data, idx_block, idx_trg, idx_aux)) < 0)
         return RLOG(result);
   }
   else if (!buffer_lshift_start_)
   {
      if ((result = MemCpy (data, n_data, idx_block, idx_trg, idx_aux)) < 0)
         return RLOG(result);
      buffer_lshift_start_ = true;
   }
   else
   {
      if ((result = LeftShift(idx_trg)) < 0) return RLOG(result);
      buffer_n_data = buffer_data_size_ * n_data;
      buffer_index = idx_trg * buffer_n_data_block_ * buffer_data_size_;
      if (buffer_index + buffer_n_data > size_) return ELOG(charEBUF_Parameter, EBUF_Parameter);
      if (!buffer_lshift_select_) memcpy (buffer_ + buffer_index, data, buffer_n_data);
      else memcpy (buffer_lshift_ + buffer_index, data, buffer_n_data);
   }
   return result;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CBufferPtr::LeftShift(unsigned long index_trg)
{
   if (lshift_)
   {
      if (index_trg < buffer_n_acquisitions_ - 1) return EBUF_NONE;
      else if (!buffer_lshift_start_) buffer_lshift_start_ = true;
      else
      {
         if (buffer_n_data_block_ * buffer_data_size_ > size_) return ELOG(charEBUF_Parameter, EBUF_Parameter);
         if (buffer_lshift_select_)
         {
            memcpy (buffer_, buffer_lshift_ + (buffer_n_data_block_ * buffer_data_size_), size_ - (buffer_n_data_block_ * buffer_data_size_));
            buffer_lshift_select_ = false;
         }
         else
         {
            memcpy (buffer_lshift_, buffer_ + (buffer_n_data_block_ * buffer_data_size_), size_ - (buffer_n_data_block_ * buffer_data_size_));
            buffer_lshift_select_ = true;
         }
      }
   }
   return EBUF_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
bool CBufferPtr::MemSize (unsigned long buffer_n_blocks_capacity, unsigned long n_blocks, unsigned long buffer_block_size, unsigned long buffer_data_size, bool lshift)
{
bool result;

    if (lshift != this->lshift_) result = false;
    else if (lshift_)
        result = (  buffer_data_size_ == buffer_data_size &&
                    buffer_n_blocks_ == n_blocks &&
                    buffer_n_data_block_ == buffer_block_size &&
                    buffer_n_acquisitions_ == buffer_n_blocks_capacity &&
                    buffer_ != NULL &&
                    buffer_lshift_ != NULL);
    else result = ( buffer_data_size_ == buffer_data_size &&
                    buffer_n_blocks_ == n_blocks &&
                    buffer_n_data_block_ == buffer_block_size &&
                    buffer_n_acquisitions_ == buffer_n_blocks_capacity &&
                    buffer_ != NULL);
    return result;
}


