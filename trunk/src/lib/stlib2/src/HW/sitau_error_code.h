#ifndef sitau_error_codeH
#define sitau_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define sitau_error_code_start               -3000

#define EHW_NONE                          0

#define EHW_Parameter                 (sitau_error_code_start - 1)
#define charEHW_Parameter             "Error: Rango de datos no válido"

#define EHW_Status_FSM            (sitau_error_code_start - 2)
#define charEHW_Status_FSM        "Error, UCI Status"

#define EHW_XML_Format            (sitau_error_code_start - 3)
#define charEHW_XML_Format       "Error en el formato del archivo de configuración 'SITAU.XML'"

#define EHW_File                       (sitau_error_code_start - 3)
#define charEHW_File                  "Error accessing a file"

#define EHW_PtrAccess_NULL			      (sitau_error_code_start - 4)
#define charEHW_PtrAccess_NULL         "ERROR: Acceso a puntero NULL."

#define EHW_BASE_Number             (sitau_error_code_start - 5)
#define charEHW_BASE_Number          "Error en el número de bases"

#define EHW_NoMemory                  (sitau_error_code_start - 6)
#define charEHW_NoMemory              "Error en la reserva de memoria dinámica"

#define EHW_NoXMLData                  (sitau_error_code_start - 7)
#define charEHW_NoXMLData              "Error no existen datos de configuración hardware XML"

#define EHW_DataInconsistency         (sitau_error_code_start - 8)
#define charEHW_DataInconsistency     "Error, inconsistencia entre los datos"

#define EHW_IdEncoder         (sitau_error_code_start - 9)
#define charEHW_IdEncoder     "SITAU XML Config File: Error en la configuración de los encoders"

#define EHW_FRONTEND_NULL         (sitau_error_code_start - 10)
#define charEHW_FRONTEND_NULL     "SITAU XML Config File: Error en la configuración del FORNTEND"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_GetSITAUErrorCode (int error_code, char *str);

#endif
