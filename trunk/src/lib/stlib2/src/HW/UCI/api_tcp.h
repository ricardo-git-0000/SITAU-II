#ifndef fp_api_tcpH
#define fp_api_tcpH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <winsock2.h>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "tad.h"
#include "xil_types.h"
#include "c_critical_section.h"

// ____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ============================================================================

#define MEGAS	                     64
#define TAMANIO	                  MEGAS*1024*1024/1
#define SIZE_BUFFER_ACQ             5242880 //5*1024*1024
#define TIMEOUT_FIRST_SEC_DEFAULT	0
#define TIMEOUT_FIRST_USEC_DEFAULT	50000//1000//1//500000

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern SOCKET uci_tcp_cfg;
extern SOCKET uci_tcp_acq;
extern short *gb_buffer_acquisition;
extern int gb_ethernet_connected;
extern int gb_connect_timeout;

extern unsigned char *datos_env;
extern unsigned char *datos_rec;

extern int gb_tcp_port_cfg;
extern int gb_tcp_port_acq;
extern char gb_ip_address[30];

extern CCriticalSection* gb_plock_tcp_cfg;
extern CCriticalSection* gb_plock_tcp_acq;

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

int TCP_Receive_ConfigPort (void *buffer, int data_max);

int TCP_ClearReceive_ConfigPort (void);

int TCP_ClearReceive_AcquisitionPort(void);

int TCPReceive_AcquisitionData (void *buffer, int data_max, bool acquire=true);

int TCP_Connected(void);

int TCP_MemoryResize(void);

int TCP_MemoryFree(void);

int TCP_OpenConnection (char *ip_address, int tcp_port_cfg, int tcp_port_acq);

int TCP_CloseConnection (void);

void prueba_velocidad(void);

void lee_qspi(const char *archivo,u32 direccion,int numbytes);

int TCP_ResetAcquisitionConnection (void);

int TCP_ResetConfigConnection (void);

int tcp_connect(char* ip_string, unsigned short port, int timeout_sec, SOCKET* tcp_socket, struct sockaddr* local_addr);

int tcp_close(SOCKET* tcp_socket);

int TCP_Ping(char *ip);

#endif

