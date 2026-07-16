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

#include "fileformat.h"
#include "common_error_code.h"
#include "c_log_file.h"                                
#include "ctimer.h"
#include "utils.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FS_DirectoryExists(char* path, char* full_path, bool create)
{
   char str_error[255];
   LPVOID lpMsgBuf;
   DWORD error;
   int i, result = ECOM_NONE;
   vector<string> directories;
   string full_path_string;
   size_t position = 0, currentPosition = 0;
   struct stat buffer;
   string path_buffer;
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   if (path[0] == 0) path = "./";
   if (GetFullPathName(path, 255, full_path, NULL) == 0)
   {
      if ((error = GetLastError()) != 0)
      {
         FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR)&lpMsgBuf,
            0,
            NULL);
         _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "(Win32 Error Code: %d) %s", error, (char*)lpMsgBuf);
         return ELOG(str_error, ECOM_PathFile);
      }
      _snprintf_s(full_path, 255, _TRUNCATE, "%s", path);
   }

   _splitpath(full_path, drive, dir, fname, ext);
   full_path_string.insert(0, dir, strlen(dir));

   if (!stat(full_path, &buffer) == 0)
   {
      if (create)
      {
         while (currentPosition != -1)
         {
            currentPosition = full_path_string.find_first_of('\\', position);
            path_buffer = full_path_string.substr(position, currentPosition - position);
            if (path_buffer.length() > 0) directories.push_back(path_buffer);
            position = currentPosition + 1;
         }
         path_buffer.insert(0, drive, strlen(drive));
         path_buffer.insert(path_buffer.length(), "\\", strlen("\\"));
         if (directories.size() > 0)
         {
            for (i = 0; i < directories.size(); i++)
            {
               path_buffer.insert(path_buffer.length(), directories[i]);
               path_buffer.insert(path_buffer.length(), "\\", strlen("\\"));
               CreateDirectory(&path_buffer[0], NULL);
            }
         }
         if (strlen(fname) > 0)
         {
            path_buffer.insert(path_buffer.length(), fname, strlen(fname));
            path_buffer.insert(path_buffer.length(), "\\", strlen("\\"));
            CreateDirectory(&path_buffer[0], NULL);
         }
      }
      else
      {
         _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "'%s' directory doesn't exist", full_path);
         result = ELOG(str_error, ECOM_PathFile);
      }
   }
   result = stat(full_path, &buffer) == 0 ? 1 : 0;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileReadWriteAccess(char *file_name)  
{  
bool result = false;

   // Check for existence.  
   if (_access(file_name, 0 ) == 0)
   {  
      // Check for read-write permission.  
      if (_access(file_name, 6) == 0)  result = true;
   }
   return result;
} 
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileReadAccess(char *file_name)  
{  
bool result = false;

   // Check for existence.  
   if (_access(file_name, 0 ) == 0)
   {  
      // Check for read-write permission.  
      if (_access(file_name, 4) == 0)  result = true;
   }
   return result;
} 
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileWriteAccess(char *file_name)  
{  
bool result = false;

   // Check for existence.  
   if (_access(file_name, 0 ) == 0)
   {  
      // Check for read-write permission.  
      if (_access(file_name, 2) == 0)  result = true;
   }
   return result;
} 
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileAccess(char *file_name)  
{  
bool result = false;

   // Check for existence.  
   if (_access(file_name, 0 ) == 0)
   {  
      // Check for read-write permission.  
      if (_access(file_name, 6) == 0)  result = true;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileExists(char* file_name)
{
   return (_access(file_name, 0) >= 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool FS_FileExists(AnsiString file_name)
{
   return (_access(file_name.c_str(), 0) >= 0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString FS_ChangeFileExtension(AnsiString file_path, AnsiString extension)
{
   int i, idx_dot;
   AnsiString result = "";
   bool flag;

   for (flag = false, i = StrLength(file_path) - 1; i > 0 && !flag; i--)
   {
      if (file_path[i] == '.') { flag = true; i++; }
      else if (file_path[i] == '\\') i = 0;
   }
   idx_dot = i;
   if (idx_dot > 0)
   {
      for (i = 0; i < idx_dot && i < StrLength(file_path); i++) result.insert(StrLength(result), 1, file_path[i]);
      result += extension;
   }
   else result = file_path + extension;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString FS_GetPathFileName(AnsiString path)
{
   AnsiString result = path;
   size_t i = path.rfind("\\", path.length());
   i++;
   if (i != string::npos)
      result = path.substr(i, path.length() - i);
   return result;
}

// _____________________________________________________________________________
// ------ fstream Type Write Access
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int64 FS_Size(fstream& f)
{
    int64 bak_position, result = 0;

    if (!f.is_open()) return 0;
    bak_position = f.tellg();
    f.seekg(0, ios_base::end);
    result = f.tellg();
    f.seekg(bak_position);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void FS_Flush(fstream& f)
{
    if (!f.is_open()) return;
    f.flush();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint64 FS_Position(fstream& f)
{
    if (!f.is_open()) return 0;
    return f.tellg();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint64 FS_RDPosition(fstream& f)
{
    uint64 result = 0;

    if (f.is_open()) result = f.tellg();
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint64 FS_WRPosition(fstream& f)
{
    uint64 result = 0;

    if (f.is_open()) result = f.tellp();
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void FS_Close(fstream& f)
{
    if (!f.is_open()) return;
    try
    {
        f.exceptions(std::ofstream::badbit | std::ofstream::failbit | std::ofstream::eofbit);
        f.close();
    }
    catch (std::ofstream::failure& e)
    {
        ELOG((char*)e.what(), ECOM_PathFile);
    }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint64 FS_Write(fstream& f, char* data, uint64 n_data)
{
    uint64 i, block = 0, result = 0;
    bool flag_error;

    try
    {
        f.exceptions(std::ofstream::badbit | std::ofstream::failbit | std::ofstream::eofbit);
        for (i = 0, flag_error = false; !flag_error && i < n_data; i += block)
        {
            if (n_data - i < (uint64)FILE_SIZE_BLOCK) block = n_data - i;
            else block = (uint64)FILE_SIZE_BLOCK;
            //uint64 kk = f.tellg();
            f.write(data + i, block);
            //kk = f.tellg();
            if (!f.good())
            {
                flag_error = true;
                if (f.eof()) { ELOG("End-of-File reached on input operation", ECOM_PathFile); }
                if (f.fail()) { ELOG("Logical error on I/O operation", ECOM_PathFile); }
                if (f.bad()) { ELOG("Read/writing error on I/O operation", ECOM_PathFile); }
            }
        }
        result = i;
    }
    catch (std::ofstream::failure& e)
    {
        ELOG((char*)e.what(), ECOM_PathFile);
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
uint64 FS_Read(fstream& f, char* data, uint64 n_data)
{
    uint64 i, block = 0, result = 0;
    bool flag_error;

    try
    {
        f.exceptions(std::ofstream::badbit | std::ofstream::failbit | std::ofstream::eofbit);
        for (i = 0, flag_error = false; !flag_error && i < n_data; i += block)
        {
            if (n_data - i < (uint64)FILE_SIZE_BLOCK) block = n_data - i;
            else block = (uint64)FILE_SIZE_BLOCK;
            //uint64 kk = f.tellg();
            f.read(data + i, block);
            //kk = f.tellg();
            if (!f.good())
            {
                flag_error = true;
                if (f.eof()) { ELOG("End-of-File reached on input operation", ECOM_PathFile); }
                if (f.fail()) { ELOG("Logical error on I/O operation", ECOM_PathFile); }
                if (f.bad()) { ELOG("Read/writing error on I/O operation", ECOM_PathFile); }
            }
        }
        result = i;
    }
    catch (std::ofstream::failure& e)
    {
        ELOG((char*)e.what(), ECOM_PathFile);
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int64 FS_Seek(fstream& f, int64 offset, T_FS_SeekOrigin origin)
{
    int64 result = 0;
    ios_base::seekdir file_origin;

    if (!f.is_open()) return 0;
    switch (origin)
    {
    case FS_SeekBegin:
        if (f.tellp() == offset) return f.tellp();
        file_origin = ios_base::beg;
        break;
    case FS_SeekCurrent: file_origin = ios_base::cur; break;
    case FS_SeekEnd: file_origin = ios_base::end; break;
    default: return result; break;
    }

    try
    {
        f.seekp(offset, file_origin);
        result = f.tellp();
    }
    catch (...)
    {
        NEELOG();
        throw;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, char* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, AnsiString data)
{
    char data_char;
    unsigned long data_size = 1;
    uint64 n_bytes;
    uint64 n_data;

    n_data = (unsigned long)data.length() + 1;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = n_data;
        if (FS_Write(f, (char*)data.c_str(), n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, float* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<float>& data)
{
    char data_char;
    unsigned long data_size = sizeof(float);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(float) * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned char* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned char data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, float data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, double* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, double data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, short int* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<short int>& data)
{
    char data_char;
    unsigned long data_size = sizeof(short int);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(short int) * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<double>& data)
{
    char data_char;
    unsigned long data_size = sizeof(double);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = data_size * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, short int data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned short* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<unsigned short>& data)
{
    char data_char;
    unsigned long data_size = sizeof(unsigned short);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = data_size * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned short data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, int* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<int>& data)
{
    char data_char;
    unsigned long data_size = sizeof(int);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = data_size * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, int data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned int data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, bool data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned long* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, unsigned long data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, long* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size = sizeof((*data));
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
        n_bytes = sizeof((*data)) * n_data;
        if (FS_Write(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, long data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, int64 data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<int64>& data)
{
    char data_char;
    unsigned long data_size = sizeof(int64);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(int64) * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, uint64 data)
{
    char data_char;
    unsigned long data_size = sizeof(data);
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(data) * n_data;
        if (FS_Write(f, (char*)&data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteField(fstream& f, vector<char>& data)
{
    char data_char;
    unsigned long data_size = sizeof(char);
    uint64 n_data = (uint64)data.size();
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data > 0)
    {
        n_bytes = sizeof(char) * n_data;
        if (FS_Write(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int WriteFieldNull(fstream& f, unsigned long data_size)
{
    char data_char;
    uint64 n_data = 1;
    uint64 n_bytes;

    data_char = APPBIN_iCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size);
    if (FS_Write(f, (char*)&data_size, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data);
    if (FS_Write(f, (char*)&n_data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    data_char = APPBIN_fCAMPO;
    n_bytes = sizeof(data_char);
    if (FS_Write(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, char* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_bytes, n_data_file;
    int result = ECOM_NONE;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data)
        return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = result = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, AnsiString* data)
{
    char data_char;
    unsigned long data_size_file, data_size = 1;
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = data_size * n_data_file;
        data->resize(n_bytes);
        if (data->length() < n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
        if (FS_Read(f, (char*)data->c_str(), n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, float* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<float>& data)
{
    char data_char;
    unsigned long  data_size_file, data_size = sizeof(float);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(float) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<short int>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(short int);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(short int) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<double>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(double);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes)
        return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = data_size * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned char* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned char* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, float* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, double* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, double* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, short int* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, short int* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned short* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<unsigned short>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(unsigned short);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(unsigned short) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned short* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64  n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, int* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<int>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(int);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(int) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, int* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;
    short int data_short;
    int i;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size)
    {
        if (data_size_file == 2)
        {
            n_bytes = sizeof(n_data_file);
            if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
            if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

            if (n_data_file > 0)
            {
                n_bytes = sizeof(data_short) * n_data_file;
                for (i = 0; i < n_data_file; i++)
                {
                    if (FS_Read(f, (char*)&data_short, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
                    data[i] = data_short;
                }
            }

            n_bytes = sizeof(data_char);
            if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
            if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
            return ECOM_NONE;
        }
        else return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned int* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, bool* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;
    int data_int = 0;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size)
    {
        if (data_size_file == 4)
        {
            n_bytes = sizeof(n_data_file);
            if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
            if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

            if (n_data_file > 0)
            {
                n_bytes = data_size_file * n_data_file;
                if (FS_Read(f, (char*)&data_int, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
            }

            n_bytes = sizeof(data_char);
            if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
            if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
            (*data) = data_int == 1 ? true : false;
        }
        else return ELOG(charECOM_PathFile, ECOM_PathFile);
    }
    else
    {
        n_bytes = sizeof(n_data_file);
        if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
        if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

        if (n_data_file > 0)
        {
            n_bytes = sizeof((*data)) * n_data_file;
            if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
        }

        n_bytes = sizeof(data_char);
        if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
        if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned long* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, unsigned long* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, long* data, uint64 n_data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, long* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, int64* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<int64>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(int64);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(int64) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, uint64* data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof((*data));
    uint64 n_data_file, n_bytes, n_data = 1;

    if (data == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (n_data_file > n_data) return ELOG(charECOM_Parameter, ECOM_Parameter);

    if (n_data_file > 0)
    {
        n_bytes = sizeof((*data)) * n_data_file;
        if (FS_Read(f, (char*)data, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ReadField(fstream& f, vector<char>& data)
{
    char data_char;
    unsigned long data_size_file, data_size = sizeof(char);
    uint64 n_data_file, n_bytes;

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_iCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(data_size_file);
    if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_size_file != data_size) return ELOG(charECOM_PathFile, ECOM_PathFile);

    n_bytes = sizeof(n_data_file);
    if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);

    if (n_data_file > 0)
    {
        data.resize(n_data_file);
        n_bytes = sizeof(char) * n_data_file;
        if (FS_Read(f, (char*)&data[0], n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    }

    n_bytes = sizeof(data_char);
    if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes) return ELOG(charECOM_PathFile, ECOM_PathFile);
    if (data_char != APPBIN_fCAMPO) return ELOG(charECOM_PathFile, ECOM_PathFile);
    return ECOM_NONE;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PositionField(fstream& f, __int64* position, unsigned long data_size, bool error_log)
{
    char data_char;
    unsigned long data_size_file;
    uint64 n_bytes, n_data_file;
    int result = ECOM_NONE;
    __int64 start_position;

    start_position = f.tellg();
    if (position != NULL) *position = start_position;
    try
    {
        n_bytes = sizeof(data_char);
        if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }
        if (data_char != APPBIN_iCAMPO)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }

        n_bytes = sizeof(data_size_file);
        if (FS_Read(f, (char*)&data_size_file, n_bytes) != n_bytes)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }
        if (data_size_file != data_size)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }

        n_bytes = sizeof(n_data_file);
        if (FS_Read(f, (char*)&n_data_file, n_bytes) != n_bytes)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }

        if (n_data_file == 0 && position != NULL) *position = -1;
        uint64 kk = (uint64)f.tellg();
        f.seekg((uint64)f.tellg() + ((uint64)n_data_file * (uint64)data_size));
        kk = (uint64)f.tellg();

        n_bytes = sizeof(data_char);
        if (FS_Read(f, (char*)&data_char, n_bytes) != n_bytes)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }
        if (data_char != APPBIN_fCAMPO)
        {
            if (!error_log) return result = ECOM_PathFile;
            else return result = ELOG(charECOM_PathFile, ECOM_PathFile);
        }
    }
    finally
    {
        if (result < 0)
        {
            f.seekg(start_position);
            if (position != NULL) *position = -1;
        }
    }
    return result;
}
