// -----------------------------------------------------------------------------
/**
 @file common_error_code.cpp

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

#include "common_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int GetCommonErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case ECOM_PathFile:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_PathFile);
      break;
   case ECOM_Parameter:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_Parameter);
      break;
   case ECOM_NoMemory:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_NoMemory);
      break;
   case ECOM_MathFunction:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_MathFunction);
      break;
   case ECOM_RecursiveAcquire:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_RecursiveAcquire);
      break;
   case ECOM_RecursiveRelease:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_RecursiveRelease);
      break;
   case ECOM_Acquire:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_Acquire);
      break;
   case ECOM_DeleteAcquire:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_DeleteAcquire);
      break;
   case ECOM_IdThreadNULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charECOM_IdThreadNULL);
      break;
   case ECOM_DataInconsistency: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charECOM_DataInconsistency); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ECOM_PtrAccess_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charECOM_PtrAccess_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default:
      sprintf(str, "(Error Code %d) 'Unknown Error'",error_code);
      result = 0;
      break;
   }
   return result;
}

