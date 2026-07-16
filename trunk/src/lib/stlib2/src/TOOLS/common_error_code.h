#ifndef common_error_codeH
#define common_error_codeH

#define common_error_code_start     -50

#define ECOM_NONE                   0

#define ECOM_PathFile               (common_error_code_start - 1)
#define charECOM_PathFile           "Error accessing path or file"

#define ECOM_Parameter              (common_error_code_start - 2)
#define charECOM_Parameter          "Error: Parámetro fuera de rango."

#define ECOM_NoMemory               (common_error_code_start - 3)
#define charECOM_NoMemory           "Error en la reserva de memoria dinámica"

#define ECOM_MathFunction           (common_error_code_start - 4)
#define charECOM_MathFunction       "Error en el cálculo de una función matemática."

#define ECOM_RecursiveAcquire       (common_error_code_start - 5)
#define charECOM_RecursiveAcquire   "Error, Recursive Acquire at critical section."

#define ECOM_RecursiveRelease       (common_error_code_start - 6)
#define charECOM_RecursiveRelease   "Error, Recursive Release at critical section."

#define ECOM_Acquire                (common_error_code_start - 7)
#define charECOM_Acquire            "Error acquiring critical section ."

#define ECOM_DeleteAcquire          (common_error_code_start - 8)
#define charECOM_DeleteAcquire      "Error deleting component whitout acquiring critical section ."

#define ECOM_IdThreadNULL           (common_error_code_start - 9)
#define charECOM_IdThreadNULL       "Error deleting component whitout acquiring critical section ."

#define ECOM_DataInconsistency       (common_error_code_start - 10)
#define charECOM_DataInconsistency    "Error, Existe una inconsistencia en los datos."

#define ECOM_PtrAccess_NULL			      (common_error_code_start - 11)
#define charECOM_PtrAccess_NULL         "ERROR: Acceso a puntero NULL."

#define common_error_code_end       ECOM_IdThreadNULL

int GetCommonErrorCode (int error_code, char *str);

#endif