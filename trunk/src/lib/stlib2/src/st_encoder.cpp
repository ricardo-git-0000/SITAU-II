// -----------------------------------------------------------------------------
/**
 @file st_encoder.cpp

 @brief Este archivo contiene la implementación de las funciones de configuración
 y acceso de los encoders
 
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

#if defined(_MSC_VER)
#include <vector>
#else
#include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_reg.h"
#include "st_encoder.h"
#include "st_encoder_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "stlib2.h"
#include "c_xml_sitau_hw.h"
#include "protocol_uci.h"
#include "st_error_code.h"
#include "sitau_error_code.h"
#include "c_virtual_channel.h"
#include "c_acq_data.h"
#include "fileformat.h"
#include "c_buffer_acq.h"
#include "st_thread_status.h"



// ----------------------------------------------------------------------
// ST_SetEncoderZero
// ===============
// Descripcion:
//     Pone a cero el encoder
//
// Parametros:
//
// Devuelve: (Ver codigos de error en 'sterr_v2.h')
// ----------------------------------------------------------------------
int ST_SetEncoderZero(int id_encoder)
{
int result;
bool locked = IsLockedStatusThread();

    if ((result = TCPSend_MSG_RESET_ENCODER(id_encoder)) < 0) return RLOG(result);
    return ES2_NONE;
}
// ----------------------------------------------------------------------
// ST_SetEncoderZero
// ===============
// Descripcion:
//     Pone a cero el encoder
//
// Parametros:
//
// Devuelve: (Ver codigos de error en 'sterr_v2.h')
// ----------------------------------------------------------------------
int ST_SetAllEncoderZero(void)
{
int result;
bool locked = IsLockedStatusThread();

    if ((result = TCPSend_MSG_RESET_ENCODER(-1)) < 0) return RLOG(result);
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Selecciona el encoder del eje principal con el que se realizan los disparos.

 @param[out]   data
               Encoder de disparo seleccionado.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_SetEncoderTrigger (int id_encoder)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderTrigger (void)
{
   return 0;
}
// -----------------------------------------------------------------------------
/**
 Configura el encoder con el que se realizará la adquisición.

 @param[in] encoder  Identificador del encoder con el que se dispara la adquisición.
                     Rango -> [0 .. (Nº de encoders del equipo - 1)]
                     Ver \c UT_InfoMaxEncodersNumber().

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_SetEncoderAuxiliar (int id_encoder)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderAuxiliar (void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetEncoderCalibration (int data, int id_encoder)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderCalibration (int id_encoder)
{
   return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetEncoderScanResolution (float data, int id_encoder)
{
    return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetEncoderScanResolution (int id_encoder)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetEncoderScanLength (float data, int id_encoder)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**
 Configura la resolución con la que se realiza la adquisición por encoder.

 @param[in] data     (mm) Longitud del barrido en el eje correspondiente a un encoder.

 @param[in] encoder  Identificador del encoder corespondiente.
                     Rango -> [0 .. (Nº de encoders del equipo - 1)]
                     Ver \c UT_InfoMaxEncodersNumber().

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
float ST_GetEncoderScanLength (int id_encoder)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderScanResolutionSteps (int id_encoder)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetEncoderSign (int id_encoder)
{
	return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderSign (int id_encoder)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetScanLengthTrigger (void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetScanResolutionTrigger (void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetScanLengthAuxiliar (void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_GetScanResolutionAuxiliar (void)
{
    return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanLengthTrigger (float data)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanResolutionTrigger (float data)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanResolutionTriggerSteps(void)
{
   return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanLengthAuxiliar (float data)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanResolutionAuxiliar (float data)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanResolutionAuxiliarSteps(void)
{
   return 1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetScanAuxiliarEnabled(int data)
{
   return EENC_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetScanAuxiliarEnabled(void)
{
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_GetEncoderConnectorName(char* name, int id_encoder)
{
    return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetEncoderPosition(long data, int id_encoder)
{
    return ES2_NONE;
}



