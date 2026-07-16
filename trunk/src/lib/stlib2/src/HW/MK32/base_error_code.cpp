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

#include "base_error_code.h"
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetBASEErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EBASE_Parameter:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_Parameter);
      break;
   case EBASE_NoMemory:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_NoMemory);
      break;
   case EBASE_File:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_File);
      break;
   case EBASE_DataInconsistency:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_DataInconsistency);
      break;
   case EBASE_PtrAccess_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_PtrAccess_NULL);
      break;
   case EBASE_MathFunction:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_MathFunction);
      break;
   case EBASE_IdBASE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_IdBASE);
      break;
   case EBASE_NUM_CPB:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_NUM_CPB);
      break;
   case EBASE_NUM_FocalLaw:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEBASE_NUM_FocalLaw);
      break;
   default: result = 0; break;
   }
   return result;
}

