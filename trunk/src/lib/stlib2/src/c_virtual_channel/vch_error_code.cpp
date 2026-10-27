// -----------------------------------------------------------------------------
/**
 @file vch_error_code.cpp

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

#include "vch_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetVCHErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EVCH_Parameter:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_Parameter);
      break;
   case EVCH_NoMemory:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_NoMemory);
      break;
   case EVCH_File:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_File);
      break;
   case EVCH_DataInconsistency:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_DataInconsistency);
      break;
   case EVCH_PtrAccess_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_PtrAccess_NULL);
      break;
   case EVCH_MathFunction:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_MathFunction);
      break;
   case EVCH_PA_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_PA_NULL);
      break;
   case EVCH_WEDGE_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_WEDGE_NULL);
      break;
   case EVCH_SPECIMEN_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_SPECIMEN_NULL);
      break;
   case EVCH_SCAN_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_SCAN_NULL);
      break;
   case EVCH_BASE_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_BASE_NULL);
      break;
   case EVCH_MODULE_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_MODULE_NULL);
      break;
   case EVCH_SYSTEM_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_SYSTEM_NULL);
      break;
   case EVCH_CONFIG_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_CONFIG_NULL);
      break;
   case EVCH_NMU_Underflow:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_NMU_Underflow);
      break;
   case EVCH_NMU_Overflow:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_NMU_Overflow);
      break;
   case EVCH_AScanNumber:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_AScanNumber);
      break;
   case EVCH_IdMOD:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_IdMOD);
      break;
   case EVCH_IdBASE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_IdBASE);
      break;
   case EVCH_IdPASystem:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEVCH_IdPASystem);
      break;
   case EVCH_USER_MODE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEVCH_USER_MODE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EVCH_ACQUISITION_TYPE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEVCH_ACQUISITION_TYPE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EVCH_PinToChannelSize: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEVCH_PinToChannelSize); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default: result = 0; break;
   }
   return result;
}

