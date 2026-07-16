// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_datalogger.h"
#include "c_xml_sitau_hw.h"
#include "st_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

C_DataLogger* gb_datalogger = NULL;

// _____________________________________________________________________________
// ------ C_DataLogger class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
C_DataLogger::C_DataLogger()
{
    Update();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int C_DataLogger::Update(void)
{
    int result = ES2_NONE;

    if (gb_xml_sitau_hw != NULL && (result = gb_xml_sitau_hw->Update()) >= 0)
    {
        enabled_ = gb_xml_sitau_hw->data_.data_logger.enabled != 0;
        pcie_user_buffer_ = gb_xml_sitau_hw->data_.data_logger.pcie_user_buffer;
        config_ = gb_xml_sitau_hw->data_.data_logger.config;
        pa_beamformer_ = gb_xml_sitau_hw->data_.data_logger.pa_beamformer;
        tfm_beamformer_ = gb_xml_sitau_hw->data_.data_logger.tfm_beamformer;
    }
    else
    {
        enabled_ = true;
        config_ = 1;
        pcie_user_buffer_ = 0;
        pa_beamformer_ = 0;
        tfm_beamformer_ = 0;
    }
    return result;
}





// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_Create(void)
{
    if (gb_datalogger != NULL) delete gb_datalogger;
    gb_datalogger = new C_DataLogger();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_Delete(void)
{
    if (gb_datalogger != NULL) delete gb_datalogger;
    gb_datalogger = NULL;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_GetEnabled(void)
{
    if (gb_datalogger == NULL) return 1;
    return gb_datalogger->Enabled();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_SetEnabled(int data)
{
    if (gb_datalogger != NULL) gb_datalogger->Enabled(data != 0);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_GetConfig(void)
{
    if (gb_datalogger == NULL) return 1;
    return gb_datalogger->Enabled() ? gb_datalogger->Config() : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_SetConfig(int data)
{
    if (gb_datalogger != NULL) gb_datalogger->Config(data);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_GetPABeamformer(void)
{
    if (gb_datalogger == NULL) return 0;
    return gb_datalogger->Enabled() ? gb_datalogger->PABeamformer() : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_SetPABeamformer(int data)
{
    if (gb_datalogger != NULL) gb_datalogger->PABeamformer(data);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_GetTFMBeamformer(void)
{
    if (gb_datalogger == NULL) return 0;
    return gb_datalogger->Enabled() ? gb_datalogger->TFMBeamformer() : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STDL_SetTFMBeamformer(int data)
{
    if (gb_datalogger != NULL) gb_datalogger->TFMBeamformer(data);
    return ES2_NONE;
}