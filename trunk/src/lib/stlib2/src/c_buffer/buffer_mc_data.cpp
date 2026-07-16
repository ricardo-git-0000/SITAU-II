// -----------------------------------------------------------------------------
/**
 @file buffer_mc_data.cpp

 @brief Este archivo contiene la implementación de las funciones de acceso a los
 datos de adquisición en modo MC.
 
 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 11/29/10 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <limits>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "buffer_mc_data.h"
#include "buffer_error_code.h"
#include "c_buffer_ptr.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "stlib2.h"
#include "st_get.h"
#include "c_virtual_channel.h"
#include "c_datalogger.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long ST_BufferPrint(FILE *f)
{
unsigned long total_size_mem = 0, size_mem = 0;
int i;

   if (STDL_GetEnabled() == 0) f = NULL;
   if (gb_buffer_hw != NULL) size_mem += gb_buffer_hw->Capacity();
   total_size_mem += size_mem;
   if (f != NULL)
   {
      fprintf (f, "\nSTLIB Library v%s.%s.%s.%s (%s) - Buffer Data Information",
         _VERSION_DLL_MAJOR, _VERSION_DLL_MINOR, _VERSION_DLL_RELEASE, _VERSION_DLL_BUILD, _VERSION_DLL_DATE);
      fprintf (f, "\n________________________________________________________________________");
      fprintf (f, "\n______ STLIB2 Library Memory Size ______________________________________");
      fprintf (f, "\n========================================================================");
      fprintf (f, "\n\n------ GE Acquisition Buffer -------------------------------------------");
      if (size_mem > 1000000)
          fprintf(f, "\nTotal Memory Size (HW Acquisition Buffer): %.2f MBytes", (float)size_mem / 1000000.);
      else if (size_mem > 1000)
          fprintf(f, "\nTotal Memory Size (HW Acquisition Buffer): %.2f KBytes", (float)size_mem / 1000.);
      else
          fprintf(f, "\nTotal Memory Size (HW Acquisition Buffer): %lu Bytes", size_mem);
      fprintf(f, "\n\n------ GE User Data Acquisition Buffer -------------------------------------------");
   }

   total_size_mem += size_mem;
   size_mem = 0;

   for (i = 0; i < gb_virtual_channel.size(); i++)
   {
       if (gb_virtual_channel[i] != NULL && gb_virtual_channel[i]->buffer_mc_hwut_ != NULL)
       {
           total_size_mem += gb_virtual_channel[i]->buffer_mc_hwut_->Print(f);
           total_size_mem += gb_virtual_channel[i]->buffer_pa_hwut_->Print(f);
           total_size_mem += gb_virtual_channel[i]->buffer_tfm_hwut_->Print(f);
       }
   }
   return total_size_mem;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long ST_BufferPrint(void)
{
FILE *f = NULL;
unsigned long size_mem = 0;

   if (STDL_GetEnabled() == 1) f = fopen((gb_st_file_path + "mem_buffer.txt").c_str(), "wt");
   size_mem = ST_BufferPrint(f);
   if (f != NULL) fclose(f);
   return size_mem;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetBuffer_LastMatrixCapture(short int* data, unsigned long size_data, unsigned long* index_trg)
{
    int result = EBUF_NONE;
    int id_vch = 0;
    bool cmd_continue = ST_Acquiring() == 1 && UCIP.pause_after_burst == 1 && UCIP.auto_continue_mode == 1;

    if ((result = ST_AcquiringError()) < 0) return RLOG(result);
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL && gb_virtual_channel[id_vch]->buffer_mc_hwut_ != NULL)
    {
        if ((result = gb_virtual_channel[id_vch]->buffer_mc_hwut_->GetBuffer_LastMCData(data, size_data, NULL, index_trg, NULL)) < 0)
            return RLOG(result);
    }
    if (cmd_continue)
    {
        if (result > 0 && (*index_trg + 1) % UCIP.n_images_burst == 0)
            ST_Continue();
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetBuffer_MatrixCapture(short int* data, unsigned long size_data, unsigned long index_trg)
{
    int result = EBUF_NONE;
    int id_vch = 0;
    bool cmd_continue = ST_Acquiring() == 1 && UCIP.pause_after_burst == 1 && UCIP.auto_continue_mode == 1;

    if ((result = ST_AcquiringError()) < 0) return RLOG(result);
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL && gb_virtual_channel[id_vch]->buffer_mc_hwut_ != NULL)
    {
        if ((result = gb_virtual_channel[id_vch]->buffer_mc_hwut_->GetBuffer_MCData(data, size_data, NULL, index_trg, 1)) < 0)
            return RLOG(result);
    }
    if (cmd_continue)
    {
        if (result > 0 && (index_trg + 1) % UCIP.n_images_burst == 0)
            ST_Continue();
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetBuffer_SingleImage(short int* data, unsigned long size_data, int index_focal_law, unsigned long index_trigger)
{
    int result = EBUF_NONE;
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL && gb_virtual_channel[id_vch]->buffer_mc_hwut_ != NULL)
    {
        if ((result = gb_virtual_channel[id_vch]->buffer_mc_hwut_->GetBuffer_FocalLawData(data, size_data, NULL, index_focal_law, index_trigger, 0)) < 0)
            return RLOG(result);
    }
    return result;
}























