#ifndef common_errH
#define common_errH

#define common_offset -0x5000

// OK, resultado de la operacion correcto, no se ha producido ningun error.
#define ECOM_NONE             0

// ERROR: no se ha podido abrir el fichero especificado
#define ECOM_FILE              (common_offset - 1)
#define charECOM_FILE          "Error de archivo, acceso al archivo incorrecto"

#define ECOM_RANGE              (common_offset - 2)
#define charECOM_RANGE          "Error: Parámetro fuera de rango."

#define ECOM_NO_MEM               (common_offset - 3)
#define charECOM_NO_MEM           "Error en la reserva de memoria dinámica"

#define ECOM_MathFunction              (common_offset - 4)
#define charECOM_MathFunction           "Error en el cálculo de una función matemática."

#endif