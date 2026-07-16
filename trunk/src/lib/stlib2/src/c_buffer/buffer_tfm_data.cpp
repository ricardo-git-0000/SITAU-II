// -----------------------------------------------------------------------------
/**
 @file buffer_tfm_data.cpp

 @brief Este archivo contiene la implementación de las funciones de acceso a los
 datos de adquisición en modo TFM
 
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

#include "buffer_pa_data.h"
#include "c_log_file.h"
#include "buffer_error_code.h"
#include "c_buffer_ptr.h"
#include "ctimer.h"
#include "st_info.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "c_acq_data.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_reg.h"
#include "st_get.h"
#include "c_virtual_channel.h"
#include "utils.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetBuffer_LastImage(short int *data, unsigned long size_data, unsigned long *n_data, unsigned long* index_trg, unsigned long* index_aux)
{
int result = EBUF_NONE;
int id_vch = 0;
bool cmd_continue = ST_Acquiring() == 1 && UCIP.pause_after_burst == 1 && UCIP.auto_continue_mode == 1;

    if ((result = ST_AcquiringError()) < 0) return RLOG(result);
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (n_data != NULL) *n_data = 0;
    if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL && gb_virtual_channel[id_vch]->buffer_tfm_hwut_ != NULL)
    {
        if ((result = gb_virtual_channel[id_vch]->buffer_tfm_hwut_->GetBuffer_LastImageData(data, size_data, n_data, index_trg, index_aux)) < 0)
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
int ST_TFM_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux)
{
    int result = EBUF_NONE;
    int id_vch = 0;
    bool cmd_continue = ST_Acquiring() == 1 && UCIP.pause_after_burst == 1 && UCIP.auto_continue_mode == 1;

    if ((result = ST_AcquiringError()) < 0) return RLOG(result);
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < gb_virtual_channel.size() && gb_virtual_channel[id_vch] != NULL && gb_virtual_channel[id_vch]->buffer_tfm_hwut_ != NULL)
    {
        if ((result = gb_virtual_channel[id_vch]->buffer_tfm_hwut_->GetBuffer_ImageData(data, size_data, n_data, index_trg, index_aux)) < 0)
            return RLOG(result);
    }
    if (cmd_continue)
    {
        if (result > 0 && (index_trg + 1) % UCIP.n_images_burst == 0)
            ST_Continue();
    }
    return result;
}
