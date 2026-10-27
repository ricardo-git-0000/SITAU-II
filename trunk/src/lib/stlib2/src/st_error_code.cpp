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

#include "st_error_code.h"
#include "common_error_code.h"
#include "buffer_error_code.h"
#include "base_error_code.h"
#include "fl_error_code.h"
#include "c_sitau_hw_err.h"
#include "tcp_error_code.h"
#include "vch_error_code.h"
#include "stlib2.h"
#include "sitau_error_code.h" // Incorpora los errores de configuracion hardware.
#include "st_encoder_error_code.h" // Incorpora el catalogo de encoders.
#include "xml_err.h" // Incorpora los errores del parser XML.
#include "c_log_file.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetSTErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case ES2_BASE_TEST:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BASE_TEST);
      break;
   case ES2_MOD_TEST:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_MOD_TEST);
      break;
   case ES2_NOINIT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NOINIT);
      break;
   case ES2_NOCONNECT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NOCONNECT);
      break;
   case ES2_RANGO:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_RANGO);
      break;
   case ES2_TIMEOUT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_TIMEOUT);
      break;
   case ES2_CH:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_CH);
      break;
   case ES2_NO_MEM:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NO_MEM);
      break;
   case ES2_BUFFER_FTR_NDAT_3:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_FTR_NDAT_3);
      break;
   case ES2_BUFFER_ADQ_OVF_1:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_ADQ_OVF_1);
      break;
   case ES2_BUFFER_ADQ_OVF_2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_ADQ_OVF_2);
      break;
   case ES2_FILE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_FILE);
      break;
   case ES2_ZERO_DIVISION:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_ZERO_DIVISION);
      break;
   case ES2_EmptyBuffer:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_EmptyBuffer);
      break;
   case ES2_NOCONECT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NOCONECT);
      break;
   case ES2_SCAN_ENC:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_SCAN_ENC);
      break;
   case ES2_SCAN:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_SCAN);
      break;
   case ES2_INVALID_ACCESS:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_INVALID_ACCESS); // Usa la descripcion de acceso, no la de lineas.
      break;
   case ES2_INVALID_LINES:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_INVALID_LINES);
      break;
   case ES2_FL_FILE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_FL_FILE);
      break;
   case ES2_NMU_LOW:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NMU_LOW);
      break;
   case ES2_NMU_HIGH:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NMU_HIGH);
      break;
   case ES2_EEPROM_CFG:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_EEPROM_CFG);
      break;
   case ES2_FILE_CFG:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_FILE_CFG);
      break;
   case ES2_AD_TEST:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_AD_TEST);
      break;
   case ES2_NOT_PA_SYSTEM:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NOT_PA_SYSTEM);
      break;
   case ES2_SCAN_LINES_NUMBER:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_SCAN_LINES_NUMBER);
      break;
   case ES2_USER_MODE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_USER_MODE);
      break;
   case ES2_DataInconsistency:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_DataInconsistency);
      break;
   case ES2_InterfaceDetect:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_InterfaceDetect);
      break;
   case ES2_NoDataAcquired:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NoDataAcquired);
      break;
   case ES2_PtrAccess_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_PtrAccess_NULL);
      break;
   case ES2_MathFunction:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_MathFunction);
      break;
   case ES2_FunctionNotAvailable:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_FunctionNotAvailable);
      break;
   case ES2_NoBuffer:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_NoBuffer);
      break;
   case ES2_IdThreadNULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_IdThreadNULL);
      break;
   case ES2_ThreadResume:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_ThreadResume);
      break;
   case ES2_IdVirtualChannel:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_IdVirtualChannel);
      break;
   case ES2_IdGate:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_IdGate);
      break;
   case ES2_IdEncoder:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_IdEncoder);
      break;
   case ES2_Acquiring:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_Acquiring);
      break;
   case ES2_RUN_NO_DATA:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_RUN_NO_DATA);
      break;
   case ES2_BUFFER_RD_WORD_INI:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_RD_WORD_INI);
      break;
   case ES2_MEM_UCI_OVERFLOW:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_MEM_UCI_OVERFLOW);
      break;
   case ES2_BUFFER_ADD_OVF_1:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_ADD_OVF_1);
      break;
   case ES2_BUFFER_ADD_OVF_2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_ADD_OVF_2);
      break;
   case ES2_BUFFER_ADD_OVF_3:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_ADD_OVF_3);
      break;
   case ES2_BUFFER_RD_WR1:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_RD_WR1);
      break;
   case ES2_BUFFER_RD_WR2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_BUFFER_RD_WR2);
      break;
   case ES2_TriggerSource:
      sprintf(str, "(Error Code %d) '%s'",error_code, charES2_TriggerSource);
      break;
   case ES2_SYSTEM_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_SYSTEM_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ES2_MODULE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_MODULE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ES2_BASE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_BASE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ES2_IdConnector: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_IdConnector); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ES2_File: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_File); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case ES2_Acquire: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charES2_Acquire); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   default: result = 0; break;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetError (int error_code, char *str)
{
    if (str == NULL) return 0; // Evita escribir en un buffer inexistente.
    switch (error_code) // Incluye las familias que no tienen traductor propio.
    { // Los valores numericos permanecen compatibles.
    case EXML_FILE: // Error definido en xml_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEXML_FILE); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EXML_Parameter: // Error definido en xml_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEXML_Parameter); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EXML_NoMemory: // Error definido en xml_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEXML_NoMemory); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_IdEncoder: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_IdEncoder); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_Parameter: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_Parameter); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_NoMemory: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_NoMemory); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_DataInconsistency: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_DataInconsistency); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_FunctionNotAvailable: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_FunctionNotAvailable); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case EENC_File: // Error definido en st_encoder_error_code.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charEENC_File); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case CFGHW_NO_BOARD: // Error definido en c_sitau_hw_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charCFGHW_NO_BOARD); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case CFGHW_BUFFER_OVERFLOW: // Error definido en c_sitau_hw_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charCFGHW_BUFFER_OVERFLOW); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case CFGHW_NO_MEM: // Error definido en c_sitau_hw_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charCFGHW_NO_MEM); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case CFGHW_EEPROM: // Error definido en c_sitau_hw_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charCFGHW_EEPROM); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    case CFGHW_FILE: // Error definido en c_sitau_hw_err.h.
        sprintf(str, "(Error Code %d) '%s'", error_code, charCFGHW_FILE); // Devuelve la descripcion existente.
        return 1; // Indica que el codigo esta reconocido.
    default: break; // Consulta los restantes catalogos.
    } // Fin de las familias auxiliares.
    if (ST_GetBASEErrorCode(error_code, str) == 1) return 1; // Traduce errores de las bases.
    if (ST_GetSITAUErrorCode(error_code, str) == 1) return 1; // Traduce errores de SITAU.XML y hardware.
    if (FL_GetErrorCode(error_code, str) == 1) return 1;
    if (ST_GetSTErrorCode(error_code, str) == 1) return 1;
    if (ST_GetVCHErrorCode(error_code, str) == 1) return 1;
    if (ST_GetTCPErrorCode(error_code, str) == 1) return 1;
    if (GetCommonErrorCode(error_code, str) == 1) return 1;
    if (ST_GetBufferErrorCode(error_code, str) == 1) return 1;
    sprintf(str, "(Error Code %d) 'Codigo de error no documentado'", error_code); // Identifica un codigo desconocido sin inventar su significado.
    return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetLastError (char *str)
{
   sprintf(str, "(Error Code %d) '%s'", gb_last_error_code, gb_last_error_msg.c_str());
   return gb_last_error_code;
}

