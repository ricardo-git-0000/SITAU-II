#ifndef encoder_error_codeH
#define encoder_error_codeH

#define encoder_error_code_start      -800

#define EENC_NONE                      0

#define EENC_IdEncoder                 (encoder_error_code_start - 1)
#define charEENC_IdEncoder             "Error al acceder a un encoder que no existe"

#define EENC_Parameter                 (encoder_error_code_start - 2)
#define charEENC_Parameter             "Error: Rango de datos no válido"

#define EENC_NoMemory                  (encoder_error_code_start - 3)
#define charEENC_NoMemory              "Error en la reserva de memoria dinámica"

#define EENC_DataInconsistency         (encoder_error_code_start - 4)
#define charEENC_DataInconsistency     "Error, inconsistencia entre los datos"

#define EENC_FunctionNotAvailable      (encoder_error_code_start - 5)
#define charEENC_FunctionNotAvailable  "Error, funcionalidad no disponible en esta versión"

#define EENC_File                      (encoder_error_code_start - 6)
#define charEENC_File                  "Error accessing file"

#define encoder_error_code_end         EENC_FunctionNotAvailable
#endif

