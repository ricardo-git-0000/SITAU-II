// -----------------------------------------------------------------------------
/**
 @file tcp_error_code.cpp

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

#include <stdio.h>
#include <string.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tcp_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetTCPErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case ETCP_FILE: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_FILE); break;

   case ETCP_RANGO: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_RANGO); break;

   case ETCP_IdMOD: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_IdMOD); break;

   case ETCP_IdBASE: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_IdBASE); break;

   case ETCP_CH: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_CH); break;

   case ETCP_NO_MEM: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_NO_MEM); break;

   case ETCP_WMPAR: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_WMPAR); break;

   case ETCP_INVALID_ACCESS: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_INVALID_ACCESS); break;

   case ETCP_NO_INI: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_NO_INI); break;

   case ETCP_N_FL: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_N_FL); break;

   case ETCP_MEM: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_MEM); break;

   case ETCP_CONNECT: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_CONNECT); break;

   case ETCP_CONNECT_TIMEOUT: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_CONNECT_TIMEOUT); break;

   case ETCP_SOCKET: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_SOCKET); break;

   case ETCP_CSIC: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_CSIC); break;

   case ETCP_MODULE_Number: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_MODULE_Number); break;

   case ETCP_BASE_Number: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_BASE_Number); break;

   case ETCP_Checksum: sprintf(str, "Error Code ETCP(%d) '%s'",error_code, charETCP_Checksum); break;

   case ETCP_WSAStartup: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charETCP_WSAStartup); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default: result = 0; break;
   }
   return result;
}

