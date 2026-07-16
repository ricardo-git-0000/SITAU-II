// -----------------------------------------------------------------------------
/**
 @file c_virtual_channel.cpp

 @brief Este archivo contiene la implementación de las funciones de conversión de datos de la curva TGC.

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

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_virtual_channel.h"
#include "c_vch_acq_file.h"
#include "c_vch_tgc.h"
#include "st_error_code.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "protocol_uci.h"
#include "st_reg.h"
#include "st_range.h"
#include "stlib2.h"
#include "stlib_dev.h"
//#include "buffer_data.h"
//#include "stload.h"
//#include "c_buffer_scan_hw_ut.h"
//#include "c_buffer_scan_hw_global.h"
#include "sitau_hw.h"
#include "fileformat.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

vector<vector< short int> > gb_bin_file_data;
vector<vector< short int> > gb_bin_file_pci_data;
vector<vector< short int> > gb_bin_file_pci_image;

// _____________________________________________________________________________
// ------ CVirtualChannel class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SaveFileConfig(fstream& f)
{
    int data_int, result;
    float data_float;
    vector<float> x_entry_point;

    if ((data_int = GET_AScanDataNumber()) < 0) return RLOG(data_int);
    if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

    if ((data_int = GET_MC_FocalLaw_AScanNumber(0)) < 0) return RLOG(data_int);
    if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

    if ((data_int = GET_FocalLawNumber()) < 0) return RLOG(data_int);
    if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

    if ((data_float = GET_AcqStartDistance()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_float = GET_AcqStartTime()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_float = GET_AcqEndDistance()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_float = GET_AcqEndTime()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_float = GET_SamplingFrequency()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_int = GET_ReductionFactor()) < 0) return RLOG(data_int);
    if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

    if ((data_float = GET_Gain()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

    if ((data_float = GET_PRFTime()) < 0) return RLOG(Round32(data_float));
    if ((result = WriteField(f, data_float * (float)1000.)) < 0) return RLOG(result);

    if ((result = GET_MC_ImageEntryPoint(x_entry_point)) < 0) return RLOG(result);
    if ((result = WriteField(f, x_entry_point)) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SaveFileAcquisition(fstream &f)
{
    int data_int, result;
    CPtr* c_ptr = NULL;
    unsigned long data_size = sizeof(short);

    if ((data_int = SaveFileConfig(f)) < 0) return RLOG(data_int);

    // (MC) Cubo de datos crudos
    if (buffer_mc_hwut_->buffer_data_ != NULL)
    {
        if ((c_ptr = buffer_mc_hwut_->buffer_data_->GetPtr(0, 0, 0, false)) != NULL)
        {
            if ((result = WriteField(f, c_ptr->n_data)) < 0) return RLOG(result);
            if ((result = buffer_mc_hwut_->buffer_data_->WriteFile(f, cfg_.pa_probe->pin_to_channel_)) < 0) return RLOG(result);
        }
    }

    // Acquisition Counter
    if ((c_ptr = buffer_mc_hwut_->buffer_counter_->GetPtr(0, 0, 0, false)) != NULL)
    {
        if ((result = WriteField(f, c_ptr->n_data)) < 0) return RLOG(result);
        if (c_ptr->n_data > 0)
        {
            if ((result = WriteField(f, (unsigned long*)c_ptr->data, c_ptr->n_data)) < 0) return RLOG(result);
        }
    }
    else
    {
        if ((result = WriteField(f, (uint64)0)) < 0) return RLOG(result);
    }
    return EVCH_NONE;
}






