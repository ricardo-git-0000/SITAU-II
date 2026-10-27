// -----------------------------------------------------------------------------
/**
 @file st_error_code.cpp

 @brief Este archivo contiene la implementación de las funciones relacionadas con los códigos de error.

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

#include "buffer_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetBufferErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EBUF_Acquire:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Acquire);
      break;
   case EBUF_Parameter:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Parameter);
      break;
   case EBUF_NoMemory:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_NoMemory);
      break;
   case EBUF_DataInconsistency:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_DataInconsistency);
      break;
   case EBUF_IdScan:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_IdScan);
      break;
   case EBUF_Overflow:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Overflow);
      break;
   case EBUF_Underflow:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Underflow);
      break;
   case EBUF_NoHeader:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_NoHeader);
      break;
   case EBUF_Frame:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Frame);
      break;
   case EBUF_IdGate:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_IdGate);
      break;
   case EBUF_IdEncoder:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_IdEncoder);
      break;
   case EBUF_File:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_File);
      break;
   case EBUF_Acquiring:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_Acquiring);
      break;
   case EBUF_DisabledDataFormat:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBUF_DisabledDataFormat);
      break;
   case EBUF_MODULE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEBUF_MODULE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EBUF_BASE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEBUF_BASE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EBUF_CONFIG_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEBUF_CONFIG_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EBUF_IdVirtualChannel: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEBUF_IdVirtualChannel); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EBUF_IdBASE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEBUF_IdBASE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default:
      sprintf(str, "(Error Code %d) 'Unknown Error'",error_code);
      result = 0;
      break;
   }
   return result;
}

