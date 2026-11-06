#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
   #define WIN32_LEAN_AND_MEAN
   #define NOMINMAX
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <process.h>
#include <limits>
#include <time.h>
#include <winsock2.h>
#include <Pathcch.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "tad.h"
#include "st_error_code.h"
#include "c_critical_section.h"
#include "protocol_uci.h"
#include "api_tcp.h"
#include "tcp_comm.h"
#include "st_reg.h"
#include "st_info.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_set.h"
#include "st_get.h"
#include "st_config.h"
#include "st_load.h"
#include "st_range.h"
#include "st_encoder.h"
#include "c_log_file.h"
#include "st_thread_acq.h"
#include "utils.h"
#include "tcp_error_code.h"
#include "c_buffer_hw.h"
#include "c_buffer_acq.h"
#include "c_buffer_mc_scan_hw_ut.h"
#include "c_buffer_pa_scan_hw_ut.h"
#include "c_buffer_tfm_scan_hw_ut.h"
#include "buffer_mc_data.h"
#include "buffer_pa_data.h"
#include "st_thread_scan.h"
#include "c_virtual_channel.h"
#include "c_pa_system.h"
#include "c_pa_system_config.h"
#include "fileformat.h"
#include "st_vch_load.h"
#include "st_vch_get.h"
#include "st_vch_set.h"
#include "st_vch_config.h"
#include "sitau_hw.h"
#include "t_base_tad.h"
#include "c_log_protocol.h"
#include "c_xml_sitau_hw.h"
#include "c_log_message.h"
#include "winsock_api.h"
#include "shared_mem.h"
#include "c_datalogger.h"
#include "st_thread_status.h"

using namespace std;

// ______________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ======================================================================

int gb_ge_buffer_n_acquisitions = 0;
bool gb_hw_format = false;
CXML_CONFIG *gb_xml_file = NULL;
T_FUS gb_fus;
vector<unsigned char> gb_high_voltage_to_da;
vector<unsigned long> gb_no_hardware_ascan_pulse_idx;
long disparo_anterior = 0;
char gb_msg_progress_open_sys[255] = "";
int gb_progress_open_sys = 0;
int gb_progress_load_config = 0;
char gb_msg_progress_save_acquisition[255] = "";
int gb_progress_save_acquisition = 0;
char gb_msg_progress_load_acquisition[255] = "";
int gb_progress_load_acquisition = 0;
int gb_selected_hw_gate = 0;
int gb_load_uci_firmware = 2;
int gb_hw_sitau_enabled = 2;

AnsiString gb_st_file_path = "";

TUCIP UCIP;
TstVAR stVAR;

vector<unsigned long> gb_no_hardware_data_idx;
vector<unsigned long> gb_no_hardware_fl_idx;
vector<unsigned long> gb_no_hardware_trg_idx;
vector<long> gb_no_hardware_data_enc;
unsigned long gb_no_hardware_acq_burst_idx;
bool gb_no_hardware_waiting_continue;
bool gb_no_hardware_waiting_continue_previous;

LWORD FirmwareProgress = 0;
LWORD FirmwareSize = 0;
int ErrorLoadFirmware = 0;

int gb_opened_system = 0;
bool gb_acquisition_process_stoped = true;

int gb_log_config_enabled = 0;
int gb_log_system_info_enabled = 1;

CCriticalSection* gb_plock_stop_acquisition_process = NULL;
CCriticalSection* gb_plock_stop = NULL;


// ______________________________________________________________________
// ------ I M L E M E N T A C I Ã“ N   D E   F U N C I O N E S
// ======================================================================

void ST_RegIni (void)
{
//int n_channels;//i
unsigned short aperture = 8;
unsigned short startelement = 1;
unsigned short endelement = 31;
unsigned short step = 1;
float angle = 0.;
float material_velocity = 6000.;

    FirmwareProgress = 0;

    UCIP.trigger_source = TRG_SCAN_PRF;
    UCIP.user_pause_after_burst = UCIP.pause_after_burst = 0;
    UCIP.auto_continue_mode = 1;
    UCIP.trigger_prf_time = 20;
    UCIP.trigger_acq_number = 1;
    UCIP.n_bursts = 1;
    UCIP.n_images_burst = 1;
    UCIP.delay_sync_time = 0;
    UCIP.delay_sync_n_acq = 0;
    UCIP.acquisition_mode = ACQUISITION_MODE_FIXED;

   gb_no_hardware_data_enc.resize(NUM_ENCODER);
   gb_no_hardware_waiting_continue = false;
   gb_no_hardware_waiting_continue_previous = true;
   gb_no_hardware_acq_burst_idx = 0;

	stVAR.hw_iniciado = false;
	stVAR.hw_conectado = false;
	stVAR.high_voltage = 0;

   gb_fus.central_frequency = 0;
   gb_fus.rapport_focal_aperture = 0;
   gb_fus.apodization_coefficient = 0;
   gb_fus.lateral_position_x = 0;
   gb_fus.pixel_size_x = 0;
   gb_fus.pixels_x = 0;
   gb_fus.depth_z = 0;
   gb_fus.pixels_z = 0;
   gb_fus.pixel_size_z = 0;
   gb_fus.single_plane = 0;
   gb_fus.samples_wavelength = 0;
   gb_fus.interpolation_coefficients = 0;

   ST_SetHighVoltageMaximum(-1);
}
// -----------------------------------------------------------------------------
/**
 Carga la configuraciÃ³n guardada en un archivo .PAC

 @param[in]    file
               Nombre, junto con la ruta, del archivo de configuraciÃ³n .PAC

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
void ST_SetHighVoltageMaximum(int data)
{
int i, result;
unsigned char da;
bool flag;

   gb_high_voltage_to_da.clear();
   if (data < 0) data = 10000;
   for (flag=false,i=0; !flag && i<data; i++)
   {
      da = Round8u(i*m_HIGH_VOLTAGE_TO_DA + b_HIGH_VOLTAGE_TO_DA);
      if (da >= HIGH_VOLTAGE_DA_MAX) flag = true;
      else gb_high_voltage_to_da.push_back(da);
   }
   if (stVAR.high_voltage > data)
   {
       if ((result = ST_SetPulseAmplitude(data)) < 0) RLOG(result);
   }
}
// -----------------------------------------------------------------------------
/**
 Carga la configuraciÃ³n guardada en un archivo .PAC

 @param[in]    file
               Nombre, junto con la ruta, del archivo de configuraciÃ³n .PAC

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_GetHighVoltageMaximum(void)
{
   return (int)gb_high_voltage_to_da.size();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int ST_AddScan (void)
{
int result;

   if ((result = ST_VirtualChannelsAdd("Default")) < 0) return RLOG(result);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int ST_ResetScan (void)
{
   return ST_VirtualChannelsDefault(); // Propaga tambien el fallo de memoria al reiniciar el scan.
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int ST_DeleteScan (int id_virtual_channel)
{
int result;

	if ((result = ST_VirtualChannelDelete(id_virtual_channel)) < 0)
      return RLOG(result);
   return ES2_NONE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//++++                       C O N F I G U R A C I Ã“ N                    ++++//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// -----------------------------------------------------------------------------
/**
 Finaliza cualquier proceso de adquisiciÃ³n y detiene el sistema.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_StartSys (char *ip_address, int tcp_port_cfg, char *file_cpu1)
{
    bool firmware_loaded;
    CTimeout* timeout = NULL;
    int result;

    if (gb_load_uci_firmware == 1)
    {
        LOG_Message("Resetting System...");
        LOG_Info(gb_msg_progress_open_sys);
        tcpcom_rst_sys(uci_tcp_cfg);

        LOG_Message("Disconnecting Ethernet Socket...");
        LOG_Info(gb_msg_progress_open_sys);
        tcp_close(&uci_tcp_cfg);
        gb_ethernet_connected = 0;

        Delay(7000, MILISECONDS);

        LOG_Message("Connecting UCI '%s:%d...", ip_address, tcp_port_cfg);
        LOG_Info(gb_msg_progress_open_sys);
        if ((result = tcpcom_connect(ip_address, tcp_port_cfg, &uci_tcp_cfg)) < 0 || uci_tcp_cfg == INVALID_SOCKET)
        {
            tcp_close(&uci_tcp_cfg);
            return ELOG(charETCP_CONNECT, ETCP_CONNECT);
        }
    }
    ST_UpdateStatus();
    if (gb_shared.cpu1.flags.BIT.CPU1_LOADED == 1)
        return ES2_NONE;

    gb_ethernet_connected = 1;

    _snprintf_s(gb_msg_progress_open_sys, sizeof(gb_msg_progress_open_sys), _TRUNCATE, "Loading Firmware UCI...");
    LOG_Info(gb_msg_progress_open_sys);
    image_exec(uci_tcp_cfg, file_cpu1);

    if ((timeout = new CTimeout(5000, MILISECONDS)) == NULL) return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
    firmware_loaded = false;
    timeout->Start();
    while (!firmware_loaded && !timeout->Timeout())
    {
        ST_UpdateStatus();
        if (gb_shared.cpu1.flags.BIT.FSM_STARTED == 1)
            firmware_loaded = true;
        else Delay(500, MILISECONDS);
    }

    if (firmware_loaded) return ES2_NONE;
    else
    {
        ST_UpdateStatus();
        if (gb_shared.cpu1.flags.BIT.CPU1_LOADED == 1) return ES2_NONE;
        else return ELOG(charES2_FILE, ES2_FILE);
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_WriteBootFirmware_1 (char *path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq, char *file_firmware)
{
int result = 0;

   if ((result = ST_WriteBootFirmware_2 (ip_address, tcp_port_cfg, file_firmware, 10000)) < 0)
      return RLOG(result);

   ST_EnabledLoadUCIFirmware(1);
   if ((result = ST_OpenSys(path_bin, ip_address, tcp_port_cfg, tcp_port_acq)) < 0)
      return RLOG(result);

   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_WriteBootFirmware_2 (char *ip_address, int tcp_port_cfg, char *file_firmware, int wait_ms)
{
int result, n_try, n_data;
CTimeout *timeout = NULL;
FILE *f = NULL;
bool delete_message = false;
int timeout_sec_connect = 5;


    LOG_Open("SITAU Upgrade Firmware");

try
{
    if (gb_msg != NULL)
    {
        delete gb_msg;
        gb_msg = NULL;
    }
    gb_msg = new CLogMessage();

    if (datos_env == NULL)
    {
        if ((datos_env = (unsigned char*)malloc(TAMANIO)) == NULL)
        {
            LOG_Message("ERROR Code %d: Memory reservation error", ETCP_NO_MEM, TAMANIO);
            return ELOG(charETCP_NO_MEM, ETCP_NO_MEM);
        }
    }

   gb_progress_open_sys = 0;
   ws_closesocket(&uci_tcp_cfg);
   gb_progress_open_sys++;
   gb_ethernet_connected = 0;
   uci_tcp_cfg = INVALID_SOCKET;

    LOG_Message("Connecting SITAU '%s:%d...", ip_address, tcp_port_cfg);
    if (!gb_no_hw && (result = tcp_connect(ip_address, tcp_port_cfg, timeout_sec_connect, &uci_tcp_cfg, NULL)) < 0)
    {
        LOG_Message("ERROR Code %d: Connecting SITAU '%s:%d", ETCP_CONNECT, ip_address, tcp_port_cfg);
        ws_closesocket(&uci_tcp_cfg);
        gb_ethernet_connected = 0;
        uci_tcp_cfg = INVALID_SOCKET;
        return RLOG(result);
    }
    gb_ethernet_connected = 1;
    gb_progress_open_sys++;

    LOG_Message("Getting Firmware From File...");
    if ((n_data = tcpcom_load_file(datos_env, file_firmware, TAMANIO)) <= 0)
    {
        LOG_Message("ERROR Code %d: Getting Firmware From File...", n_data);
        return RLOG(n_data);
    }
    gb_progress_open_sys++;

    LOG_Message("Sending Firmware (%d Bytes)...",n_data);
    if ((result = tcpcom_large_send(uci_tcp_cfg, (char*)datos_env, n_data, -1)) < 0)
    {
        LOG_Message("ERROR Code %d: Sending Firmware (%d Bytes)...", result, n_data);
        return RLOG(result);
    }
    gb_progress_open_sys++;

	LOG_Message("Writing Flash Memory...");
	tcpcom_write_qspi(uci_tcp_cfg, 0, -1, n_data);
    gb_progress_open_sys++;

	LOG_Message("Disconnecting SITAU...");
    ws_closesocket(&uci_tcp_cfg);
    gb_ethernet_connected = 0;   
    uci_tcp_cfg = INVALID_SOCKET;
    gb_progress_open_sys++;

    LOG_Message("Disconnected, waiting %d seconds...", Round32((float)wait_ms / (float)1000));
    LOG_StartDelay(wait_ms);
    Delay (wait_ms, MILISECONDS);
    LOG_StopDelay();
    gb_progress_open_sys++;

    LOG_Message("Waiting Connection...");
    if (!gb_no_hw && (result = tcp_connect(ip_address, tcp_port_cfg, timeout_sec_connect, &uci_tcp_cfg, NULL)) < 0)
    {
        n_try = 0;
        timeout = new CTimeout(wait_ms, MILISECONDS);
        timeout->Start();
        while (uci_tcp_cfg == INVALID_SOCKET && !timeout->Timeout())
        {
            n_try++;
            LOG_Message("Trying to connect (%d)...", n_try);
            tcp_connect(ip_address, tcp_port_cfg, timeout_sec_connect, &uci_tcp_cfg, NULL);
        }
        if (timeout != NULL) delete timeout;
    }

    if (uci_tcp_cfg != INVALID_SOCKET)
    {
        LOG_Message("Connected SITAU '%s:%d", ip_address, tcp_port_cfg);
        LOG_Message("Resetting SITAU System...");
        tcpcom_rst_sys(uci_tcp_cfg);
        gb_progress_open_sys++;

        LOG_Message("Disconnecting SITAU...");
        ws_closesocket(&uci_tcp_cfg);
        gb_ethernet_connected = 0;
        uci_tcp_cfg = INVALID_SOCKET;

        LOG_Message("Waiting Connection...");
        if (!gb_no_hw && (result = tcp_connect(ip_address, tcp_port_cfg, timeout_sec_connect, &uci_tcp_cfg, NULL)) < 0)
        {
            n_try = 0;
            timeout = new CTimeout(wait_ms, MILISECONDS);
            timeout->Start();
            while (uci_tcp_cfg == INVALID_SOCKET && !timeout->Timeout())
            {
                n_try++;
                LOG_Message("Trying to connect (%d)...", n_try);
                tcp_connect(ip_address, tcp_port_cfg, timeout_sec_connect, &uci_tcp_cfg, NULL);
            }
            if (timeout != NULL) delete timeout;
        }
        
        if (uci_tcp_cfg != INVALID_SOCKET) LOG_Message("Successful firmware upgrade!");
        else LOG_Message("ERROR Connecting SITAU '%s:%d", ip_address, tcp_port_cfg);
    }
    else
    {
        LOG_Message("ERROR Connecting SITAU '%s:%d", ip_address, tcp_port_cfg);
        return ELOG(charETCP_CONNECT, ETCP_CONNECT);
    }
    gb_progress_open_sys++;

    ws_closesocket(&uci_tcp_cfg);
    gb_ethernet_connected = 0;
    uci_tcp_cfg = INVALID_SOCKET;
}
__finally
{
    gb_msg->SetDeleteSignal();
}
   return ES2_NONE;
}
// ----------------------------------------------------------------------
// ST_ConfigSys
// ============
// Descripcion:
//     Carga los .bit de todas las FPGAs del sistema y ejecuta un test sobre
//     ellas para verificar su correcta programaciÃ³n.
//
// Parametros:
//
// Devuelve: (Ver codigos de error en 'sterr_v2.h')
// ----------------------------------------------------------------------
int ST_ConfigSys (char* path, char *ip_address, int tcp_port_cfg, int tcp_port_acq)
{
int result;
char file_base[255];
char file_cpu1[255];
char str_info[255];
int try_firmware = 1;
AnsiString str_data;

#if defined(_MSC_VER)
System::String ^file_string = nullptr;
#endif

    gb_cancel_connect = false;
    if (!gb_acquisition_process_stoped)
    {
        if ((result = ST_Stop()) < 0) { RLOG(result); }
    }

   if (!stVAR.hw_iniciado)
      return ELOG(charES2_NOINIT,ES2_NOINIT);

   _snprintf_s(file_cpu1, sizeof(file_cpu1), _TRUNCATE, "%s%s", path, FILENAME_CPU1);
   file_string = gcnew System::String(file_cpu1);
   if (!System::IO::File::Exists(file_string))
   {
      _snprintf_s(str_info, sizeof(str_info), _TRUNCATE, "Error, '%s' doesn't exist", file_cpu1);
      return ELOG(str_info, ES2_FILE);
   }
   delete file_string; 

   _snprintf_s(file_base, sizeof(file_base), _TRUNCATE, "%s%s", path, FILENAME_BASE);
   file_string = gcnew System::String(file_base);
   if (!System::IO::File::Exists(file_string))
   {
      _snprintf_s(str_info, sizeof(str_info), _TRUNCATE, "Error, '%s' doesn't exist", file_base);
      return ELOG(str_info, ES2_FILE);
   }
   delete file_string; 

   if (!gb_no_hw)
   {
      LOG_Message("Connecting UCI '%s:%d...", ip_address, tcp_port_cfg);
      LOG_Info(gb_msg_progress_open_sys);
      if ((result = tcpcom_connect(ip_address, tcp_port_cfg, &uci_tcp_cfg)) < 0 || uci_tcp_cfg == INVALID_SOCKET) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
      gb_ethernet_connected = 1;
      gb_progress_open_sys++;

      LOG_Message("Stopping Acquisition Process...");
      LOG_Info(gb_msg_progress_open_sys);
      gb_progress_open_sys++;

      if ((result = ST_StopProcess()) < 0) { RLOG(result); }

      LOG_Message("Cleaning TCP Config Port Buffer...");
      LOG_Info(gb_msg_progress_open_sys);
      gb_progress_open_sys++;

      TCP_ClearReceive_ConfigPort();

        if (gb_load_uci_firmware > 0)
        {
            if ((result = ST_StartSys(ip_address, tcp_port_cfg, file_cpu1)) < 0)
            {
                gb_load_uci_firmware = 1;
                if ((result = ST_StartSys(ip_address, tcp_port_cfg, file_cpu1)) < 0) return RLOG(result);
            }
        }
        gb_progress_open_sys++;

        if ((result = TCPSend_MSG_SITAUHardwareEnabled(gb_hw_sitau_enabled)) < 0) return RLOG(result);
        if (gb_hw_sitau_enabled > 0)
        {
            Delay(500, MILISECONDS);
            LOG_Message("Loading Firmware BASE...");
            LOG_Info(gb_msg_progress_open_sys);
            if ((result = ST_LoadFirmware(file_base)) < 0) return RLOG(result);
        }

        LOG_Message("Connecting Acquisition Port %d...", tcp_port_acq);
        LOG_Info(gb_msg_progress_open_sys);
        if ((result = tcpcom_connect(ip_address, tcp_port_acq, &uci_tcp_acq)) < 0 || uci_tcp_acq == INVALID_SOCKET) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
        gb_ethernet_connected = 1;
    }
    else gb_ethernet_connected = 1;

   _snprintf_s(gb_ip_address, sizeof(gb_ip_address), _TRUNCATE, "%s", ip_address);
   gb_tcp_port_cfg = tcp_port_cfg;
   gb_tcp_port_acq = tcp_port_acq;

   gb_progress_open_sys++;

   LOG_Message("Setting Initial System Configuration...");
   LOG_Info(gb_msg_progress_open_sys);
   gb_progress_open_sys++;

	stVAR.hw_conectado = true;
   return ES2_NONE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//++++                        A D Q U I S I C I Ã“ N                       ++++//

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetNoHardwareData_v0(CVirtualChannel *vch, int id_fl)
{
int l, b, inc_signal;
unsigned long no_hardware_data_idx = 0, no_hardware_trg_idx = 0, ascan_pulse_idx;
int i_data, id_vch, ibuf;
short int v_data;
float signal_factor;
unsigned long n_samples, data_u32;
int n_bases, min_signal = ST_RangeFP_MinSignalAmplitude();
int max_ascan_signal, max_signal = ST_RangeFP_MaxSignalAmplitude();
float pa_frequency = 5;
float sampling_frequency, signal_offset, pulse_width, amplitude_factor;
unsigned long time_stamp, size_buffer;
short int *ptr_short = NULL;
short int *ptr_buffer = NULL;
long data_long;
int n_ascan, l_base, n_fl;
int n_samples_cicles, n_burst = 1, n_cycles = 1;// , i_burst;
float noise_factor = 0;

   if (vch == NULL) return ES2_NONE;
   n_fl = vch->cfg_.mc_emission_focal_law.size();
   if (id_fl >= n_fl) return ES2_NONE;

   id_vch = vch->GET_Id();
   if ((n_bases = ST_VCH_GET_BASENumber(id_vch)) <= 0) return ELOG(charES2_BASE_Number, ES2_BASE_Number);

   sampling_frequency = vch->GET_SamplingFrequency();
   pulse_width = vch->GET_PulseWidth();

   ptr_buffer = gb_buffer_acquisition;
   size_buffer = SIZE_BUFFER_ACQ;

/* =====================================================
	CABECERA DE LA TRAMA
   ===================================================== */
   
   ibuf = 0;

   // ESCRIBE LA PALABRA DE INICIO DE LA IMAGEN
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = (unsigned long)TRM_FP_HEADER_v0;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ESCRIBE EL ID DEL CANAL VIRTUAL
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = (unsigned long)id_vch;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ESCRIBE EL NUMERO TOTAL DE DATOS DE LA TRAMA
   ptr_short = (short int*)&vch->cfg_.fp_focal_law[id_fl].frame_size;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ESCRIBE EL ID DE LA LEY FOCAL
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = (unsigned long)id_fl;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ESCRIBE EL NUMERO DE SEÃ‘ALES A-SCAN (Canales habilitados para la recepciÃ³n)
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   n_ascan = vch->cfg_.fp_focal_law[id_fl].n_ascan;
   data_u32 = (unsigned long)n_ascan;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ESCRIBE EL NUMERO DE DATOS DE CADA ASCAN (Canal)
   if (vch->cfg_.acq.n_acq_samples % 2 != 0) vch->cfg_.acq.n_acq_samples++;
   n_samples = vch->cfg_.acq.n_acq_samples;
   data_u32 = (unsigned long)n_samples;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;
   //ptr_buffer[ibuf] = n_samples; ibuf++;

   // ESCRIBE EL CONTADOR DE ADQUISICIONES
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   no_hardware_data_idx = 0;
   if (id_vch < gb_no_hardware_data_idx.size())
   {
	   //ptr_buffer[ibuf] = gb_no_hardware_data_idx[id_vch];
      gb_no_hardware_data_idx[id_vch]++;
      no_hardware_data_idx = gb_no_hardware_data_idx[id_vch];
   }
   ptr_short = (short int*)&no_hardware_data_idx;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   ptr_buffer[ibuf] = ptr_short[1]; ibuf++;
   if (id_vch < gb_no_hardware_trg_idx.size()) no_hardware_trg_idx = gb_no_hardware_trg_idx[id_vch];

   // ESCRIBE EL TIME STAMP
   time_stamp = no_hardware_data_idx * 10;

   // ESCRIBE EL VALOR DEL OFFSET APLICADO AL ENCODER DEL DISPARO
   data_long = 0xA5A5A5A5;
   ptr_short = (short int*)&data_long;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;


/* =====================================================
   --> BUCLE DE ADQUISICION DE LINEAS
   ===================================================== */

   inc_signal = 1;
   //signal_factor = (float)2. * (float)M_PI * pa_frequency / sampling_frequency;
   n_samples_cicles = 10 * sampling_frequency * pulse_width / 1000;
   if (n_samples_cicles < 12) n_samples_cicles = 12;
   n_samples_cicles += (id_fl * (n_samples_cicles / 2));
   signal_factor = (float)2. * (float)M_PI / n_samples_cicles;

   if (id_vch < gb_no_hardware_ascan_pulse_idx.size())
   {
       if (gb_no_hardware_ascan_pulse_idx[id_vch] >= n_samples) gb_no_hardware_ascan_pulse_idx[id_vch] = 0;
       else gb_no_hardware_ascan_pulse_idx[id_vch]++;
       ascan_pulse_idx = gb_no_hardware_ascan_pulse_idx[id_vch];
   }
   else ascan_pulse_idx = 0;

    inc_signal = 1;
    for (l = b = 0; b < n_bases && l < n_ascan; b++)
    {
        signal_offset = (id_fl + b * 10) * (float)100.;
        for (v_data = min_signal, l_base = i_data = 0; i_data < n_samples; v_data += inc_signal, i_data++)
        {
            max_ascan_signal = Round32((float)max_signal / (float)n_ascan);
            amplitude_factor = (float)max_ascan_signal / (float)n_cycles;
            for (l_base = 0; l < n_ascan && l_base < MOD_MAX_CH; l_base++, ibuf++)
            {
                //no_hardware_trg_idx = 0;
                if (min_signal < 0) v_data = Round16((l + l_base + 1) * amplitude_factor * sin(signal_factor * (i_data + no_hardware_trg_idx * (n_samples_cicles / 8))) + (-16000 + rand() % 32000) * noise_factor);
                else v_data = Round16u((l + l_base + 1) * amplitude_factor * sin(signal_factor * (i_data + no_hardware_trg_idx))  + (rand() % 32000) * noise_factor);
                ptr_buffer[ibuf] = v_data;
            }
        }
        l += l_base;
    }

// ESCRIBE LA PALABRA DE FIN DE LA IMAGEN
   data_u32 = (unsigned long)TRM_FOOTER;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

/* =====================================================
    --> FIN DEL BUCLE PRINCIPAL
   ===================================================== */

   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);

    if (id_vch < gb_no_hardware_fl_idx.size())// && id_vch < gb_no_hardware_trg_idx.size())
    {
        gb_no_hardware_fl_idx[id_vch] = id_fl;
        if (gb_no_hardware_fl_idx[id_vch] >= n_fl - 1)
        {
            gb_no_hardware_fl_idx[id_vch] = 0;
            //gb_no_hardware_trg_idx[id_vch]++;
        }
    }

    return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetNoHardwareData_v0(CVirtualChannel* vch)
{
    int l, inc_signal;//b
    unsigned long no_hardware_data_idx = 0, no_hardware_trg_idx = 0, ascan_pulse_idx;
    int i_data, id_vch, ibuf, n_lines;
    short int v_data;
    float signal_factor;
    unsigned long n_samples, data_u32;
    int n_bases, min_signal = ST_RangeFP_MinSignalAmplitude();
    int max_ascan_signal, max_signal = ST_RangeFP_MaxSignalAmplitude();
    float pa_frequency = 5;
    float sampling_frequency, signal_offset, pulse_width;// , amplitude_factor;
    unsigned long time_stamp, size_buffer;
    short int* ptr_short = NULL;
    short int* ptr_buffer = NULL;
    long data_long;
    int n_ascan;// , l_base;
    int n_parallel_beamformed_lines, n_burst = 1, n_cycles = 1, n_blocks, n_blocks_32b;//i_burst, n_samples_cicles
    float noise_factor = 0;/// (float)0.1;
    int i, j, i_block, n_lines_last_block, l_block, n_beamformers, i_beam;//i_block_beam, n_blocks_beam
    vector<unsigned char> n_lines_block;

    if (vch == NULL) return ES2_NONE;
    n_beamformers = STHW_GetBeamformersNumber();
    min_signal = ST_RangePA_MinSignalAmplitude(vch->cfg_.id);
    max_signal = ST_RangePA_MaxSignalAmplitude(vch->cfg_.id);

    id_vch = vch->GET_Id();
    if ((n_bases = ST_VCH_GET_BASENumber(id_vch)) <= 0) return ELOG(charES2_BASE_Number, ES2_BASE_Number);

    //n_data = vch->cfg_.fp_focal_law[id_fl].image_size;

    sampling_frequency = vch->GET_SamplingFrequency();
    pulse_width = vch->GET_PulseWidth();

    ptr_buffer = gb_buffer_acquisition;
    size_buffer = SIZE_BUFFER_ACQ;

    /* =====================================================
        CABECERA DE LA TRAMA
       ===================================================== */

    ibuf = 0;

    // ESCRIBE LA PALABRA DE INICIO DE LA IMAGEN
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    data_u32 = (unsigned long)TRM_PA_HEADER_v0;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // ESCRIBE EL ID DEL CANAL VIRTUAL
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    data_u32 = (unsigned long)id_vch;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // ESCRIBE EL NUMERO TOTAL DE DATOS DE LA TRAMA
    ptr_short = (short int*)&vch->cfg_.tfm.frame_size;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    data_u32 = vch->GET_TFM_ParallelBeamformedLines();
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // NUMERO DE SEÃ‘ALES A-SCAN (Lineas de la imagen TFM)
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    n_ascan = vch->GET_TFM_BackwardLinesNumber();
    data_u32 = (unsigned long)n_ascan;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // ESCRIBE EL NUMERO DE DATOS DE CADA ASCAN (LÃ­nea)
    if (vch->cfg_.acq.n_acq_samples % 2 != 0) vch->cfg_.acq.n_acq_samples++;
    n_samples = vch->cfg_.acq.n_beamformed_samples;
    data_u32 = (unsigned long)n_samples;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
    n_parallel_beamformed_lines = vch->GET_TFM_ParallelBeamformedLines();
    n_blocks = Round32u((float)n_ascan / (float)n_parallel_beamformed_lines + (float)0.5);
    n_blocks_32b = Round32u((float)n_blocks / (float)4 + (float)0.5);
    data_u32 = n_blocks << 16 | n_blocks_32b;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // NUMERO DE SEÃ‘ALES A-SCAN (LINEAS) DE CADA BLOQUE
    n_lines_last_block = n_parallel_beamformed_lines > 0 ? n_ascan % n_parallel_beamformed_lines : n_ascan;
    n_lines_last_block = n_lines_last_block != 0 ? n_lines_last_block : n_parallel_beamformed_lines;
    for (i_block = 0; i_block < n_blocks - 1; i_block++) n_lines_block.push_back(n_parallel_beamformed_lines);
    n_lines_block.push_back(n_lines_last_block);
    for (i = i_block = 0; i < n_blocks_32b && i_block < n_lines_block.size(); i++)
    {
        data_u32 = 0;
        for (j = 0; j < 4; j++, i_block++)
        {
            if (i_block < n_lines_block.size()) n_lines = n_lines_block[i_block];
            else n_lines = 0;
            data_u32 = data_u32 | (n_lines << j * 8);
        }
        ptr_short = (short int*)&data_u32;
        if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
        ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
        if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
        ptr_buffer[ibuf] = ptr_short[1]; ibuf++;
    }

    // CONTADOR DE IMAGENES TFM
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    no_hardware_data_idx = 0;
    if (id_vch < gb_no_hardware_data_idx.size())
    {
        ptr_buffer[ibuf] = gb_no_hardware_data_idx[id_vch];
        gb_no_hardware_data_idx[id_vch]++;
        no_hardware_data_idx = gb_no_hardware_data_idx[id_vch];
    }
    ptr_short = (short int*)&no_hardware_data_idx;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    // ESCRIBE EL TIME STAMP
    time_stamp = no_hardware_data_idx * 10;

    // ESCRIBE EL VALOR DEL OFFSET APLICADO AL ENCODER DEL DISPARO
    data_long = 0xA5A5A5A5;
    ptr_short = (short int*)&data_long;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

    /* =====================================================
       --> BUCLE DE ADQUISICION DE LINEAS
       ===================================================== */

    inc_signal = 1;
    signal_factor = (float)2. * (float)M_PI * pa_frequency / sampling_frequency;
    signal_offset = (float)100.;

    //n_blocks_beam = n_lines_block.size() / n_beamformers;
    for (i_beam = i_block = l = 0; i_beam < n_beamformers; i_beam++)
    {
        for (i_block = 0; i_block < n_lines_block.size(); i_block++)
        {
            for (i_data = 0; i_data < n_samples; i_data++)
            {
                for (l_block = 0; l_block < n_lines_block[i_block]; l_block++, ibuf++)
                {
                    max_ascan_signal = Round32((float)max_signal * (float)((l_block + l) + (float)1.) / (float)n_ascan);
                    v_data = Round16(signal_offset + max_ascan_signal * sin(signal_factor * (i_data + no_hardware_data_idx)));
                    //v_data = Round16((float)i_data * (float)max_signal / (float)n_samples);
                    if (v_data > max_signal) v_data = max_signal;
                    if (v_data < min_signal) v_data = min_signal;
                    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
                    ptr_buffer[ibuf] = v_data;
                }
            }
            l += n_lines_block[i_block];
        }
    }

    //inc_signal = 1;
    //n_samples_cicles = 10 * sampling_frequency * pulse_width / 1000;
    //if (n_samples_cicles < 12) n_samples_cicles = 12;
    //signal_factor = (float)2. * (float)M_PI / n_samples_cicles;

    //if (id_vch < gb_no_hardware_ascan_pulse_idx.size())
    //{
    //    if (gb_no_hardware_ascan_pulse_idx[id_vch] >= n_samples) gb_no_hardware_ascan_pulse_idx[id_vch] = 0;
    //    else gb_no_hardware_ascan_pulse_idx[id_vch]++;
    //    ascan_pulse_idx = gb_no_hardware_ascan_pulse_idx[id_vch];
    //}
    //else ascan_pulse_idx = 0;

    //inc_signal = 1;
    //for (l = b = 0; b < n_bases && l < n_ascan; b++)
    //{
    //    signal_offset = (b * 10) * (float)100.;
    //    for (v_data = min_signal, l_base = i_data = 0; i_data < n_samples; v_data += inc_signal, i_data++)
    //    {
    //        max_ascan_signal = Round32((float)max_signal / (float)n_ascan);
    //        amplitude_factor = (float)max_ascan_signal / (float)n_cycles;
    //        for (l_base = 0; l < n_ascan && l_base < MOD_MAX_CH; l_base++, ibuf++)
    //        {
    //            if (min_signal < 0) v_data = Round16((l + l_base) * amplitude_factor * sin(signal_factor * (i_data + no_hardware_trg_idx * (n_samples_cicles / 4))) + (-16000 + rand() % 32000) * noise_factor);
    //            else v_data = Round16u((l + l_base) * amplitude_factor * sin(signal_factor * (i_data + no_hardware_trg_idx * (n_samples_cicles / 4))) + (rand() % 32000) * noise_factor);
    //            if (v_data > max_signal) v_data = max_signal;
    //            if (v_data < min_signal) v_data = min_signal;
    //            ptr_buffer[ibuf] = v_data;
    //        }
    //    }
    //    l += l_base;
    //}

 // ESCRIBE LA PALABRA DE FIN DE LA IMAGEN
    data_u32 = (unsigned long)TRM_FOOTER;
    ptr_short = (short int*)&data_u32;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

 /* =====================================================
     --> FIN DEL BUCLE PRINCIPAL
    ===================================================== */

    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetNoHardwareData_v0(CVirtualChannel *vch)
{
int l, b, n_data, inc_signal;
unsigned long no_hardware_data_idx;
int i_data, id_vch, ibuf;
short int v_data;
float signal_factor;
unsigned long n_samples, data_u32, n_lines;
int n_bases, min_signal;
int max_ascan_signal, max_signal;
float pa_frequency = 5;
float sampling_frequency, signal_offset;
unsigned long time_stamp, size_buffer, n_blocks, n_blocks_32b;
short int *ptr_short = NULL;
short int *ptr_buffer = NULL;
long data_long;
int i, j, i_block, n_ascan, n_parallel_beamformed_lines, n_lines_last_block, l_block, n_beamformers, i_block_beam, n_blocks_beam, i_beam;
vector<unsigned char> n_lines_block;

   if (vch == NULL) return ES2_NONE;
   //if (cfg_hw == NULL) return ES2_NONE;
   n_beamformers = STHW_GetBeamformersNumber();
   min_signal = ST_RangePA_MinSignalAmplitude(vch->cfg_.id);
   max_signal = ST_RangePA_MaxSignalAmplitude(vch->cfg_.id);

   id_vch = vch->GET_Id();
   if ((n_bases = ST_VCH_GET_BASENumber(id_vch)) <= 0) return ELOG(charES2_BASE_Number, ES2_BASE_Number);
   
   n_data = vch->cfg_.pa_focal_law.image_size_user;

   sampling_frequency = vch->GET_SamplingFrequency();

   ptr_buffer = gb_buffer_acquisition;
   size_buffer = SIZE_BUFFER_ACQ;

/* =====================================================
	CABECERA DE LA TRAMA
   ===================================================== */
   
   ibuf = 0;

   // HEADER
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = (unsigned long)TRM_PA_HEADER_v0;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // ID DEL CANAL VIRTUAL
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = (unsigned long)id_vch;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO TOTAL DE DATOS DE LA TRAMA
   ptr_short = (short int*)&vch->cfg_.pa_focal_law.frame_size;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   data_u32 = vch->GET_PA_ParallelBeamformedLines();//(unsigned long)id_fl;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO DE SEÃ‘ALES A-SCAN (Lineas de la imagen PA)
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   n_ascan = vch->cfg_.pa_focal_law.beamformer.size();
   data_u32 = (unsigned long)n_ascan;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO DE DATOS DE CADA A-SCAN (LÃ­nea)
   if (vch->cfg_.acq.n_beamformed_samples % 2 != 0) vch->cfg_.acq.n_beamformed_samples++;
   n_samples = vch->cfg_.acq.n_beamformed_samples;
   data_u32 = (unsigned long)n_samples;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
   n_parallel_beamformed_lines = vch->GET_PA_ParallelBeamformedLines();
   n_blocks = Round32u((float)n_ascan / (float)n_parallel_beamformed_lines + (float)0.5);
   n_blocks_32b = Round32u((float)n_blocks / (float)4 + (float)0.5);
   data_u32 = n_blocks << 16 | n_blocks_32b;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // NUMERO DE SEÃ‘ALES A-SCAN (LINEAS) DE CADA BLOQUE
   n_lines_last_block = n_parallel_beamformed_lines > 0 ? n_ascan % n_parallel_beamformed_lines : n_ascan;
   n_lines_last_block = n_lines_last_block != 0 ? n_lines_last_block : n_parallel_beamformed_lines;
   for (i_block=0; i_block<n_blocks-1; i_block++)
      n_lines_block.push_back(n_parallel_beamformed_lines);
   n_lines_block.push_back(n_lines_last_block);

   for (i=i_block=0; i<n_blocks_32b && i_block<n_lines_block.size(); i++)
   {
      data_u32 = 0;
      for (j=0; j<4; j++,i_block++)
      {
         if (i_block<n_lines_block.size()) n_lines = n_lines_block[i_block];         
         else n_lines = 0;
         data_u32 = data_u32 | (n_lines << j*8);
      }
      ptr_short = (short int*)&data_u32;
      if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	   ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
      if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	   ptr_buffer[ibuf] = ptr_short[1]; ibuf++;
   }

    // CONTADOR DE IMAGENES PA
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    no_hardware_data_idx = 0;
    if (id_vch < gb_no_hardware_data_idx.size())
    {
	    ptr_buffer[ibuf] = gb_no_hardware_data_idx[id_vch];
        gb_no_hardware_data_idx[id_vch]++;
        no_hardware_data_idx = gb_no_hardware_data_idx[id_vch];
    }
    ptr_short = (short int*)&no_hardware_data_idx;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
    if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
    ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

   // TIME STAMP
   time_stamp = no_hardware_data_idx * 10;

   // OFFSET APLICADO AL ENCODER DEL DISPARO
   data_long = 0xA5A5A5A5;
   ptr_short = (short int*)&data_long;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

/* =====================================================
   --> BUCLE DE ADQUISICION DE LINEAS
   ===================================================== */

   inc_signal = 1;
   signal_factor = (float)2. * (float)M_PI * pa_frequency / sampling_frequency;
   signal_offset = (float)100.;

   n_blocks_beam = n_lines_block.size() / n_beamformers + 1;
   for (i_beam = i_block = l = 0; i_beam < n_beamformers; i_beam++)
   {
      for (i_block_beam = 0; i_block_beam < n_blocks_beam && i_block < n_lines_block.size(); i_block_beam++, i_block++)
      {
         for (i_data = 0; i_data < n_samples; i_data++)
         {
            for (l_block=0; l_block<n_lines_block[i_block]; l_block++, ibuf++)
            {
               max_ascan_signal = Round32((float)max_signal * (float)((l_block + l) + (float)1.) / (float)n_ascan);
               v_data = Round16(signal_offset + max_ascan_signal * sin(signal_factor * (i_data + no_hardware_data_idx)));
               //v_data = Round16((float)i_data * (float)max_signal / (float)n_samples);
               if (v_data > max_signal) v_data = max_signal;
               if (v_data < min_signal) v_data = min_signal;
               if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
               ptr_buffer[ibuf] = v_data;
            }
         }
         l += n_lines_block[i_block];
      }
   }

   // FOOTER
   data_u32 = (unsigned long)TRM_FOOTER;
   ptr_short = (short int*)&data_u32;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[0]; ibuf++;
   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
	ptr_buffer[ibuf] = ptr_short[1]; ibuf++;

/* =====================================================
    --> FIN DEL BUCLE PRINCIPAL
   ===================================================== */

   if (ibuf >= size_buffer) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   return ibuf;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MC_GetNoHardwareData (CVirtualChannel *vch, int id_fl)
{
int result;
 
   if ((result = ST_MC_GetNoHardwareData_v0 (vch, id_fl)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TFM_GetNoHardwareData(CVirtualChannel* vch)
{
    int result;

    if ((result = ST_TFM_GetNoHardwareData_v0(vch)) < 0) return RLOG(result);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PA_GetNoHardwareData (CVirtualChannel *vch)
{
int result;
 
   if ((result = ST_PA_GetNoHardwareData_v0 (vch)) < 0) return RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short TimeToTicks(float A)
{
   return Round16u(A * (float)FCLK4_MHz);
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_OpenSys().

 @param[out]   str
               Puntero a una cadena de caracteres donde se devuelven los mensajes que
               describen el progreso.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.\n
               Si no se desean recibir estos mensajes el valor de este parÃ¡metro debe
               ser NULL (Ã³ 0).

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_OpenSys()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la 
         funciÃ³n \c ST_OpenSys()
*/
// -----------------------------------------------------------------------------
int ST_OpenSys_Progress (char* str)
{
int n_bases, result=0;

    if (str != NULL)
    {
        if (gb_msg != NULL) gb_msg->Readlast(str);
        strcpy(str, gb_msg_progress_open_sys);
    }
    if (gb_xml_sitau_hw == NULL) n_bases = 4;
    else n_bases = gb_xml_sitau_hw->GetBasesNumber();   
    if (gb_progress_open_sys > 0) result = (int)((100 * gb_progress_open_sys) / (68 + 33 * n_bases));
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetLastMessage(char* message)
{
    if (gb_msg != NULL) gb_msg->Readlast(message);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_OpenSys_Cancel(void)
{
    gb_cancel_connect = true;
    strcpy_s(gb_msg_progress_open_sys, sizeof(gb_msg_progress_open_sys), "Waiting for the timeout...");
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_OpenSys().

 @param[out]   str
               Puntero a una cadena de caracteres donde se devuelven los mensajes que
               describen el progreso.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.\n
               Si no se desean recibir estos mensajes el valor de este parÃ¡metro debe
               ser NULL (Ã³ 0).

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_OpenSys()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la
         funciÃ³n \c ST_OpenSys()
*/
// -----------------------------------------------------------------------------
int ST_GetProgressMessage(char* str)
{
    int result = 0;

    if (gb_msg != NULL)
    {
        result = gb_msg->Read(str);
        if (result == 0 && gb_msg->GetDeleteSignal())
        {
            delete gb_msg;
            gb_msg = NULL;
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_OpenSys().

 @param[out]   str
               Puntero a una cadena de caracteres donde se devuelven los mensajes que
               describen el progreso.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.\n
               Si no se desean recibir estos mensajes el valor de este parÃ¡metro debe
               ser NULL (Ã³ 0).

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_OpenSys()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la
         funciÃ³n \c ST_OpenSys()
*/
// -----------------------------------------------------------------------------
int ST_GetProgressLastMessage(char* str, int *delay_ms)
{
    int result = 0;

    if (gb_msg != NULL)
    {
        if (gb_progress_open_sys > 0) result = (int)((100 * gb_progress_open_sys) / (10));
        gb_msg->Readlast(str);
        if (delay_ms != NULL) *delay_ms = gb_msg->Delay() / 1000;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadFirmware_Progress (char* str)
{
int result=0;

   if (str != NULL) strcpy(str, gb_msg_progress_open_sys);
   if (gb_progress_open_sys > 0) result = (int)(100. * gb_progress_open_sys / 7.);
   return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_LoadFileConfig().

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_LoadFileConfig()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la 
         funciÃ³n \c ST_LoadFileConfig()
*/
// -----------------------------------------------------------------------------
int ST_LoadFileConfig_Progress (void)
{
int result  = 0;

   if (gb_progress_load_config > 0) result = (int)((100 * gb_progress_load_config) / 116);
   return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_LoadFileAcquisition().

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_LoadFileConfig()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la 
         funciÃ³n \c ST_LoadFileConfig()
*/
// -----------------------------------------------------------------------------
int ST_LoadFileAcquisition_Progress (char* str)
{
int result  = 0;

   if (str != NULL) strcpy(str, gb_msg_progress_load_acquisition);
   if (gb_progress_load_acquisition > 0) 
      result = (int)((100 * gb_progress_load_acquisition) / (2 + 1 * gb_virtual_channel.size()));
   return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c ST_SaveFileAcquisition().

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c ST_LoadFileConfig()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la 
         funciÃ³n \c ST_LoadFileConfig()
*/
// -----------------------------------------------------------------------------
int ST_SaveFileAcquisition_Progress (char* str)
{
int result  = 0;

   if (str != NULL) strcpy(str, gb_msg_progress_save_acquisition);
   if (gb_progress_save_acquisition > 0) 
      result = (int)((100 * gb_progress_save_acquisition) / (3 + 2 * gb_virtual_channel.size()));
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_EnabledLogFile(int data)
{
    STDL_SetEnabled(data);
}
// -----------------------------------------------------------------------------
/**
 Abre la comunicaciÃ³n con el equipo Phased Array SITAU.

 @param[in]    path
               Ruta del directorio donde se encuentran los archivos del sistema:
               - base_vXrX.bit
               - foc8_vXrX.bit
               - boot_cpu1.bin
               - sitau.sc

 @param[in]    ip_address
               DirecciÃ³n IP del equipo Phased Array SITAU.\n
               El valor por defecto es '192.168.2.10'

 @param[in]    tcp_port_cfg
               Puerto de comunicaciones asociado al canal de control.\n
               El valor por defecto es '6002'.

 @param[in]    tcp_port_cfg
               Puerto de comunicaciones asociado al canal de envÃ­o de los datos
               de adquisiciÃ³n.\n
               El valor por defecto es '6008'.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
// 192.168.2.10 - 6002 - 6008
int ST_OpenSys (char* path_bin, char *ip_address, int tcp_port_cfg, int tcp_port_acq)
{
int result;
char str_info[255], full_path[255];
AnsiString str;

    UCIP.status_enabled = 1;
    UCIP.status_period_ms = 500;
    UCIP.status_pcie = 0;

    if (gb_msg != NULL)
    {
        delete gb_msg;
        gb_msg = NULL;
    }
    gb_msg = new CLogMessage();

    if (gb_plock_stop_acquisition_process != NULL)
    {
        delete gb_plock_stop_acquisition_process;
        gb_plock_stop_acquisition_process = NULL;
    }
    gb_plock_stop_acquisition_process = new CCriticalSection("Stop Acquisition Process");

    if (gb_plock_stop != NULL)
    {
        delete gb_plock_stop;
        gb_plock_stop = NULL;
    }
    gb_plock_stop = new CCriticalSection("Stop Process");

    if ((result = FS_DirectoryExists(path_bin, full_path)) < 0) return RLOG(result);
    if ((result = STHW_CreateXMLData(path_bin)) < 0)
    {
        strcpy_s(str_info, sizeof(str_info), "SITAU XML Config File doesn't exist");
        return ELOG(str_info, ES2_FILE_CFG);
    }
    if (gb_xml_sitau_hw != NULL)
    {
        UCIP.status_enabled = gb_xml_sitau_hw->data_.user_config.status_enabled;
        UCIP.status_period_ms = gb_xml_sitau_hw->data_.user_config.status_period_ms;
    }

    if (gb_opened_system == 1)
    {
        if (UCIP.status_enabled == 1) CreateStatusThread(UCIP.status_period_ms);
        ST_SetStatusConfig(UCIP.status_enabled, UCIP.status_period_ms, UCIP.status_pcie);
        return ES2_NONE;
    }
    ST_RegIni();
    STDL_Create();
    
    ST_InfoLibraryVersion(str_info);
    LOG_Open(str_info);

    if (log_protocol != NULL) delete log_protocol;
    log_protocol = new CLOG_PROTOCOL();

    if ((result = FS_DirectoryExists (path_bin, full_path)) < 0) return RLOG(result);

    gb_progress_open_sys = 0;
    //if ((result = ST_GetSystemInfo(path_bin)) < 0) return RLOG(result);

	ErrorLoadFirmware = 0;

    if ((result = TCP_MemoryResize()) < 0) return RLOG(result);

    if ((ErrorLoadFirmware = ST_OpenHW (path_bin, ip_address, tcp_port_cfg, tcp_port_acq)) < 0) ST_CloseSys();
    else
    {
      gb_progress_open_sys++;

      LOG_Message("Setting Emission Sampling Frequency...");
      gb_progress_open_sys++;

      LOG_Message("Setting Reception Sampling Frequency...");
      gb_progress_open_sys++;
   }

   if (ErrorLoadFirmware == 0)
   {
      ST_ResetTerminalLog();
      gb_progress_open_sys++;
      gb_progress_open_sys++;
      gb_opened_system = 1;

      LOG_Message("Setting Log Data Circular Buffer...");
      ST_SetSelectLogData_HWCircularBuffer (false);
      LOG_Message("Setting Log Data HW Circular Buffer...");
      ST_SetSelectLogData_AcqBufferCounter (false);
      LOG_Message("Setting Log Data Acq. Buffer Size...");
      ST_SetSelectLogData_AcqBufferSize (false);
      LOG_Message("Setting Log Data Acq. Buffer Number...");
      ST_SetSelectLogData_AcqBufferNumber (false);
      LOG_Message("Setting Log Data Buffer Counter...");
      ST_FP_SetSelectLogData_GetBufferCounter (false);
      ST_PA_SetSelectLogData_GetBufferCounter(false);
      gb_progress_open_sys++;

      LOG_Message("Setting Default Config...");
      ST_UpdateConfig();
      if ((result = ST_SetConfig()) < 0) return RLOG(result);
      gb_progress_open_sys++;

      if ((result = TCP_ClearReceive_ConfigPort()) != 0) return RLOG(result);

      if ((result = ST_SetPulseAmplitude(0)) != 0) return RLOG(result);      

      if (UCIP.status_enabled == 1) CreateStatusThread(UCIP.status_period_ms);
      ST_SetStatusConfig(UCIP.status_enabled, UCIP.status_period_ms, UCIP.status_pcie);
    }
    return ErrorLoadFirmware;
}
// -----------------------------------------------------------------------------
/**
 ST_OpenHW
 ==========
 Descripcion:
     Abre la comunicaciÃ³n con el sistema y configura la UCI.

 Parametros:


 Devuelve: (Ver codigos de error en 'sterr_v2.h')
*/
// -----------------------------------------------------------------------------
int ST_OpenHW (char* path, char *ip_address, int tcp_port_cfg, int tcp_port_acq)
{
int result;


    gb_progress_open_sys = 0;
    LOG_Message("Initiating internal registers...");

    if ((result = ST_VirtualChannelsDefault()) < 0) return RLOG(result);

   gb_progress_open_sys++;

   stVAR.hw_iniciado = true;

   if ((result = ST_ConfigSys(path, ip_address, tcp_port_cfg, tcp_port_acq)) < 0)
   {
      return RLOG(result);
   }

   gb_progress_open_sys++;

   gb_progress_open_sys++;
   gb_progress_open_sys++;
   gb_progress_open_sys++;
   gb_progress_open_sys++;
   gb_progress_open_sys++;

   LOG_Message("Updating Image Entry Point Coordinates...");
   gb_progress_open_sys++;

   LOG_Message("End Open Hardware");
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Cierra la comunicaciÃ³n con el equipo Phased Array SITAU.\n
 Esta funciÃ³n debe ser siempre ejecuta antes de finalizar la aplicaciÃ³n.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_CloseSys(void)
{
int result = ES2_NONE;

   //LOG_SetEnabledLastError(0);
    if ((result = ST_StopProcess()) < 0) { RLOG(result); }
    DeleteStatusThread();
    //ST_BufferDelete();
    if (gb_buffer_hw != NULL)
    {
        delete gb_buffer_hw;
        gb_buffer_hw = NULL;
    }
    if (gb_fp_buffer_acq != NULL)
    {
        delete gb_fp_buffer_acq;
        gb_fp_buffer_acq = NULL;
    }
    if (gb_pa_buffer_acq != NULL)
    {
        delete gb_pa_buffer_acq;
        gb_pa_buffer_acq = NULL;
    }
    if (gb_tfm_buffer_acq != NULL)
    {
        delete gb_tfm_buffer_acq;
        gb_tfm_buffer_acq = NULL;
    }

    stVAR.hw_iniciado = false;
    stVAR.hw_conectado = false;

    gb_opened_system = 0;

    TCP_CloseConnection();
    TCP_MemoryFree();

    ST_VirtualChannelsDestroy();
   //LOG_DeleteMsg();
    LOG_Delete();
    if (log_protocol != NULL)
    {
        delete log_protocol;
        log_protocol = NULL;
    }
    if (gb_msg != NULL)
    {
        delete gb_msg;
        gb_msg = NULL;
    }
    STDL_Delete();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PRG (bool update)
{
int result;
int FLChange=0;
bool print_base_config;
vector<unsigned long> vch_acq_size;

	if (!stVAR.hw_iniciado) return ELOG(charES2_NOINIT,ES2_NOINIT);
   print_base_config = true;//(result > 0);

	//Calcular el tamaÃ±o de la informaciÃ³n a recibir del buffer segÃºn el tipo de
	//configuraciÃ³n del sistema:
   if (gb_buffer_hw != NULL)
   {
      ST_GetFP_AcquisitionSize(vch_acq_size);
      if ((result = gb_buffer_hw->ImageSize (vch_acq_size)) < 0)
         return RLOG(result);
   }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Start (void)
{
int result;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    if (gb_no_hw)
    {
        gb_no_hardware_data_idx.assign(gb_no_hardware_data_idx.size(), 0);
        gb_no_hardware_fl_idx.assign(gb_no_hardware_fl_idx.size(), 0);
        //gb_no_hardware_trg_idx.assign(gb_no_hardware_trg_idx.size(), 0);
        for (int i=0; i<gb_no_hardware_data_enc.size(); i++) gb_no_hardware_data_enc[i] = 0;
    }

    switch (UCIP.trigger_source)
    {
    case TRG_SCAN_EXT:
        if ((result = ST_StartExternalTriggerScan()) < 0) return RLOG(result);
        break;
    default:
        if ((result = ST_StartPRFScan()) < 0) return RLOG(result);
        break;
    }
    //if (gb_no_hw)
    //{
    //    gb_no_hardware_data_idx.assign(gb_no_hardware_data_idx.size(), 0);
    //    gb_no_hardware_fl_idx.assign(gb_no_hardware_fl_idx.size(), 0);
    //    gb_no_hardware_trg_idx.assign(gb_no_hardware_trg_idx.size(), 0);
    //    for (int i=0; i<gb_no_hardware_data_enc.size(); i++)
    //        gb_no_hardware_data_enc[i] = 0;
    //}
    if (STDL_GetConfig())
    {
        ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
    }
   //if (gb_process_acq != NULL) gb_process_acq->Locked(false);
   //if (gb_process_acq_scan != NULL) gb_process_acq_scan->Locked(false);

   //timeout = new CTimeout(3000, MILISECONDS);
   //timeout->Start();
   //while (!timeout->Timeout() && gb_process_acq != NULL && gb_process_acq->GetStatus() == ST_ThreadAcq_Running && gb_process_acq_scan != NULL && gb_process_acq_scan->GetStatus() == ST_ThreadAcqScan_Running);
   //delete timeout;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Pause(void)
{
    int result;
    TMSG_Pause msg;
    bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

    msg.n_acquisitions = UCIP.n_images_burst;
    if ((result = TCPSend_MSG_PAUSE(&msg)) < 0) return RLOG(result);

    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Continue(void)
{
    int result;
    //bool locked = IsLockedStatusThread();

    if (!gb_no_hw)
    {
        //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

        if ((result = TCPSend_MSG_CONTINUE()) < 0) return RLOG(result);

        //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    }
    else gb_no_hardware_waiting_continue = false;
    return ES2_NONE;
}

// -----------------------------------------------------------------------------
/**
 Inicia el proceso de adquisiciÃ³n tomando como evento de disparo el periodo de un timer interno.\n
 Mientras se encuentre en este proceso de adquisiciÃ³n no es posible ningÃºn parÃ¡metro de configuraciÃ³n.\n
 La funciÃ³n \c ST_Stop() finaliza este proceso.

 @param[in]    prf_time
               (ms) Tiempo en milisegundos que transcurre entre disparo y disparo.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_StarAcquireProcess (void)
{
int result;
unsigned long prf_time;

    if (UCIP.status_pcie == 0)
    {
        if ((result = ST_MemoryResize()) < 0) return RLOG(result);

        if (gb_no_hw)
        {
            if (UCIP.trigger_source == TRG_SCAN_PRF) prf_time = UCIP.trigger_prf_time;
            else prf_time = 0;
            if ((gb_process_acq = new TThread_Acq(true, prf_time)) == NULL)
            {
                if ((result = ST_StopProcess()) < 0) { RLOG(result); }
                return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
            }
        }
        else
        {
            if ((gb_process_acq = new TThread_Acq(true)) == NULL)
            {
                if ((result = ST_StopProcess()) < 0) { RLOG(result); }
                return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
            }
        }

    //   gb_process_acq->Priority = tpLower; // set the priority lower than normal
       //gb_process_acq->Priority = tpNormal; // set the priority lower than normal
        gb_process_acq->Priority = tpHighest; // set the priority lower than normal
       //gb_process_acq->Priority = tpTimeCritical; // set the priority lower than normal
        gb_process_acq->Resume(); // now start the thread running

        if ((gb_process_acq_scan = new TThread_AcqScan (true)) == NULL)
        {
            if ((result = ST_StopProcess()) < 0) { RLOG(result); }
            return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
        }
        gb_process_acq_scan->Priority = tpNormal; // set the priority lower than normal
    //   gb_process_acq_scan->Priority = tpHighest; // set the priority lower than normal
    //   gb_process_acq_scan->Priority = tpTimeCritical; // set the priority lower than normal
        gb_process_acq_scan->Resume(); // now start the thread running

        //if (gb_no_hw)
        //{
        //    gb_no_hardware_data_idx.assign(gb_no_hardware_data_idx.size(), 0);
        //    gb_no_hardware_fl_idx.assign(gb_no_hardware_fl_idx.size(), 0);
        //    gb_no_hardware_trg_idx.assign(gb_no_hardware_trg_idx.size(), 0);
        //    for (int i=0; i<gb_no_hardware_data_enc.size(); i++) gb_no_hardware_data_enc[i] = 0;
        //}
    }
    gb_acquisition_process_stoped = false;

    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Inicia el proceso de adquisiciÃ³n tomando como evento de disparo el periodo de un timer interno.\n
 Mientras se encuentre en este proceso de adquisiciÃ³n no es posible ningÃºn parÃ¡metro de configuraciÃ³n.\n
 La funciÃ³n \c ST_Stop() finaliza este proceso.

 @param[in]    prf_time
               (ms) Tiempo en milisegundos que transcurre entre disparo y disparo.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_SetAcquisitionsNumber(unsigned long n_bursts, unsigned long images_bursts)
{
    float acq_time;
    int n_processing_signals;

   if (UCIP.n_bursts != n_bursts || UCIP.n_images_burst != images_bursts)
   {
       acq_time = gb_virtual_channel[0]->cfg_.prf_time * (float)1000 * (int)gb_virtual_channel[0]->cfg_.fp_focal_law.size();
       n_processing_signals = pow(2, gb_virtual_channel[0]->GET_LOG2_AVERAGE());
       acq_time *= n_processing_signals;
       UCIP.n_bursts = n_bursts;
       UCIP.n_images_burst = images_bursts;
       UCIP.trigger_acq_number = n_bursts * images_bursts;
       if (acq_time > 0) UCIP.delay_sync_n_acq = Round32(UCIP.delay_sync_time / acq_time);
       else UCIP.delay_sync_n_acq = 0;
       if (UCIP.delay_sync_n_acq >= UCIP.trigger_acq_number) UCIP.delay_sync_n_acq = UCIP.trigger_acq_number - 1;
       for (int i = 0; i < gb_virtual_channel.size(); i++)
       {
           if (gb_virtual_channel[i] != NULL) gb_virtual_channel[i]->UpdateAcquisitionSize();
       }
   }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Inicia el proceso de adquisiciÃ³n tomando como evento de disparo el periodo de un timer interno.\n
 Mientras se encuentre en este proceso de adquisiciÃ³n no es posible ningÃºn parÃ¡metro de configuraciÃ³n.\n
 La funciÃ³n \c ST_Stop() finaliza este proceso.

 @param[in]    prf_time
               (ms) Tiempo en milisegundos que transcurre entre disparo y disparo.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_StartPRFScan (void)
{
int result;
TMSG_TriggerSource msg;

    if ((result = ST_StopProcess()) < 0) { RLOG(result); }
    UCIP.trigger_source = TRG_SCAN_PRF;

   msg.trigger_source.REG = 0;
   msg.trigger_source.BIT.PRF_2 = 1;
   msg.prf_time = UCIP.trigger_prf_time / (float)1000.;
   msg.encoder_steps = 0;
   msg.n_acquisitions = UCIP.trigger_acq_number;
   msg.n_images_burst = UCIP.n_images_burst;
   msg.delay_sync_n_acq = UCIP.delay_sync_n_acq;
   msg.acquisition_mode = UCIP.acquisition_mode;
   msg.data_link_mode = UCIP.status_pcie;
   msg.pause_after_burst = UCIP.pause_after_burst;

   if ((result = TCPSend_MSG_IMAGE_ENABLED()) < 0) RLOG(result);
   if ((result = TCPSend_MSG_TriggerSource(&msg)) < 0) return RLOG(result);
   if ((result = ST_StarAcquireProcess()) < 0) return RLOG(result);
   LOG_Message("Acquiring PRF Trigger...");
   LOG_Info(gb_msg_progress_open_sys);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetStatusConfig(int enabled, int period_ms, int pcie)
{
    int result;
    TMSG_StatusConfig msg;
    bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    
    msg.enabled = enabled;
    msg.period_ms = period_ms;
    msg.pcie = pcie;

    if ((result = TCPSend_MSG_StatusConfig(&msg)) < 0) RLOG(result);
    UCIP.status_enabled = msg.enabled;
    UCIP.status_period_ms = msg.period_ms;
    UCIP.status_pcie = msg.pcie;

    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetStatusConfigPCIe(int pcie)
{
    int result;
    TMSG_StatusConfig msg;
    bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

    msg.enabled = UCIP.status_enabled;
    msg.period_ms = UCIP.status_period_ms;
    msg.pcie = pcie;

    if ((result = TCPSend_MSG_StatusConfig(&msg)) < 0) RLOG(result);
    UCIP.status_pcie = msg.pcie;

    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    LOG_Message("Setting Status PCIe = %d...", pcie);
    LOG_Info(gb_msg_progress_open_sys);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Inicia el proceso de adquisiciÃ³n tomando como evento de disparo el periodo de un timer sofware del PC.\n
 Mientras se encuentre en este proceso de adquisiciÃ³n no es posible ningÃºn parÃ¡metro de configuraciÃ³n.\n
 La funciÃ³n \c ST_Stop() finaliza este proceso.

 @param[in]    id_encoder
               Indice del encoder con el que se realiza la adquisiciÃ³n.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_StartExternalTriggerScan(void)
{
    int result;
    TMSG_TriggerSource msg;

    if ((result = ST_StopProcess()) < 0) { RLOG(result); }
    UCIP.trigger_source = TRG_SCAN_EXT;

    msg.trigger_source.REG = 0;
    msg.trigger_source.BIT.EXT = 1;
    msg.prf_time = UCIP.trigger_prf_time / (float)1000.;
    msg.encoder_steps = 0;
    msg.n_acquisitions = UCIP.trigger_acq_number;
    msg.n_images_burst = UCIP.n_images_burst;
    msg.delay_sync_n_acq = UCIP.delay_sync_n_acq;
    msg.acquisition_mode = UCIP.acquisition_mode;
    msg.data_link_mode = UCIP.status_pcie;
    msg.pause_after_burst = UCIP.pause_after_burst;

    if ((result = TCPSend_MSG_IMAGE_ENABLED()) < 0) RLOG(result);
    if ((result = TCPSend_MSG_TriggerSource(&msg)) < 0) return RLOG(result);
    if ((result = ST_StarAcquireProcess()) < 0) return RLOG(result);
    LOG_Message("Acquiring External Trigger...");
    LOG_Info(gb_msg_progress_open_sys);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Inicia el proceso de adquisiciÃ³n tomando como evento de disparo el periodo de un timer sofware del PC.\n
 Mientras se encuentre en este proceso de adquisiciÃ³n no es posible ningÃºn parÃ¡metro de configuraciÃ³n.\n
 La funciÃ³n \c ST_Stop() finaliza este proceso.

 @param[in]    id_encoder
               Indice del encoder con el que se realiza la adquisiciÃ³n.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_SetDelaySync(float data)
{
   float acq_time = gb_virtual_channel[0]->cfg_.prf_time * (float)1000 * (int)gb_virtual_channel[0]->cfg_.fp_focal_law.size();
   int n_processing_signals = pow(2, gb_virtual_channel[0]->GET_LOG2_AVERAGE());

   acq_time *= n_processing_signals;
   UCIP.delay_sync_time = data;
   if (acq_time > 0) UCIP.delay_sync_n_acq = Round32(data / acq_time);
   else UCIP.delay_sync_n_acq = 0;
   if (UCIP.delay_sync_n_acq >= UCIP.trigger_acq_number) UCIP.delay_sync_n_acq = UCIP.trigger_acq_number - 1;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_StopProcess (void)
{
CTimeout *timeout = NULL;
int result;
bool fin;
bool locked = IsLockedStatusThread();
   
    if (gb_plock_stop_acquisition_process != NULL && !gb_plock_stop_acquisition_process->Acquire(CS_WR))
        return ELOG(charES2_Acquire, ES2_Acquire);
    try
    {
        if (!gb_acquisition_process_stoped)
        {
            //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

            LOG_Message("Stoping Acquisition...");
            if ((result = TCPSend_MSG_IMAGE_DISABLED()) < 0) RLOG(result);
            if ((result = TCPSend_MSG_STOP()) < 0) RLOG(result);
            LOG_Message("Waiting for the SITAU to stop...");

            timeout = new CTimeout(3000, MILISECONDS);
            timeout->Start();
            fin = false;
            do
            {
                TCPSend_MSG_SHARED_MEM();
                Sleep(50);
            }
            while (gb_shared.cpu1.status != 0 && !timeout->Timeout());

            if (gb_shared.cpu1.status != 0)
            {
                LOG_Message("SITAU non stopped (Timeout)");                
                //return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
            }
            else LOG_Message("SITAU stopped");

            //LOG_Info("Stopped Time: %f ms", timeout->ElapsedTime_ms());

            if (timeout != NULL) delete timeout;
            timeout = NULL;
            gb_acquisition_process_stoped = true;

            //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
        }

        if (gb_process_acq_scan != NULL) gb_process_acq_scan->Locked(true);

        if (gb_process_acq != NULL)
        {
            gb_process_acq->Locked(true);
            gb_process_acq->Terminate();
            gb_process_acq->WaitFor();
            timeout = new CTimeout(3000, MILISECONDS);
            timeout->Start();

            fin = false;
            while (!fin)
            {
                if (gb_process_acq->End()) fin = true;
                else if (timeout->Timeout())
                    fin = true;
            }
            if (timeout != NULL) delete timeout;
            timeout = NULL;
            delete gb_process_acq;
            gb_process_acq = NULL;
        }
        if (gb_process_acq_scan != NULL)
        {
            gb_process_acq_scan->Terminate();
            gb_process_acq_scan->WaitFor();
            timeout = new CTimeout(3000, MILISECONDS);
            timeout->Start();
            fin = false;
            while (!fin)
            {
                if (gb_process_acq_scan->End()) fin = true;
                else if (timeout->Timeout())
                    fin = true;
            }
            if (timeout != NULL) delete timeout;
            timeout = NULL;
            delete gb_process_acq_scan;
            gb_process_acq_scan = NULL;
        }
        if (stVAR.hw_conectado == false) return ES2_NONE;
    }
    __finally
    {
        if (timeout != NULL) delete timeout;
        if (gb_plock_stop_acquisition_process != NULL) gb_plock_stop_acquisition_process->Release();
    }
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_IsStopped(void)
{
    return gb_shared.cpu1.status == 0 ? 1 : 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_Release (void)
{
int i;

   if (gb_buffer_hw != NULL) gb_buffer_hw->Release();
   for (i=0; i<(int)gb_virtual_channel.size(); i++)
   {
      if (gb_virtual_channel[i] != NULL) gb_virtual_channel[i]->Release();
   }
}
// -----------------------------------------------------------------------------
/**
 Finaliza cualquier proceso de adquisiciÃ³n y detiene el sistema.

 @return Ver cÃ³digos de error
*/
// -----------------------------------------------------------------------------
int ST_Stop (void)
{
int result;
bool locked = IsLockedStatusThread();

    if (gb_plock_stop != NULL && !gb_plock_stop->Acquire(CS_TRY))
        return ELOG(charES2_Acquire, ES2_Acquire);
    try
    {
        //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
        if ((result = ST_StopProcess()) < 0) return RLOG(result);
        if ((result = TCP_ClearReceive_AcquisitionPort()) < 0) return RLOG(result);
        LOG_Message("SITAU stopped");
        ST_Release();

        //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

        return ES2_NONE;
    }
    __finally
    {
        if (gb_plock_stop != NULL) gb_plock_stop->Release();
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Run (int wait)
{
int result;
int trigger_source = ST_GetTriggerSource();
bool locked = IsLockedStatusThread();

    if (UCIP.trigger_source == TRG_SCAN_ENC)
        return ELOG(charES2_SCAN_ENC,ES2_SCAN_ENC);
    if (!gb_no_hw)
    {
        //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
        if ((result = TCPSend_MSG_ACQUIRE(1)) < 0) return RLOG(result);
        //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
        return ES2_NONE;
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_PrintPASystemConfig(void)
{
int result;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    if ((result = TCPSend_MSG_PrintPASystem()) < 0) return RLOG(result);
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_PrintConfig(void)
{
   int result;
   bool locked = IsLockedStatusThread();

   //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

   if ((result = TCPSend_MSG_PrintConfig()) < 0) return RLOG(result);

   //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   ST_PrintFileConfig("VCH.TXT", 1);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_TerminalPrintVCH_Config(void)
{
   int result;
   bool locked = IsLockedStatusThread();

   //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);

   if ((result = TCPSend_MSG_PrintConfig()) < 0) return RLOG(result);
   ST_PrintFileConfig("VCH.TXT", 1);
   //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog(unsigned short data)
{
int result;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    if ((result = TCPSend_MSG_TERMINAL_LOG(data)) < 0) return RLOG(result);
    gb_terminal_log.REG = data;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog_PRG_REG(int data)
{
int result;
TMSG_TerminalLog terminal_log = gb_terminal_log;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    terminal_log.BIT.PRG_REG = data == 0 ? 0 : 1;
    if ((result = TCPSend_MSG_TERMINAL_LOG(terminal_log.REG)) < 0) return RLOG(result);
    gb_terminal_log = terminal_log;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetTerminalLog_PRG_REG(void)
{
   return gb_terminal_log.BIT.PRG_REG == 0 ? 0 : 1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog_ACQUIRING(int data)
{
int result;
TMSG_TerminalLog terminal_log = gb_terminal_log;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    terminal_log.BIT.ACQUIRING = data == 0 ? 0 : 1;
    if ((result = TCPSend_MSG_TERMINAL_LOG(terminal_log.REG)) < 0) return RLOG(result);
    gb_terminal_log = terminal_log;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetTerminalLog_ACQUIRING(void)
{
   return gb_terminal_log.BIT.ACQUIRING == 0 ? 0 : 1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog_ACQUIRING_FRAME(int data)
{
int result;
TMSG_TerminalLog terminal_log = gb_terminal_log;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    terminal_log.BIT.ACQUIRING_FRAME = data == 0 ? 0 : 1;
    if ((result = TCPSend_MSG_TERMINAL_LOG(terminal_log.REG)) < 0) return RLOG(result);
    gb_terminal_log = terminal_log;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetTerminalLog_ACQUIRING_FRAME(void)
{
   return gb_terminal_log.BIT.ACQUIRING_FRAME == 0 ? 0 : 1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog_PROTOCOL(int data)
{
int result;
TMSG_TerminalLog terminal_log = gb_terminal_log;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    terminal_log.BIT.PROTOCOL = data == 0 ? 0 : 1;
    if ((result = TCPSend_MSG_TERMINAL_LOG(terminal_log.REG)) < 0) return RLOG(result);
    gb_terminal_log = terminal_log;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetTerminalLog_PROTOCOL(void)
{
   return gb_terminal_log.BIT.PROTOCOL == 0 ? 0 : 1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetTerminalLog_BEAMFORMING(int data)
{
int result;
TMSG_TerminalLog terminal_log = gb_terminal_log;
bool locked = IsLockedStatusThread();

    //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    terminal_log.BIT.BEAMFORMING = data == 0 ? 0 : 1;
    if ((result = TCPSend_MSG_TERMINAL_LOG(terminal_log.REG)) < 0) return RLOG(result);
    gb_terminal_log = terminal_log;
    //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_GetTerminalLog_BEAMFORMING(void)
{
   return gb_terminal_log.BIT.BEAMFORMING == 0 ? 0 : 1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetCacheEnabled(void)
{
   int result;
   bool locked = IsLockedStatusThread();

   //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   if ((result = TCPSend_MSG_CACHE_ENABLED()) < 0) return RLOG(result);
   //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int ST_SetCacheDisabled(void)
{
   int result;
   bool locked = IsLockedStatusThread();

   //if (!locked && !LockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   if ((result = TCPSend_MSG_CACHE_DISABLED()) < 0) return RLOG(result);
   //if (!locked && !UnLockStatusThread()) return ELOG(charES2_TIMEOUT, ES2_TIMEOUT);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_MemoryResize (void)
{
int result = ES2_NONE, n_vch;

   //if ((result = ST_BufferCreate()) < 0) return RLOG(result);
   n_vch = (int)gb_virtual_channel.size();
   if (n_vch <= 0) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);
   if ((result = ST_EnsureHWBuffer()) < 0) return RLOG(result); // Propaga el fallo de memoria antes de usar el buffer.
   if (gb_fp_buffer_acq == NULL) gb_fp_buffer_acq = new CBufferAcq();
   if (gb_pa_buffer_acq == NULL) gb_pa_buffer_acq = new CBufferAcq();
   if (gb_tfm_buffer_acq == NULL) gb_tfm_buffer_acq = new CBufferAcq();

   if ((result = ST_VirtualChannels_MemoryResize(-1, gb_ge_buffer_n_acquisitions)) < 0) return RLOG(result);
   gb_no_hardware_data_idx.resize(gb_virtual_channel.size());
   gb_no_hardware_fl_idx.resize(gb_virtual_channel.size());
   gb_no_hardware_trg_idx.resize(gb_virtual_channel.size());
   gb_no_hardware_ascan_pulse_idx.resize(gb_virtual_channel.size());
   ST_MemoryClear();
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_MemoryClear(void)
{
CTimeout *timeout = NULL;
int result, acquiring = ST_Acquiring();

    if (acquiring == 1 && UCIP.trigger_source != TRG_SCAN_ENC)
    {
        if ((result = ST_Stop()) < 0) { RLOG(result); }
    }
   if (gb_process_acq_scan != NULL) gb_process_acq_scan->Locked(true);
   if (gb_process_acq != NULL) gb_process_acq->Locked(true);

   if (gb_buffer_hw != NULL) gb_buffer_hw->Clear_AcquisitionBuffer();
   BufferMC_HWUT_Clear();
   BufferPA_HWUT_Clear();
   BufferTFM_HWUT_Clear();

   if (gb_process_acq_scan != NULL)
   {
      gb_process_acq_scan->Locked(false);
   }
   if (gb_process_acq != NULL) gb_process_acq->Locked(false);

   if (acquiring == 1 && UCIP.trigger_source != TRG_SCAN_ENC) ST_Start();
}
// -----------------------------------------------------------------------------
/**
 Genera un evento de disparo, y realiza la adquisiciÃ³n de cada uno de
 los canales virtuales configurados.

 @return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int ST_FullAcquisition (int *fl_image_counter)
{
int i, counter = 0, result = 0;

   if (gb_virtual_channel.size() == 0) result = 1;
   else
   {
      for (i=0; i<gb_virtual_channel.size(); i++)
      {
         if (gb_virtual_channel[i] != NULL)
         {
            counter = gb_virtual_channel[i]->GET_AcquisitionCounter();
            if (  counter >= gb_virtual_channel[i]->cfg_.fp_focal_law.size())
               result = 1;
            else if ((counter > 0 && gb_virtual_channel[i]->GET_LastFocalLawAcquired() <= gb_virtual_channel[i]->cfg_.fp_focal_law.size() - 1))
            {
               counter = gb_virtual_channel[i]->GET_AcquisitionCounter();
               result = 1;
            }
            else 
               result = 0;
         }
      }
   }
   if (fl_image_counter != NULL) *fl_image_counter = counter;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Trigger (float timeout)
{
int i, acq_counter = 0, result = ES2_NONE;
bool fin = false;
CTimeout *timer = NULL;

   if (timeout > 0) timer = new CTimeout(timeout*1000, MILISECONDS);

try
{
   if (ST_Acquiring() == 0)
   {
      if ((result = ST_StarAcquireProcess()) < 0) return RLOG(result);
   }
   ST_VCH_RESET_AcquisitionCounter();
   if ((result = ST_Run(1)) < 0) return RLOG(result);
   if (timer != NULL) timer->Start();
   while (!fin)
   {
      if (gb_virtual_channel.size() == 0) fin = true;
      else
      {
         for (i=0; i<gb_virtual_channel.size(); i++)
         {
            if (gb_virtual_channel[i] != NULL)
            {
               acq_counter = gb_virtual_channel[i]->GET_AcquisitionCounter();
               if (acq_counter >= gb_virtual_channel[i]->cfg_.fp_focal_law.size())
                  fin = true;
               else 
                  fin = false;
            }
         }
         if (!fin && timer != NULL) fin = timer->Timeout();
      }
   }
}
finally
{
   if (timer != NULL) delete timer;
}
	return acq_counter;
}
unsigned long static gb_counter_continue = 0;
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_Acquire (void)
{
int i, j, result;
int n_data = 0;
bool acquiring = true;


    if (gb_no_hw)
    {
        if ((UCIP.pause_after_burst == 1 || UCIP.acquisition_mode == ACQUISITION_MODE_CONTINOUS) && UCIP.auto_continue_mode == 1) 
            acquiring = !gb_no_hardware_waiting_continue;

        if (acquiring)// || gb_no_hardware_waiting_continue_previous)
        {
            for (i = 0; i < gb_virtual_channel.size(); i++)
            {
                if (gb_virtual_channel[i] != NULL)
                {
                    switch (gb_virtual_channel[i]->GET_AcquisitionType())
                    {
                    case ACQUISITION_TYPE_MC:
                        for (j = 0; j < gb_virtual_channel[i]->cfg_.fp_focal_law.size(); j++)
                        {
                            if ((result = ST_MC_GetNoHardwareData(gb_virtual_channel[i], j)) < 0) return RLOG(result);
                            n_data = result;

                            if (n_data > 0 && gb_buffer_hw != NULL)
                            {
                                if ((result = gb_buffer_hw->Add((short int*)gb_buffer_acquisition, n_data)) < 0) return RLOG(result);
                                gb_buffer_hw->ReadSwap_AllBuffer();
                            }
                        }
                        gb_buffer_hw->IncreaseAcquiredImages();
                        break;
                    case ACQUISITION_TYPE_PA:
                        if ((result = ST_PA_GetNoHardwareData(gb_virtual_channel[i])) < 0) return RLOG(result);
                        n_data = result;

                        if (n_data > 0 && gb_buffer_hw != NULL)
                        {
                            if ((result = gb_buffer_hw->Add((short int*)gb_buffer_acquisition, n_data)) < 0) return RLOG(result);
                            gb_buffer_hw->ReadSwap_AllBuffer();
                            gb_buffer_hw->IncreaseAcquiredImages();
                        }
                        break;
                    case ACQUISITION_TYPE_TFM:
                        if ((result = ST_TFM_GetNoHardwareData(gb_virtual_channel[i])) < 0) return RLOG(result);
                        n_data = result;

                        if (n_data > 0 && gb_buffer_hw != NULL)
                        {
                            if ((result = gb_buffer_hw->Add((short int*)gb_buffer_acquisition, n_data)) < 0) return RLOG(result);
                            gb_buffer_hw->ReadSwap_AllBuffer();
                            gb_buffer_hw->IncreaseAcquiredImages();
                        }
                        break;
                    }
                    if (i < gb_no_hardware_trg_idx.size())
                    {
                        gb_no_hardware_trg_idx[i]++;
                        if (gb_no_hardware_trg_idx[i] > 8)
                            gb_no_hardware_trg_idx[i] = 0;
                    }
                }
            }
            gb_no_hardware_acq_burst_idx++;
            if (gb_no_hardware_acq_burst_idx % UCIP.n_images_burst == 0)
            {
                gb_no_hardware_waiting_continue = true;
                gb_counter_continue++;
            }
            gb_no_hardware_waiting_continue_previous = acquiring;
        }
    }
   else
   {
      if (TCP_Connected())
      {
         if ((n_data = TCPReceive_AcquisitionData (gb_buffer_acquisition, SIZE_BUFFER_ACQ)) < 0)
         {
            n_data = 0;
         }
         else n_data /= 2;
      }

      if (n_data > 0)
      {
         if (gb_buffer_hw != NULL && (result = gb_buffer_hw->Add((short int *)gb_buffer_acquisition, n_data)) < 0)
            return RLOG(result);
         if (gb_buffer_hw != NULL) gb_buffer_hw->ReadSwap_AllBuffer();
      }
      else if (n_data == 0 && gb_buffer_hw != NULL) gb_buffer_hw->ReadSwap_AllBuffer();
   }
   //if (img_size != NULL && gb_buffer_hw != NULL)
   //{
   //   if ((*img_size = gb_buffer_hw->MC_GetAcqImgSize()) == 0)
   //      *img_size = gb_buffer_hw->PA_GetAcqImgSize();
   //}
   return n_data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SITAUHardwareEnabled (int data)
{
//int result;

    //if (!gb_no_hw)
    //{
    //    if ((result = TCPSend_MSG_SITAUHardwareEnabled(data)) < 0) return RLOG(result);
    //}
    gb_hw_sitau_enabled = data;
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_ResetEthernetAcquisitionPort (void)
{
int result, acquiring = ST_Acquiring();;

    if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
    if ((result = TCP_ResetAcquisitionConnection()) < 0)
        return RLOG(result);
    if (acquiring == 1) ST_Start();
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_ConfigTerminalLog (   bool ACQUIRING,
                              bool ACQUIRING_FRAME,
                              bool PROTOCOL,
                              bool PRG_BASE_WMPAR,
                              bool PRG_MOD_WMPAR,
                              bool PRG_REG,
                              bool AMPLIA_SEND_COMMAND)
{
   TMSG_TerminalLog data;

   data.REG = 0;
   if (PRG_BASE_WMPAR) data.BIT.PRG_BASE_WMPAR = 1;
   if (PRG_MOD_WMPAR) data.BIT.PRG_MOD_WMPAR = 1;
   if (AMPLIA_SEND_COMMAND) data.BIT.AMPLIA_SEND_COMMAND = 1;
   if (PRG_REG) data.BIT.PRG_REG = 1;
   if (ACQUIRING) data.BIT.ACQUIRING = 1;
   if (ACQUIRING_FRAME) data.BIT.ACQUIRING_FRAME = 1;
   if (PROTOCOL) data.BIT.PROTOCOL = 1;
   ST_SetTerminalLog(data.REG);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_ResetTerminalLog (void)
{
   TMSG_TerminalLog data;

   data.REG = 0;
   ST_SetTerminalLog(data.REG);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadFirmware(char *file_name)
{
FILE *f = NULL;
char *buffer = NULL;
int result;
size_t size_file, read_size;

    if (gb_hw_sitau_enabled == 2)
    {
        ST_UpdateStatus();
        if (gb_shared.cpu1.flags.BIT.MINI_BASE_LOADED == 1)
            return ETCP_NONE;
    }
    if (file_name == NULL) return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
    try
    {
        if ((f = fopen(file_name, "rb")) != NULL)
        {
            fseek(f, 0, SEEK_END);
            size_file = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (size_file > 0)
            {
                if ((buffer = (char *)malloc(size_file+100)) != NULL)
                {
                    if ((read_size = fread(buffer, 1, size_file, f)) == size_file)
                    {
                        LOG_Message("Sending BASE Firmware...");
                        LOG_Info(gb_msg_progress_open_sys);
                        if ((result = TCPSend_MSG_LoadFirmware(buffer, size_file)) < 0) return RLOG(result);
                    }               
                }
            }         
        }
        else return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
    }
    __finally
    {
        if (f != NULL) fclose(f);
        if (buffer != NULL) free(buffer);
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SaveAcquisitionData (char *file)
{
FILE *f = NULL;
int result, idx_acq, idx_fp_acq, n_acq, n_fp_acq, n_fp_ascan, n_pa_ascan, n_fp_samples, n_pa_samples;
unsigned long n_data, fp_bscan_size, pa_bscan_size;
short int *fp_bscan = NULL, *pa_bscan = NULL;

try
{
   if (file != NULL) f = fopen(file, "wb");
   else f = fopen("ACQUISITION.DAT", "wb");

   if (f == NULL) return ELOG(charES2_File, ES2_File);

   if ((n_acq = ST_GetAcquisitionsNumber()) < 0) return RLOG(n_acq);

   // Acquisitions Number
   fwrite(&n_acq, sizeof(int), 1, f);

   // Recorre todas las adquisiciones
	for (idx_acq = 0; idx_acq<n_acq; idx_acq++)
  	{
      // ______________
      // FP ACQUISITION

      if ((n_fp_acq = ST_GetFocalLawNumber(0)) < 0) return RLOG(n_fp_acq);

      // FP Acquisitions Number
      fwrite(&n_fp_acq, sizeof(int), 1, f);

      for (idx_fp_acq=0; idx_fp_acq<n_fp_acq; idx_fp_acq++)
  	   {
         if ((n_fp_ascan = ST_MC_GetFocalLaw_AScanNumber(idx_fp_acq)) < 0) return RLOG(n_fp_ascan);
         if ((n_fp_samples = ST_MC_GetAScanDataNumber(-1)) < 0) return RLOG(n_fp_samples);
         
         fp_bscan_size = n_fp_ascan * n_fp_samples;
         if ((fp_bscan = (short *)malloc (sizeof(short) * fp_bscan_size)) == NULL) return ELOG(charES2_NO_MEM, ES2_NO_MEM);
         
         //if ((result = ST_GetBufferFP_FocalLaw_2 (fp_bscan, fp_bscan_size, idx_fp_acq, &n_data, idx_acq)) < 0) return RLOG(result);
         if ((result = ST_MC_GetBuffer_SingleImage(fp_bscan, fp_bscan_size, idx_fp_acq, idx_acq)) < 0) return RLOG(result);

         // FP Acquisition A-Scan Signals Number
         fwrite(&n_fp_ascan, sizeof(int), 1, f);

         // FP Acquisition Samples Number
         fwrite(&n_fp_samples, sizeof(int), 1, f);

         // FP Acquisition Data
         fwrite(fp_bscan, sizeof(short int), fp_bscan_size, f);
      }

      // ______________
      // PA ACQUISITION

      if ((n_pa_ascan = ST_PA_GetReceptionFocalLawNumber(-1)) < 0) return RLOG(n_pa_ascan);
      if ((n_pa_samples = ST_GetAScanDataNumber()) < 0) return RLOG(n_pa_samples);

      pa_bscan_size = n_pa_ascan * n_pa_samples;
      if ((pa_bscan = (short *)malloc (sizeof(short) * pa_bscan_size)) == NULL) return ELOG(charES2_NO_MEM, ES2_NO_MEM);

      if ((result = ST_PA_GetBuffer_LastImage(pa_bscan, pa_bscan_size, &n_data, NULL, NULL)) < 0) return RLOG(result);

      // PA Acquisition A-Scan Signals Number
      fwrite(&n_pa_ascan, sizeof(int), 1, f);

      // PA Acquisition Samples Number
      fwrite(&n_pa_samples, sizeof(int), 1, f);

      // PA Acquisition Data
      fwrite(pa_bscan, sizeof(short int), pa_bscan_size, f);
   }
}
__finally
{
   fclose(f);
   if (fp_bscan != NULL) free(fp_bscan);
   if (pa_bscan != NULL) free(pa_bscan);
}
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_NetConfigToFlash(unsigned char ip_part1,
   unsigned char ip_part2,
   unsigned char ip_part3,
   unsigned char ip_part4,
   unsigned char gw_part1,
   unsigned char gw_part2,
   unsigned char gw_part3,
   unsigned char gw_part4,
   unsigned char mask_part1,
   unsigned char mask_part2,
   unsigned char mask_part3,
   unsigned char mask_part4,
   unsigned char mac_part1,
   unsigned char mac_part2,
   unsigned char mac_part3,
   unsigned char mac_part4,
   unsigned char mac_part5,
   unsigned char mac_part6,
   unsigned short tcp_port,
   unsigned short udp_port,
   unsigned char dhcp)
{
   int result;
   struct network_parameters_s net_params;// , get_net_params;

   if ((result = HW_RDUserFlashNetConfig(&net_params)) < 0) return RLOG(result);

   //net_params.signature_head = 0xDA5E;
   net_params.ip_addr[0] = ip_part1;
   net_params.ip_addr[1] = ip_part2;
   net_params.ip_addr[2] = ip_part3;
   net_params.ip_addr[3] = ip_part4;

   net_params.gateway[0] = gw_part1;
   net_params.gateway[1] = gw_part2;
   net_params.gateway[2] = gw_part3;
   net_params.gateway[3] = gw_part4;

   net_params.mask[0] = mask_part1;
   net_params.mask[1] = mask_part2;
   net_params.mask[2] = mask_part3;
   net_params.mask[3] = mask_part4;

   net_params.mac_addr[0] = mac_part1;
   net_params.mac_addr[1] = mac_part2;
   net_params.mac_addr[2] = mac_part3;
   net_params.mac_addr[3] = mac_part4;
   net_params.mac_addr[4] = mac_part5;
   net_params.mac_addr[5] = mac_part6;

   net_params.conf_port = tcp_port;
   net_params.data_port = udp_port;

   net_params.options |= (dhcp & 0xFE);

   if ((result = HW_WRUserFlashNetConfig(&net_params)) < 0) return RLOG(result);

   return ES2_NONE;// RLOG(HW_RDConnectionConfig(&get_net_params));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetUserFlashNetConfig(char* ip_address, int port_config, int port_data, char* mac, int dhcp)
{
   int i, j, result, length = strlen(mac);
   struct sockaddr_in sa;
   unsigned char mac_addr[6] = {0};
   char str[3];

   for (i = j = 0; j < 6 && i < length; i++, j++)
   {
      if (i + 1 >= length) return ELOG(charES2_RANGO, ES2_RANGO);
      str[0] = mac[i]; i++;
      str[1] = mac[i]; i++;
      str[2] = 0;
      if (sscanf(str, "%x", &result) != 1 || result < 0 || result > 0xFF)
         return ELOG(charES2_RANGO, ES2_RANGO);
      mac_addr[j] = result;
   }
   if (j != 6) return ELOG(charES2_RANGO, ES2_RANGO);

   if ((result = inet_pton(AF_INET, ip_address, &(sa.sin_addr))) == 1)
   {
      if ((result = ST_NetConfigToFlash(sa.sin_addr.S_un.S_un_b.s_b1,
         sa.sin_addr.S_un.S_un_b.s_b2,
         sa.sin_addr.S_un.S_un_b.s_b3,
         sa.sin_addr.S_un.S_un_b.s_b4,
         sa.sin_addr.S_un.S_un_b.s_b1,
         sa.sin_addr.S_un.S_un_b.s_b2,
         sa.sin_addr.S_un.S_un_b.s_b3,
         1,
         255,
         255,
         255,
         0,
         mac_addr[5],
         mac_addr[4],
         mac_addr[3],
         mac_addr[2],
         mac_addr[1],
         mac_addr[0],
         port_config,
         port_data,
         dhcp)) < 0)
         return RLOG(result);
   }
   else return ELOG(charES2_RANGO, ES2_RANGO);
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetUserFlashNetConfig(char* ip_address, int* port_config, int* port_data, char* mac, int *dhcp)
{
   int result;
   struct network_parameters_s net_params;

   if ((result = HW_RDUserFlashNetConfig(&net_params)) < 0) return RLOG(result);

   if (ip_address != NULL)
      sprintf(ip_address, "%d.%d.%d.%d", net_params.ip_addr[3], net_params.ip_addr[2], net_params.ip_addr[1], net_params.ip_addr[0]);
   if (mac != NULL)
      sprintf(mac, "%X-%X-%X-%X-%X-%X", net_params.mac_addr[5], net_params.mac_addr[4], net_params.mac_addr[3], net_params.mac_addr[2], net_params.mac_addr[1], net_params.mac_addr[0]);
   if (port_config != NULL) *port_config = net_params.conf_port;
   if (port_data != NULL) *port_data = net_params.data_port;
   if (dhcp != NULL) *dhcp = net_params.options | 0x0001;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetActualNetConfig(char* ip_address, int* port_config, int* port_data, char* mac, int* dhcp)
{
   int result;
   struct network_parameters_s net_params;

   if ((result = HW_RDActualNetConfig(&net_params)) < 0)
      return RLOG(result);

   if (ip_address != NULL)
      sprintf(ip_address, "%d.%d.%d.%d", net_params.ip_addr[3], net_params.ip_addr[2], net_params.ip_addr[1], net_params.ip_addr[0]);
   if (mac != NULL)
      sprintf(mac, "%X-%X-%X-%X-%X-%X", net_params.mac_addr[5], net_params.mac_addr[4], net_params.mac_addr[3], net_params.mac_addr[2], net_params.mac_addr[1], net_params.mac_addr[0]);
   if (port_config != NULL) *port_config = net_params.conf_port;
   if (port_data != NULL) *port_data = net_params.data_port;
   if (dhcp != NULL) *dhcp = net_params.options | 0x0001;
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetStatus(unsigned __int64 *status_code, unsigned long* rd_address, unsigned long* wr_address, unsigned long* n_overflows, unsigned char* sitau_status, unsigned char* trigger_source, unsigned long* n_acquisitions, unsigned long* ind_acquisitions, unsigned long* size_buffer_command)
{
   int result = 0;

    if ((result = ST_UpdateStatus()) < 0) return RLOG(result);
    if (status_code != NULL) *status_code = gb_shared.cpu1.acquisition.status_code;
    if (rd_address != NULL) *rd_address = gb_shared.cpu1.acquisition.rd_address;
    if (wr_address != NULL) *wr_address = gb_shared.cpu1.acquisition.wr_address;
    if (n_overflows != NULL) *n_overflows = gb_shared.cpu1.acquisition.n_overflows;
    if (sitau_status != NULL) *sitau_status = gb_shared.cpu1.acquisition.sitau_status;
    if (trigger_source != NULL) *trigger_source = gb_shared.cpu1.acquisition.trigger_source;
    if (n_acquisitions != NULL) *n_acquisitions = gb_shared.cpu1.acquisition.n_acquisitions;
    if (ind_acquisitions != NULL) *ind_acquisitions = gb_shared.cpu1.acquisition.ind_acquisitions;
    if (size_buffer_command != NULL) *size_buffer_command = gb_shared.cpu1.acquisition.size_buffer_command;
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetStatusFrame(unsigned __int8* status_mem, int size_mem)
{
    return ST_GetStatusFrame_v1(status_mem, size_mem);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetStatusFrame_v1(unsigned __int8* status_mem, int size_mem)
{
    int idx = 0;
    unsigned long size_bytes;
    u32 version = SHRD_FRAME_VERSION;

    // Version
    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&version, size_bytes);
    idx += size_bytes;

    // General Status
    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.AFES_align_ok, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.LVDS_deskew_ok, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.errores_lvds, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.errores_ddr, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.codigo_pulser_leido, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.n_bases_bcc, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.retardo_extra_bcc, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.qsfp_channel_rdy_up, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&gb_shared.qsfp_channel_rdy_dn, (u8*)&status_mem[idx], size_bytes);
    idx += size_bytes;

    // CPU0 Status
    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu0.flags.REG, size_bytes);
    idx += size_bytes;

    size_bytes = SHRD_CPU0_INFO_SIZE * sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu0.info[0], size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu0.status, size_bytes);
    idx += size_bytes;

    // CPU1 Status
    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.flags.REG, size_bytes);
    idx += size_bytes;

    size_bytes = SHRD_CPU1_INFO_SIZE * sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.info[0], size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.status, size_bytes);
    idx += size_bytes;

    // CPU1 Status -> Acquisition
    size_bytes = sizeof(u64);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u64*)&status_mem[idx], (u64*)&gb_shared.cpu1.acquisition.status_code, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.cpu1.acquisition.sitau_status, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.acquisition.rd_address, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.acquisition.wr_address, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.acquisition.n_overflows, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.acquisition.n_acquisitions, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.cpu1.acquisition.ind_acquisitions, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u8);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u8*)&status_mem[idx], (u8*)&gb_shared.cpu1.acquisition.trigger_source, size_bytes);
    idx += size_bytes;

    // QSFP Status
    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.qsfp_link_errors_up.gtx_status_counters_reg_32, size_bytes);
    idx += size_bytes;

    size_bytes = sizeof(u32);
    if (idx + size_bytes > size_mem) return ELOG(charES2_RANGO, ES2_RANGO);
    memcpy((u32*)&status_mem[idx], (u32*)&gb_shared.qsfp_link_errors_dn.gtx_status_counters_reg_32, size_bytes);
    idx += size_bytes;

    return idx;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAvailable_PA(void)
{
#if defined(_PA)
    return 1;
#else
    return 0;
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetAvailable_TFM(void)
{
#if defined(_PA)
    return 1;
#else
    return 0;
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_UpdateStatus(void)
{
    int result;

    if (!IsRunningStatusThread())
    {
        if ((result = TCPSend_MSG_SHARED_MEM()) < 0) return RLOG(result);
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_MC_SetGEBufferSize(int n_acquisitions)
{
    gb_ge_buffer_n_acquisitions = n_acquisitions;
}
