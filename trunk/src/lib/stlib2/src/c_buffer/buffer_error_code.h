#ifndef fp_buffer_error_codeH
#define fp_buffer_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define buffer_error_code_start           -700

#define EBUF_NONE                         0

#define EBUF_Acquire                      (buffer_error_code_start - 1)
#define charEBUF_Acquire                  "Error al intentar desbloquear el buffer"

#define EBUF_Parameter                    (buffer_error_code_start - 2)
#define charEBUF_Parameter                "Error: Rango de datos no válido"

#define EBUF_NoMemory                     (buffer_error_code_start - 3)
#define charEBUF_NoMemory                 "Error en la reserva de memoria dinámica"

#define EBUF_DataInconsistency            (buffer_error_code_start - 4)
#define charEBUF_DataInconsistency        "Error, inconsistencia entre los datos"

#define EBUF_IdScan                       (buffer_error_code_start - 5)
#define charEBUF_IdScan                   "Error en el identificador del barrido 'scan'"

#define EBUF_Overflow		               (buffer_error_code_start - 6)
#define charEBUF_Overflow	               "Error, desbordamiento del buffer al intentar escribir datos"

#define EBUF_Underflow		               (buffer_error_code_start - 7)
#define charEBUF_Underflow	               "Error, desbordamiento del buffer al intenter leer datos"

#define EBUF_NoHeader		               (buffer_error_code_start - 8)
#define charEBUF_NoHeader	               "Error, no se encuentra la palabra de inicio de trama en el buffer"

#define EBUF_Frame						      (buffer_error_code_start - 9)
#define charEBUF_Frame			            "Error en el format de la trama"

#define EBUF_IdGate                       (buffer_error_code_start - 10)
#define charEBUF_IdGate                   "Error en el identificador de la puerta"

#define EBUF_IdEncoder                    (buffer_error_code_start - 11)
#define charEBUF_IdEncoder                "Error en el identificador del encoder"

#define EBUF_File                         (buffer_error_code_start - 12)
#define charEBUF_File                     "Error de archivo, acceso al archivo incorrecto"

#define EBUF_Acquiring 	                  (buffer_error_code_start - 13)
#define charEBUF_Acquiring	               "Error, intento de lectura mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan()"

#define EBUF_DisabledDataFormat 	         (buffer_error_code_start - 14)
#define charEBUF_DisabledDataFormat	      "Error, formato de datos deshabilitado (Ver funciones ST_SetDataEnabled_XXX())"

#define EBUF_MODULE_Number              (buffer_error_code_start - 15)
#define charEBUF_MODULE_Number          "Error en el número de módulos"

#define EBUF_BASE_Number             (buffer_error_code_start - 16)
#define charEBUF_BASE_Number          "Error en el número de bases"

#define EBUF_CONFIG_NULL			      (buffer_error_code_start - 17)
#define charEBUF_CONFIG_NULL           "Config structure doesn't exist (Null Pointer)"

#define EBUF_IdVirtualChannel           (buffer_error_code_start - 18)
#define charEBUF_IdVirtualChannel       "Error en el identificador del barrido 'scan' o canal virtual"

#define EBUF_IdBASE                    (buffer_error_code_start - 19)
#define charEBUF_IdBASE                "Error de número de base, la base a la que se desea acceder esta fuera de rango"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_GetBufferErrorCode (int error_code, char *str);

#endif

