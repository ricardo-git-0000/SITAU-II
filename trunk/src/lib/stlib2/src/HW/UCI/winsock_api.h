#ifndef fp_winsock_apiH
#define fp_winsock_apiH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include <winsock2.h>
#if !defined(_MSC_VER)
   #include <vcl.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "tad.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define USND_MSG_HEADER			(0x1234)
#define USND_MSG_FOOTER			(0x4321)

int ws_WSAStartup (unsigned char minor_version, unsigned char major_version);

int ws_WSACleanup(void);

int ws_socket (   int af,
                  int type,
                  int protocol,
                  SOCKET *socket_id);

int ws_connect (  char* ip_string,
                  unsigned short port,
                  int timeout_sec,
                  SOCKET tcp_socket);

int ws_ioctlsocket ( long cmd,
                     unsigned long *argp,
                     SOCKET tcp_socket);

int ws_bind (  struct sockaddr_in *name,
               int namelen,
               SOCKET socket_id);

int ws_setsockopt (  int level,
                     int optname,
                     char *optval,
                     int optlen,
                     SOCKET socket_id);

int ws_getsockopt (  int level,
                     int optname,
                     char *optval,
                     int *optlen,
                     SOCKET socket_id);

int ws_select_rdwr ( timeval *timeout,
                     SOCKET socket_id);

int ws_select_rd (   timeval *timeout,
                     SOCKET socket_id);

int ws_select_wr (   timeval *timeout,
                     SOCKET socket_id);

int ws_closesocket (SOCKET *socket_id);

int ws_getsockname (struct sockaddr *local_addr, SOCKET socket_id);

int ws_recv(char *data, int n_data, SOCKET socket_id);

int ws_send (char *data, int data_len, SOCKET socket_id);

int ws_send_msg (void *data, int data_len, SOCKET socket_id);

extern bool gb_wsa_startup; 
extern int gb_winsock_minor_version;
extern int gb_winsock_major_version;
extern AnsiString gb_winsock_description;
extern AnsiString gb_winsock_status;

#endif
