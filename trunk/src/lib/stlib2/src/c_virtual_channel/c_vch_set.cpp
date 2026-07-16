// -----------------------------------------------------------------------------
/**
 @file c_vch_set.cpp

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

#include "c_vch_set.h"
#include "c_virtual_channel.h"
#include "vch_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "c_pa_system.h"
#include "st_calc.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CVCH_Set class Methods
// _____________________________________________________________________________
// =============================================================================
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//CVCH_Set::CVCH_Set() : public CVCH_Load()
//{
//}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//CVCH_Set::~CVCH_Set()
//{
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_Name(char *name)
{
   if (name == NULL) return ELOG(charEVCH_Parameter, EVCH_Parameter);
   cfg_.name = AnsiString(name);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_ReceiverMode (int data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_ReceiverMode(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_Pulser (int n_pulses, float pulse_width)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_Pulser(n_pulses, pulse_width)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PulseWidth (float pulse_width)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PulseWidth(pulse_width)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PulsesNumber (int n_pulses)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PulsesNumber(n_pulses)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_PRFEnabled(int data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PRFEnabled(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_SamplingFrequency (int data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_SamplingFrequency(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_Attenuator (unsigned short data)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_Attenuator(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_PRFTime(float data)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PRFTime(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_MaterialVelocity (float data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_MaterialVelocity (data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_AcqTime (float acq_start_time, float acq_end_time)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_AcqTime (acq_start_time, acq_end_time)) < 0)
        return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CVirtualChannel::SET_AcqDistance (float acq_start_distance, float acq_end_distance, int pulse_echo)
{
int result, acquiring = ST_Acquiring();

try
{
    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_AcqDistance (acq_start_distance, acq_end_distance, pulse_echo)) < 0) return RLOG(result);
    if ((result = RELOAD_FocalLaw()) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
}
__finally
{
   if (acquiring == 1) ST_Start();
}
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**
Envía el mensaje de configuración de los registros de una BASE.

@return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_Gain (float gain_db)
{
int result;

    if ((result = LOAD_Gain(gain_db)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**
Envía el mensaje de configuración de los registros de una BASE.

@return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_AFEGain(int attn, int lna, int pga)
{
    int result;
    //int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_DigitalTGC_Attenuator(attn)) < 0) return RLOG(result);
    if ((result = LOAD_PGA_GAIN(pga)) < 0) return RLOG(result);
    if ((result = LOAD_LNA_GAIN(lna)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_SignalTest (int data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_SignalTest(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PulserEnabled (bool data)
{
int result;
//int acquiring = ST_Acquiring();

    //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    LOAD_PulserEnabled(data);
    if ((result = Set()) < 0) return RLOG(result);
    //if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PWICFocalLaw_Angles(float *angle, int n_angles)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PWICFocalLaw_Angles(angle, n_angles)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_TFM_PWICFocalLaw_Angles(float* angle, int n_angles)
{
    int result;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TFM_LOAD_PWI_Forward_Angles(angle, n_angles)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PWICFocalLaw(float start_angle, float step_angle, float end_angle)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PWICFocalLaw(start_angle, step_angle, end_angle)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_TFM_PWIForwardConfig(float start_angle, float step_angle, float end_angle)
{
    int result;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TFM_LOAD_PWI_Forward(start_angle, step_angle, end_angle)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_FMCFocalLaw(void)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_FMCFocalLaw()) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::DELETE_FocalLaws(void)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    cfg_.fp_focal_law.clear();
   //if ((result = LOAD_FMCFocalLaw()) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   if (acquiring == 1) ST_Start();
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CVirtualChannel::RESET_AcquisitionCounter(void)
{
   if (buffer_mc_hwut_ != NULL) buffer_mc_hwut_->ResetAcquisitionCounter();
   if (buffer_pa_hwut_ != NULL) buffer_pa_hwut_->ResetAcquisitionCounter();
   if (buffer_tfm_hwut_ != NULL) buffer_tfm_hwut_->ResetAcquisitionCounter();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_PAConfig (unsigned short n_element, float pitch, float frequency)
{
int result;
int acquiring = ST_Acquiring();

try
{
    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = LOAD_PAConfig(n_element, pitch, frequency)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
}
__finally
{
    if (acquiring == 1) ST_Start();
}
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_RFFilterEnabled(bool data)
{
   int result;

   if ((result = LOAD_RFFilterEnabled(data)) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_RFFilter(float low_frequency, float high_frequency)
{
   int result;

   if ((result = LOAD_RFFilter(low_frequency, high_frequency)) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_Processing(int processing_type, int n_signals, int wait_prf)
{
   int result;

   if ((result = LOAD_Processing(processing_type, n_signals, wait_prf)) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::FP_SET_EmissionFocalLawFile(char* file)
{
   int result;
   int acquiring = ST_Acquiring();

   if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
   if ((result = FP_LOAD_EmissionFocalLawFile(file)) < 0) return RLOG(result);
   if ((result = Set()) < 0) return RLOG(result);
   if (acquiring == 1) ST_Start();
   return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_AcquisitionType(int data)
{ 
int result;

    cfg_.acquisition_type = data;
    if ((result = LOAD_AcquisitionType(data)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::SET_SignalMode(int data)
{
    int result;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    cfg_.signal_mode = data;
    update_register_ = true;
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CVirtualChannel::TFM_SET_Config(vector<int> &rec_elements, vector<int> &emi_elements, float start_x, float end_x, int n_lines, float angle)
//{
//    int result;
//    int acquiring = ST_Acquiring();
//
//    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
//    if ((result = TFM_LOAD_FMC_Config(rec_elements, emi_elements, start_x, end_x, n_lines, angle)) < 0) return RLOG(result);
//    if ((result = Set()) < 0) return RLOG(result);
//    if (acquiring == 1) ST_Start();
//    return EVCH_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_SET_BackwardConfig(float start_x, float end_x, int n_lines, float angle)
{
    int result;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TFM_LOAD_Backward(start_x, end_x, n_lines, angle)) < 0) return RLOG(result);
    switch (cfg_.tfm.focal_laws_type)
    {
    case VCH_FP_FL_TYPE_FMC:
        if ((result = TFM_LOAD_FMC_Forward(cfg_.tfm.emi_start_element, cfg_.tfm.emi_end_element, cfg_.tfm.emi_step_element)) < 0) return RLOG(result);
        break;
    case VCH_FP_FL_TYPE_PWI:
        if ((result = TFM_LOAD_PWI_Forward(cfg_.tfm.pwic_start_angle, cfg_.tfm.pwic_step_angle, cfg_.tfm.pwic_end_angle)) < 0) return RLOG(result);
        break;
    }
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_SET_FMC_ForwardConfig(vector<int>& elements)
{
    int result;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TFM_LOAD_FMC_Forward(elements)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CVirtualChannel::TFM_SET_FMC_ForwardConfig(int start_element, int end_element, int step_element)
{
    int result, id_vch = 0;
    int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TFM_LOAD_FMC_Forward(start_element, end_element, step_element)) < 0) return RLOG(result);
    if ((result = Set()) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return EVCH_NONE;
}














