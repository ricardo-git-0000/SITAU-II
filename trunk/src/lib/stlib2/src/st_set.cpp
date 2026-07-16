// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <float.h>
#if defined(_MSC_VER)
   #include <sstream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <fstream.h>
   #include <sstream.h>
#endif
#include <limits>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "st_reg.h"
#if defined(_GETHERNET)
   #include "protocol_uci.h"
#endif
#include "st_set.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_load.h"
#include "st_calc.h"
#include "st_get.h"
#include "st_info.h"

#include "st_error_code.h"
#include "math.h"
#include "c_log_file.h"
#include "utils.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"
#include "c_acq_data.h"
#include "c_virtual_channel.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "fileformat.h"
#include "st_vch_load.h"
#include "st_vch_set.h"
#include "st_vch_get.h"
#include "st_vch_tgc.h"
#include "sitau_hw.h"
#include "st_vch_config.h"
#include "c_datalogger.h"
#include "st_thread_status.h"

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S -----------------
// ======================================================================

/* ______________________________________________________________________
   ------ F U N C I O N E S   E X P O R T A D A S -----------------------
   ====================================================================== */

/* ______________________________________________________________________

   ------ F U N C I O N E S   E X P O R T A D A S -----------------------
   ====================================================================== */

/* ----------------------------------------------------------------------

   USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_SetPulser (int n_pulses, float pulse_width)
{
int result, id_virtual_channel = 0;

	if ((result = ST_VCH_SET_Pulser(n_pulses, pulse_width, id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
/* ----------------------------------------------------------------------

   USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_SetPulseWidth (float pulse_width)
{
int result, id_virtual_channel = 0;

	if ((result = ST_VCH_SET_PulseWidth(pulse_width, id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
/* ----------------------------------------------------------------------

   USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_SetPulsesNumber (int n_pulses)
{
int result, id_virtual_channel = 0;

	if ((result = ST_VCH_SET_PulsesNumber(n_pulses, id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
/* ----------------------------------------------------------------------

	USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_SetPulserEnable (int data)
{
int result, id_virtual_channel = 0;

	if ((result = ST_VCH_SET_PulserEnabled((data == 1), id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
/* ----------------------------------------------------------------------

   USC_SetMeasure
   ==============
Descripcion:
   ---------------------------------------------------------------------- */
int ST_SetSignalTest (int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_SignalTest(data, id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
/* ----------------------------------------------------------------------

   ST_SetGain
   ==============
Descripcion: Configura el valor de la ganancia.
ATT: 0, 6, 12, 18, 24, 30, 36, 42
LNA: 12, 18, 24
PGA: 24, 30

Gain:   ATT: -42, -36, -30, -24, -18, -12, -6, 0
        LNA: 12, 18, 24, 
        PGA: 24, 30



arámetros
   	data (dB)	(FLOAT)
	Ganancia. [-20 dB .. 100 dB]

Devuelve (INT)
   (Ver códigos de error)

Dependencias
   Para poder ejecutarla es imprescindible que se hayan definido la frecuencia de muestreo y el tiempo de adquisición.

//
   ---------------------------------------------------------------------- */
int ST_SetGain (float data)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if ((result = gb_virtual_channel[id_vch]->SET_Gain(data)) < 0) return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAFEGain(int attn, int lna, int pga)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if ((result = gb_virtual_channel[id_vch]->SET_AFEGain(attn, lna, pga)) < 0) return RLOG(result);
    return result;
}

// _____________________________________________________________________________
// ------ TGC Functions
// start =======================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetDigitalTGC_Attenuator(int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_DigitalTGC_Attenuator(data, id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetLog2_Average(int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_LOG2_AVERAGE(data, id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetLNAGain(int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_LNA_GAIN(data, id_virtual_channel)) < 0) return result;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPGAGain(int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_PGA_GAIN(data, id_virtual_channel)) < 0) return result;
   return result;
}
/*-----------------------------------------------------------------------------

   ST_SetTGCEnable
   ==============

Descripción:
   Función para la habilitación de la curva CAT

Parámetros
   data    (*USHORT)
       0 -> Curva CAT deshabilitada.
      1 -> Curva CAT habilitada.

Devuelve (INT)
   (Ver códigos de error)
-----------------------------------------------------------------------------*/
int ST_SetTGCEnable (int data)
{
int result = ES2_NONE;
int id_virtual_channel = 0;

   if ((result = ST_VCH_SET_TGC_Enabled(data, id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetTGCType (int data)
{
int result = ES2_NONE;
int id_virtual_channel = 0;

   if ((result = ST_VCH_SET_TGC_Enabled(data, id_virtual_channel)) < 0) return RLOG(result);
   //{
   //   if (data == 1)
   //   {
   //      if ((result = ST_LoadTGCEnable(0, id_virtual_channel)) < 0)
   //         return RLOG(result);
   //   }
   //   else
   //   {
   //      if ((result = ST_LoadTGCEnable(1, id_virtual_channel)) < 0)
   //         return RLOG(result);
   //   }
   //}
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetTGCAttenuationCoefficient (float attn_coefficient)
{
int result = ES2_NONE;
int id_virtual_channel = 0;
//int acquiring = ST_Acquiring();

   if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
   if ((result = ST_VCH_SET_TGC_AttenuationCoefficient (attn_coefficient, id_virtual_channel)) < 0)
      return RLOG(result);

   //if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
   if ((result = ST_PRG()) < 0) return RLOG(result);
   //if (acquiring == 1) ST_Start();
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetTGCConfigType (int data)
{
   return ES2_NONE;
}

// _____________________________________________________________________________
// ------ TGC Functions
// end =========================================================================

//------------------------------------------------------------------------------
int ST_SetTriggerSource (int data)
{
    if (data != TRG_SCAN_PRF && data != TRG_SCAN_EXT) return ELOG(charES2_RANGO, ES2_RANGO);
    if (UCIP.trigger_source != data)
    {
        UCIP.trigger_source = data;
        if (UCIP.status_pcie == 0 && UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS && UCIP.trigger_source == TRG_SCAN_PRF)
            UCIP.pause_after_burst = 1;
        else UCIP.pause_after_burst = UCIP.user_pause_after_burst;
    }
    return ES2_NONE;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ST_SetReceiverMode  (int data)
{
int result;

   if ((result = ST_VCH_SET_ReceiverMode(data)) < 0) return RLOG(result);
   return ES2_NONE;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ST_ResetTimeStamp (void)
{
//int s=0;
//int result;
//int acquiring = ST_Acquiring();
//
//    if (acquiring == 1) ST_Stop();
//   if ((result = ST_PRG()) < 0) return RLOG(result);
//    if (acquiring == 1) ST_Start();
   return ES2_NONE;
}
//------------------------------------------------------------------------------
// Resetea el valor de los coeficientes de apodización extraídos mediante
// calibración de la sensibilidad. Los inicializa a 1.
//------------------------------------------------------------------------------
int ST_ResetElementApodizationCoeff(int set_hw)
{
int result, n_channel;

   if ((n_channel = ST_VCH_GET_ChannelsNumber()) < 0) return RLOG(n_channel);
   if (set_hw == 1)
   {
      if ((result = ST_PRG()) < 0) return RLOG(result);
   }
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**
Periodo de Muestreo = data * 25 ns.
*/
//------------------------------------------------------------------------------
int ST_SetSamplingFrequency (int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_SamplingFrequency(data, id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPRF (float data)
{
int result, id_virtual_channel = 0;

   if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
   if ((result = ST_LoadPRF (data, id_virtual_channel)) < 0) return RLOG(result);
   if ((result = ST_SetVirtualChannel()) < 0) return RLOG(result);
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPRFBurst(float data)
{
    int result, id_virtual_channel = 0;

    if (id_virtual_channel < 0) id_virtual_channel = gb_selected_virtual_channel;
    if ((result = ST_LoadPRFBurst(data, id_virtual_channel)) < 0) return RLOG(result);
    if ((result = ST_SetVirtualChannel()) < 0) return RLOG(result);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_SetTimePRFTrigger (unsigned long data)
//{
//   UCIP.trigger_prf_time = data;
//	return ES2_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAuxiliarEncoderEnabled(int data)
{
    //if (gb_buffer_acq != NULL) gb_buffer_acq->SetAuxiliarEncoderEnabled(data);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPRFTrigger(float prf_hz)
{
    UCIP.trigger_prf_time = Round32((float)1000 / prf_hz);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAcqDistance (float start_distance, float end_distance, int pulse_echo)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if ((result = gb_virtual_channel[id_vch]->SET_AcqDistance(start_distance, end_distance, pulse_echo)) < 0) return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAcqTime (float start_time, float end_time)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_AcqTime(start_time, end_time, id_virtual_channel)) < 0)
      return RLOG(result);
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetMaterialVelocity (float data)
{
int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->SET_MaterialVelocity(data)) < 0) return RLOG(result);
    return result;
}
//------------------------------------------------------------------------------
int ST_SetPulseAmplitudeHW (unsigned char data)
{
int result;
int acquiring = ST_Acquiring();

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TCPSend_MSG_PulseAmplitude(data)) < 0) return RLOG(result);
    if (acquiring == 1) ST_Start();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPulseAmplitude (int data)
{
int result;
unsigned char hw_value;

	if (data < gb_high_voltage_to_da.size()) hw_value = gb_high_voltage_to_da[data];
    else
    {
        hw_value = gb_high_voltage_to_da.back();
        data = gb_high_voltage_to_da.size();
    }
    if ((result = ST_SetPulseAmplitudeHW (hw_value)) < 0) return RLOG(result);    
    stVAR.high_voltage = data;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetDisabledChannel (int disabled, int channel)
{
int result;

	if ((result = ST_LoadDisabledChannel(disabled, channel)) < 0)
      return RLOG(result);
	if ((result = ST_PRG()) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAPULVersion(int data)
{
int n_bases;

   if (!gb_acquisition_process_stoped) return ELOG(charES2_SCAN,ES2_SCAN);
   if ((n_bases = STHW_GetHWBASENumber()) <= 0) return ELOG(charES2_BASE_Number, ES2_BASE_Number);
   return RLOG(ST_PRG());
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAttenuator(int data)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_Attenuator(data, id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAcqCounterZero (void)
{
#if defined(_GETHERNET)
   TCPSend_MSG_RESET_ACQ_COUNTER();
#endif
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_SetEnabledConfigFileText(int data)
{
    STDL_SetEnabled(data);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetSelectLogData_HWCircularBuffer (bool data)
{
   if (gb_buffer_hw != NULL) gb_buffer_hw->SelectLogData_HWCircularBuffer(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetSelectLogData_AcqBufferCounter (bool data)
{
   if (gb_buffer_hw != NULL) gb_buffer_hw->SelectLogData_AcqBufferCounter(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetSelectLogData_AcqBufferSize (bool data)
{
   if (gb_buffer_hw != NULL) gb_buffer_hw->SelectLogData_AcqBufferSize(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetSelectLogData_AcqBufferNumber (bool data)
{
   if (gb_buffer_hw != NULL) gb_buffer_hw->SelectLogData_AcqBufferNumber(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_FP_SetSelectLogData_GetBufferCounter (bool data)
{
   if (gb_fp_buffer_acq != NULL) gb_fp_buffer_acq->SelectLogData_GetBufferCounter(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_SetSelectLogData_GetBufferCounter(bool data)
{
   if (gb_pa_buffer_acq != NULL) gb_pa_buffer_acq->SelectLogData_GetBufferCounter(data);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_SetFileLogProtocol (int data)
{
   if (data == 0) gb_log_protocol_enabled = 0;
   else gb_log_protocol_enabled = 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_SetFileLogHWRegisters (int data)
{
   if (data == 0) gb_log_config_enabled = 0;
   else gb_log_config_enabled = 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_SetFileLogSystemInfo (int data)
{
   if (data == 0) gb_log_system_info_enabled = 0;
   else gb_log_system_info_enabled = 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_EnabledLoadUCIFirmware (int data)
{
   if (data >= 0 && data <= 2) gb_load_uci_firmware = data;
   else gb_load_uci_firmware = 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetTemporaryFilesPath (char *path)
{
char full_path[255];
int result = ES2_NONE;

   if ((result = FS_DirectoryExists(path, full_path)) < 0)
      return RLOG(result);
   gb_st_file_path = AnsiString(full_path);
   gb_log_file_path = AnsiString(full_path);
   //gb_base_file_path = AnsiString(full_path);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetVirtualChannelName(char *data, int id_virtual_channel)
{
int result;

   if ((result = ST_VCH_SET_Name(data, id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetNoHardware (int data)
{
   if (data == 0) gb_no_hw = false;
   else gb_no_hw = true;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_SetPlaneWaveImaging_Angles_deg(float *angle, int n_angles)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_PlaneWaveImagingCompounding_Angles(angle, n_angles, id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_SetPlaneWaveImaging_Angles_rad(float* angle, int n_angles)
{
    int i, result, id_virtual_channel = 0;
    vector<float> angles_deg;

    for (i = 0; i < n_angles; i++) angles_deg.push_back(angle[i] * (180. / M_PI));
    if ((result = ST_VCH_SET_PlaneWaveImagingCompounding_Angles(&angles_deg[0], angles_deg.size(), id_virtual_channel)) < 0) return RLOG(result);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_SetPlaneWaveImaging(float start_angle, float step_angle, float end_angle)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_PlaneWaveImagingCompounding(start_angle, step_angle, end_angle, id_virtual_channel)) < 0) 
      return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_SetEmissionFocalLawFile(char *file)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->FP_SET_EmissionFocalLawFile(file)) < 0) return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_SetFullMatrixCapture(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_FulMatrixCapture (id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_DeleteFocalLaws(void)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_DELETE_FocalLaws (id_virtual_channel)) < 0) return RLOG(result);
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetProbeConfig(int n_element, float pitch, float frequency)
{
int result, id_virtual_channel = 0;

   if ((result = ST_VCH_SET_PAConfig (n_element, pitch, frequency, id_virtual_channel)) < 0)
      return RLOG(result);
	return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetAcquisitionType (int data)
{
    int result, id_vch = 0;
    int acquiring = ST_Acquiring();

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if (acquiring == 1 && (result = ST_Stop()) < 0) { return RLOG(result); }
    gb_virtual_channel[id_vch]->SET_AcquisitionType(data);


    if (UCIP.status_pcie == 0 && UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS && UCIP.trigger_source == TRG_SCAN_PRF)
        UCIP.pause_after_burst = 1;
    else UCIP.pause_after_burst = UCIP.user_pause_after_burst;

    if (acquiring == 1) ST_Start();
    return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetSignalMode(int data)
{
    int id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    gb_virtual_channel[id_vch]->SET_SignalMode(data);
    return ES2_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int ST_SetAcquisitionMode(int data)
{
    if (data < 0) return ELOG(charES2_RANGO, ES2_RANGO)
    UCIP.acquisition_mode = (unsigned short)data;
    if (UCIP.status_pcie == 0 && UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS && UCIP.trigger_source == TRG_SCAN_PRF)
        UCIP.pause_after_burst = 1;
    else UCIP.pause_after_burst = UCIP.user_pause_after_burst;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetFocalPoint(float emission_focus, float reception_focus, int id_vch)
{
   int result;

   if (id_vch < 0) id_vch = gb_selected_virtual_channel;
   if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
      return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

   if ((result = gb_virtual_channel[id_vch]->SET_PA_FocalPoint(emission_focus,
      reception_focus)) < 0)
      return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_SetEmissionFocalPoint(float emission_focus)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->SET_PA_EmissionFocalPoint(emission_focus)) < 0)
        return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanCarriageReturn(int data)
{
    //if (gb_buffer_acq != NULL) gb_buffer_acq->SetScanCarriageReturn(data == 1);
    //gb_scan_planing_carriage_return = data == 1;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanStartLeft(int data)
{
    //if (gb_buffer_acq != NULL) gb_buffer_acq->SetScanStartLeft(data == 1);
    //gb_scan_planing_start_left = data == 1;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanStartUp(int data)
{
    //if (gb_buffer_acq != NULL) gb_buffer_acq->SetScanStartUp(data == 1);
    //gb_scan_planing_start_up = data == 1;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetHWFormatData(int data)
{
    int i;

    gb_hw_format = data == 1;
    for (i = 0; i < gb_virtual_channel.size(); i++)
    {
        if (gb_virtual_channel[i] != NULL && gb_virtual_channel[i]->buffer_mc_hwut_ != NULL) gb_virtual_channel[i]->buffer_mc_hwut_->HWFormatData(data == 1);
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetMaxAcquisitionsNumber(void)
{
    int result;

    if ((result = ST_GetMaxAcquisitionsNumber()) < 0) return RLOG(result);
    if (UCIP.n_images_burst > 0)
    {
        if ((result = ST_SetAcquisitionsNumber(result / UCIP.n_images_burst, UCIP.n_images_burst)) < 0) return RLOG(result);
    }
    else
    {
        if ((result = ST_SetAcquisitionsNumber(result / UCIP.n_images_burst, 1)) < 0) return RLOG(result);
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_TFM_SetConfig(int n_rec_elements, int n_emi_elements, float start_x, float end_x, int n_lines, float angle)
//{
//    int i, result, id_vch = 0;
//    vector<int> rec_elements, emi_elements;
//
//    emi_elements.push_back(n_emi_elements);
//    for (i = 0; i < n_rec_elements; i++) rec_elements.push_back(i);
//    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
//    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
//    if ((result = gb_virtual_channel[id_vch]->TFM_SET_Config(rec_elements, emi_elements, start_x, end_x, n_lines, angle)) < 0) return RLOG(result);
//    if (STDL_GetConfig())
//    {
//        ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
//    }
//    return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_SetBackwardConfig(float start_x, float end_x, int n_lines, float angle)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if ((result = gb_virtual_channel[id_vch]->TFM_SET_BackwardConfig(start_x, end_x, n_lines, angle)) < 0) return RLOG(result);
    if (STDL_GetConfig())
    {
        ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_SeFMCForwardConfig(int start_element, int end_element, int step_element)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL) 
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
    if ((result = gb_virtual_channel[id_vch]->TFM_SET_FMC_ForwardConfig(start_element, end_element, step_element)) < 0) 
        return RLOG(result);
    if (STDL_GetConfig())
    {
        ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_SetPlaneWaveImaging_Angles_deg(float* angle, int n_angles)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->SET_PWICFocalLaw_Angles(angle, n_angles)) < 0)
        return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_SetPlaneWaveImaging_Angles_rad(float* angle, int n_angles)
{
    int i, result, id_vch = 0;
    vector<float> angles_deg;

    for (i = 0; i < n_angles; i++) angles_deg.push_back(angle[i] * (180. / M_PI));
    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->SET_TFM_PWICFocalLaw_Angles(angle, n_angles)) < 0)
        return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_SetPWIForwardConfig(float start_angle, float step_angle, float end_angle)
{
    int result, id_vch = 0;

    if (id_vch < 0) id_vch = gb_selected_virtual_channel;
    if (id_vch < 0 || id_vch >= gb_virtual_channel.size() || gb_virtual_channel[id_vch] == NULL)
        return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);

    if ((result = gb_virtual_channel[id_vch]->SET_TFM_PWIForwardConfig(start_angle, step_angle, end_angle)) < 0)
        return RLOG(result);
    if (STDL_GetConfig())
    {
        ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetPauseAfterBurst(int data)
{
    if (UCIP.status_pcie == 0 && UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS)
        return ELOG(charES2_FunctionNotAvailable, ES2_FunctionNotAvailable);
    UCIP.user_pause_after_burst = UCIP.pause_after_burst = data;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetAutoContinueMode(int data)
{
    UCIP.auto_continue_mode = data;
    return ES2_NONE;
}












