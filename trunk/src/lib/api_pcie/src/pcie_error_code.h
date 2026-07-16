#ifndef pcie_error_codeH
#define pcie_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define pcie_error_code_start         -100

#define EPCIE_NONE                       0
#define EPCIE_USER						-1000

#define EPCIE_DRV_NULL                  (pcie_error_code_start - 1)
#define charEPCIE_DRV_NULL              "PCIe driver handle null"

#define EPCIE_BUFFER_LEAKAGE            (pcie_error_code_start - 2)
#define charEPCIE_BUFFER_LEAKAGE        "There are some bufferes used by user (leakege buffers)"

#define EPCIE_PARAMETER                      (pcie_error_code_start - 5)
#define charEPCIE_PARAMETER                  "Error: Rango de datos no válido"

#define EPCIE_NOINIT                     (pcie_error_code_start - 3)
#define charEPCIE_NOINIT                 "Error: Se ha llamado a ST_ConfigSys sin una llamada previa a ST_OpenSys"

#define EPCIE_NOCONNECT                  (pcie_error_code_start - 4)
#define charEPCIE_NOCONNECT              "Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys"

#define EPCIE_TIMEOUT                    (pcie_error_code_start - 6)
#define charEPCIE_TIMEOUT                "Se ha salido de un bucle por timeout"

#define EPCIE_CH                         (pcie_error_code_start - 7)
#define charEPCIE_CH                     "Error número de canales, el numero de canales es distinto al configurado por la función ST_OpenSys()"

#define EPCIE_NO_MEMORY                     (pcie_error_code_start - 8)
#define charEPCIE_NO_MEMORY                 "Error en la reserva de memoria dinámica"

#define EPCIE_BUFFER_NULL          (pcie_error_code_start - 9)
#define charEPCIE_BUFFER_NULL      "Error buffer is null"

#define EPCIE_BUFFER_ADQ_OVF_1           (pcie_error_code_start - 10)
#define charEPCIE_BUFFER_ADQ_OVF_1       "ERROR Se ha llenado el buffer de adquisición"

#define EPCIE_BUFFER_ADQ_OVF_2           (pcie_error_code_start - 11)
#define charEPCIE_BUFFER_ADQ_OVF_2       "Error desbordamiento de buffer (durante la adquisicion), el numero de datos leídos es superior al tamaño del buffer temporal de adquisición"

#define EPCIE_FILE                       (pcie_error_code_start - 12)
#define charEPCIE_FILE                   "Error accessing a file"

#define EPCIE_ZERO_DIVISION		         (pcie_error_code_start - 13)
#define charEPCIE_ZERO_DIVISION	         "Se va a intentar dividir por cero. Se aborta la operación"

#define EPCIE_EmptyBuffer 	            (pcie_error_code_start - 14)
#define charEPCIE_EmptyBuffer	         "Intento de lectura de Buffer vacío"

#define EPCIE_NOCONECT 	               (pcie_error_code_start - 15)
#define charEPCIE_NOCONECT	            "Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys()"

#define EPCIE_SCAN_ENC 	               (pcie_error_code_start - 16)
#define charEPCIE_SCAN_ENC	            "Error de Disparo: El sistema se encuentra en un estado de barrido por encoder ST_RunScanENC()"

#define EPCIE_SCAN 	                     (pcie_error_code_start - 17)
#define charEPCIE_SCAN	                  "Error de Programación: Intento de programación mientras el sistema encuentra en un estado de barrido configurado con ST_RunScanSW() o ST_RunScanENC()"

#define EPCIE_INVALID_ACCESS             (pcie_error_code_start - 18)
#define charEPCIE_INVALID_ACCESS         "Error Se ha intentado acceder a una posición no válida del vector"

#define EPCIE_INVALID_LINES              (pcie_error_code_start - 19)
#define charEPCIE_INVALID_LINES          "Error No se han calculado correctamente las líneas del barrido"

#define EPCIE_FL_FILE                    (pcie_error_code_start - 20)
#define charEPCIE_FL_FILE                "Error en el formato del archivo de leyes focales"

#define EPCIE_NMU_LOW			            (pcie_error_code_start - 21)
#define charEPCIE_NMU_LOW		            "Error en el número de muestras, no puede ser inferior a 31 después del diezmado"

#define EPCIE_NMU_HIGH		               (pcie_error_code_start - 22)
#define charEPCIE_NMU_HIGH	            "Error en el número de muestras, no puede ser superior a 20478 después del diezmado"// Error número de muestras, debe ser mayor de 31 a la salida del diezmador.

#define EPCIE_EEPROM_CFG			         (pcie_error_code_start - 23)
#define charEPCIE_EEPROM_CFG	   	      "Error de formato en la información del sistema leida de la memoria EEPROM"

#define EPCIE_FILE_CFG			            (pcie_error_code_start - 24)
#define charEPCIE_FILE_CFG	   	      "Error accediendo al archivo de configuración del sistema 'SITAU.SC'"

#define EPCIE_AD_TEST                    (pcie_error_code_start - 25)
#define charEPCIE_AD_TEST                "Error en los conversores AD de un módulo con el patrón de test"

#define EPCIE_NOT_PA_SYSTEM      	      (pcie_error_code_start - 26)
#define charEPCIE_NOT_PA_SYSTEM 	      "Intento de obtener datos de un sistema phased array en un sistema de tipo multicanal"

#define EPCIE_SCAN_LINES_NUMBER          (pcie_error_code_start - 27)
#define charEPCIE_SCAN_LINES_NUMBER 	   "Error, el número de lineas total de la adquisición excede el número máximo"

#define EPCIE_USER_MODE                  (pcie_error_code_start - 28)
#define charEPCIE_USER_MODE 	            "Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode()"

#define EPCIE_DataInconsistency          (pcie_error_code_start - 29)
#define charEPCIE_DataInconsistency 	   "Error, inconsistencia entre los datos"

#define EPCIE_InterfaceDetect            (pcie_error_code_start - 30)
#define charEPCIE_InterfaceDetect 	      "Error en la detección d ela interfaz"

#define EPCIE_NoDataAcquired             (pcie_error_code_start - 31)
#define charEPCIE_NoDataAcquired 	      "Error en la adquisición de datos"

#define EPCIE_PtrAccess_NULL			      (pcie_error_code_start - 32)
#define charEPCIE_PtrAccess_NULL         "ERROR: Acceso a puntero NULL."

#define EPCIE_MathFunction               (pcie_error_code_start - 33)
#define charEPCIE_MathFunction           "Error en el cálculo de una función matemática."

#define EPCIE_FunctionNotAvailable       (pcie_error_code_start - 34)
#define charEPCIE_FunctionNotAvailable   "Error function not available."

#define EPCIE_NoBuffer                   (pcie_error_code_start - 35)
#define charEPCIE_NoBuffer 	            "Error, el buffer interno ha sido creado"

#define EPCIE_IdThreadNULL               (pcie_error_code_start - 36)
#define charEPCIE_IdThreadNULL           "Error Thread handle is NULL."

#define EPCIE_ThreadResume               (pcie_error_code_start - 37)
#define charEPCIE_ThreadResume           "Error starting thread."

#define EPCIE_IdVirtualChannel           (pcie_error_code_start - 38)
#define charEPCIE_IdVirtualChannel       "Error en el identificador del barrido 'scan' o canal virtual"

#define EPCIE_IdGate                     (pcie_error_code_start - 39)
#define charEPCIE_IdGate                 "Error en el identificador de la puerta"

#define EPCIE_IdEncoder                  (pcie_error_code_start - 40)
#define charEPCIE_IdEncoder              "Error en el identificador del encoder"

#define EPCIE_Acquiring 	               (pcie_error_code_start - 41)
#define charEPCIE_Acquiring	            "Error, intento de disparo mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan()"

#define EPCIE_RUN_NO_DATA                (pcie_error_code_start - 42)
#define charEPCIE_RUN_NO_DATA            "Error intento de disparo con cero muestras programado"

#define EPCIE_BUFFER_RD_WORD_INI         (pcie_error_code_start - 43)
#define charEPCIE_BUFFER_RD_WORD_INI     "Error trama incorrecta en el buffer del canal, la trama leida del buffer no tiene la palabra de control inicial"

#define EPCIE_OVERFLOW           (pcie_error_code_start - 44)
#define charEPCIE_OVERFLOW       "Error desbordamiento de la memoria"

#define EPCIE_BUFFER_ADD_OVERFLOW           (pcie_error_code_start - 45)
#define charEPCIE_BUFFER_ADD_OVERFLOW       "Error buffer overflow"

#define EPCIE_BUFFER_ADD_OVF_2           (pcie_error_code_start - 46)
#define charEPCIE_BUFFER_ADD_OVF_2       "Error desbordamiento de buffer TIPO 2, el numero de datos leidos es superior al tamaño del buffer de los canales"

#define EPCIE_BUFFER_ADD_OVF_3           (pcie_error_code_start - 47)
#define charEPCIE_BUFFER_ADD_OVF_3       "Error desbordamiento de buffer TIPO 3, el numero de datos leidos es superior al tamaño del buffer de los canales"

#define EPCIE_BUFFER_RD_WR1              (pcie_error_code_start - 48)
#define charEPCIE_BUFFER_RD_WR1          "Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura"

#define EPCIE_BUFFER_RD_WR2              (pcie_error_code_start - 49)
#define charEPCIE_BUFFER_RD_WR2          "Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura"

#define EPCIE_SYSTEM_NULL			      (pcie_error_code_start - 50)
#define charEPCIE_SYSTEM_NULL           "PA Sytem structure doesn't exist (Null Pointer)"

#define EPCIE_MODULE_Number              (pcie_error_code_start - 51)
#define charEPCIE_MODULE_Number          "Error en el número de módulos"

#define EPCIE_BASE_Number             (pcie_error_code_start - 52)
#define charEPCIE_BASE_Number          "Error en el número de bases"

#define EPCIE_IdConnector                  (pcie_error_code_start - 53)
#define charEPCIE_IdConnector              "Error en el identificador del conector"

#define EPCIE_TriggerSource                  (pcie_error_code_start - 54)
#define charEPCIE_TriggerSource              "Error, la fuente de disparo no existe."

#define EPCIE_File                      (pcie_error_code_start - 55)
#define charEPCIE_File                  "Error accessing file"

#define EPCIE_IMAGE_HANDLE_NULL                  (pcie_error_code_start - 56)
#define charEPCIE_IMAGE_HANDLE_NULL              "Image handle null"

#define EPCIE_ACQUIRE                      (pcie_error_code_start - 57)
#define charEPCIE_ACQUIRE                  "Error al intentar desbloquear el buffer"

#define EPCIE_RecursiveRelease       (pcie_error_code_start - 58)
#define charEPCIE_RecursiveRelease   "Error, Recursive Release at critical section."

#define EPCIE_NO_THREAD_ACQ       (pcie_error_code_start - 59)
#define charEPCIE_NO_THREAD_ACQ   "Error, Acquisition Thread is not running."

#define EPCIE_IMAGE_SIZE       (pcie_error_code_start - 60)
#define charEPCIE_IMAGE_SIZE   "Image Size Error."

#define EPCIE_THREAD_ACQ       (pcie_error_code_start - 61)
#define charEPCIE_THREAD_ACQ   "Error, Acquisition Thread is running."

#define EPCIE_MUTEX_ACQUIRE 	               (pcie_error_code_start - 62)
#define charEPCIE_MUTEX_ACQUIRE	            "Error, No Mutex Acquired"

#define EPCIE_NO_THREAD_DMA       (pcie_error_code_start - 63)
#define charEPCIE_NO_THREAD_DMA   "Error, DMA Thread is not running."

#define EPCIE_EVENT       (pcie_error_code_start - 64)
#define charEPCIE_EVENT  "Event function return error."

#define EPCIE_SET_EVENT       (pcie_error_code_start - 65)
#define charEPCIE_SET_EVENT  "SetEvent() function return error."

#define EPCIE_NO_THREAD_GE       (pcie_error_code_start - 66)
#define charEPCIE_NO_THREAD_GE   "Error, SITAU GE Thread is not running."

#define EPCIE_DISABLED       (pcie_error_code_start - 67)
#define charEPCIE_DISABLED   "Error, SITAU PCIe interface disabled."

#define EPCIE_GTX_BLOCKS      (pcie_error_code_start - 68)
#define charEPCIE_GTX_BLOCKS 	"Error, el número de bloques gtx es superior a 256."

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
