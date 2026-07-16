#ifndef c_buffer_ptrH
#define c_buffer_ptrH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_critical_section.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define BUFFER_FORMAT_NOT_ORTHOGONAL  0
#define BUFFER_FORMAT_ORTHOGONAL      1

// _____________________________________________________________________________
// ------ CPtr class
// _____________________________________________________________________________
// =============================================================================
class CPtr
{
public:

   unsigned char *data;
   uint64 n_data;
   unsigned long data_size;

   CPtr::CPtr(AnsiString name);
   CPtr::~CPtr();
   BOOL WaitFor (void);
   void Release (void);
   BOOL Acquire (int type, bool safe_access=true, bool *non_blocking=NULL);

protected:

private:

   bool locked_;
   CCriticalSection *plock_data_;
   CCriticalSection *plock_access_;
   AnsiString name_;
};

// _____________________________________________________________________________
// ------ CBufferPtr class
// _____________________________________________________________________________
// =============================================================================
class CBufferPtr
{
public:

   CPtr *ptr_;

   CBufferPtr::CBufferPtr (bool lshift, AnsiString name, int image_format);
   CBufferPtr::~CBufferPtr();
   void Release (void);
   BOOL Acquire (int type, bool safe_access=true);
   unsigned long GetDataSize(void) {return buffer_data_size_;}
   unsigned long long GetSize(void) { return size_; }
   unsigned long GetBlockDataNumber(void) {return buffer_n_data_block_;}
   unsigned long GetAcquisitionsNumber(void) {return buffer_n_acquisitions_;}
   unsigned long GetAcquisitionDataNumber(void) { return buffer_n_data_acquisition_; }
   
   bool MemSize (unsigned long buffer_n_blocks_capacity, unsigned long n_blocks, unsigned long buffer_block_size, unsigned long buffer_data_size, bool lshift);
   int LeftShift(unsigned long index_trg);
   void SetFormatType(int data) {image_format_ = data;}
   CPtr* GetPtr(unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux, bool safe_access = true);
   CPtr* GetPtrLastTrigger(unsigned long* idx_trg, unsigned long* idx_aux, bool safe_access = true);
   CPtr* GetPtrTrigger(unsigned long idx_trg, unsigned long idx_aux, bool safe_access);
   int SetFileData(fstream& f, __int64 position, unsigned long n_data, int data_size, unsigned long offset = 0);
   void DataFromFile(bool data) { data_from_file_ = data; }
   bool AcquiredAcquisition(unsigned long index_acquisition) { return (long)index_acquisition <= buffer_last_acquisition_; }

protected:

   unsigned long long size_;
   unsigned long long buffer_capacity_;
   unsigned long buffer_data_size_;
   unsigned long n_packs_;
   unsigned long n_data_pack_;
   unsigned long buffer_n_blocks_;
   unsigned long buffer_n_data_block_;
   unsigned long buffer_n_acquisitions_;
   unsigned long buffer_n_data_acquisition_;
   int image_format_;
   unsigned long n_lines_trg_;
   unsigned long buffer_n_data_trigger_;
   unsigned long n_lines_aux_;
   bool data_from_file_;
   long buffer_last_block_;
   long buffer_last_acquisition_;

   void Clear (void);
   int Resize (unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packs, unsigned long n_data_pack, unsigned long data_size, bool lshift);
   int Write (unsigned char *source, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux);
   int MemCpy (unsigned char *source, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux);
   int MemCpyBlock(unsigned char* source, unsigned long n_data, unsigned long idx_trg, unsigned long idx_aux);
   int MemLeftShift (unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux);
   unsigned long GetDataNumber (void);
   int GetBlock(unsigned char *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux);
   unsigned long long GetBufferCapacity(void) {return buffer_capacity_;}
   int SetData(unsigned char* data, unsigned long n_data, long index);

private:

   unsigned char *buffer_;
   unsigned char *buffer_lshift_;
   bool buffer_lshift_select_;
   bool buffer_lshift_start_;
   bool buffer_lshift_flag_;
   long idx_last_aux_; 
   long idx_last_trg_;
   long idx_last_block_;
   long idx_last_aux_read_;
   long idx_last_trg_read_;
   long idx_last_block_read_;
   long idx_last_acquisition_written_;
   long idx_actual_acquisition_writing_;
   long counter_blocks_;
   bool lshift_;
   AnsiString name_;
};

// _____________________________________________________________________________
// ------ D A T A   T Y P E S
// =============================================================================

#endif
