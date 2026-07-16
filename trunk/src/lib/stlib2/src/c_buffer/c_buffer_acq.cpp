// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_buffer_acq.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_reg.h"
#include "buffer_error_code.h"
#include "c_log_file.h"
#include "ctimer.h"
#include "st_get.h"
#include "utils.h"
#include "c_acq_data.h"
#include "c_buffer_hw.h"
#include "st_vch_get.h"

using namespace std;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CBufferAcq *gb_fp_buffer_acq = NULL;
CBufferAcq* gb_pa_buffer_acq = NULL;
CBufferAcq* gb_tfm_buffer_acq = NULL;

// _____________________________________________________________________________
// ------ CBufferAcq class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CBufferAcq::CBufferAcq()
{
   auxiliar_encoder_start_ = false;
   auxiliar_encoder_wait_first_half_ = true;
   end_auxiliar_line_ = false;
   end_auxiliar_encoder_ = false;
   end_scan_ = false;
   app_end_scan_ = false;
   log_buffer_counter_ = false;
   flog_buffer_counter_ = NULL;
   logname_buffer_counter_ = "LogGetBufferCounter.txt";
   last_acq_counter_.resize(N_ACQ_CHANNELS);
   index_trg_.resize(N_ACQ_CHANNELS);
   offset_trg_.resize(N_ACQ_CHANNELS);
   index_aux_.resize(N_ACQ_CHANNELS);
   index_trg_last_.resize(N_ACQ_CHANNELS);
   index_aux_last_.resize(N_ACQ_CHANNELS);
   encoder_aux_enabled_ = 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CBufferAcq::~CBufferAcq()
{
   DBG_FileClose();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferAcq::UpdateConfig (void)
{
int result, i, id_vch, n_fl;

   auxiliar_encoder_start_ = false;
   auxiliar_encoder_wait_first_half_ = true;
   n_ch_ = ST_GetVirtualChannelsNumber();

   if (n_ch_ != last_acq_counter_.size()) last_acq_counter_.resize(n_ch_);
   if (n_ch_ != index_trg_.size()) index_trg_.resize(n_ch_);
   if (n_ch_ != index_trg_last_.size()) index_trg_.resize(n_ch_);
   if (n_ch_ != index_aux_.size()) index_trg_.resize(n_ch_);
   if (n_ch_ != index_aux_last_.size()) index_trg_.resize(n_ch_);
   for (id_vch=0; id_vch<n_ch_; id_vch++)
   {
      if ((n_fl = ST_GetFocalLawNumber(id_vch)) < 0) return RLOG(n_fl);
      if (n_fl > 0) 
      {
         if (last_acq_counter_[id_vch].size() != n_fl) last_acq_counter_[id_vch].resize(n_fl);
         if (index_trg_[id_vch].size() != n_fl) index_trg_[id_vch].resize(n_fl);
         if (index_trg_last_[id_vch].size() != n_fl) index_trg_last_[id_vch].resize(n_fl);
         if (index_aux_[id_vch].size() != n_fl) index_aux_[id_vch].resize(n_fl);
         if (index_aux_last_[id_vch].size() != n_fl) index_aux_last_[id_vch].resize(n_fl);
         for (i=0; i<last_acq_counter_[id_vch].size(); i++) last_acq_counter_[id_vch][i] = 0;
         for (i=0; i<index_trg_[id_vch].size(); i++) index_trg_[id_vch][i] = -1;
         for (i=0; i<index_trg_last_[id_vch].size(); i++) index_trg_last_[id_vch][i] = 0;
         for (i=0; i<index_aux_[id_vch].size(); i++) index_aux_[id_vch][i] = 0;
         for (i=0; i<index_aux_last_[id_vch].size(); i++) index_aux_last_[id_vch][i] = 0;
      }
      else return ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency);
   }

   if (n_ch_ != n_ascan_.size()) n_ascan_.resize(n_ch_);
   if (n_ch_ != acq_mode_.size()) acq_mode_.resize(n_ch_);

   trigger_source_ = ST_GetTriggerSource();

	if ((result = ST_GetScanLinesTrigger()) < 0) return RLOG(result);
	n_lines_trg_ = result;

   if ((result = ST_GetScanLinesAuxiliar()) < 0) return RLOG(result);
	n_lines_aux_ = result;

   n_lines_trg_lost_ = 0;
   index_sw_inc_ = -1;
   index_trg_change_direction_ = 0;
   offset_trg_one_direction_ = 0;
   out_of_end_line_ = 0;
   out_of_start_line_ = 0;
   encoder_trg_change_direction_ = false;
   end_scan_ = false;
   app_end_scan_ = false;
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferAcq::GetNoEncoder_AcqIndex (unsigned long acq_counter, int id_vch, int id_fl, unsigned long *index_trigger, unsigned long *index_auxiliar)
{
int result = EBUF_NONE;
int diff_acq_counter, diff_encoder_trg_index = 0;
long diff_encoder_aux = 0, diff_encoder_trg = 0;

   if (trigger_source_ == TRG_SCAN_ENC) return EBUF_NONE;
   if (id_vch >= last_acq_counter_.size()) return ELOG(charEBUF_IdScan, EBUF_IdScan);
   if (id_fl >= last_acq_counter_[id_vch].size()) return ELOG(charEBUF_DataInconsistency, EBUF_DataInconsistency);

   if (id_vch >= index_aux_last_.size()) return ELOG(charEBUF_IdScan, EBUF_IdScan);

   if (last_acq_counter_[id_vch][id_fl] != 0 && acq_counter != 0)
   {
      if (acq_counter == last_acq_counter_[id_vch][id_fl]) return EBUF_NONE;
      diff_acq_counter = abs((long)(acq_counter - last_acq_counter_[id_vch][id_fl]));
   }

   last_acq_counter_[id_vch][id_fl] = acq_counter;
   if (log_buffer_counter_ && flog_buffer_counter_ != NULL)
	   fprintf(flog_buffer_counter_,"%dl ",acq_counter);

   //Calculo el índice del Ascan correspondiente
   if (index_sw_inc_ < 0)
   {
      index_sw_inc_ = 1;
      index_trg_last_[id_vch][id_fl] = index_trg_[id_vch][id_fl] = 0;
      index_aux_last_[id_vch][id_fl] = index_aux_[id_vch][id_fl] = 0;
   }
   else if (index_sw_inc_)
   {
      index_trg_[id_vch][id_fl]++;
      if (index_trg_[id_vch][id_fl] >= n_lines_trg_)
      {
         end_auxiliar_line_ = true;
         end_auxiliar_encoder_ = true;
         index_trg_[id_vch][id_fl] = 0;
         if (n_lines_aux_> 1) index_sw_inc_ = 0;

         if (n_lines_aux_ > 1 && !encoder_aux_enabled_)
         {
            index_aux_[id_vch][id_fl]++;
            if (index_aux_[id_vch][id_fl] >= n_lines_aux_)
            {
               index_aux_[id_vch][id_fl] = n_lines_aux_ - 1;
               end_scan_ = true;
               app_end_scan_ = true;
            }
         }
         else
         {
            end_scan_ = true;
            app_end_scan_ = true;
         }
      }      
   }
   else
   {
      if (index_trg_[id_vch][id_fl] > 0 )
      {
         index_trg_[id_vch][id_fl]--;
      }

      if (index_trg_[id_vch][id_fl] <= 0)
      {
         end_auxiliar_line_ = true;
         end_auxiliar_encoder_ = true;
         index_trg_[id_vch][id_fl] = 0;
         if (n_lines_aux_> 1) index_sw_inc_ = 1;

         if (n_lines_aux_ > 1 && !encoder_aux_enabled_)
         {
            index_aux_[id_vch][id_fl]++;
            if (index_aux_[id_vch][id_fl] >= n_lines_aux_)
            {
               index_aux_[id_vch][id_fl] = n_lines_aux_ - 1;
               end_scan_ = true;
               app_end_scan_ = true;
            }
         }
         else
         {
            end_scan_ = true;
            app_end_scan_ = true;
         }
      }      
   }
   if (index_aux_[id_vch][id_fl] != index_aux_last_[id_vch][id_fl])
      end_auxiliar_line_ = true;
   index_aux_last_[id_vch][id_fl] = index_aux_[id_vch][id_fl];
   result = 1;

   if (index_trigger != NULL)
   {
      *index_trigger = index_trg_[id_vch][id_fl] + offset_trg_[id_vch];
      if (*index_trigger < 0) *index_trigger = 0;
      if (*index_trigger >= n_lines_trg_) *index_trigger = n_lines_trg_ - 1;
   }
   if (index_auxiliar != NULL) *index_auxiliar = index_aux_[id_vch][id_fl];
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferAcq::GetAScanNumber (int id_vch)
{
   if (id_vch >= n_ascan_.size())
      return ELOG(charEBUF_IdScan, EBUF_IdScan);
   return n_ascan_[id_vch];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferAcq::DBG_FileOpen(void)
{
   DBG_FileClose();
   if (log_buffer_counter_)
	   flog_buffer_counter_ = fopen(logname_buffer_counter_.c_str(),"w");
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferAcq::DBG_FileClose(void)
{
	if (flog_buffer_counter_ != NULL) fclose(flog_buffer_counter_);
   flog_buffer_counter_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CBufferAcq::ResetLastAcqCounter(void)
{
int i, j;

   for (i=0; i<last_acq_counter_.size(); i++) 
   {
      for (j=0; j<last_acq_counter_[i].size(); j++) 
         last_acq_counter_[i][j] = 0;
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferAcq::SetOffsetTriggerLine(int data, int id_vch)
{
   if (id_vch < offset_trg_.size()) offset_trg_[id_vch] = data;
   else return ELOG(charEBUF_IdScan, EBUF_IdScan);
   return EBUF_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CBufferAcq::ResetOffsetTriggerLine(void)
{
int i;

   for (i=0; i<offset_trg_.size(); i++) offset_trg_[i] = 0;
   return EBUF_NONE;
}
