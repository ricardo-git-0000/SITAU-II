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

#include "pcie_error_code.h"
#include "api_pcie.h"
#include "log.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PCIe_GetErrorCode (int error_code, char *str)
{
int result = 1;

   switch(error_code)
   {
   case EPCIE_DRV_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_DRV_NULL);
      break;
   case EPCIE_BUFFER_LEAKAGE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_LEAKAGE);
      break;
   case EPCIE_PARAMETER:
	   sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_PARAMETER);
	   break;
   case EPCIE_NOINIT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NOINIT);
      break;
   case EPCIE_NOCONNECT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NOCONNECT);
      break;
   case EPCIE_TIMEOUT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_TIMEOUT);
      break;
   case EPCIE_CH:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_CH);
      break;
   case EPCIE_NO_MEMORY:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NO_MEMORY);
      break;
   case EPCIE_BUFFER_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_NULL);
      break;
   case EPCIE_BUFFER_ADQ_OVF_1:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_ADQ_OVF_1);
      break;
   case EPCIE_BUFFER_ADQ_OVF_2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_ADQ_OVF_2);
      break;
   case EPCIE_FILE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_FILE);
      break;
   case EPCIE_ZERO_DIVISION:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_ZERO_DIVISION);
      break;
   case EPCIE_EmptyBuffer:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_EmptyBuffer);
      break;
   case EPCIE_NOCONECT:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NOCONECT);
      break;
   case EPCIE_SCAN_ENC:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_SCAN_ENC);
      break;
   case EPCIE_SCAN:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_SCAN);
      break;
   case EPCIE_INVALID_ACCESS:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_INVALID_ACCESS); // Usa la descripcion de acceso, no la de lineas.
      break;
   case EPCIE_INVALID_LINES:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_INVALID_LINES);
      break;
   case EPCIE_FL_FILE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_FL_FILE);
      break;
   case EPCIE_NMU_LOW:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NMU_LOW);
      break;
   case EPCIE_NMU_HIGH:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NMU_HIGH);
      break;
   case EPCIE_EEPROM_CFG:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_EEPROM_CFG);
      break;
   case EPCIE_FILE_CFG:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_FILE_CFG);
      break;
   case EPCIE_AD_TEST:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_AD_TEST);
      break;
   case EPCIE_NOT_PA_SYSTEM:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NOT_PA_SYSTEM);
      break;
   case EPCIE_SCAN_LINES_NUMBER:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_SCAN_LINES_NUMBER);
      break;
   case EPCIE_USER_MODE:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_USER_MODE);
      break;
   case EPCIE_DataInconsistency:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_DataInconsistency);
      break;
   case EPCIE_InterfaceDetect:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_InterfaceDetect);
      break;
   case EPCIE_NoDataAcquired:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NoDataAcquired);
      break;
   case EPCIE_PtrAccess_NULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_PtrAccess_NULL);
      break;
   case EPCIE_MathFunction:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_MathFunction);
      break;
   case EPCIE_FunctionNotAvailable:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_FunctionNotAvailable);
      break;
   case EPCIE_NoBuffer:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_NoBuffer);
      break;
   case EPCIE_IdThreadNULL:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_IdThreadNULL);
      break;
   case EPCIE_ThreadResume:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_ThreadResume);
      break;
   case EPCIE_IdVirtualChannel:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_IdVirtualChannel);
      break;
   case EPCIE_IdGate:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_IdGate);
      break;
   case EPCIE_IdEncoder:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_IdEncoder);
      break;
   case EPCIE_Acquiring:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_Acquiring);
      break;
   case EPCIE_RUN_NO_DATA:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_RUN_NO_DATA);
      break;
   case EPCIE_BUFFER_RD_WORD_INI:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_RD_WORD_INI);
      break;
   case EPCIE_OVERFLOW:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_OVERFLOW);
      break;
   case EPCIE_BUFFER_ADD_OVERFLOW:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_ADD_OVERFLOW);
      break;
   case EPCIE_BUFFER_ADD_OVF_2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_ADD_OVF_2);
      break;
   case EPCIE_BUFFER_ADD_OVF_3:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_ADD_OVF_3);
      break;
   case EPCIE_BUFFER_RD_WR1:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_RD_WR1);
      break;
   case EPCIE_BUFFER_RD_WR2:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_BUFFER_RD_WR2);
      break;
   case EPCIE_TriggerSource:
      sprintf(str, "(Error Code %d) '%s'",error_code, charEPCIE_TriggerSource);
      break;
   case EPCIE_SYSTEM_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_SYSTEM_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_MODULE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_MODULE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_BASE_Number: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_BASE_Number); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_IdConnector: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_IdConnector); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_File: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_File); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_IMAGE_HANDLE_NULL: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_IMAGE_HANDLE_NULL); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_ACQUIRE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_ACQUIRE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_RecursiveRelease: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_RecursiveRelease); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_NO_THREAD_ACQ: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_NO_THREAD_ACQ); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_IMAGE_SIZE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_IMAGE_SIZE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_THREAD_ACQ: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_THREAD_ACQ); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_MUTEX_ACQUIRE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_MUTEX_ACQUIRE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_NO_THREAD_DMA: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_NO_THREAD_DMA); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_EVENT: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_EVENT); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_SET_EVENT: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_SET_EVENT); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_NO_THREAD_GE: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_NO_THREAD_GE); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_DISABLED: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_DISABLED); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_GTX_BLOCKS: // Traduce el codigo definido en el catalogo existente.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_GTX_BLOCKS); // Incluye codigo y descripcion.
      break; // Finaliza la traduccion de este codigo.
   case EPCIE_Geometry: // Traduce la validacion de geometria PCIe.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_Geometry); // Incluye el numero y su descripcion especifica.
      break; // Conserva el contrato de codigo reconocido.
   case EPCIE_GeometrySize: // Traduce la validacion de geometria PCIe.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_GeometrySize); // Incluye el numero y su descripcion especifica.
      break; // Conserva el contrato de codigo reconocido.
   case EPCIE_ChannelMap: // Traduce errores de permutacion del mapa.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_ChannelMap); // Devuelve codigo y causa.
      break; // Termina este caso.
   case EPCIE_BlockLayout: // Traduce bloques incompletos o inconsistentes.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_BlockLayout); // Devuelve codigo y causa.
      break; // Termina este caso.
   case EPCIE_EmulatorGeometry: // Explica la limitacion del generador del driver.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_EmulatorGeometry); // Devuelve codigo y causa.
      break; // Termina este caso.
   case EPCIE_GeometryPending: // Traduce la validacion de geometria PCIe.
      sprintf(str, "(Error Code %d) '%s'", error_code, charEPCIE_GeometryPending); // Incluye el numero y su descripcion especifica.
      break; // Conserva el contrato de codigo reconocido.
   default: result = 0; break;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int PCIe_GetError(int error_code, char *str)
{
	if (PCIe_GetErrorCode(error_code, str) == 1) return 1;
	return 0;
}

