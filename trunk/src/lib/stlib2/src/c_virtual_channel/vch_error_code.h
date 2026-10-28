#ifndef vch_error_codeH
#define vch_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define vch_error_code_start           -1500

#define EVCH_NONE                      0

#define EVCH_Parameter                 (vch_error_code_start - 1)
#define charEVCH_Parameter             "Error: Rango de datos no válido"

#define EVCH_NoMemory                  (vch_error_code_start - 2)
#define charEVCH_NoMemory              "Error en la reserva de memoria dinámica"

#define EVCH_File                      (vch_error_code_start - 3)
#define charEVCH_File                  "Error accessing file"

#define EVCH_DataInconsistency         (vch_error_code_start - 4)
#define charEVCH_DataInconsistency     "Error, inconsistencia entre los datos"

#define EVCH_PtrAccess_NULL			   (vch_error_code_start - 5)
#define charEVCH_PtrAccess_NULL        "ERROR: Acceso a puntero NULL."

#define EVCH_MathFunction              (vch_error_code_start - 6)
#define charEVCH_MathFunction          "Error en el cálculo de una función matemática."

#define EVCH_PA_NULL			            (vch_error_code_start - 7)
#define charEVCH_PA_NULL               "PA structure doesn't exist (Null Pointer)"

#define EVCH_WEDGE_NULL			         (vch_error_code_start - 8)
#define charEVCH_WEDGE_NULL            "Wedge structure doesn't exist (Null Pointer)"

#define EVCH_SPECIMEN_NULL			      (vch_error_code_start - 9)
#define charEVCH_SPECIMEN_NULL         "Specimen structure doesn't exist (Null Pointer)"

#define EVCH_SCAN_NULL			         (vch_error_code_start - 10)
#define charEVCH_SCAN_NULL             "Scan structure doesn't exist (Null Pointer)"

#define EVCH_BASE_NULL			         (vch_error_code_start - 11)
#define charEVCH_BASE_NULL             "BASE structure doesn't exist (Null Pointer)"

#define EVCH_MODULE_NULL			      (vch_error_code_start - 12)
#define charEVCH_MODULE_NULL           "MODULE structure doesn't exist (Null Pointer)"

#define EVCH_SYSTEM_NULL			      (vch_error_code_start - 13)
#define charEVCH_SYSTEM_NULL           "PA Sytem structure doesn't exist (Null Pointer)"

#define EVCH_CONFIG_NULL			      (vch_error_code_start - 14)
#define charEVCH_CONFIG_NULL           "Config structure doesn't exist (Null Pointer)"

#define EVCH_NMU_Underflow			      (vch_error_code_start - 15)
#define charEVCH_NMU_Underflow		   "Error en el número de muestras, no puede ser inferior a 31 después del diezmado"

#define EVCH_NMU_Overflow			      (vch_error_code_start - 16)
#define charEVCH_NMU_Overflow	         "Error en el número de muestras, no puede ser superior a 20478 después del diezmado"// Error número de muestras, debe ser mayor de 31 a la salida del diezmador.

#define EVCH_AScanNumber               (vch_error_code_start - 17)
#define charEVCH_AScanNumber           "Error, el número de leyes focales excede el máxio permitido (el número máximo de líneas es 255)"

#define EVCH_IdMOD                     (vch_error_code_start - 18)
#define charEVCH_IdMOD                 "Error de número de módulo, el módulo al que se desea acceder esta fuera de rango"

#define EVCH_IdBASE                    (vch_error_code_start - 19)
#define charEVCH_IdBASE                "Error de número de base, la base a la que se desea acceder esta fuera de rango"

#define EVCH_IdPASystem                    (vch_error_code_start - 20)
#define charEVCH_IdPASystem                "Error de número de sub-sistema, el sub-sistema al que se desea acceder esta fuera de rango"

#define EVCH_USER_MODE      (vch_error_code_start - 21)
#define charEVCH_USER_MODE 	"Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode()"

#define EVCH_ACQUISITION_TYPE      (vch_error_code_start - 22)
#define charEVCH_ACQUISITION_TYPE 	"Error, tipo de adquisición incorrecto (MC, PA, TFM)"

#define EVCH_GTXGeometry (vch_error_code_start - 24) // -1524: dimensiones que no permiten un bloque GTX valido.
#define charEVCH_GTXGeometry "GTX: muestras, canales o rafaga invalidos; una ley debe caber en el bloque y los contadores deben ser representables." // Evita divisiones por cero y overflow durante la carga.
#define EVCH_PinToChannelSize (vch_error_code_start - 23) // -1523: mapa incompatible con los elementos/bloques configurados.
#define charEVCH_PinToChannelSize "pin_to_channel: longitud incompatible con los elementos configurados (BASES_NUMBER x 32 para sonda completa); se admite un patron valido de 128 para 256." // Describe el error sin cambiar los codigos existentes.

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_GetVCHErrorCode (int error_code, char *str);

#endif
