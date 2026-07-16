#ifndef fp_c_buffer_acqH
#define fp_c_buffer_acqH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <vcl.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"

#define N_ACQ_CHANNELS 32

using namespace std;

// _____________________________________________________________________________
// ------ CBufferAcq class
// _____________________________________________________________________________
// =============================================================================
class CBufferAcq
{
private:

   bool auxiliar_encoder_start_;
   bool auxiliar_encoder_wait_first_half_;
   bool end_auxiliar_line_;
   bool end_auxiliar_encoder_;
   bool end_scan_;
   bool app_end_scan_;
   bool log_buffer_counter_;
   FILE *flog_buffer_counter_;
   AnsiString logname_buffer_counter_;

   // Ultrascope Acquisitipon Data
   vector<long> usc_encoder_;

   // Config Parameters
   int n_ch_;
      vector<int> n_ascan_;
   vector<int> acq_mode_;

   int trigger_source_;
   int encoder_aux_enabled_;
   unsigned long n_lines_trg_, n_lines_aux_;

   int n_lines_trg_lost_;
   unsigned long index_trg_change_direction_, offset_trg_one_direction_;
   int index_sw_inc_;
   bool encoder_trg_change_direction_;

   vector<vector<unsigned long> > last_acq_counter_;
   vector<vector<unsigned long> > index_trg_, index_aux_, index_trg_last_, index_aux_last_;
   vector<int> offset_trg_;

   int out_of_end_line_;
   int out_of_start_line_;

protected:

public:

   CBufferAcq();

   ~CBufferAcq();

   void DBG_FileOpen (void);

   void DBG_FileClose (void);

   int UpdateConfig (void);

   int GetNoEncoder_AcqIndex (unsigned long acq_counter, int id_vch, int id_fl, unsigned long* index_trigger, unsigned long* index_auxiliar);

   int GetChannelsNumber(void) {return n_ch_;}

   int GetScanLinesTrigger(void) {return n_lines_trg_;}

   int GetScanLinesAuxiliar(void) {return n_lines_aux_;}

   int GetTriggerSource(void) {return trigger_source_;}

   int GetAScanNumber (int id_vch);

   bool GetEndAuxiliarLine(void) {return end_auxiliar_line_;}

   int GetOutOfEndLine(void) {return out_of_end_line_;}

   int GetOutOfStartLine(void) {return out_of_start_line_;}

   bool GetEndScan(void) {return end_scan_;}

   bool GetAppEndScan(void) {return app_end_scan_;}

   void ResetEndScan(void) {end_scan_ = false;}

   void ResetAppEndScan(void) {app_end_scan_ = false;}

   void ResetLastAcqCounter(void);

   void SelectLogData_GetBufferCounter (bool data) {log_buffer_counter_ = data;}

   bool SelectLogData_GetBufferCounter (void) {return log_buffer_counter_;}

   int SetOffsetTriggerLine(int data, int id_vch);

   int ResetOffsetTriggerLine(void);
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern CBufferAcq *gb_fp_buffer_acq;
extern CBufferAcq* gb_pa_buffer_acq;
extern CBufferAcq* gb_tfm_buffer_acq;

#endif
