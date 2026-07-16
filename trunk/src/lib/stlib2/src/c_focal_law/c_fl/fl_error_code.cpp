// -----------------------------------------------------------------------------
/**
 @file st_error_code.cpp

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

#include "fl_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FL_GetErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EFL_PARAMETER:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PARAMETER);
       break;
   case EFL_FILE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_FILE);
       break;
   case EFL_PA_Z:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PA_Z);
       break;
   case EFL_PA_X:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PA_X);
       break;
   case EFL_PA_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PA_NULL);
       break;
   case EFL_WEDGE_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_WEDGE_NULL);
       break;
   case EFL_SPECIMEN_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SPECIMEN_NULL);
       break;
   case EFL_SCAN_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SCAN_NULL);
       break;
   case EFL_DDF_APERTURE_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_DDF_APERTURE_NULL);
       break;
   case EFL_NO_MEM:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_NO_MEM);
       break;
   case EFL_NO_SCAN:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_NO_SCAN);
       break;
   case EFL_ANGLE_OVF:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_ANGLE_OVF);
       break;
   case EFL_XYZ_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_XYZ_SIZE);
       break;
   case EFL_IANGLE_CRITICAL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_IANGLE_CRITICAL);
       break;
   case EFL_APERTURE_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_APERTURE_SIZE);
       break;
   case EFL_FOCAL_LAW_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_FOCAL_LAW_SIZE);
       break;
   case EFL_PA_COORD_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PA_COORD_SIZE);
       break;
   case EFL_NO_WEDGE_SURF:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_NO_WEDGE_SURF);
       break;
   case EFL_SURFACE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SURFACE);
       break;
   case EFL_VIRTUAL_ARRAY_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_VIRTUAL_ARRAY_SIZE);
       break;
   case EFL_DDF_NFOCUS:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_DDF_NFOCUS);
       break;
   case EFL_X_AP_SIZE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_X_AP_SIZE);
       break;
   case EFL_PQ:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PQ);
       break;
   case EFL_ACQ_RANGE:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_ACQ_RANGE);
       break;
   case EFL_SQRT_NEG:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SQRT_NEG);
       break;
   case EFL_PtrAccess_NULL:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PtrAccess_NULL);
       break;
   case EFL_PAElement:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_PAElement);
       break;
   case EFL_IFAZ_ERR:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_IFAZ_ERR);
       break;
   case EFL_DataInconsistency:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_DataInconsistency);
       break;
   case EFL_ASIN:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_ASIN);
       break;
   case EFL_SurfacePoints_Izqd:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SurfacePoints_Izqd);
       break;
   case EFL_SurfacePoints_Drch:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SurfacePoints_Drch);
       break;
   case EFL_SAD_NoGate:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SAD_NoGate);
       break;
   case EFL_NoSurface:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_NoSurface);
       break;
   case EFL_SCAN_LINES_NUMBER:
       sprintf(str, "(Error Code %d) '%s'", error_code, charEFL_SCAN_LINES_NUMBER);
       break;
   default: result = 0; break;
   }
   return result;
}
