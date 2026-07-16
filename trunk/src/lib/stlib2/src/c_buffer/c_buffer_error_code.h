#ifndef fp_c_buffer_error_codeH
#define fp_c_buffer_error_codeH

#define c_buffer_error_code_start         0

#define EBUF_NONE                         0

#define EBUF_Acquire                      (c_buffer_error_code_start - 1)
#define charEBUF_Acquire                  "Error al intentar desbloquear el buffer"

#define EBUF_Parameter                    (c_buffer_error_code_start - 2)
#define charEBUF_Parameter                "Error: Rango de datos no válido"

#define EBUF_NoMemory                     (c_buffer_error_code_start - 3)
#define charEBUF_NoMemory                 "Error en la reserva de memoria dinámica"

#define EBUF_DataInconsistency            (c_buffer_error_code_start - 4)
#define charEBUF_DataInconsistency        "Error, inconsistencia entre los datos"

#define EBUF_IdScan                       (c_buffer_error_code_start - 5)
#define charEBUF_IdScan                   "Error en el identificador del barrido 'scan'"

#define EBUF_Overflow		               (c_buffer_error_code_start - 6)
#define charEBUF_Overflow	               "Error, desbordamiento del buffer al intentar escribir datos"

#define EBUF_Underflow		               (c_buffer_error_code_start - 7)
#define charEBUF_Underflow	               "Error, desbordamiento del buffer al intenter leer datos"

#define EBUF_NoHeader		               (c_buffer_error_code_start - 8)
#define charEBUF_NoHeader	               "Error, no se encuentra la palabra de inicio de trama en el buffer"

#define EBUF_Frame						      (c_buffer_error_code_start - 9)
#define charEBUF_Frame			            "Error en el format de la trama"

#define EBUF_IdGate                       (c_buffer_error_code_start - 10)
#define charEBUF_IdGate                   "Error en el identificador de la puerta"

#define EBUF_IdEncoder                    (c_buffer_error_code_start - 11)
#define charEBUF_IdEncoder                "Error en el identificador del encoder"

#define EBUF_File                         (c_buffer_error_code_start - 12)
#define charEBUF_File                     "Error de archivo, acceso al archivo incorrecto"

#define EBUF_Acquiring 	                  (c_buffer_error_code_start - 13)
#define charEBUF_Acquiring	               "Error, intento de lectura mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan()"

#define EBUF_DisabledDataFormat 	         (c_buffer_error_code_start - 14)
#define charEBUF_DisabledDataFormat	      "Error, formato de datos deshabilitado (Ver funciones ST_SetEnabledScanXXX_XXX())"

#define c_buffer_error_code_end           (c_buffer_error_code_start - EBUF_DisabledDataFormat)

#endif

