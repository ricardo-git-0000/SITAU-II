//---------------------------------------------------------------------------
#ifndef fp_c_sitau_hw_errH
#define fp_c_sitau_hw_errH

#define cfghw_error_code_start      -600

#define CFGHW_OK                    0

#define CFGHW_NO_BOARD              (cfghw_error_code_start - 1)
#define charCFGHW_NO_BOARD          "Error: la tarjeta no existe"

#define CFGHW_BUFFER_OVERFLOW       (cfghw_error_code_start - 2)
#define charCFGHW_BUFFER_OVERFLOW   "Error: Buffer overflow"

#define CFGHW_NO_MEM                (cfghw_error_code_start - 3)
#define charCFGHW_NO_MEM            "Error en la reserva de memoria dinámica"

#define CFGHW_EEPROM                (cfghw_error_code_start - 4)
#define charCFGHW_EEPROM            "Error en la lectura de la EEPROM, dato corrupto"

#define CFGHW_FILE                  (cfghw_error_code_start - 5)
#define charCFGHW_FILE              "Error al abrir el archivo de configuración .SC'"

#define cfghw_error_code_end        CFGHW_FILE
#endif

