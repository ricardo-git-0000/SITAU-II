#ifndef base_error_codeH
#define base_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define base_error_code_start           -1700

#define EBASE_NONE                      0

#define EBASE_Parameter                 (base_error_code_start - 1)
#define charEBASE_Parameter             "Error: Rango de datos no válido"

#define EBASE_NoMemory                  (base_error_code_start - 2)
#define charEBASE_NoMemory              "Error en la reserva de memoria dinámica"

#define EBASE_File                      (base_error_code_start - 3)
#define charEBASE_File                  "Error accessing file"

#define EBASE_DataInconsistency         (base_error_code_start - 4)
#define charEBASE_DataInconsistency     "Error, inconsistencia entre los datos"

#define EBASE_PtrAccess_NULL			    (base_error_code_start - 5)
#define charEBASE_PtrAccess_NULL        "ERROR: Acceso a puntero NULL."

#define EBASE_MathFunction              (base_error_code_start - 6)
#define charEBASE_MathFunction          "Error en el cálculo de una función matemática."

#define EBASE_IdBASE                    (base_error_code_start - 7)
#define charEBASE_IdBASE			       "Error de número de base, la base a la que se desea acceder esta fuera de rango"

#define EBASE_NUM_CPB                   (base_error_code_start - 8)
#define charEBASE_NUM_CPB               "Error de número de coeficiente del filtro, el número de coeficiente al que se desea acceder está fuera de rango"

#define EBASE_NUM_FocalLaw              (base_error_code_start - 9)
#define charEBASE_NUM_FocalLaw          "Error, el número de leyes focales excede el máxio permitido (el número máximo de líneas es 255)"

#define EBASE_MODULE_Number              (base_error_code_start - 10)
#define charEBASE_MODULE_Number          "Error en el número de módulos"

#define EBASE_BASE_Number             (base_error_code_start - 11)
#define charEBASE_BASE_Number          "Error en el número de bases"

#define EBASE_AMPLIACommand_Number             (base_error_code_start - 12)
#define charEBASE_AMPLIACommand_Number          "Error en el número de comandos AMPLIA"

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
