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
   default: result = 0; break;
   }
   return result;
}

