// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_log_file.h"
#include "c_datalogger.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CLogFile gb_log_default;
vector<CLogFile*> gb_log_file;
int gb_log_enabled = 1;
string gb_log_file_path = "";
extern int gb_last_error_code = 0;
extern string gb_last_error_msg = "";

// -----------------------------------------------------------------------------
// USER INFORMATION ============================================================

int LOG_STATUS = -1;

void LOG_Open(char* version)
{
    if (STDL_GetEnabled() == 1)
    {
        gb_log_default.Default();
        gb_log_default.IDThread(GetCurrentThreadId());

        LOG_STATUS = LOG_Create(LOG_STATUS_FILE_NAME, LOG_STATUS_NAME, version);
    }
}

// -----------------------------------------------------------------------------
// USER INFORMATION ============================================================

// _____________________________________________________________________________
// ------ CLogFile class Methods
// _____________________________________________________________________________
// =============================================================================
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
CLogFile::CLogFile(char* file, char *name, char *version, bool file_opened, bool lock_enabled)
{
    error_ = Create(file, name, version, file_opened, lock_enabled);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CLogFile::~CLogFile()
{
    Close();
    if (plock_ != nullptr)
    {
        plock_->Release();
        delete plock_;
    }
    plock_ = nullptr;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogFile::Default(void)
{
    error_ = Create(LOG_DEFAULT_FILE_NAME, LOG_DEFAULT_NAME, NULL, false, true);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogFile::Close(void)
{
    char str[255];
    time_t t;

    if (created_)// && error_ >= 0)
    {
        created_ = false;
        if (fs_ != nullptr || (fs_ = fopen(log_file_path_.c_str(), "at")) != nullptr)
        {
            t = time(nullptr);
            strftime(str, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
            fprintf(fs_, "\n\n-----------------------------------------------------------------");
            fprintf(fs_, "\nCLOSE LOG FILE - ");
            if (name_ != "") fprintf(fs_, "(%s) - ", name_.c_str());
            fprintf(fs_, "%s", str);
            fflush(fs_);
            fclose(fs_);
            fs_ = nullptr;
        }
    }

    if (fs_ != nullptr)
    {
        fflush(fs_);
        fclose(fs_);
        fs_ = nullptr;
    }
    created_ = false;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CLogFile::Create(char* file, char* name, char* version, bool file_opened, bool lock_enabled)
{
    char str[255];
    time_t t;

    error_ = 0;
    carriage_return_ = false;
    id_thread_ = -1;
    file_opened_ = file_opened;
    GetCurrentDirectoryA(255, str);
    log_file_path_ = string(str) + "\\";
    if (name != nullptr) name_ = string(name);
    else name_ = "";
    if (lock_enabled) plock_ = new CCriticalSection("CLogFile");
    else plock_ = nullptr;
    last_error_msg_ = "";
    last_error_code_ = 0;
    last_error_enabled_ = true;
    fs_ = nullptr;
    created_ = false;

    if (file != nullptr)
    {
        file_name_ = string(file);
        log_file_path_ += file_name_;
        if ((fs_ = fopen(log_file_path_.c_str(), "wt")) != nullptr)
        {
            t = time(nullptr);
            strftime(str, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
            fprintf(fs_, "OPEN LOG FILE - ");
            if (name_ != "") fprintf(fs_, "(%s) - ", name_.c_str());
            fprintf(fs_, "%s", str);
            if (version != nullptr) fprintf(fs_, "\nLibrary Version: v%s", version);
            fprintf(fs_, "\n-----------------------------------------------------------------\n");
            fflush(fs_);
            if (!file_opened_)
            {
                fclose(fs_);
                fs_ = nullptr;
            }
            created_ = true;
        }
    }
    else return -1;
    return 0;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CLogFile::FileOpened(bool data)
{
    int result = 0;

    file_opened_ = data;
    if (file_opened_)
    {
        if (fs_ == nullptr) result = (fs_ = fopen(log_file_path_.c_str(), "at")) != nullptr ? 0 : -1;
    }
    else
    {
        if (fs_ != nullptr)
        {
            fclose(fs_);
            fs_ = nullptr;
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogFile::Info(char* msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);
    Info(msg, argptr);
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CLogFile::Info(char* msg, va_list argptr)
{
    char str[255];
    time_t t;

    if (msg != nullptr && error_ >= 0)
    {
        if (fs_ != nullptr || (fs_ = fopen(log_file_path_.c_str(), "at")) != nullptr)
        {
            time(&t);
            strftime(str, sizeof(str), "%d/%m/%Y %H:%M:%S", localtime(&t));
            if (carriage_return_) fprintf(fs_, "\n");
            fprintf(fs_, "\n[%s]: ", str);
            vfprintf(fs_, msg, argptr);
            fflush(fs_);
            if (!file_opened_)
            {
                fclose(fs_);
                fs_ = nullptr;
            }
            carriage_return_ = false;
        }
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
template <typename T_Generic>
T_Generic CLogFile::Error(char* msg_err, T_Generic code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    LPVOID lpMsgBuf = nullptr;
    LPVOID lpDisplayBuf = nullptr;
    DWORD error;
    char str[255];
    time_t t;
    int code = (int)code_parameter;

    if (code < 0)
    {
        error = GetLastError();
        if (error != 0)
        {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
            lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, lstrlen((LPCTSTR)lpMsgBuf) * sizeof(TCHAR));
        }

        if (plock_ != nullptr)
        {
            if (!plock_->Acquire(CS_TRY))
                return code_parameter;
        }
        if (fs_ != nullptr || (fs_ = fopen(log_file_path_.c_str(), "at")) != nullptr)
        {
            time(&t);
            strftime(str, sizeof(str), "%d/%m/%Y %H:%M:%S", localtime(&t));
            fprintf(fs_, "\n\n[%s]: ", str);
            fprintf(fs_, "(App Error Code:  %d) ", code);
            if (iteration >= 0) fprintf(fs_, " --> Iteration: %d ", iteration);
            if (msg_err != nullptr) fprintf(fs_, msg_err);
            if (error != 0) fprintf(fs_, "\n                       (Win32 Error Code: %d) %s", error, (char*)lpDisplayBuf);
            if (file != nullptr)
            {
                file = (strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file);
                fprintf(fs_, "\n                       FILE:     %s ", file);
            }
            if (line != 0) fprintf(fs_, "\n                       LINE:     %d ", line);
            if (function != nullptr) fprintf(fs_, "\n                       FUNCTION: %s ", function);
            fflush(fs_);
            if (!file_opened_)
            {
                fclose(fs_);
                fs_ = nullptr;
            }
            if (error != 0)
            {
                LocalFree(lpMsgBuf);
                LocalFree(lpDisplayBuf);
            }
            SetLastError(0);
            last_error_code_ = code;
            if (msg_err != NULL) last_error_msg_ = string(msg_err);
            carriage_return_ = true;
        }
        if (plock_ != nullptr) plock_->Release();
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//void CLogFile::Error(T_LOG_ErrorRegister *data)
//{
//    int i, j, k;
//    char* file;
//
//    if (data == NULL) return;
//    if (fs_ != nullptr || (fs_ = fopen(log_file_path_.c_str(), "at")) != nullptr)
//    {
//        for (i = 0; i < data->e_idx && i < LOG_ERROR_SIZE; i++)
//        {
//            fprintf(fs_, "\n\n---> [%d]: %s", i, data->e[i].msg);
//            //for (k = 0; data->e[i].msg[k] != 0 && i < LOG_ERROR_MSG_SIZE; k++)
//            //    fprintf(fs_, "%c", data->e[i].msg[k]);
//            for (j = 0; j < data->e[i].s_idx && j < LOG_ERROR_STACK_SIZE; j++)
//            {
//                fprintf(fs_, "\n     Error code: %d", data->e[i].s[j].code);
//                file = strrchr(data->e[i].s[j].file, '\\');
//                if (file == NULL) file =  data->e[i].s[j].file;
//                fprintf(fs_, "\n     File:       %s ", file);
//                fprintf(fs_, "\n     Line:       %d ", data->e[i].s[j].line);
//                //fprintf(f, "\n   File:       ");
//                //for (k = 0; data->e[i].s[j].file[k] != 0 && i < LOG_ERROR_FILE_SIZE; k++)
//                //    fprintf(f, "%c", data->e[i].s[j].file[k]);
//            }
//        }
//        fflush(fs_);
//        if (!file_opened_)
//        {
//            fclose(fs_);
//            fs_ = nullptr;
//        }
//    }
//}

// _____________________________________________________________________________
// ------ A C C E S S   F U N C T I O N S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Close(void)
{
    int i;

    gb_log_default.Close();
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL) gb_log_file[i]->Close();
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Delete(void)
{
    int i;

    LOG_Close();
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL)
        {
            //gb_log_file[i]->Close();
            delete gb_log_file[i];
            gb_log_file[i] = NULL;
        }
    }
    gb_log_file.clear();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_Exists(char* file)
{
    int i;

    if (gb_log_default.FileName() == string(file)) return LOG_DEFAULT_ID;
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL && gb_log_file[i]->FileName() == string(file)) return i;
    }
    return -1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_Exists(int id_thread)
{
    int i;

    if (gb_log_default.IDThread() == id_thread) return LOG_DEFAULT_ID;
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL && gb_log_file[i]->IDThread() == id_thread) return i;
    }
    return -1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_Create(char* file, char* name, char* version)
{
    int result = -1;

    if ((result = LOG_Exists(file)) < 0)
    {
        gb_log_file.push_back(new CLogFile(file, name, version));
        result = gb_log_file.size() - 1;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_IDThread(char* file, int id)
{
    int i;

    if (gb_log_default.FileName() == string(file))
    {
        gb_log_default.IDThread(id);
        return 0;
    }
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL && gb_log_file[i]->FileName() == string(file))
        {
            gb_log_file[i]->IDThread(id);
            return 0;
        }
    }
    return -1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_Error(int id, char* msg_err, int code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) 
            gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float LOG_Error(int id, char* msg_err, float code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
double LOG_Error(int id, char* msg_err, double code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long LOG_Error(int id, char* msg_err, unsigned long code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long long LOG_Error(int id, char* msg_err, unsigned long long code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
long long LOG_Error(int id, char* msg_err, long long code_parameter, int iteration, unsigned long line, char* file, char* function)
{
    if (id < 0)
    {
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    else
    {
        if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Error(msg_err, code_parameter, iteration, line, file, function);
        else gb_log_file[id]->Error(msg_err, code_parameter, iteration, line, file, function);
    }
    return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_LastError(char* msg, int id)
{
    int result = 0;

    if (id > 0 && id < gb_log_file.size() && gb_log_file[id] != NULL)
    {
        result = gb_log_file[id]->LastErrorCode();
        strcpy(msg, gb_log_file[id]->LastErrorMsg().c_str());
    }
    else
    {
        result = gb_log_default.LastErrorCode();
        strcpy(msg, gb_log_default.LastErrorMsg().c_str());
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//void LOG_Register(T_LOG_ErrorRegister* data, int id)
//{
//    if (id > 0 && id < gb_log_file.size() && gb_log_file[id] != NULL) gb_log_file[id]->Error(data);
//    else gb_log_default.Error(data);
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_SetLastErrorEnabled(bool data)
{
    int i;

    gb_log_default.LastErrorEnabled(data);
    for (i = 0; i < gb_log_file.size(); i++)
    {
        if (gb_log_file[i] != NULL) gb_log_file[i]->LastErrorEnabled(data);
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Info(char* msg, ...)
{
    va_list argptr;
    int id;

    va_start(argptr, msg);
    //if (id < 0)
    //{
        id = LOG_Exists(GetCurrentThreadId());
        if (id < 0 || id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Info(msg, argptr);
        else gb_log_file[id]->Info(msg, argptr);
    //}
    //else
    //{
    //    if (id == LOG_DEFAULT_ID || id >= gb_log_file.size()) gb_log_default.Info(msg, argptr);
    //    else gb_log_file[id]->Info(msg, argptr);
    //}
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Info(int id, char* msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);
    if (id == LOG_DEFAULT_ID || id >= gb_log_file.size() || gb_log_file[id] == NULL) gb_log_default.Info(msg, argptr);
    else gb_log_file[id]->Info(msg, argptr);
    va_end(argptr);
}
