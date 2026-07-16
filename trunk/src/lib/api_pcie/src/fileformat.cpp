// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <io.h>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include <vector>
#include <vcclr.h>  
#include <stdio.h>
#include <time.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fileformat.h"
#include "pcie_error_code.h"
#include "log.h"                                
#include "c_timeout.h"
#include "pcie_utils.h"
#include "api_pcie.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

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

   for (flag = false, i = (int)file_path.size() - 1; i > 0 && !flag; i--)
   {
      if (file_path[i] == '.') { flag = true; i++; }
      else if (file_path[i] == '\\') i = 0;
   }
   idx_dot = i;
   if (idx_dot > 0)
   {
      for (i = 0; i < idx_dot && i < file_path.size(); i++) result.insert(result.size(), 1, file_path[i]);
      result += extension;
   }
   else result = file_path + extension;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long FS_Write (TFileStream ^f, char *data, unsigned long n_data)
{
unsigned long n_written_bytes = 0;
AnsiString str;
cli::array<System::Byte> ^buffer_data = nullptr;//gcnew cli::array<System::Byte>(n_data);
pin_ptr<byte> ptr_buffer_data;// = &buffer_data[0];

   if (f != nullptr && data != NULL)// && buffer_data != nullptr)
   {

      buffer_data = gcnew cli::array<System::Byte>(n_data+100);
      ptr_buffer_data = &buffer_data[0];
      //System::Runtime::InteropServices::Marshal::Copy (  IntPtr((void *)data),
      //                                                   buffer_data,
      //                                                   0,
      //                                                   n_data);
      memcpy(ptr_buffer_data, data, n_data);
      try
      {
         f->Flush();
//         f->WriteAsync(buffer_data, 0, (System::Int32)n_data);
         f->Write(buffer_data, 0, (System::Int32)n_data);
         f->Flush();
      }
      catch(...)
      {	
		  ELOG("Exception FS_Write()", -1);
         if (buffer_data != nullptr) delete buffer_data;
         throw;
      }
      if (buffer_data != nullptr) delete buffer_data;
      n_written_bytes = n_data;
   }
   return n_written_bytes;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long FS_Read (TFileStream ^f, char *data, unsigned long n_data)
{
unsigned long n_read_bytes = 0;
cli::array<System::Byte> ^buffer_data = gcnew cli::array<System::Byte>(n_data);
pin_ptr<byte> ptr_buffer_data = &buffer_data[0];

   if (f != nullptr && data != NULL && buffer_data != nullptr)
   {
      n_read_bytes = (unsigned long)f->Read(buffer_data, 0, (System::Int32)n_data);
      memcpy(data, ptr_buffer_data, n_read_bytes);
   }
   return n_read_bytes;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int64 FS_Seek (TFileStream ^f, int64 offset, T_FS_SeekOrigin origin)
{
int64 result = 0;
System::IO::SeekOrigin file_origin;

   if (f != nullptr)
   {
      switch(origin)
      {
      case FS_SeekBegin:
         if (f->Position == offset) return f->Position;
         file_origin = System::IO::SeekOrigin::Begin;
         break;
      case FS_SeekCurrent: file_origin = System::IO::SeekOrigin::Current; break;
      case FS_SeekEnd: file_origin = System::IO::SeekOrigin::End; break;
      default: return result; break;
      }
      result = f->Seek(offset, file_origin);
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int64 FS_Size (TFileStream ^f)
{
   if (f != nullptr) return f->Length;
   else return 0;
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
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void FS_Flush (TFileStream ^f)
{
   if (f != nullptr) f->Flush();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void FS_Close(TFileStream^ f)
{
   if (f != nullptr) f->Close();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TFileStream^ FS_CreateFileImageData(char *file)
{
	int result, data_int, acquisition_number, n_fl;
	short int *fp_bscan = NULL, *pa_bscan = NULL;
	unsigned short data_ushort;
	char str[255];
	gcroot<TFileStream^> f;
	time_t t;

	if (file == NULL)
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}
	f = OpenFileStream(file, FS_Create);
	if (f.operator ->() == nullptr)
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}

	data_ushort = 0x1234;

	// Mascara Big-Little Endian
	data_ushort = 0x1234;
	if (FS_Write(f, (char*)&data_ushort, sizeof(unsigned short)) != sizeof(unsigned short))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}

	// Date, Time
	t = time(NULL);
	strftime(str, 20, "%d/%m/%Y %H:%M:%S", localtime(&t));
	data_int = 20;
	if (FS_Write(f, (char*)&data_int, sizeof(int)) != sizeof(int))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}
	if ((result = FS_Write(f, str, 20) != 20)) 
	{
		RLOG(result);
		return nullptr;
	}

	// Signal Range Max. Amplitude
	data_int = ST2_RangePA_MaxSignalAmplitude(0);
	if (FS_Write(f, (char*)&data_int, sizeof(int)) != sizeof(int))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}

	// Signal Range Min. Amplitude
	data_int = ST2_RangePA_MinSignalAmplitude(0);
	if (FS_Write(f, (char*)&data_int, sizeof(int)) != sizeof(int))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}

	// Number of acquisitions
	acquisition_number = ST2_GetAcquisitionsNumber();
	if (FS_Write(f, (char*)&acquisition_number, sizeof(int)) != sizeof(int))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}

	// Number of focal laws
	n_fl = ST2_GetFocalLawNumber();
	if (FS_Write(f, (char*)&n_fl, sizeof(int)) != sizeof(int))
	{
		ELOG(charEPCIE_File, EPCIE_File);
		return nullptr;
	}
	return f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FS_AddFileImageData(TFileStream^ f, short int *data, unsigned long n_data)
{
	int j, n_fl, n_ascan, n_samples, n_data_fl;
	unsigned long i_data;
	short int *fp_bscan = NULL, *pa_bscan = NULL;

	if (f == nullptr) return ELOG(charEPCIE_File, EPCIE_File);

	FS_Seek(f, 0, FS_SeekEnd);

	// Number of focal laws
	n_fl = ST2_GetFocalLawNumber();

	for (j = i_data = 0; j < n_fl; j++)
	{
		// Number of A-Scan signals
		n_ascan = ST2_MC_GetFocalLaw_AScanNumber(j);
		if (FS_Write(f, (char*)&n_ascan, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		// Number of the samples of the each A-Scan
		n_samples = ST2_GetAScanDataNumber();
		if (FS_Write(f, (char*)&n_samples, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		// Number of the samples of the focal law image
		n_data_fl = (int)(n_samples * n_ascan);
		if (FS_Write(f, (char*)&n_data_fl, sizeof(int)) != sizeof(int)) return ELOG(charEPCIE_File, EPCIE_File);

		if (i_data + n_data_fl > n_data) return ELOG(charEPCIE_DataInconsistency, EPCIE_DataInconsistency);

		// Acquisition data
		if (FS_Write(f, (char*)(data + i_data), n_data_fl * sizeof(short int)) != n_data_fl * sizeof(short int)) return ELOG(charEPCIE_File, EPCIE_File);
		i_data += n_data_fl;
	}
	return EPCIE_NONE;
}


// _____________________________________________________________________________
// ------ TFileStream Type Open Access
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TFileStream ^OpenFileStream (char *file, T_FS_OpenMode mode)
{
bool retry;
TFileStream ^result = nullptr;
System::String ^file_name = nullptr;
C_Timeout *timeout = NULL;
System::IO::FileMode file_mode;
System::IO::FileAccess file_access;
   
   switch (mode)
   {
   case FS_OpenRead: 
      if (!FS_FileReadAccess(file)) return nullptr;
      file_mode = System::IO::FileMode::Open;
      file_access = System::IO::FileAccess::Read;  
      break;
   case FS_OpenReadWrite: 
      if (!FS_FileReadWriteAccess(file)) return nullptr;
      file_mode = System::IO::FileMode::Open;
      file_access = System::IO::FileAccess::ReadWrite;  
      break;
   case FS_Create: 
      file_mode = System::IO::FileMode::Create;  
      file_access = System::IO::FileAccess::ReadWrite;  
      break;
   default: return result; break;
   }

   file_name = gcnew System::String(file);
   if ((timeout = new C_Timeout(300.)) == NULL) return result;

   timeout->Start();
   do
   {
      retry = false;
      try
      {
         if ((result = gcnew TFileStream(file_name, file_mode)) == nullptr)
         {
            if (timeout != NULL) delete timeout;
            timeout = NULL;
            if (timeout != nullptr) delete file_name; 
            file_name = nullptr;
            return result;
         }
      }
      catch (...)
      {
         if (timeout != NULL) delete timeout;
         timeout = NULL;
         if (timeout != nullptr) delete file_name; 
         file_name = nullptr;
         result = nullptr;
      }
   }
   while (retry && !timeout->Timeout());
   if (timeout != NULL) delete timeout;
   if (file_name != nullptr) delete file_name;
   return result;
}

