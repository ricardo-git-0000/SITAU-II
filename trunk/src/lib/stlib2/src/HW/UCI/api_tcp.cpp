// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include <stdio.h>
#include <stdlib.h>
//#include <winsock2.h>
#include <time.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include "tad.h"
#include "api_tcp.h"
#include "c_log_file.h"
#include "tcp_error_code.h"
#include "winsock_api.h"
#include "tcp_comm.h"
#include "common_error_code.h"
#include "tcp_error_code.h"
#include "icmplib.h"
#include "c_log_message.h"

// _____________________________________________________________________________
// ------ C O N S T A N T E S
// =============================================================================

//#define	DEFDATALEN	(64-ICMP_MINLEN)
//#define	MAXIPLEN	60
//#define	MAXICMPLEN	76
//#define	MAXPACKET	(65536 - 60 - ICMP_MINLEN)
//
//using namespace std;
//
//uint16_t in_cksum(uint16_t* addr, unsigned len);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

unsigned char *datos_env = NULL;//[TAMANIO];
unsigned char *datos_rec = NULL;//[TAMANIO];
short *gb_buffer_acquisition = NULL;//[SIZE_BUFFER_UDP];

char ip_char[20] = "161.111.29.177";
//char ip_char[20] = "192.168.2.10";
unsigned short puerto = 6002;
unsigned short puerto_img = 6008;

int gb_connect_timeout = 3;
SOCKET uci_tcp_cfg = INVALID_SOCKET;
SOCKET uci_tcp_acq = INVALID_SOCKET;

CCriticalSection *gb_plock_tcp_cfg = NULL;
CCriticalSection *gb_plock_tcp_acq = NULL;

static long gb_timeout_first_sec  = TIMEOUT_FIRST_SEC_DEFAULT;
static long gb_timeout_first_usec = TIMEOUT_FIRST_USEC_DEFAULT;

int gb_tcp_port_cfg = 0;
int gb_tcp_port_acq = 0;
char gb_ip_address[30] = "XXX.XXX.XXX.XXX";

int gb_ethernet_connected = 0;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_Connected(void)
{
   return gb_ethernet_connected;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_MemoryResize(void)
{
	if (datos_env == NULL)
   {
      if ((datos_env = (unsigned char *)malloc(TAMANIO)) == NULL)
		   return ELOG(charETCP_NO_MEM, ETCP_NO_MEM);
   }
	if (datos_rec == NULL)
   {
   	if ((datos_rec = (unsigned char *)malloc(TAMANIO)) == NULL)
		return ELOG(charETCP_NO_MEM, ETCP_NO_MEM);
   }
	if (gb_buffer_acquisition == NULL)
   {
   	if ((gb_buffer_acquisition = (short *)malloc(SIZE_BUFFER_ACQ * sizeof(short))) == NULL)
		return ELOG(charETCP_NO_MEM, ETCP_NO_MEM);
   }
   gb_plock_tcp_cfg = new CCriticalSection("TCP CFG Lock");
   gb_plock_tcp_acq = new CCriticalSection("TCP ACQ Lock");
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_MemoryFree(void)
{
	if (datos_env != NULL) free(datos_env);
   datos_env = NULL;
	if (datos_rec != NULL) free(datos_rec);
   datos_rec = NULL;
	if (gb_buffer_acquisition != NULL) free(gb_buffer_acquisition);
   gb_buffer_acquisition = NULL;
   if (gb_plock_tcp_cfg != NULL) delete gb_plock_tcp_cfg;
   gb_plock_tcp_cfg = NULL;
   if (gb_plock_tcp_acq != NULL) delete gb_plock_tcp_acq;
   gb_plock_tcp_acq = NULL;
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int tcp_close(SOCKET *tcp_socket)
{
   if (!gb_no_hw)
   {
      if (*tcp_socket != INVALID_SOCKET) ws_closesocket(tcp_socket);
   }
//   WSACleanup();
	return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int tcp_connect (char* ip_string, unsigned short port, int timeout_sec, SOCKET *tcp_socket, struct sockaddr *local_addr)
{
SOCKET socket_id = INVALID_SOCKET;
unsigned long non_blocking = 1;
int result;
int buffer_lenght = 1*1024*1024;
int output_len = sizeof (int);
int output_value = 0;
DWORD timeout = 3000 ,timeout_out;
int timeout_len=sizeof(timeout);
bool tcp_nodelay=1;
int tcp_nodelay_len=sizeof(tcp_nodelay);

   if (!gb_no_hw)
   {
      // Initialize Winsock
      if ((result = ws_WSAStartup(2,2)) < 0)
         return RLOG(result);

      // Create a SOCKET for connecting to server
      if ((result = ws_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &socket_id)) < 0)
      {
         tcp_close(&socket_id);
         return RLOG(result);
      }

      // buffer_length
      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_SNDBUF,
                                 (char *)&output_value,
                                 &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = setsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_SNDBUF,
                                 (char *)&buffer_lenght,
                                 sizeof(buffer_lenght))) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_SNDBUF,
                                 (char *)&output_value,
                                 &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      // Specifies the total per-socket buffer space reserved for sends.
      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_SNDBUF,
                                 (char *)&output_value,
                                 &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = setsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_SNDBUF,
                                 (char *)&buffer_lenght,
                                 sizeof(buffer_lenght))) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

       if ((result = getsockopt(socket_id, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      // Specifies the total per-socket buffer space reserved for receives.
      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVBUF,
                                 (char *)&output_value,
                                 &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = setsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVBUF,
                                 (char *)&buffer_lenght,
                                 sizeof(buffer_lenght))) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVBUF,
                                 (char *)&output_value,
                                 &output_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      // Sets the timeout, in milliseconds, for blocking receive calls.
      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVTIMEO,
                                 (char *)&timeout_out,
                                 &timeout_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = setsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVTIMEO,
                                 (char *)&timeout,
                                 timeout_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = getsockopt ( socket_id,
                                 SOL_SOCKET,
                                 SO_RCVTIMEO,
                                 (char *)&timeout_out,
                                 &timeout_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      // Disables the Nagle algorithm for send coalescing.
      // This socket option is included for backward compatibility with Windows Sockets 1.1
      if ((result = setsockopt ( socket_id,
                                 IPPROTO_TCP,
                                 TCP_NODELAY,
                                 (char *)&tcp_nodelay,
                                 tcp_nodelay_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      if ((result = getsockopt ( socket_id,
                                 IPPROTO_TCP,
                                 TCP_NODELAY,
                                 (char *)&tcp_nodelay,
                                 &tcp_nodelay_len)) == SOCKET_ERROR)
      {
         tcp_close(&socket_id);
         return ELOG(charETCP_SOCKET, ETCP_SOCKET);
      }

      // Put socket in non-blocking state
      //if ((result = ws_ioctlsocket(FIONBIO, &non_blocking, socket_id)) < 0)
    //  {
    //     tcp_close(&socket_id);
    //     return RLOG(result);
    //  }

      // Connect to server.
      if ((result = ws_connect(ip_string, port, timeout_sec, socket_id)) < 0)
      {
         tcp_close(&socket_id);
         return RLOG(result);
      }

      // Determine the completion of the connection request
      //timeout_select.tv_sec = timeout_sec;
      //timeout_select.tv_usec = 0;
      //if ((result = ws_select_rdwr(&timeout_select, socket_id)) < 0)
      //{
      //   tcp_close(&socket_id);
      //   return RLOG(result);
      //}

      //// Checking if the sockect is connected.
      //if ((result = ws_connect(ip_string, port, timeout_sec, socket_id)) < 0)
      //{
      //   tcp_close(&socket_id);
      //   return RLOG(result);
      //}

      //if ((result = ws_getsockname (local_addr, socket_id)) < 0)
      //{
      //   tcp_close(&socket_id);
      //   return RLOG(result);
      //}
      *tcp_socket = socket_id;
      return ETCP_NONE;//socket_id;
    }
//#else
	else return 1;
//#endif


//#if defined(TCP_RICARDO)
//
//
//SOCKET socket_id = INVALID_SOCKET;
//unsigned long non_blocking = 1;
//int result;
//timeval timeout_select;
//int buffer_lenght = 1*1024*1024;
//int output_len = sizeof (int);
//int output_value = 0;
//DWORD timeout = 3000, timeout_out;
//int timeout_len=sizeof(timeout);
//bool tcp_nodelay=1;
//int tcp_nodelay_len=sizeof(tcp_nodelay);
//
////#if !defined(_NO_HW)
////
////WSADATA wsaData;
////   result = WSAStartup(MAKEWORD(2,2), &wsaData);
////	if (result != 0) return INVALID_SOCKET;
////
////	socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
////	if (socket_id == INVALID_SOCKET)
////	{
////		WSACleanup();
////		return socket_id;
////	}
////
////   // Initialize Winsock
//// //  if ((result = ws_WSAStartup(2,2)) < 0)
//// //     return RLOG(result);
////
//// //  // Create a SOCKET for connecting to server
////	//if ((result = ws_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &socket_id)) < 0)
//// //  {
//// //     tcp_close(&socket_id);
//// //     return RLOG(result);
//// //  }
////
////	// buffer_length
////	if ((result = getsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_SNDBUF,
////                              (char *)&output_value,
////                              &output_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = setsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_SNDBUF,
////                              (char *)&buffer_lenght,
////                              sizeof(buffer_lenght))) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////    if ((result = getsockopt(socket_id, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////	// buffer_length
////   if ((result = getsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_RCVBUF,
////                              (char *)&output_value,
////                              &output_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = setsockopt ( socket_id,
////                              SOL_SOCKET, SO_RCVBUF,
////                              (char *)&buffer_lenght,
////                              sizeof(buffer_lenght))) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = getsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_RCVBUF,
////                              (char *)&output_value,
////                              &output_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////	// recv_timeout
////   if ((result = getsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_RCVTIMEO,
////                              (char *)&timeout_out,
////                              &timeout_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = setsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_RCVTIMEO,
////                              (char *)&timeout,
////                              timeout_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = getsockopt ( socket_id,
////                              SOL_SOCKET,
////                              SO_RCVTIMEO,
////                              (char *)&timeout_out,
////                              &timeout_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////	// NO DELAY
////   if ((result = setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, tcp_nodelay_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   if ((result = getsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, &tcp_nodelay_len)) == SOCKET_ERROR)
////   {
////      tcp_close(&socket_id);
////      return ELOG(charETCP_SOCKET, ETCP_SOCKET);
////   }
////
////   // Put socket in non-blocking state
////	/*if ((result = ws_ioctlsocket(FIONBIO, &non_blocking, socket_id)) < 0)
////   {
////      tcp_close(&socket_id);
////      return RLOG(result);
////   }*/
////
////	// Connect to server.
////struct sockaddr_in server_addr;
////
////	memset(&server_addr, 0, sizeof(server_addr));
////	server_addr.sin_family = AF_INET;
////	server_addr.sin_addr.s_addr = inet_addr(ip_string);
////	server_addr.sin_port = htons(port);
////
////	if ((result = connect(socket_id, (struct sockaddr *) &server_addr, sizeof(server_addr))) == SOCKET_ERROR)
//////	if ((result = ws_connect(ip_string, port, timeout_sec, socket_id)) < 0)
////   {
////      tcp_close(&socket_id);
////      return RLOG(result);
////   }
////
////   // Determine the completion of the connection request
////   timeout_select.tv_sec = timeout_sec;
////   timeout_select.tv_usec = 0;
////   if ((result = ws_select_rdwr(&timeout_select, socket_id)) < 0)
////   {
////      tcp_close(&socket_id);
////      return RLOG(result);
////   }
////
////   // Checking if the sockect is connected.
////   if ((result = ws_connect(ip_string, port, timeout_sec, socket_id)) < 0)
////   {
////      tcp_close(&socket_id);
////      return RLOG(result);
////   }
////
////   if ((result = ws_getsockname (local_addr, socket_id)) < 0)
////   {
////      tcp_close(&socket_id);
////      return RLOG(result);
////   }
////   *tcp_socket = socket_id;
////#endif
//
////#ifdef kk
////	WSADATA wsaData;
////	SOCKET tcpcom_socket = INVALID_SOCKET;
////	struct sockaddr_in server_addr;
////	unsigned long non_blocking = 1;
////	int ret;
////
////	int buffer_lenght = 1*1024*1024;
////    int output_len = sizeof (int);
////    int output_value = 0;
////
////	DWORD timeout = 15000,timeout_out;
////	int timeout_len=sizeof(timeout);
////	bool tcp_nodelay=1;
////	int tcp_nodelay_len=sizeof(tcp_nodelay);
////
////	ret = WSAStartup(MAKEWORD(2,2), &wsaData);
////	if (ret != 0) return tcpcom_socket;
////
////	tcpcom_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
////
////	if (tcpcom_socket == INVALID_SOCKET)
////	{
////		WSACleanup();
////		return tcpcom_socket;
////	}
////	// buffer_length
////	ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
////
////    ret = setsockopt(tcpcom_socket, SOL_SOCKET, SO_SNDBUF, (char *) &buffer_lenght, sizeof(buffer_lenght));
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"setsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"Set SO_RCVBUF: %d\n",buffer_lenght);
////
////    ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_SNDBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
////
////	// buffer_length
////	ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVBUF, (char *) &output_value, &output_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
////
////    ret = setsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVBUF, (char *) &buffer_lenght, sizeof(buffer_lenght));
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"setsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"Set SO_RCVBUF: %d\n",buffer_lenght);
////
////    ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVBUF, (char *) &output_value, &output_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
////
////	// recv_timeout
////	ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout_out, &timeout_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVTIMEO Value: %ld\n", timeout_out);
////
////    ret = setsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, timeout_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"setsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"Set SO_RCVTIMEO: %d\n",timeout_out);
////
////    ret = getsockopt(tcpcom_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout_out, &timeout_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"SO_RCVTIMEO Value: %ld\n", timeout_out);
////
////
////	// NO DELAY
////    ret = setsockopt(tcpcom_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, tcp_nodelay_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"setsockopt for TCP_NODELAY failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"Set TCP_NODELAY: %d\n",tcp_nodelay_len);
////
////    ret = getsockopt(tcpcom_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, &tcp_nodelay_len);
////    if (ret == SOCKET_ERROR) {
////        wprintf(L"getsockopt for TCP_NODELAY failed with error: %u\n", WSAGetLastError());
////    } else
////        wprintf(L"TCP_NODELAY Value: %ld\n", tcp_nodelay);
////
////	// Connect to server.
////	memset(&server_addr, 0, sizeof(server_addr));
////	server_addr.sin_family = AF_INET;
////	server_addr.sin_addr.s_addr = inet_addr(ip_str_dst);
////	server_addr.sin_port = htons(port);
////
////#if !defined(_NO_HW)
////   ret = connect(tcpcom_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
////
////   if (ret == SOCKET_ERROR)
////	{
////		closesocket(tcpcom_socket);
////		WSACleanup();
////		return INVALID_SOCKET;
////	}
////#endif
////	dbg_printf("Conected to %s:%d\n",ip_str_dst,port);
////#endif
//	return ETCP_NONE;
//#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_CloseConnection (void)
{
   if (!gb_no_hw)
   {
      tcp_close(&uci_tcp_cfg);
      tcp_close(&uci_tcp_acq);
      WSACleanup();
   }
   uci_tcp_cfg = INVALID_SOCKET;
   uci_tcp_acq = INVALID_SOCKET;
   gb_tcp_port_cfg = 0;
   gb_tcp_port_acq = 0;
   strcpy_s(gb_ip_address, sizeof(gb_ip_address), "XXX.XXX.XXX.XXX");
   gb_ethernet_connected = 0;
	return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_OpenConnection (   char *ip_address,
                           int tcp_port_cfg,
                           int tcp_port_acq)
{
int result;
struct sockaddr local_addr;

   if (ip_address == NULL) return ELOG(charETCP_RANGO,ETCP_RANGO);
   if (tcp_port_cfg < 0) return ELOG(charETCP_RANGO,ETCP_RANGO);
   if (tcp_port_acq < 0) return ELOG(charETCP_RANGO,ETCP_RANGO);

   if ((result = tcpcom_connect(ip_address, tcp_port_cfg, &uci_tcp_cfg)) < 0 || uci_tcp_cfg == INVALID_SOCKET)
   {
      TCP_CloseConnection();
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   if ((result = tcpcom_connect(ip_address, tcp_port_acq, &uci_tcp_acq)) < 0 || uci_tcp_acq == INVALID_SOCKET)
   {
      TCP_CloseConnection();
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   gb_ethernet_connected = 1;

   _snprintf_s(gb_ip_address, sizeof(gb_ip_address), _TRUNCATE, "%s", ip_address);
   gb_tcp_port_cfg = tcp_port_cfg;
   gb_tcp_port_acq = tcp_port_acq;
	return ETCP_NONE;

   if ((result = tcp_connect(ip_address, tcp_port_cfg, gb_connect_timeout, &uci_tcp_cfg, &local_addr)) < 0)
   {
      TCP_CloseConnection();
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   if ((result = tcp_connect(ip_address, tcp_port_acq, gb_connect_timeout, &uci_tcp_acq, &local_addr)) < 0)
   {
      TCP_CloseConnection();
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   gb_ethernet_connected = 1;

   _snprintf_s(gb_ip_address, sizeof(gb_ip_address), _TRUNCATE, "%s", ip_address);
   gb_tcp_port_cfg = tcp_port_cfg;
   gb_tcp_port_acq = tcp_port_acq;
	return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_Receive_ConfigPort (void *buffer, int data_max)
{
int result, bytes_recv = 0;
struct timeval timeout;

   timeout.tv_sec = gb_timeout_first_sec;
   timeout.tv_usec = gb_timeout_first_usec;

   if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
      return ELOG(charECOM_Acquire, ECOM_Acquire);
try
{
   if (!gb_no_hw)
   {
      if ((result = ws_select_rd(&timeout, uci_tcp_cfg)) < 0) return RLOG(result);
      if (result == 0) return 0; // TIMEOUT: No hay datos

      if ((bytes_recv = ws_recv((char *)buffer, data_max, uci_tcp_cfg)) < 0)
         return RLOG(result);
      return bytes_recv;
   }
   else return 0;
}
__finally
{
   if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
}
	return bytes_recv;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_ClearReceive_ConfigPort (void)
{
int result, bytes_recv = 0;
struct timeval timeout;
vector<char> buffer;
int n_bytes_recv, buffer_size = 1024;
CTimeout *timer = NULL;
bool fin;

   timeout.tv_sec = gb_timeout_first_sec;
   timeout.tv_usec = gb_timeout_first_usec;

   if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
      return ELOG(charECOM_Acquire, ECOM_Acquire);
try
{
   if (!gb_no_hw)
   {
      buffer.resize(buffer_size);
      if ((timer = new CTimeout(10000, MILISECONDS)) != NULL)
         timer->Start();
      n_bytes_recv = 0;
      fin = false;
      while (!fin)
      {
          if ((result = ws_select_rd(&timeout, uci_tcp_cfg)) < 0) return RLOG(result);
         if (result == 0) fin = true;
         else
         {
            if ((bytes_recv = ws_recv((char *)&buffer[0], buffer_size, uci_tcp_cfg)) < 0)
               return RLOG(result);
            n_bytes_recv += bytes_recv;
            if (timer != NULL && timer->Timeout()) fin = true;
         }
      }
      return bytes_recv;
   }
   else return 0;
}
__finally
{
   if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
}
	return bytes_recv;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_ClearReceive_AcquisitionPort(void)
{
    bool fin;
    CTimeout* timeout = NULL;
    int n_data, bytes_recv = 0;

    if (gb_plock_tcp_acq != NULL && !gb_plock_tcp_acq->Acquire(CS_WR))
        return ELOG(charECOM_Acquire, ECOM_Acquire);
    try
    {        
        if (!gb_no_hw)
        {
            LOG_Message("Removing the data from the acquisition ethernet port...");
            if (TCP_Connected())
            {
                fin = false;
                if ((timeout = new CTimeout(5000, MILISECONDS)) != NULL)
                    timeout->Start();
                while (!fin)
                {
                    n_data = TCPReceive_AcquisitionData(gb_buffer_acquisition, SIZE_BUFFER_ACQ, false);
                    bytes_recv += n_data;
                    if (n_data <= 0) fin = true;
                    else if (timeout != NULL) fin = timeout->Timeout();
                }
                if (n_data > 0) ELOG("WARNNING TCP_ClearReceive_AcquisitionPort()", -1);
                if (timeout != NULL) delete timeout;
            }
            LOG_Message("Ethernet Acquisition Port Empty");
        }
    }
    __finally
    {
        if (gb_plock_tcp_acq != NULL) gb_plock_tcp_acq->Release();
    }
    return bytes_recv;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPReceive_AcquisitionData (void *buffer, int data_max, bool acquire)
{
int result, bytes_recv = 0;
struct timeval timeout;

   if (uci_tcp_acq == INVALID_SOCKET) return 0;
   timeout.tv_sec = gb_timeout_first_sec;
   timeout.tv_usec = gb_timeout_first_usec;

   if (acquire && gb_plock_tcp_acq != NULL && !gb_plock_tcp_acq->Acquire(CS_WR))
      return ELOG(charECOM_Acquire, ECOM_Acquire);
try
{
   if (!gb_no_hw)
   {
      if ((result = ws_select_rd(&timeout, uci_tcp_acq)) < 0) return RLOG(result);
      if (result == 0) 
          return 0; // TIMEOUT: No hay datos

      if ((bytes_recv = ws_recv((char *)buffer, data_max, uci_tcp_acq)) < 0)
         return RLOG(result);
   }
   else return 0;
}
__finally
{
   if (acquire && gb_plock_tcp_acq != NULL) gb_plock_tcp_acq->Release();
}
	return bytes_recv;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned int comprobacion(const unsigned char *datos_env,const unsigned char *datos_rec,unsigned int ndatos)
{
	unsigned int bytes_mal=0,i;//ret
	for(i=0;i<ndatos&&bytes_mal<200;i++)
		if(datos_env[i]!=datos_rec[i]) printf("%4d %02X %02X, (%d)\n",i,datos_env[i],datos_rec[i],bytes_mal++);
	bytes_mal=0;
	for(i=0;i<ndatos;i++)
		if(*datos_env++!=*datos_rec++) bytes_mal++;

	return bytes_mal;
}

void prueba_velocidad(void)
{
	clock_t tic,toc;
	SOCKET tcpcom;
	float execution_time;
	int ret,i,ndatos,datos_mal;
	unsigned int dir_ini=0;

	printf("Conectando a: %s:%d...\n",ip_char,puerto);
	system("pause");
    if ((ret = tcpcom_connect(ip_char, puerto, &tcpcom)) < 0 || tcpcom == INVALID_SOCKET) return;
	for(i=0;i<TAMANIO;i++)
		datos_env[i]=((dir_ini+i+'a'))&0xFF;
	for(i=0;i<TAMANIO;i++)
		datos_rec[i]='j';
	ndatos=TAMANIO;//0x18001210
	printf("Enviando %d MB...\n",MEGAS);
	system("pause");
	ret=tcpcom_large_send(tcpcom, (char *)datos_env,ndatos*sizeof(unsigned char),-1);
	printf("tcpcom_large_send devuelve: %d\n\n",ret);
	system("pause");
	tic=clock();
	ret=tcpcom_large_recv(tcpcom, (char *)datos_rec,ndatos*sizeof(unsigned char),-1);
	toc=clock();
	execution_time = (float)(toc-tic)/CLOCKS_PER_SEC;
	printf("Tiempo de recepción = %fs\n",execution_time);
	printf("Velocidad de recepción = %fMB/s\n",(float)ndatos/1024/1024/execution_time);
	system("pause");
	datos_mal=comprobacion(datos_env,datos_rec,ndatos);
	printf("%d bytes mal\n\r",datos_mal);
	system("pause");
    tcp_close(&tcpcom);
}
void lee_qspi(const char *archivo,u32 direccion,int numbytes)
{
	clock_t tic,toc;
	SOCKET tcpcom;
	float execution_time;
	int ret;
	unsigned int dir_ini=0;

	printf("Conectando a: %s:%d...\n",ip_char,puerto);
	system("pause");
    if ((ret = tcpcom_connect(ip_char, puerto, &tcpcom)) < 0 || tcpcom == INVALID_SOCKET) return;
    printf("Leyendo qspi...\n");
	tcpcom_read_qspi(tcpcom,direccion,-1,numbytes);
	system("pause");	
	tic=clock();
	ret=tcpcom_large_recv(tcpcom, (char *)datos_rec,numbytes*sizeof(unsigned char),-1);
	toc=clock();
	execution_time = (float)(toc-tic)/CLOCKS_PER_SEC;
	printf("Tiempo de transferencia (ida y vuelta) = %fs\n",execution_time);
	printf("Velocidad de transferencia = %fMB/s\n",(float)numbytes/1024/1024/execution_time);
	system("pause");

	system("pause");
	tcpcom_save_file(datos_rec,archivo,numbytes);
	system("pause");

    tcp_close(&tcpcom);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_ResetAcquisitionConnection (void)
{
int result;
struct sockaddr local_addr;

   tcp_close(&uci_tcp_acq);
   uci_tcp_acq = INVALID_SOCKET;
   if ((result = tcp_connect(gb_ip_address, gb_tcp_port_acq, gb_connect_timeout, &uci_tcp_acq, &local_addr)) < 0)
   {
      tcp_close(&uci_tcp_acq);
      uci_tcp_acq = INVALID_SOCKET;
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   gb_ethernet_connected = 1;
	return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_ResetConfigConnection (void)
{
int result;
struct sockaddr local_addr;

   tcp_close(&uci_tcp_cfg);
   uci_tcp_cfg = INVALID_SOCKET;
//   WSACleanup();
   if ((result = tcp_connect(gb_ip_address, gb_tcp_port_cfg, gb_connect_timeout, &uci_tcp_cfg, &local_addr)) < 0)
   {
      tcp_close(&uci_tcp_cfg);
      uci_tcp_cfg = INVALID_SOCKET;
      return ELOG(charETCP_CONNECT,ETCP_CONNECT);
   }
   gb_ethernet_connected = 1;
	return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCP_Ping(char *ip)
{
    char str_error[255];

    if (ip == NULL) return ELOG(charETCP_RANGO, ETCP_RANGO);

    std::string address = ip, resolved;
    try 
    {
        if (!icmplib::IPAddress::IsCorrect(address, icmplib::IPAddress::Type::Unknown)) 
        {
            resolved = address; address = icmplib::IPAddress(address);
        }
    }
    catch (...) 
    {
        _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Ping request could not find host. Please check the name and try again.", ip);
        return ELOG(str_error, ETCP_CONNECT);
    }

    auto result = icmplib::Ping(address, ICMPLIB_TIMEOUT_1S);
    switch (result.response) {
    case icmplib::PingResponseType::Failure:
        _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Network error.", ip);
        return ELOG(str_error, ETCP_CONNECT);
        break;
    case icmplib::PingResponseType::Timeout:
        _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Request timed out.", ip);
        return ELOG(str_error, ETCP_CONNECT);
        break;
    default:
        //std::cout << "Reply from " << static_cast<std::string>(result.address) << ": ";
        switch (result.response) {
        case icmplib::PingResponseType::Success:
            //std::cout << "time=" << result.delay;
            //if (result.address.GetType() != icmplib::IPAddress::Type::IPv6) {
            //    std::cout << " TTL=" << static_cast<unsigned>(result.ttl);
            //}
            break;
        case icmplib::PingResponseType::Unreachable:
            _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Destination unreachable.", ip);
            return ELOG(str_error, ETCP_CONNECT);
            break;
        case icmplib::PingResponseType::TimeExceeded:
            _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Time exceeded.", ip);
            return ELOG(str_error, ETCP_CONNECT);
            break;
        default:
            _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Ping '%s': Response not supported.", ip);
            return ELOG(str_error, ETCP_CONNECT);
        }
    }
    return ETCP_NONE;
}
//int TCP_Ping(string target)
//{
//
//    int s, i, cc, packlen, datalen = DEFDATALEN;
//    struct hostent* hp;
//    struct sockaddr_in to, from;
//    struct ip* ip;
//    u_char* packet, outpack[MAXPACKET];
//    char hnamebuf[MAXHOSTNAMELEN];
//    string hostname;
//    struct icmp* icp;
//    int ret, fromlen, hlen;
//    fd_set rfds;
//    struct timeval tv;
//    int retval;
//    struct timeval start, end;
//    int end_t;
//    bool cont = true;
//
//    to.sin_family = AF_INET;
//
//    to.sin_addr.s_addr = inet_addr(target.c_str());
//    if (to.sin_addr.s_addr != (u_int)-1)
//        hostname = target;
//    else
//    {
//        hp = gethostbyname(target.c_str());
//        if (!hp)
//        {
//            cerr << "unknown host " << target << endl;
//            return -1;
//        }
//        to.sin_family = hp->h_addrtype;
//        bcopy(hp->h_addr, (caddr_t)&to.sin_addr, hp->h_length);
//        strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
//        hostname = hnamebuf;
//    }
//    packlen = datalen + MAXIPLEN + MAXICMPLEN;
//    if ((packet = (u_char*)malloc((u_int)packlen)) == NULL)
//    {
//        cerr << "malloc error\n";
//        return -1;
//    }
//
//
//    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
//    {
//        return -1; /* Needs to run as superuser!! */
//    }
//
//    icp = (struct icmp*)outpack;
//    icp->icmp_type = ICMP_ECHO;
//    icp->icmp_code = 0;
//    icp->icmp_cksum = 0;
//    icp->icmp_seq = 12345;
//    icp->icmp_id = getpid();
//
//
//    cc = datalen + ICMP_MINLEN;
//    icp->icmp_cksum = in_cksum((unsigned short*)icp, cc);
//
//    gettimeofday(&start, NULL);
//
//    i = sendto(s, (char*)outpack, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));
//    if (i < 0 || i != cc)
//    {
//        if (i < 0)
//            perror("sendto error");
//        cout << "wrote " << hostname << " " << cc << " chars, ret= " << i << endl;
//    }
//
//    FD_ZERO(&rfds);
//    FD_SET(s, &rfds);
//    tv.tv_sec = 1;
//    tv.tv_usec = 0;
//
//    while (cont)
//    {
//        retval = select(s + 1, &rfds, NULL, NULL, &tv);
//        if (retval == -1)
//        {
//            perror("select()");
//            return -1;
//        }
//        else if (retval)
//        {
//            fromlen = sizeof(sockaddr_in);
//            if ((ret = recvfrom(s, (char*)packet, packlen, 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) < 0)
//            {
//                perror("recvfrom error");
//                return -1;
//            }
//
//            // Check the IP header
//            ip = (struct ip*)((char*)packet);
//            hlen = sizeof(struct ip);
//            if (ret < (hlen + ICMP_MINLEN))
//            {
//                cerr << "packet too short (" << ret << " bytes) from " << hostname << endl;;
//                return -1;
//            }
//
//            // Now the ICMP part 
//            icp = (struct icmp*)(packet + hlen);
//            if (icp->icmp_type == ICMP_ECHOREPLY)
//            {
//                if (icp->icmp_seq != 12345)
//                {
//                    cout << "received sequence # " << icp->icmp_seq << endl;
//                    continue;
//                }
//                if (icp->icmp_id != getpid())
//                {
//                    cout << "received id " << icp->icmp_id << endl;
//                    continue;
//                }
//                cont = false;
//            }
//            else
//            {
//                cout << "Recv: not an echo reply" << endl;
//                continue;
//            }
//
//            gettimeofday(&end, NULL);
//            end_t = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
//
//            if (end_t < 1)
//                end_t = 1;
//
//            cout << "Elapsed time = " << end_t << " usec" << endl;
//            return end_t;
//        }
//        else
//        {
//            cout << "No data within one seconds.\n";
//            return 0;
//        }
//    }
//    return 0;
//}
//// -----------------------------------------------------------------------------
///**
//     Algorithm is simple, using a 32 bit accumulator (sum), add
//     sequential 16 bit words to it, and at the end, fold back all the
//     carry bits from the t   16 bits into the lower 16 bits.
//*/
//// -----------------------------------------------------------------------------
//uint16_t in_cksum(uint16_t* addr, unsigned len)
//{
//    uint16_t answer = 0;
//    uint32_t sum = 0;
//    while (len > 1) 
//    {
//        sum += *addr++;
//        len -= 2;
//    }
//
//    if (len == 1) 
//    {
//        *(unsigned char*)&answer = *(unsigned char*)addr;
//        sum += answer;
//    }
//
//    sum = (sum >> 16) + (sum & 0xffff);
//    sum += (sum >> 16);
//    answer = ~sum;
//    return answer;
//}
//
