#ifndef c_buffer_scanH
#define c_buffer_scanH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_buffer_ptr.h"
#include "c_critical_section.h"

// _____________________________________________________________________________
// ------ D E F I N E S
// =============================================================================

#define CBufferScan_NameSize 100

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CScanBuffer class
// _____________________________________________________________________________
// =============================================================================
class CBufferScan : public CBufferPtr
{
private:

   AnsiString name_;
   int block_index_last_;
   int trigger_index_last_;
   int auxiliar_index_last_;
   int block_index_last_read_;
   int trigger_index_last_read_;
   int auxiliar_index_last_read_;

   unsigned long n_data_last_;
   unsigned long n_acquisitions_to_end_;
   vector<vector<unsigned char> > available_data_;
   bool swap_;
   vector<short int> swap_fl_;

public:

   CBufferScan (unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packs, unsigned long n_data_pack, unsigned long data_size, bool lshift, int *error, AnsiString name="", int image_format=0);
   ~CBufferScan();
   unsigned long Print(FILE *f);
   unsigned long long GetMemory(void) {return size_;}
   int Clear (void);
   int Resize(unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packages, unsigned long n_data_package, unsigned long data_size, bool lshift, int image_format);
   int Write (void *data, unsigned long n_data, unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux=0);
   CPtr* GetPtrBufferAcquisition(unsigned long idx_trg, unsigned long idx_aux, bool safe_access);
   int GetBufferAcquisition(unsigned char* data, unsigned long size_data, unsigned long* n_data, unsigned long* n_acquisitions_to_end, unsigned char* available_images, unsigned long index_trigger, unsigned long index_auxiliar);
   CPtr *GetPtrBufferBlock (unsigned long idx_block, unsigned long idx_trg, unsigned long idx_aux, bool safe_access);
   int GetBufferBlock (unsigned char *data, unsigned long size_data, unsigned long *n_data, unsigned long *n_acquisitions_to_end, unsigned char *available_images, unsigned long index_focal_law, unsigned long index_trigger, unsigned long index_auxiliar);
	void SetBlockIndexLast(unsigned long data) { block_index_last_ = data; }
   void SetTriggerIndexLast(unsigned long data) {trigger_index_last_ = data;}
   void SetAuxiliarIndexLast(unsigned long data) {auxiliar_index_last_ = data;}
   void SetDataNumberLast(unsigned long data) {n_data_last_ = data;}
   void UpdateDataNumberLast(void) { n_data_last_ = buffer_n_data_block_; }
   int GetBlockIndexLast(void) { return block_index_last_; }
   int GetTriggerIndexLast(void) { return trigger_index_last_; }
   int GetAuxiliarIndexLast(void) { return auxiliar_index_last_; }
   int GetTriggerIndexLastRead(void) {return trigger_index_last_read_;}
   int GetAuxiliarIndexLastRead(void) {return auxiliar_index_last_read_;}
   int Release (void);
   unsigned long GetTriggerLines() {return n_lines_trg_;}
   unsigned long GetBlockSize() {return buffer_n_data_block_;}
   unsigned long GetPacksNumber(void) { return n_packs_; }
   unsigned long GetPackDataNumber(void) { return n_data_pack_; }
   unsigned long GetBlocksNumber(void) { return buffer_n_blocks_; }
   unsigned long GetAcquisitionDataNumber(void) { return buffer_n_data_acquisition_; }
   
   bool Swap(void) {return swap_;}
   void Swap(bool data) {swap_ = data;}
   unsigned long long GetBufferCapacity(void) {return CBufferPtr::GetBufferCapacity();}
   void ResetAcquisitionsCounter(bool availableData = false);
   unsigned long GetAcquisitionsToEndLine(void) {return n_acquisitions_to_end_;}
   int GetAvailableAcquisitions(unsigned char *available_images, unsigned long index_auxiliar=0);
   int WriteFile(fstream& f, vector<unsigned short>& pin_to_channel);
};

// _____________________________________________________________________________
// ------ D A T A   T Y P E S
// =============================================================================

// _____________________________________________________________________________
// ------ 'CBufferScan' Access Functions
// _____________________________________________________________________________
// =============================================================================

CBufferScan *CBufferScan_Resize (CBufferScan* buffer_scan, unsigned long n_lines_trg, unsigned long n_lines_aux, unsigned long n_blocks, unsigned long n_data_block, unsigned long n_packages, unsigned long n_data_package, unsigned long data_size, bool lshift, int* error, AnsiString name="", int image_format=0);

#endif
