#ifndef fp_tcp_error_codeH
#define fp_tcp_error_codeH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define tcp_error_code_start        -100

#define ETCP_NONE                   0

#define ETCP_FILE                   (tcp_error_code_start - 1)
#define charETCP_FILE               "Error de archivo, acceso al archivo incorrecto"

#define ETCP_RANGO                  (tcp_error_code_start - 2)
#define charETCP_RANGO              "Error de rango, la variable pasada como parámetro está fuera de rango"

#define ETCP_IdMOD                  (tcp_error_code_start - 3)
#define charETCP_IdMOD              "Error de número de módulo, el módulo al que se desea acceder esta fuera de rango"

#define ETCP_IdBASE                 (tcp_error_code_start - 4)
#define charETCP_IdBASE             "Error de número de base, la base a la que se desea acceder esta fuera de rango"

#define ETCP_CH                     (tcp_error_code_start - 5)
#define charETCP_CH                 "Error de número de canal, el canal al que se desea acceder esta fuera de rango"

#define ETCP_NO_MEM                 (tcp_error_code_start - 6)
#define charETCP_NO_MEM             "Error en la reserva de memoria"

#define ETCP_WMPAR                  (tcp_error_code_start - 7)
#define charETCP_WMPAR              "Error de desboramiento de la memoria de parámetros"

#define ETCP_INVALID_ACCESS         (tcp_error_code_start - 8)
#define charETCP_INVALID_ACCESS     "Error Se ha intentado acceder a una posición no válida del vector"

#define ETCP_NO_INI                 (tcp_error_code_start - 9)
#define charETCP_NO_INI             "Error Se ha intentado acceder las estructuras de los módulos antes de su inicialización MOD_RegIni()"

#define ETCP_N_FL                   (tcp_error_code_start - 10)
#define charETCP_N_FL               "Error en el número de lineas (leyes focales)"

#define ETCP_MEM                    (tcp_error_code_start - 11)
#define charETCP_MEM                "Error en la reserva de memoria dinámica"

#define ETCP_CONNECT                (tcp_error_code_start - 12)
#define charETCP_CONNECT            "Error connecting with the SITAU equipment"

#define ETCP_CONNECT_TIMEOUT        (tcp_error_code_start - 13)
#define charETCP_CONNECT_TIMEOUT    "Timeout en la conexión TCP"

#define ETCP_SOCKET                 (tcp_error_code_start - 14)
#define charETCP_SOCKET             "Error socket"

#define ETCP_CSIC                   (tcp_error_code_start - 15)
#define charETCP_CSIC               "Error en la lectura de la palabra de control de la FPGA"

#define ETCP_MODULE_Number          (tcp_error_code_start - 16)
#define charETCP_MODULE_Number      "Error en el número de módulos"

#define ETCP_BASE_Number            (tcp_error_code_start - 17)
#define charETCP_BASE_Number        "Error en el número de bases"

#define ETCP_Checksum               (tcp_error_code_start - 18)
#define charETCP_Checksum       "Error de Checksum"

#define ETCP_WSAStartup               (tcp_error_code_start - 19)
#define charETCP_WSAStartup			   "Error WSAStartup (winsock is not started)"

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

int ST_GetTCPErrorCode (int error_code, char *str);

#endif


