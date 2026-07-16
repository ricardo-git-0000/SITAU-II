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

#include "pcie_tad.h"

using namespace std;

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// Caracteres para el formato del archivo de configuración.
#define APPBIN_iCAMPO    			'['
#define APPBIN_fCAMPO    			']'

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

TFileStream ^OpenFileStream (char *file, T_FS_OpenMode mode);//, bool locked_exception=true);

unsigned long FS_Write (TFileStream ^f, char *data, unsigned long n_data);

unsigned long FS_Read (TFileStream ^f, char *data, unsigned long n_data);

int64 FS_Seek (TFileStream ^f, int64 offset, T_FS_SeekOrigin origin);

int64 FS_Size (TFileStream ^f);

void FS_Flush (TFileStream ^f);

void FS_Close (TFileStream^ f);

TFileStream^ FS_CreateFileImageData(char *file);

int FS_AddFileImageData(TFileStream^ f, short int *data, unsigned long n_data);

// _____________________________________________________________________________
// ------ FILE Write Type Access
// =============================================================================

int WriteField (FILE *f, char *data, unsigned long n_data);

int WriteField (FILE *f, vector<unsigned char> &data);

int WriteField (FILE *f, unsigned char data);

int WriteField (FILE *f, AnsiString data);

int WriteField (FILE *f, float *data, unsigned long n_data);

int WriteField (FILE *f, vector<float> &data);

int WriteField (FILE *f, float data);

int WriteField (FILE *f, double *data, unsigned long n_data);

int WriteField (FILE *f, double data);

int WriteField (FILE *f, short int *data, unsigned long n_data);

int WriteField (FILE *f, vector<short int> &data);

int WriteField (FILE *f, unsigned short *data, unsigned long n_data);

int WriteField (FILE *f, vector<unsigned short> &data);

int WriteField (FILE *f, unsigned short data);

int WriteField (FILE *f, int *data, unsigned long n_data);

int WriteField (FILE *f, vector<int> &data);

int WriteField (FILE *f, int data);

int WriteField (FILE *f, bool data);

int WriteField (FILE *f, unsigned long *data, unsigned long n_data);

int WriteField (FILE *f, unsigned long data);

int WriteField (FILE *f, unsigned long long data);

int WriteField (FILE *f, long *data, unsigned long n_data);

// _____________________________________________________________________________
// ------ FILE Read Type Access
// =============================================================================

int ReadField (FILE *f, char *data, unsigned long n_data);

int ReadField (FILE *f, AnsiString *data);

int ReadField (FILE *f, float *data, unsigned long n_data);

int ReadField (FILE *f, vector<float> &data);

int ReadField (FILE *f, unsigned char *data, unsigned long n_data);

int ReadField (FILE *f, vector<unsigned char> &data);

int ReadField (FILE *f, unsigned char *data);

int ReadField (FILE *f, float *data);

int ReadField (FILE *f, double *data, unsigned long n_data);

int ReadField (FILE *f, double *data);

int ReadField (FILE *f, short int *data, unsigned long n_data);

int ReadField (FILE *f, short int *data);

int ReadField (FILE *f, vector<short int> &data);

int ReadField (FILE *f, unsigned short *data, unsigned long n_data);

int ReadField (FILE *f, vector<unsigned short> &data);

int ReadField (FILE *f, unsigned short *data);

int ReadField (FILE *f, int *data, unsigned long n_data);

int ReadField (FILE *f, vector<int> &data);

int ReadField (FILE *f, int *data);

int ReadField (FILE *f, unsigned int *data);

int ReadField (FILE *f, bool *data);

int ReadField (FILE *f, unsigned long *data, unsigned long n_data);

int ReadField (FILE *f, unsigned long *data);

int ReadField (FILE *f, unsigned long long *data);

int ReadField (FILE *f, long *data, unsigned long n_data);

int ReadField (FILE *f, long *data);

int ReadField (FILE *f, int64 *data);

int ReadField (FILE *f, vector<int64> &data);

// _____________________________________________________________________________
// ------ TFileStream Type Write Access
// =============================================================================

int64 FS_Seek (TFileStream ^f, int64 offset, T_FS_SeekOrigin origin);

int WriteField (TFileStream ^f, char *data, unsigned long n_data);

int WriteField (TFileStream ^f, AnsiString data);

int WriteField (TFileStream ^f, float *data, unsigned long n_data);

int WriteField (TFileStream ^f, vector<float> &data);

int WriteField (TFileStream ^f, unsigned char *data, unsigned long n_data);

int WriteField (TFileStream ^f, unsigned char data);

int WriteField (TFileStream ^f, float data);

int WriteField (TFileStream ^f, double *data, unsigned long n_data);

int WriteField (TFileStream ^f, double data);

int WriteField (TFileStream ^f, short int *data, unsigned long n_data);

int WriteField (TFileStream ^f, vector<short int> &data);

int WriteField(TFileStream^ f, vector<double>& data);

int WriteField (TFileStream ^f, short int data);

int WriteField (TFileStream ^f, unsigned short *data, unsigned long n_data);

int WriteField (TFileStream ^f, vector<unsigned short> &data);

int WriteField (TFileStream ^f, unsigned short data);

int WriteField (TFileStream ^f, int *data, unsigned long n_data);

int WriteField (TFileStream ^f, vector<int> &data);

int WriteField (TFileStream ^f, int data);

int WriteField (TFileStream ^f, unsigned int data);

int WriteField (TFileStream ^f, bool data);

int WriteField (TFileStream ^f, unsigned long *data, unsigned long n_data);

int WriteField (TFileStream ^f, unsigned long data);

int WriteField (TFileStream ^f, long *data, unsigned long n_data);

int WriteField (TFileStream ^f, long data);

int WriteField (TFileStream ^f, int64 data);

int WriteField (TFileStream ^f, vector<int64> &data);

int WriteField (TFileStream ^f, uint64 data);

// _____________________________________________________________________________
// ------ TFileStream Type Write Access
// =============================================================================

int ReadField (TFileStream ^f, char *data, unsigned long n_data);

int ReadField (TFileStream ^f, AnsiString *data);

int ReadField (TFileStream ^f, float *data, unsigned long n_data);

int ReadField (TFileStream ^f, vector<float> &data);

int ReadField (TFileStream ^f, unsigned char *data, unsigned long n_data);

int ReadField (TFileStream ^f, unsigned char *data);

int ReadField (TFileStream ^f, float *data);

int ReadField (TFileStream ^f, double *data, unsigned long n_data);

int ReadField (TFileStream ^f, double *data);

int ReadField (TFileStream ^f, short int *data, unsigned long n_data);

int ReadField (TFileStream ^f, vector<short int> &data);

int ReadField (TFileStream ^f, vector<double> &data);

int ReadField (TFileStream ^f, short int *data);

int ReadField (TFileStream ^f, unsigned short *data, unsigned long n_data);

int ReadField (TFileStream ^f, vector<unsigned short> &data);

int ReadField (TFileStream ^f, unsigned short *data);

int ReadField (TFileStream ^f, int *data, unsigned long n_data);

int ReadField (TFileStream ^f, vector<int> &data);

int ReadField (TFileStream ^f, int *data);

int ReadField (TFileStream ^f, unsigned int *data);

int ReadField (TFileStream ^f, bool *data);

int ReadField (TFileStream ^f, unsigned long *data, unsigned long n_data);

int ReadField (TFileStream ^f, unsigned long *data);

int ReadField (TFileStream ^f, long *data, unsigned long n_data);

int ReadField (TFileStream ^f, long *data);

int ReadField (TFileStream ^f, int64 *data);

int ReadField (TFileStream ^f, vector<int64> &data);

int ReadField (TFileStream ^f, uint64 *data);

#endif
