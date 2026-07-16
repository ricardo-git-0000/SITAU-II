// -----------------------------------------------------------------------------
/**
 @file ut_acq_file.cpp

 @brief Este archivo contiene la implementaciÃ³n de las funciones de acceso a los archivos de registro de los datos de adquisiciÃ³n

 @author (rg) Ricardo GonzÃ¡lez

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 02/29/20 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <io.h>
#include <cstring>
#include <fstream>
#include <iostream>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_acq_file_v1.h"
#include "stlib2.h"
#include "c_buffer_ptr.h"
#include "c_log_file.h"
#include "st_error_code.h"
#include "c_acq_data.h"
#include "st_load.h"
#include "st_reg.h"
#include "utils.h"
#include "fileformat.h"
#include "c_virtual_channel.h"
#include "st_config.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

unsigned int gb_progress_file_acquisition_v1 = 0;
unsigned int gb_progress_file_acquisition_max_v1 = 1;
char gb_msg_progress_file_acquisition_v1[255] = "";

unsigned int gb_progress_trigger_update_scan_data_v1 = 0;
unsigned int gb_progress_auxiliar_update_scan_data_v1;
char gb_msg_progress_update_scan_data_v1[255] = "";

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   F U N C I O N E S
// =============================================================================

// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c UT_LoadFileAcquisition().

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c UT_LoadFileConfig()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la
         funciÃ³n \c UT_LoadFileConfig()
*/
// -----------------------------------------------------------------------------
int UT_FileAcquisition_Progress_v1(char* str, int* progress)
{
    int result = 0;

    if (progress != NULL) *progress = Round32((float)100. * gb_progress_file_acquisition_v1 / gb_progress_file_acquisition_max_v1);
    if (str != NULL) strcpy(str, gb_msg_progress_file_acquisition_v1);
    return result;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el porcentaje del procesamiento que ya ha sido realizado por la funciÃ³n \c UT_LoadFileAcquisition().

 @return Porcentaje de procesamiento realizado por la funciÃ³n \c UT_LoadFileConfig()
         El valor mÃ¡ximo devuelto es 100, es cuando finaliza la ejecuciÃ³n de la
         funciÃ³n \c UT_LoadFileConfig()
*/
// -----------------------------------------------------------------------------
int ST_UpdateScanData_Progress_v1(char* str, int* progress_auxiliar, int* progress_trigger)
{
    int result = 0;

    if (progress_auxiliar != NULL) *progress_auxiliar = gb_progress_auxiliar_update_scan_data_v1;
    if (progress_trigger != NULL) *progress_trigger = gb_progress_trigger_update_scan_data_v1;
    if (str != NULL) strcpy(str, gb_msg_progress_update_scan_data_v1);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int64 ST_SaveConfigData_v1(fstream& f)
{
    int result;
    int64 buffer_size;
    vector<char> buffer;

    if ((result = ST_SaveXMLFileConfig(XML_FILE_TMP_v1)) < 0) return RLOG(result);
    fstream f_cfg(XML_FILE_TMP_v1, ios_base::in | ios_base::binary);
    if ((buffer_size = FS_Size(f_cfg)) > 0)
    {
        buffer.resize(buffer_size);
        if (FS_Read(f_cfg, &buffer[0], buffer_size) == buffer_size)
        {
            if ((result = WriteField(f, buffer)) < 0) return RLOG(result);
        }
    }
    return (int64)f.tellp();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SaveFileAcquisition_v1(char* file)
{
    int ch, result, data_int;
    float data_float;
    unsigned short data_ushort;
    AnsiString ansi_str;
    char str[255], firmware_ethernet[255], firmware_uci_sw[255], firmware_uci_hw[255],firmware_module[255];// firmware_base[255], firmware_boot[255],
    time_t t;
    //int minor;//branch, major
    int64 result_int64;

    fstream f(file, ios_base::out | ios_base::binary);
    if (!f.is_open()) return ELOG(charES2_FILE, ES2_FILE);

    try
    {
        _snprintf_s(gb_msg_progress_file_acquisition_v1, sizeof(gb_msg_progress_file_acquisition_v1), _TRUNCATE, "Saving Acquisition Data '%s'...", file);
        gb_progress_file_acquisition_v1 = 0;
        gb_progress_file_acquisition_max_v1 = 14;

        // _____________________________________________________________________________
        // ----- Mascara Big-Little Endian

        data_ushort = 0x1234;
        if (FS_Write(f, (char*)&data_ushort, (uint64)sizeof(unsigned short)) != (uint64)sizeof(unsigned short)) return ELOG(charES2_FILE, ES2_FILE);

        // _____________________________________________________________________________
        // ----- Configuration File Version

        if ((result = WriteField(f, AnsiString(BINFILE_VERSION_v1))) < 0) return RLOG(result);

        // _____________________________________________________________________________
        // ----- DATE, TIME

        t = time(NULL);
        strftime(str, FILE_SIZE_DATE, "%d/%m/%Y %H:%M:%S", localtime(&t));
        if ((result = WriteField(f, AnsiString(str))) < 0) return RLOG(result);

        // _____________________________________________________________________________
        // ----- APIUT Version

        ansi_str = AnsiString(_VERSION_DLL_MAJOR) + "." + AnsiString(_VERSION_DLL_MINOR) + "." + AnsiString(_VERSION_DLL_RELEASE) + "." + AnsiString(_VERSION_DLL_BUILD) + " (" + AnsiString(_VERSION_DLL_DATE) + ")";
        if ((result = WriteField(f, ansi_str)) < 0) return RLOG(result);

        // _____________________________________________________________________________
        // ----- Firmware Version

        if ((result = ST_InfoFirmwareVersion(firmware_ethernet, firmware_uci_sw, firmware_uci_hw, firmware_module)) < 0) return (result);
        if ((result = WriteField(f, AnsiString(firmware_ethernet))) < 0) return RLOG(result);
        if ((result = WriteField(f, AnsiString(firmware_ethernet))) < 0) return RLOG(result);
        if ((result = WriteField(f, AnsiString(firmware_uci_sw))) < 0) return RLOG(result);
        if ((result = WriteField(f, AnsiString(firmware_uci_hw))) < 0) return RLOG(result);
        if ((result = WriteField(f, AnsiString(firmware_module))) < 0) return RLOG(result);

        // _____________________________________________________________________________
        // ----- Acquisition Config

        if ((data_int = ST_GetScanLinesTrigger()) < 0) return RLOG(data_int);
        if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

        if ((data_int = ST_GetScanLinesAuxiliar()) < 0) return RLOG(data_int);
        if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

        if ((data_float = ST_GetScanResolutionTrigger()) < 0) return RLOG(Round32(data_float));
        if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

        if ((data_float = ST_GetScanLengthTrigger()) < 0) return RLOG(Round32(data_float));
        if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

        if ((data_float = ST_GetScanResolutionAuxiliar()) < 0) return RLOG(Round32(data_float));
        if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

        if ((data_float = ST_GetScanLengthAuxiliar()) < 0) return RLOG(Round32(data_float));
        if ((result = WriteField(f, data_float)) < 0) return RLOG(result);

        if ((data_int = ST_GetTriggerSource()) < 0) return RLOG(data_int);
        if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

        // _____________________________________________________________________________
        // ----- Virtual Channels Data

        gb_progress_file_acquisition_v1++;

        data_int = (int)gb_virtual_channel.size();
        if ((result = WriteField(f, data_int)) < 0) return RLOG(result);

        for (ch = 0; ch < gb_virtual_channel.size(); ch++)
        {
            if (gb_virtual_channel[ch] != NULL)
            {
                if ((result = gb_virtual_channel[ch]->SaveFileAcquisition(f)) < 0) return RLOG(result);
                gb_progress_file_acquisition_v1++;
            }
        }

        gb_progress_file_acquisition_v1++;

        if ((result_int64 = ST_SaveConfigData_v1(f)) < 0) return RLOG((int)result_int64);

        return ES2_NONE;
    }
    __finally
    {
        f.close();        
    }
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadFilePositions_v1(fstream &f, T_FilePositionData_v1* file_position)
{
    int i, result, n_fl, data_int;//n_samples
    float data_float;
    vector<float> vector_float;
    unsigned long long data_uint64;
    //unsigned long data_uint32;

    if (file_position == NULL) return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);

    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if ((result = ReadField(f, &n_fl)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, vector_float)) < 0) return RLOG(result);

    if ((result = ReadField(f, &data_uint64)) < 0) return RLOG(result);
    file_position->file_raw_data.clear();
    if (data_uint64 > 0)
    {
        if (n_fl > 0 && n_fl < 256)
        {
            file_position->file_raw_data.resize(n_fl);
            for (i=0; i< file_position->file_raw_data.size(); i++)
                PositionField(f, &file_position->file_raw_data[i], sizeof(short int), false);
        }
    }

    gb_progress_file_acquisition_v1++;

    if ((result = ReadField(f, &data_uint64)) < 0) return RLOG(result);
    if (data_uint64 > 0) PositionField(f, &file_position->file_acq_counter, sizeof(unsigned long), false);
    else file_position->file_acq_counter = -1;

    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadConfigData_v1(fstream& f)
{
    int result;
    vector<char> buffer;

    if ((result = ReadField(f, buffer)) < 0) return RLOG(result);
    if ((result = ST_LoadXMLBufferConfig(buffer)) < 0) return RLOG(result);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadFileAcquisitionData_v1(fstream& f)
{
    int i, result, data_int;// , n_aux_lines;// , n_sw_gates;// , n_encoders;//n_trg_lines, n_hw_gates, j
    float data_float;
    AnsiString ansi_str;
    vector<short int> data;
    vector<T_FilePositionData_v1> file_pos;
    int64 file_position;
    CBufferMC_ScanHWUT* buffer;

    // _____________________________________________________________________________
    // ----- DATE, TIME

    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);

    // _____________________________________________________________________________
    // ----- STLIB Version

    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);

    // _____________________________________________________________________________
    // ----- Firmware Version

    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);
    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);
    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);
    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);
    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);
    if ((result = ReadField(f, &ansi_str)) < 0) return RLOG(result);

    // _____________________________________________________________________________
    // ----- Acquisition Config

    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_float)) < 0) return RLOG(result);
    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);

    // _____________________________________________________________________________
    // ----- Virtual Channels Data

    gb_progress_file_acquisition_v1++;

    if ((result = ReadField(f, &data_int)) < 0) return RLOG(result);
    if (data_int <= 0) return ELOG(charES2_FILE, ES2_FILE);
    file_pos.resize(data_int);
    for (i = 0; i < data_int; i++)
    {
        if ((result = ST_LoadFilePositions_v1(f, &file_pos[i])) < 0) return RLOG(result)
    }

    gb_progress_file_acquisition_v1++;

    if ((file_position = ST_LoadConfigData_v1(f)) < 0) return RLOG((int)file_position);

    gb_progress_file_acquisition_v1++;

    if ((result = ST_MemoryResize()) < 0) return RLOG(result)

    //if ((result = UT_SetConfig()) < 0) return RLOG(result)

    gb_progress_file_acquisition_v1++;

    for (i = 0; i < gb_virtual_channel.size(); i++)
    {
        if (gb_virtual_channel[i] != NULL && gb_virtual_channel[i]->buffer_mc_hwut_ != NULL && i < file_pos.size())
        {
            buffer = gb_virtual_channel[i]->buffer_mc_hwut_;
            //if (file_pos[i].file_raw_data.size() != buffer->buffer_fl_.size()) return ELOG(charES2_DataInconsistency, ES2_DataInconsistency);

            gb_progress_file_acquisition_v1++;

            // Raw Data
            //if (buffer->buffer_fl_.size() > 0)
            //{
            //    for (j = 0; j < buffer->buffer_fl_.size(); j++)
            //    {
            //        if (file_pos[i].file_raw_data[j] > 0)
            //        {
            //            if ((result = buffer->buffer_fl_[j]->SetFileData(f, file_pos[i].file_raw_data[j], buffer->buffer_fl_[j]->GetBufferCapacity(), sizeof(short int))) < 0)
            //                return RLOG(result);
            //            buffer->buffer_fl_[j]->UpdateDataNumberLast();
            //        }
            //    }
            //}

            gb_progress_file_acquisition_v1++;

            // Acquisition Counter
            if (file_pos[i].file_acq_counter > 0 && buffer->buffer_counter_ != NULL)
            {
                if ((result = buffer->buffer_counter_->SetFileData(f, file_pos[i].file_acq_counter, buffer->buffer_counter_->GetBufferCapacity(), sizeof(unsigned long))) < 0)
                    return RLOG(result);
            }
        }
    }
    return ES2_NONE;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadFileAcquisition_v1(char* file)
{
    int result, n_bytes;
    unsigned short data_ushort;
    AnsiString ansi_str;
    int acquiring = ST_Acquiring();

    fstream f(file, ios_base::in | ios_base::binary);
    if (!f.is_open()) return ELOG(charES2_FILE, ES2_FILE);

    try
    {
        if (acquiring == 1 && (result = ST_Stop()) < 0) { RLOG(result); }
        _snprintf_s(gb_msg_progress_file_acquisition_v1, sizeof(gb_msg_progress_file_acquisition_v1), _TRUNCATE, "loading Acquisition Data '%s'...", file);
        gb_progress_file_acquisition_v1 = 0;
        gb_progress_file_acquisition_max_v1 = 17;

        // _____________________________________________________________________________
        // ----- Mascara Big-Little Endian

        n_bytes = sizeof(data_ushort);
        if (FS_Read(f, (char*)&data_ushort, n_bytes) != n_bytes) return ELOG(charES2_FILE, ES2_FILE);
        if (data_ushort != 0x1234) return ELOG(charES2_FILE, ES2_FILE);

        // _____________________________________________________________________________
        // ----- Configuration File Version

        if ((result = ReadField(f, &ansi_str)) < 0) return ELOG(charES2_FILE, ES2_FILE);
        if (strcmp(ansi_str.c_str(), BINFILE_VERSION_v1) == 0)
        {
            if ((result = ST_LoadFileAcquisitionData_v1(f)) < 0) return RLOG(result);
        }
        else return ELOG(charES2_FILE, ES2_FILE);

        return ES2_NONE;
    }
    __finally
    {
        FS_Close(f);
        if (acquiring == 1) ST_Start();
    }
    return ES2_NONE;
}
