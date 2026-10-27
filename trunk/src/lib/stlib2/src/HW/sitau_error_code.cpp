// -----------------------------------------------------------------------------
/**
 @file base_error_code.cpp

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

#include "sitau_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetSITAUErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EHW_Status_FSM:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEHW_Status_FSM);
      break;
   case EHW_Parameter: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_Parameter); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_XML_Format: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, "Error de acceso a archivo o formato de SITAU.XML (EHW_File / EHW_XML_Format)"); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_PtrAccess_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_PtrAccess_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_BASE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_BASE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_NoMemory: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_NoMemory); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_NoXMLData: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_NoXMLData); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_DataInconsistency: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_DataInconsistency); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_IdEncoder: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_IdEncoder); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EHW_FRONTEND_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEHW_FRONTEND_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default: result = 0; break;
   }
   return result;
}

