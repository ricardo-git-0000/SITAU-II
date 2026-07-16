#define NOMINMAX

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <windows.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>
#include <string.h>
#include <io.h>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "log.h"
#include "pcie_utils.h"
#include "fileformat.h"
#include "c_critical_section.h"
#include "pcie.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define LOG_FILE_NAME "LOG_PCIe.TXT"
#define LOG_THREAD_FILE_NAME "LOG_Thread.TXT"

// ____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ============================================================================

string gb_log_file = ""; 
string gb_log_file_path = "";
string gb_log_pcie_thread_file_path = "LOG_ThreadStatus.txt";
int gb_log_enabled = 1;
char *gb_log_msg = nullptr;
char *gb_log_thr_msg = nullptr;
CCriticalSection* gb_plock_log = NULL;

/* ----------------------------------------------------------------------------
   LOG_Open
   =======
Descripcion:
   En el caso de que se mantengan en disco más de 1 archivo (_NIFILES_LOG),
   esta función borra el más antiguo, y renombra el resto.
   ------------------------------------------------------------------------- */
void LOG_Open(char *str)
{
char str_char[255];
FILE *f = nullptr;
time_t t;

	if (gb_log_enabled != 1) return;
	if (gb_log_msg != nullptr) free(gb_log_msg);
	if (gb_log_thr_msg != nullptr) free(gb_log_thr_msg);
	gb_log_msg = (char *)malloc(SIZE_MSG_LOG);
	gb_log_thr_msg = (char *)malloc(SIZE_MSG_LOG);
	GetCurrentDirectoryA(255, str_char);
	gb_log_file_path = string(str_char) + "\\";

	if (gb_log_file_path == "") gb_log_file = "\\" + string(LOG_FILE_NAME);
	else gb_log_file = gb_log_file_path + string(LOG_FILE_NAME);
	if ((f = fopen(gb_log_file.c_str(), "wt")) != nullptr)
	{
		t = time(nullptr);
		strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
		fprintf(f, "OPEN ERROR LOG FILE - %s\n", str_char);
		if (str != nullptr) fprintf(f, "PCIe Library Version: v%s", str);
		fprintf(f, "\n-----------------------------------------------------------------");
		if (gb_log_msg == nullptr) fprintf(f, "\n ERROR user message string is nullptr");
		fflush(f);
		fclose(f);
	}

	//if (gb_log_file_path == "")  gb_log_pcie_thread_file_path = "\\" + string(LOG_THREAD_FILE_NAME);
	//else gb_log_pcie_thread_file_path = gb_log_file_path + string(LOG_THREAD_FILE_NAME);;

	//if (gb_log_pcie_thread_file_path != "")
	//{
	//	if ((f = fopen(gb_log_pcie_thread_file_path.c_str(), "wt")) != nullptr)
	//	{
	//		t = time(nullptr);
	//		strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
	//		fprintf(f, "OPEN THREAD LOG FILE - %s\n", str_char);
	//		if (str != nullptr) fprintf(f, "\nPCIe Library Version: v%s", str);
	//		fprintf(f, "\n-----------------------------------------------------------------");
	//		if (gb_log_thr_msg == nullptr) fprintf(f, "\n ERROR user message string is nullptr");
	//		fflush(f);
	//		fclose(f);
	//	}
	//}
	gb_plock_log = new CCriticalSection();
}
/* ----------------------------------------------------------------------------
   LOG_Open
   =======
Descripcion:
   En el caso de que se mantengan en disco más de 1 archivo (_NIFILES_LOG),
   esta función borra el más antiguo, y renombra el resto.
   ------------------------------------------------------------------------- */
void LOG_Open(char *file_log, char *str)
{
	char str_char[255];
	FILE *f = nullptr;
	time_t t;

	if (gb_log_enabled != 1 || file_log == nullptr) return;
	if ((f = fopen(file_log, "wt")) != nullptr)
	{
		t = time(nullptr);
		strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
		fprintf(f, "OPEN LOG FILE - %s", str_char);
		fprintf(f, "\nPCIe Library Version: v%s.%s.%s.%s (%s)", _VERSION_DLL_MAJOR, _VERSION_DLL_MINOR, _VERSION_DLL_RELEASE, _VERSION_DLL_BUILD, _VERSION_DLL_DATE);
		if (str != nullptr) fprintf(f, "\n%s", str);
		fprintf(f, "\n-----------------------------------------------------------------\n");
		fflush(f);
		fclose(f);
	}
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_Info(char *file_log, char* msg)
{
	FILE* fd = nullptr;
	char str[255];
	time_t t;

	if (msg != nullptr && file_log != nullptr)
	{
		if ((fd = fopen(file_log, "at")) != NULL)
		{
			time(&t);
			strftime(str, sizeof(str), "%d/%m/%Y %H:%M:%S", localtime(&t));
			fprintf(fd, "\n[%s]: %s", str, msg);
			fflush(fd);
			fclose(fd);
		}
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Close(void)
{
FILE *f = nullptr;
time_t t;
char str_char[255];

   if (gb_log_enabled != 1) return;

   if (gb_log_file != "")
   {
      if ((f = fopen (gb_log_file.c_str(), "at")) != nullptr)
      {
         t = time(nullptr);
         strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
         fprintf (f, "\r\n-----------------------------------------------------------------");
         fprintf (f, "\nCLOSE ERROR LOG FILE - %s", str_char);
         fclose(f);
      }
   }

   if (gb_log_msg != nullptr) free(gb_log_msg);
   if (gb_log_thr_msg != nullptr) free(gb_log_thr_msg);
   gb_log_msg = nullptr;
   gb_log_thr_msg = nullptr;
   string gb_log_file = "";
   string gb_log_file_path = "";
   if (gb_plock_log != NULL) delete gb_plock_log;
   gb_plock_log = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_Close(char *file_log)
{
	FILE *f = nullptr;
	time_t t;
	char str_char[255];

	if (gb_log_enabled != 1 || file_log == nullptr) return;
	if ((f = fopen(file_log, "at")) != nullptr)
	{
		t = time(nullptr);
		strftime(str_char, 255, "%d/%m/%Y %H:%M:%S", localtime(&t));
		fprintf(f, "\n-----------------------------------------------------------------");
		fprintf(f, "\nCLOSE LOG FILE - %s", str_char);
		fclose(f);
	}
}
/* ---------------------------------------------------------------------------
   log_err
   =======
Descripcion:
   Escribe en el archivo .log, la información del error generado.
   El formato de la información que se incluye en el archivo .log es:

09-07-2008 12:30:01 (App Error Code:   "code") "msg"
                    (Win32 Error Code: x) Texto de error Win32.
                    FILE:     "file"
                    LINE:     "line"
                    FUNCTION: "func"

Parametros:
   msg:
   	Mensaje de error.
   code:
   	Código de error.
   exception:
   	0 --> No se ha producido una exepción
   	1 --> Se ha producido una exepción
   exception_vcl:
   	Puntero a la excepción VCL
   exception_cpp:
   	Puntero a la excepción estandar C++
   iteration:
   	indice de la iteración del bucle ne la que se encuentra
   line:
   	línea de código fuente.
   file:
   	Nombre del archivo del código fuente
   func:
   	Nombre de la función.

Devuelve: El código pasado como parámetro 'code'.
   ------------------------------------------------------------------------- */
float log (char *msg_err, float code_parameter, unsigned long line, char *file, char *func)
{  
LPVOID lpMsgBuf = nullptr;
LPVOID lpDisplayBuf = nullptr;
DWORD error;
FILE *fd = nullptr;
char mode[5];
time_t t;
int code = Round32(code_parameter);

	if (gb_log_enabled != 1) return code_parameter;
	if (code < 0)
	{
		error = GetLastError();
		if (error != 0)
		{
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, nullptr );

			lpDisplayBuf = (LPVOID)LocalAlloc (LMEM_ZEROINIT, lstrlen((LPCTSTR)lpMsgBuf) * sizeof(TCHAR));
		}

		if (gb_log_file == "")
		{
			if (gb_log_file_path == "") gb_log_file = "\\" + string(LOG_FILE_NAME);
			else gb_log_file = gb_log_file_path + string(LOG_FILE_NAME);
			strcpy(mode, "wt");
		}
		else strcpy(mode, "at");

		if (gb_plock_log != NULL)
		{
			if (gb_plock_log->Acquire()) fd = fopen(gb_log_file.c_str(), mode);
			if (fd == NULL && gb_plock_log != NULL) gb_plock_log->Release();
		}
		else fd = fopen(gb_log_file.c_str(), mode);

		time(&t);
		if (fd != nullptr) fseek(fd, 0, SEEK_END);

		if (fd != nullptr) fprintf(fd, "\r\n%s", ctime(&t));

		if (code)
		{
			if (fd != nullptr) fprintf(fd, "(App Error Code:  %d) ", code);
		}
		if (msg_err != nullptr)
		{
			if (fd != nullptr) fprintf(fd, msg_err);
		}
		if (error != 0)
		{
			if (fd != nullptr) fprintf(fd, "\r\n                    (Win32 Error Code: %d) %s\r\n", error, (char*)lpDisplayBuf);
		}
		else if (fd != nullptr) fprintf(fd, "\r\n");
		if (file != nullptr)
		{
			if (fd != nullptr) fprintf(fd, "                    FILE:     %s ", file);
		}
		if (line != 0)
		{
			if (fd != nullptr) fprintf(fd, "\r\n                    LINE:     %d ", line);
		}
		if (func != nullptr)
		{
			if (fd != nullptr) fprintf(fd, "\r\n                    FUNCTION: %s ", func);
		}
		if (fd != nullptr)
		{
			fprintf(fd, "\r\n");
			fflush(fd);
			fclose(fd);
		}
		if (gb_plock_log != NULL) gb_plock_log->Release();

		if (error != 0)
		{
			LocalFree(lpMsgBuf);
			LocalFree(lpDisplayBuf);
		}
		SetLastError(0);
	}
	return code_parameter;
}
/* ---------------------------------------------------------------------------
   log_err
   =======
Descripcion:
   Escribe en el archivo .log, la información del error generado.
   El formato de la información que se incluye en el archivo .log es:

09-07-2008 12:30:01 (App Error Code:   "code") "msg"
					(Win32 Error Code: x) Texto de error Win32.
					FILE:     "file"
					LINE:     "line"
					FUNCTION: "func"

Parametros:
   msg:
	Mensaje de error.
   code:
	Código de error.
   exception:
	0 --> No se ha producido una exepción
	1 --> Se ha producido una exepción
   exception_vcl:
	Puntero a la excepción VCL
   exception_cpp:
	Puntero a la excepción estandar C++
   iteration:
	indice de la iteración del bucle ne la que se encuentra
   line:
	línea de código fuente.
   file:
	Nombre del archivo del código fuente
   func:
	Nombre de la función.

Devuelve: El código pasado como parámetro 'code'.
   ------------------------------------------------------------------------- */
double LOG(char *file_log, char *msg_err, double code_parameter, unsigned long line, char *file, char *func)
{
	LPVOID lpMsgBuf = nullptr;
	LPVOID lpDisplayBuf = nullptr;
	DWORD error;
	FILE *fd = nullptr;
	time_t t;
	int code = Round32(code_parameter);

	if (gb_log_enabled != 1) return code_parameter;
	if (code < 0 && file_log != "" && file_log != nullptr)
	{
		error = GetLastError();
		if (error != 0)
		{
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf,
				0, nullptr);

			lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, lstrlen((LPCTSTR)lpMsgBuf) * sizeof(TCHAR));
		}

		if ((fd = fopen(file_log, "at")) != nullptr)
		{
			time(&t);
			//if (fd != nullptr) fseek(fd, 0, SEEK_END);
			if (fd != nullptr) fprintf(fd, "\r\n%s", ctime(&t));
			if (code)
			{
				if (fd != nullptr) fprintf(fd, "(App Error Code:  %d) ", code);
			}
			if (msg_err != nullptr)
			{
				if (fd != nullptr) fprintf(fd, msg_err);
			}
			if (error != 0)
			{
				if (fd != nullptr) fprintf(fd, "\n                    (Win32 Error Code: %d) %s\n", error, (char*)lpDisplayBuf);
			}
			else if (fd != nullptr) fprintf(fd, "\n");
			if (file != nullptr)
			{
				if (fd != nullptr) fprintf(fd, "                    FILE:     %s ", file);
			}
			if (line != 0)
			{
				if (fd != nullptr) fprintf(fd, "\n                    LINE:     %d ", line);
			}
			if (func != nullptr)
			{
				if (fd != nullptr) fprintf(fd, "\n                    FUNCTION: %s ", func);
			}
			if (fd != nullptr)
			{
				fprintf(fd, "\n");
				fflush(fd);
				fclose(fd);
			}
			if (gb_plock_log != NULL) gb_plock_log->Release();

			if (error != 0)
			{
				LocalFree(lpMsgBuf);
				LocalFree(lpDisplayBuf);
			}
		}
		SetLastError(0);
	}
	return code_parameter;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int LOG_GetEnabledFile(void)
{
   return gb_log_enabled;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void LOG_SetEnabledFile(int data)
{
   gb_log_enabled = data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float log_err(char *msg_err, float code, unsigned long line, char *file, char *func)
{
	return (float)log(msg_err, code, line, file, func);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int log_err(char* msg_err, int code, unsigned long line, char* file, char* func)
{
	return (int)log(msg_err, (float)code, line, file, func);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
long log_err(char* msg_err, long code, unsigned long line, char* file, char* func)
{
	return (long)log(msg_err, (float)code, line, file, func);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float log_err(char* file_log, char *msg_err, float code, unsigned long line, char *file, char *func)
{
	return (float)LOG(file_log, msg_err, code, line, file, func);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int log_err(char* file_log, char* msg_err, int code, unsigned long line, char* file, char* func)
{
	return (int)LOG(file_log, msg_err, (float)code, line, file, func);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
long log_err(char* file_log, char* msg_err, long code, unsigned long line, char* file, char* func)
{
	return (long)LOG(file_log, msg_err, (float)code, line, file, func);
}
