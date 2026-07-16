#ifndef stfp_error_codeH
#define stfp_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define st_error_code_start            -1900

#define ES2_NONE                       0

#define ES2_BASE_TEST                  (st_error_code_start - 1)
#define charES2_BASE_TEST              "One BASE device has not been configured correctly"

#define ES2_MOD_TEST                   (st_error_code_start - 2)
#define charES2_MOD_TEST               "One MODULE device has not been configured correctly"

#define ES2_NOINIT                     (st_error_code_start - 3)
#define charES2_NOINIT                 "Error: Se ha llamado a ST_ConfigSys sin una llamada previa a ST_OpenSys"

#define ES2_NOCONNECT                  (st_error_code_start - 4)
#define charES2_NOCONNECT              "Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys"

#define ES2_RANGO                      (st_error_code_start - 5)
#define charES2_RANGO                  "Error: Rango de datos no válido"

#define ES2_TIMEOUT                    (st_error_code_start - 6)
#define charES2_TIMEOUT                "Se ha salido de un bucle por timeout"

#define ES2_CH                         (st_error_code_start - 7)
#define charES2_CH                     "Error número de canales, el numero de canales es distinto al configurado por la función ST_OpenSys()"

#define ES2_NO_MEM                     (st_error_code_start - 8)
#define charES2_NO_MEM                 "Error en la reserva de memoria dinámica"

#define ES2_BUFFER_FTR_NDAT_3          (st_error_code_start - 9)
#define charES2_BUFFER_FTR_NDAT_3      "Error trama incorrecta en el buffer del canal, la trama leida del buffer del canal indica que contiene un numero de datos distinto al programado"

#define ES2_BUFFER_ADQ_OVF_1           (st_error_code_start - 10)
#define charES2_BUFFER_ADQ_OVF_1       "ERROR Se ha llenado el buffer de adquisición"

#define ES2_BUFFER_ADQ_OVF_2           (st_error_code_start - 11)
#define charES2_BUFFER_ADQ_OVF_2       "Error desbordamiento de buffer (durante la adquisicion), el numero de datos leídos es superior al tamaño del buffer temporal de adquisición"

#define ES2_FILE                       (st_error_code_start - 12)
#define charES2_FILE                   "Error accessing a file"

#define ES2_ZERO_DIVISION		         (st_error_code_start - 13)
#define charES2_ZERO_DIVISION	         "Se va a intentar dividir por cero. Se aborta la operación"

#define ES2_EmptyBuffer 	            (st_error_code_start - 14)
#define charES2_EmptyBuffer	         "Intento de lectura de Buffer vacío"

#define ES2_NOCONECT 	               (st_error_code_start - 15)
#define charES2_NOCONECT	            "Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys()"

#define ES2_SCAN_ENC 	               (st_error_code_start - 16)
#define charES2_SCAN_ENC	            "Error de Disparo: El sistema se encuentra en un estado de barrido por encoder ST_RunScanENC()"

#define ES2_SCAN 	                     (st_error_code_start - 17)
#define charES2_SCAN	                  "Error de Programación: Intento de programación mientras el sistema encuentra en un estado de barrido configurado con ST_RunScanSW() o ST_RunScanENC()"

#define ES2_INVALID_ACCESS             (st_error_code_start - 18)
#define charES2_INVALID_ACCESS         "Error Se ha intentado acceder a una posición no válida del vector"

#define ES2_INVALID_LINES              (st_error_code_start - 19)
#define charES2_INVALID_LINES          "Error No se han calculado correctamente las líneas del barrido"

#define ES2_FL_FILE                    (st_error_code_start - 20)
#define charES2_FL_FILE                "Error en el formato del archivo de leyes focales"

#define ES2_NMU_LOW			            (st_error_code_start - 21)
#define charES2_NMU_LOW		            "Error en el número de muestras, no puede ser inferior a 31 después del diezmado"

#define ES2_NMU_HIGH		               (st_error_code_start - 22)
#define charES2_NMU_HIGH	            "Error en el número de muestras, no puede ser superior a 20478 después del diezmado"// Error número de muestras, debe ser mayor de 31 a la salida del diezmador.

#define ES2_EEPROM_CFG			         (st_error_code_start - 23)
#define charES2_EEPROM_CFG	   	      "Error de formato en la información del sistema leida de la memoria EEPROM"

#define ES2_FILE_CFG			            (st_error_code_start - 24)
#define charES2_FILE_CFG	   	      "Error accediendo al archivo de configuración del sistema 'SITAU.XML'"

#define ES2_AD_TEST                    (st_error_code_start - 25)
#define charES2_AD_TEST                "Error en los conversores AD de un módulo con el patrón de test"

#define ES2_NOT_PA_SYSTEM      	      (st_error_code_start - 26)
#define charES2_NOT_PA_SYSTEM 	      "Intento de obtener datos de un sistema phased array en un sistema de tipo multicanal"

#define ES2_SCAN_LINES_NUMBER          (st_error_code_start - 27)
#define charES2_SCAN_LINES_NUMBER 	   "Error, el número de lineas total de la adquisición excede el número máximo"

#define ES2_USER_MODE                  (st_error_code_start - 28)
#define charES2_USER_MODE 	            "Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode()"

#define ES2_DataInconsistency          (st_error_code_start - 29)
#define charES2_DataInconsistency 	   "Error, inconsistencia entre los datos"

#define ES2_InterfaceDetect            (st_error_code_start - 30)
#define charES2_InterfaceDetect 	      "Error en la detección d ela interfaz"

#define ES2_NoDataAcquired             (st_error_code_start - 31)
#define charES2_NoDataAcquired 	      "Error en la adquisición de datos"

#define ES2_PtrAccess_NULL			      (st_error_code_start - 32)
#define charES2_PtrAccess_NULL         "ERROR: Acceso a puntero NULL."

#define ES2_MathFunction               (st_error_code_start - 33)
#define charES2_MathFunction           "Error en el cálculo de una función matemática."

#define ES2_FunctionNotAvailable       (st_error_code_start - 34)
#define charES2_FunctionNotAvailable   "Error function not available."

#define ES2_NoBuffer                   (st_error_code_start - 35)
#define charES2_NoBuffer 	            "Error, el buffer interno ha sido creado"

#define ES2_IdThreadNULL               (st_error_code_start - 36)
#define charES2_IdThreadNULL           "Error Thread handle is NULL."

#define ES2_ThreadResume               (st_error_code_start - 37)
#define charES2_ThreadResume           "Error starting thread."

#define ES2_IdVirtualChannel           (st_error_code_start - 38)
#define charES2_IdVirtualChannel       "Error en el identificador del barrido 'scan' o canal virtual"

#define ES2_IdGate                     (st_error_code_start - 39)
#define charES2_IdGate                 "Error en el identificador de la puerta"

#define ES2_IdEncoder                  (st_error_code_start - 40)
#define charES2_IdEncoder              "Error en el identificador del encoder"

#define ES2_Acquiring 	               (st_error_code_start - 41)
#define charES2_Acquiring	            "Error, intento de disparo mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan()"

#define ES2_RUN_NO_DATA                (st_error_code_start - 42)
#define charES2_RUN_NO_DATA            "Error intento de disparo con cero muestras programado"

#define ES2_BUFFER_RD_WORD_INI         (st_error_code_start - 43)
#define charES2_BUFFER_RD_WORD_INI     "Error trama incorrecta en el buffer del canal, la trama leida del buffer no tiene la palabra de control inicial"

#define ES2_MEM_UCI_OVERFLOW           (st_error_code_start - 44)
#define charES2_MEM_UCI_OVERFLOW       "Error desbordamiento de la memoria interna del equipo"

#define ES2_BUFFER_ADD_OVF_1           (st_error_code_start - 45)
#define charES2_BUFFER_ADD_OVF_1       "Error desbordamiento de buffer TIPO 1, el numero de datos leidos es superior a la capacidad del buffer"

#define ES2_BUFFER_ADD_OVF_2           (st_error_code_start - 46)
#define charES2_BUFFER_ADD_OVF_2       "Error desbordamiento de buffer TIPO 2, el numero de datos leidos es superior al tamaño del buffer de los canales"

#define ES2_BUFFER_ADD_OVF_3           (st_error_code_start - 47)
#define charES2_BUFFER_ADD_OVF_3       "Error desbordamiento de buffer TIPO 3, el numero de datos leidos es superior al tamaño del buffer de los canales"

#define ES2_BUFFER_RD_WR1              (st_error_code_start - 48)
#define charES2_BUFFER_RD_WR1          "Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura"

#define ES2_BUFFER_RD_WR2              (st_error_code_start - 49)
#define charES2_BUFFER_RD_WR2          "Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura"

#define ES2_SYSTEM_NULL			      (st_error_code_start - 50)
#define charES2_SYSTEM_NULL           "PA Sytem structure doesn't exist (Null Pointer)"

#define ES2_MODULE_Number              (st_error_code_start - 51)
#define charES2_MODULE_Number          "Error en el número de módulos"

#define ES2_BASE_Number             (st_error_code_start - 52)
#define charES2_BASE_Number          "Error en el número de bases"

#define ES2_IdConnector                  (st_error_code_start - 53)
#define charES2_IdConnector              "Error en el identificador del conector"

#define ES2_TriggerSource                  (st_error_code_start - 54)
#define charES2_TriggerSource              "Error, la fuente de disparo no existe."

#define ES2_File                      (st_error_code_start - 55)
#define charES2_File                  "Error accessing file"

#define ES2_Acquire                      (st_error_code_start - 56)
#define charES2_Acquire                  "Error al intentar acceder a una sección crítica"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

//int ST_GetError (int error_code, char *str);

#endif
