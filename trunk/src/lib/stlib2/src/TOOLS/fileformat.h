#ifndef fileformatH
#define fileformatH

// _____________________________________________________________________________
// ------ C O M P I L E R   L I B R A R I E S
// =============================================================================

#include <vector>
#include <stdio.h>

// _____________________________________________________________________________
// ------ P R O J E C T   L I B R A R I E S
// =============================================================================

#include "tad.h"

using namespace std;

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// Caracteres para el formato del archivo de configuración.
#define APPBIN_iCAMPO    			'['
#define APPBIN_fCAMPO    			']'
#define FILE_SIZE_DATE				20
#define FILE_SIZE_BLOCK				1024 * 1024 * 1024

typedef enum T_FS_OpenMode
{
   FS_OpenRead        = 0,
   FS_OpenReadWrite   = 1,
   FS_Create          = 2
} T_FS_OpenMode;

typedef enum T_FS_SeekOrigin
{
   FS_SeekBegin     = 0,
   FS_SeekCurrent   = 1,
   FS_SeekEnd       = 2
} T_FS_SeekOrigin;

// _____________________________________________________________________________
// ------ D A T A   T Y P E
// =============================================================================

// _____________________________________________________________________________
// ------ F U N C T I O N   P R O T O T Y P E
// =============================================================================

// _____________________________________________________________________________
// ------ File Access Utils
// =============================================================================

int FS_DirectoryExists (char *path, char *full_path, bool create=false);

AnsiString FS_GetPathFileName(AnsiString path);

bool FS_FileReadAccess(char *file_name);

bool FS_FileReadWriteAccess(char *file_name);

bool FS_FileWriteAccess(char *file_name);

bool FS_FileAccess(char *file_name);

bool FS_FileExists(char *file_name);

bool FS_FileExists(AnsiString file_name);

AnsiString FS_ChangeFileExtension(AnsiString file_path, AnsiString extension);

AnsiString FS_GetPathFileName(AnsiString path);

// _____________________________________________________________________________
// ------ fstream Type Write Access
// =============================================================================

int64 FS_Size(fstream& f);
void FS_Flush(fstream& f);
uint64 FS_Position(fstream& f);
uint64 FS_RDPosition(fstream& f);
uint64 FS_WRPosition(fstream& f);
void FS_Close(fstream& f);
uint64 FS_Write(fstream& f, char* data, uint64 n_data);
uint64 FS_Read(fstream& f, char* data, uint64 n_data);
int64 FS_Seek(fstream& f, int64 offset, T_FS_SeekOrigin origin);

int WriteField(fstream& f, char* data, uint64 n_data);
int WriteField(fstream& f, AnsiString data);
int WriteField(fstream& f, float* data, uint64 n_data);
int WriteField(fstream& f, vector<float>& data);
int WriteField(fstream& f, unsigned char* data, uint64 n_data);
int WriteField(fstream& f, unsigned char data);
int WriteField(fstream& f, float data);
int WriteField(fstream& f, double* data, uint64 n_data);
int WriteField(fstream& f, double data);
int WriteField(fstream& f, short int* data, uint64 n_data);
int WriteField(fstream& f, vector<short int>& data);
int WriteField(fstream& f, vector<double>& data);
int WriteField(fstream& f, short int data);
int WriteField(fstream& f, unsigned short* data, uint64 n_data);
int WriteField(fstream& f, vector<unsigned short>& data);
int WriteField(fstream& f, unsigned short data);
int WriteField(fstream& f, int* data, uint64 n_data);
int WriteField(fstream& f, vector<int>& data);
int WriteField(fstream& f, int data);
int WriteField(fstream& f, unsigned int data);
int WriteField(fstream& f, bool data);
int WriteField(fstream& f, unsigned long* data, uint64 n_data);
int WriteField(fstream& f, unsigned long data);
int WriteField(fstream& f, long* data, uint64 n_data);
int WriteField(fstream& f, long data);
int WriteField(fstream& f, int64 data);
int WriteField(fstream& f, vector<int64>& data);
int WriteField(fstream& f, uint64 data);
int WriteField(fstream& f, vector<char>& data);
int WriteFieldNull(fstream& f, unsigned long data_size);

int ReadField(fstream& f, char* data, uint64 n_data);
int ReadField(fstream& f, AnsiString* data);
int ReadField(fstream& f, float* data, uint64 n_data);
int ReadField(fstream& f, vector<float>& data);
int ReadField(fstream& f, vector<short int>& data);
int ReadField(fstream& f, vector<double>& data);
int ReadField(fstream& f, unsigned char* data, uint64 n_data);
int ReadField(fstream& f, unsigned char* data);
int ReadField(fstream& f, float* data);
int ReadField(fstream& f, double* data, uint64 n_data);
int ReadField(fstream& f, double* data);
int ReadField(fstream& f, short int* data, uint64 n_data);
int ReadField(fstream& f, short int* data);
int ReadField(fstream& f, unsigned short* data, uint64 n_data);
int ReadField(fstream& f, vector<unsigned short>& data);
int ReadField(fstream& f, unsigned short* data);
int ReadField(fstream& f, int* data, uint64 n_data);
int ReadField(fstream& f, vector<int>& data);
int ReadField(fstream& f, int* data);
int ReadField(fstream& f, unsigned int* data);
int ReadField(fstream& f, bool* data);
int ReadField(fstream& f, unsigned long* data, uint64 n_data);
int ReadField(fstream& f, unsigned long* data);
int ReadField(fstream& f, long* data, uint64 n_data);
int ReadField(fstream& f, long* data);
int ReadField(fstream& f, int64* data);
int ReadField(fstream& f, vector<int64>& data);
int ReadField(fstream& f, uint64* data);
int ReadField(fstream& f, vector<char>& data);
int PositionField(fstream& f, __int64* position, unsigned long data_size, bool error_log);

#endif
