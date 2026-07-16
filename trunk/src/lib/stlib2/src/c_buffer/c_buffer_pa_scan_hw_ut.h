//---------------------------------------------------------------------------

#ifndef fp_c_buffer_pa_scan_hw_utH
#define fp_c_buffer_pa_scan_hw_utH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_buffer_scan.h"
#include "c_critical_section.h"
#include "t_vch.h"

// _____________________________________________________________________________
// ------ CBufferPA_ScanHWUT class
// _____________________________________________________________________________
// =============================================================================
class CBufferPA_ScanHWUT
{
private:
   
   int id_last_fl_;
   unsigned long acquisition_counter_;
   TFP_VCH_ConfigData *vch_;
   int n_hw_gates_, n_lines_trg_, n_lines_aux_, trigger_source_, last_focal_law_acquired_;
   bool lshift_;
   unsigned long index_auxiliar_;
   bool get_data_;
   bool enabled_counter_;
   int id_vch_;

public:
   
   CBufferScan *buffer_data_;
   CBufferScan* buffer_counter_;

   CBufferPA_ScanHWUT (int ch, TFP_VCH_ConfigData *vch=NULL);
   ~CBufferPA_ScanHWUT();
   unsigned long Print(FILE *f);
   unsigned long long GetMemory(void);

   int MemoryResize (void);
   int MemoryFree(void);
   int GetScanLinesTrigger(void) {return n_lines_trg_;}
   int GetScanLinesAuxiliar(void) {return n_lines_aux_;}
   int GetLeftShift(void) {return lshift_;}

   int WriteData (short int *data, unsigned long n_data, unsigned long index_trg, unsigned long index_aux, unsigned long *n_acquisitions_to_end=NULL);
   int WriteCounter(unsigned long data, unsigned long index_trg, unsigned long index_aux, unsigned long* n_acquisitions_to_end = NULL);

   int GetBuffer_LastImageData ( short int *data,
                                 unsigned long size_data,
                                 unsigned long *n_data,
                                 unsigned long *index_trigger,
                                 unsigned long *index_auxiliar);

   int GetBuffer_ImageData (  short int *data,
                              unsigned long size_data,
                              unsigned long *n_data,
                              unsigned long index_trigger,
                              unsigned long index_auxiliar);

   int Release (void);
   void IndexAuxiliar(unsigned long data) {index_auxiliar_ = data;}
   void SelectAcquisitionData_AScan (bool data) {get_data_ = data;}
   bool SelectAcquisitionData_AScan (void) {return get_data_;}
   void Clear (void);
   
   void ResetAcquisitionCounter(void) {acquisition_counter_ = 0; last_focal_law_acquired_ = 0;}

   void IncAcquisitionCounter(int id_fl) {acquisition_counter_++; last_focal_law_acquired_ = id_fl;}
   
   void DecAcquisitionCounter(void) {acquisition_counter_--;}

   unsigned long GetAcquisitionCounter(void) {return acquisition_counter_;}

   int GetLastFocalLawAcquired(void) {return last_focal_law_acquired_;}

   void Enabled_Counter(bool data) { enabled_counter_ = data; }
   bool Enabled_Counter(void) { return enabled_counter_; }
};

void BufferPA_HWUT_Clear(void);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#endif


