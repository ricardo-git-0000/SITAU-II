// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <winsock2.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "winsock_api.h"
//#include "hwdsc_comm.h"
#include "ctimer.h"
#include "c_log_file.h"
#include "tcp_error_code.h"

#define SEND_BUFFER_SIZE  (128*1024*2)
static char gb_send_buffer[SEND_BUFFER_SIZE];

bool gb_wsa_startup = false;
int gb_winsock_minor_version = 0;
int gb_winsock_major_version = 0;
AnsiString gb_winsock_description = "";
AnsiString gb_winsock_status = "";

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_WSAStartup (unsigned char minor_version, unsigned char major_version)
{
WSADATA wsa_data;
int result;
char str_error[255];
unsigned short version;
FILE *f = NULL;

   if (gb_wsa_startup) return ETCP_NONE;
   // Initialize Winsock
   version = MAKEWORD(major_version, minor_version);
   if ((result = WSAStartup(version, &wsa_data)) != 0)
   {
      switch (result)
      {
      case WSASYSNOTREADY:
         result = ELOG("The underlying network subsystem is not ready for network communication.", ETCP_CONNECT);
         break;
      case WSAVERNOTSUPPORTED:
         result = ELOG("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.", ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 operation is in progress.", ETCP_CONNECT);
         break;
      case WSAEPROCLIM:
         result = ELOG("A limit on the number of tasks supported by the Windows Sockets implementation has been reached.", ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("The lpWSAData parameter is not a valid pointer.", ETCP_CONNECT);
         break;
      default:
         sprintf(str_error, "Socket function failed with error: %ld\n", result);
         result = ELOG(str_error, ETCP_CONNECT);
         break;
      }
   }
   else gb_wsa_startup = true;
   gb_winsock_minor_version = LOBYTE(wsa_data.wVersion);
   gb_winsock_major_version = HIBYTE(wsa_data.wVersion);
   gb_winsock_description = AnsiString(wsa_data.szDescription);
   gb_winsock_status = AnsiString(wsa_data.szSystemStatus);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_WSACleanup(void)
{
   int result, error;
   char str_error[255];

   if ((result = WSACleanup()) != 0)
   {
      switch (result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.", ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("The network subsystem has failed.", ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.", ETCP_CONNECT);
         break;
      default:
          error = GetLastError();
          switch (error)
          {
          case WSANOTINITIALISED:
              break;
          default:
              sprintf(str_error, "Socket function failed with error: %ld - GetLastError: %d\n", result, error);
              result = ELOG(str_error, ETCP_CONNECT);
              break;
          }
         break;
      }
   }
   gb_wsa_startup = false;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_socket (int af, int type, int protocol, SOCKET *socket_id)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if (((*socket_id) = socket(af, type, protocol)) == INVALID_SOCKET)
	{
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("The network subsystem or the associated service provider has failed.",ETCP_CONNECT);
         break;
      case WSAEAFNOSUPPORT:
         result = ELOG("The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAEMFILE:
         result = ELOG("No more socket descriptors are available.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.",ETCP_CONNECT);
         break;
      case WSAEINVALIDPROVIDER:
         result = ELOG("The service provider returned a version other than 2.2.",ETCP_CONNECT);
         break;
      case WSAEINVALIDPROCTABLE:
         result = ELOG("The service provider returned an invalid or incomplete procedure table to the WSPStartup.",ETCP_CONNECT);
         break;
      case WSAENOBUFS:
         result = ELOG("No buffer space is available. The socket cannot be created.",ETCP_CONNECT);
         break;
      case WSAEPROTONOSUPPORT:
         result = ELOG("The specified protocol is not supported.",ETCP_CONNECT);
         break;
      case WSAEPROTOTYPE:
         result = ELOG("The specified protocol is the wrong type for this socket.",ETCP_CONNECT);
         break;
      case WSAEPROVIDERFAILEDINIT:
         result = ELOG("The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly.",ETCP_CONNECT);
         break;
      case WSAESOCKTNOSUPPORT:
         result = ELOG("The specified socket type is not supported in this address family.",ETCP_CONNECT);
         break;
      default:
         sprintf(str_error,"Socket function failed with error: %ld\n", result);
         result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
	}
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_ioctlsocket (long cmd, unsigned long *argp, SOCKET socket_id)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if ((result = ioctlsocket(socket_id, cmd, argp)) == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("The descriptor s is not a socket.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("The argp parameter is not a valid part of the user address space.",ETCP_CONNECT);
         break;
      default:
         sprintf(str_error,"Socket function failed with error: %ld\n", result);
         result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
   }
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_bind (struct sockaddr_in *name, int namelen, SOCKET socket_id)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if ((result = bind(socket_id, (struct sockaddr *)name, namelen)) == SOCKET_ERROR)
	{
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("Note  A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
			result = ELOG("The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEACCES:
         result = ELOG("An attempt was made to access a socket in a way forbidden by its access permissions.",ETCP_CONNECT);
         break;
      case WSAEADDRINUSE:
         result = ELOG("Only one usage of each socket address (protocol/network address/port) is normally permitted.",ETCP_CONNECT);
         result = 0;
         break;
      case WSAEADDRNOTAVAIL:
         result = ELOG("The requested address is not valid in its context.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("The system detected an invalid pointer address in attempting to use a pointer argument in a call.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("An invalid argument was supplied.",ETCP_CONNECT);
         break;
      case WSAENOBUFS:
         result = ELOG("An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("An operation was attempted on something that is not a socket.",ETCP_CONNECT);
         break;
      default:
			sprintf(str_error,"Socket function failed with error: %ld", result);
			result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
	}
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_setsockopt (int level, int optname, char *optval, int optlen, SOCKET socket_id)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if ((result = setsockopt(socket_id, level, optname, optval, optlen)) == SOCKET_ERROR)
	{
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("The buffer pointed to by the optval parameter is not in a valid part of the process address space or the optlen parameter is too small.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("The level parameter is not valid, or the information in the buffer pointed to by the optval parameter is not valid.",ETCP_CONNECT);
         break;
      case WSAENETRESET:
         result = ELOG("The connection has timed out when SO_KEEPALIVE is set.",ETCP_CONNECT);
         break;
      case WSAENOPROTOOPT:
         result = ELOG("The option is unknown or unsupported for the specified provider or socket (see SO_GROUP_PRIORITY limitations).",ETCP_CONNECT);
         break;
      case WSAENOTCONN:
         result = ELOG("The connection has been reset when SO_KEEPALIVE is set.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("The descriptor is not a socket.",ETCP_CONNECT);
         break;
      default:
			sprintf(str_error,"Socket function failed with error: %ld", result);
			result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
	}
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_getsockopt (int level, int optname, char *optval, int *optlen, SOCKET socket_id)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if ((result = getsockopt(socket_id, level, optname, optval, optlen)) == SOCKET_ERROR)
	{
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("Note  The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("One of the optval or the optlen parameters is not a valid part of the user address space, or the optlen parameter is too small.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("The level parameter is unknown or invalid.",ETCP_CONNECT);
         break;
      case WSAENOPROTOOPT:
         result = ELOG("The option is unknown or unsupported by the indicated protocol family.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("The descriptor is not a socket.",ETCP_CONNECT);
         break;
      default:
			sprintf(str_error,"Socket function failed with error: %ld", result);
			result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
	}
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_connect (char* ip_string, unsigned short port, int timeout_sec, SOCKET socket_id)
{
struct sockaddr_in server_addr;
char str_error[255];
int result = ETCP_CONNECT, get_last_error;
CTimeout *timeout = new CTimeout(timeout_sec * 1000., MILISECONDS);
bool retry = false;
//struct hostent* hp;

    if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);

	// Connect to server.
	memset(&server_addr, 0, sizeof(server_addr));
//	server_addr.sin_family = AF_UNSPEC;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_string);
	server_addr.sin_port = htons(port);

   timeout->Start();
   do
   {
      if (connect(socket_id, (struct sockaddr *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR)
      {
         get_last_error = WSAGetLastError();
         if (get_last_error != WSAEWOULDBLOCK)
         {
            switch(get_last_error)
            {
            case WSANOTINITIALISED:
               result = ELOG("WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
               break;
            case WSAENETDOWN:
               result = ELOG("WSAENETDOWN: The network subsystem has failed.",ETCP_CONNECT);
               break;
            case WSAEADDRINUSE:
               result = ELOG("WSAEADDRINUSE: The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until the connect function if the bind was to a wildcard address (INADDR_ANY or in6addr_any) for the local IP address. A specific address needs to be implicitly bound by the connect function.",ETCP_CONNECT);
               break;
            case WSAEINTR:
               result = ELOG("WSAEINTR: The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.",ETCP_CONNECT);
               break;
            case WSAEINPROGRESS:
               result = ELOG("WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
               break;
            case WSAEALREADY:
//               result = ELOG("WSAEALREADY: A nonblocking connect call is in progress on the specified socket. Note In order to preserve backward compatibility, this error is reported as WSAEINVAL to Windows Sockets 1.1 applications that link to either Winsock.dll or Wsock32.dll.",ETCP_CONNECT);
//               result = ETCP_NONE;
               retry = true;
               break;
            case WSAEADDRNOTAVAIL:
               result = ELOG("WSAEADDRNOTAVAIL: The remote address is not a valid address (such as INADDR_ANY or in6addr_any).",ETCP_CONNECT);
               break;
            case WSAEAFNOSUPPORT:
               result = ELOG("WSAEAFNOSUPPORT: Addresses in the specified family cannot be used with this socket.",ETCP_CONNECT);
               break;
            case WSAECONNREFUSED:
               result = ELOG("WSAECONNREFUSED: The attempt to connect was forcefully rejected.",ETCP_CONNECT);
               break;
            case WSAEFAULT:
               result = ELOG("WSAEFAULT: The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small. This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space.",ETCP_CONNECT);
               break;
            case WSAEINVAL:
               result = ELOG("WSAEINVAL: The parameter s is a listening socket.",ETCP_CONNECT);
               retry = true;
               break;
            case WSAEISCONN: // El socket ya está conectado.
               result = ETCP_NONE;
               break;
            case WSAENETUNREACH:
               result = ELOG("WSAENETUNREACH: The network cannot be reached from this host at this time.",ETCP_CONNECT);
               break;
            case WSAEHOSTUNREACH:
               result = ELOG("WSAEHOSTUNREACH: A socket operation was attempted to an unreachable host.",ETCP_CONNECT);
               break;
            case WSAENOBUFS:
               result = ELOG("WSAENOBUFS: Note  No buffer space is available. The socket cannot be connected.",ETCP_CONNECT);
               break;
            case WSAENOTSOCK:
               result = ELOG("WSAENOTSOCK: The descriptor specified in the s parameter is not a socket.",ETCP_CONNECT);
               break;
            case WSAETIMEDOUT:
               result = ELOG("WSAETIMEDOUT: An attempt to connect timed out without establishing a connection.",ETCP_CONNECT);
               break;
            case WSAEWOULDBLOCK:
               result = ELOG("WSAEWOULDBLOCK: The socket is marked as nonblocking and the connection cannot be completed immediately.",ETCP_CONNECT);
               break;
            case WSAEACCES:
               result = ELOG("WSAEACCES: An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.",ETCP_CONNECT);
               break;
            default:
               sprintf(str_error,"Socket function failed with error: %d\n", get_last_error);
               result = ELOG(str_error,ETCP_CONNECT);
               break;
            }
         }
         else retry = true;
      }
      else result = ETCP_NONE;
   }
   while (retry && !timeout->Timeout());

   if (result != ETCP_NONE && timeout->Timeout())
   {
      sprintf(str_error,"Socket status timeout: %d seconds\n", timeout_sec);
      result = ELOG(str_error,ETCP_CONNECT_TIMEOUT);
   }

   delete timeout;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout)
{
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if (timeout != NULL && (timeout->tv_sec > 0 || timeout->tv_usec > 0))
   	result = select(nfds, readfds, writefds, exceptfds, timeout);
	else
      result = select(nfds, readfds, writefds, exceptfds, NULL);

//	if ((result = select(nfds, readfds, writefds, exceptfds, timeout)) != 1)
	if (result != 1)
   {
      if (result == 0) return 0;
      else
      {
         result = WSAGetLastError();
         switch(result)
         {
         case WSANOTINITIALISED:
            result = ELOG("WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
            break;
         case WSAEFAULT:
            result = ELOG("WSAEFAULT: The Windows Sockets implementation was unable to allocate needed resources for its internal operations, or the readfds, writefds, exceptfds, or timeval parameters are not part of the user address space.",ETCP_CONNECT);
            break;
         case WSAENETDOWN:
            result = ELOG("WSAENETDOWN: The network subsystem has failed.",ETCP_CONNECT);
            break;
         case WSAEINVAL:
            result = ELOG("WSAEINVAL: A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.",ETCP_CONNECT);
            break;
         case WSAEINTR:
            result = ELOG("WSAEINTR: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
            break;
         case WSAEINPROGRESS:
            result = ELOG("WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
            break;
         case WSAENOTSOCK:
            result = ELOG("WSAENOTSOCK: One of the descriptor sets contains an entry that is not a socket.",ETCP_CONNECT);
            break;
         default:
            sprintf(str_error,"Socket function failed with error: %ld\n", result);
            result = ELOG(str_error,ETCP_CONNECT);
            break;
         }
      }
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_select_rdwr (timeval *timeout, SOCKET socket_id)
{
fd_set read_flags, write_flags, except_flag;
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);

   FD_ZERO(&read_flags);
   FD_ZERO(&write_flags);
   FD_ZERO(&except_flag);
   FD_SET(socket_id, &read_flags);
   FD_SET(socket_id, &write_flags);
   FD_SET(socket_id, &except_flag);

   // Checking the number of available sockets
//   timeout_app->Start();
//   do
//   {
      if ((result = ws_select(0, &read_flags, &write_flags, &except_flag, timeout)) < 0)
         return RLOG(result);
//   }
//   while (!timeout_app->Timeout());

   if (result == 0)
   {
      if (timeout->tv_sec > 0)
         sprintf(str_error,"Socket status timeout: %d seconds\n", timeout->tv_sec);
      else
         sprintf(str_error,"Socket status timeout: %d microseconds\n", timeout->tv_usec);
      result = ELOG(str_error,ETCP_CONNECT_TIMEOUT);
   }
//   if (timeout_app != NULL) delete timeout_app;
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_select_rd (timeval *timeout, SOCKET socket_id)
{
   fd_set read_flags;
   int result;
   char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);

   // Checking the number of available sockets to read
   FD_ZERO(&read_flags);
   FD_SET(socket_id, &read_flags);

   // Checking the number of available sockets
   if ((result = ws_select((int)socket_id, &read_flags, NULL, NULL, timeout)) < 0)
      return RLOG(result);

   if (result == 1)
   {
      // Socket ready for reading
      if (FD_ISSET(socket_id, &read_flags) == 0)
      {
         sprintf(str_error,"Socket is not ready for reading.");
         result = ELOG(str_error,ETCP_CONNECT);
      }
      //FD_CLR(socket_id, &read_flags); //clear set
   }
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_select_wr (timeval *timeout, SOCKET socket_id)
{
fd_set write_flags;
int result;
char str_error[255];

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   FD_ZERO(&write_flags);
   FD_SET(socket_id, &write_flags);

   // Checking the number of available sockets
   if ((result = ws_select(0, NULL, &write_flags, NULL, timeout)) < 0)
      return RLOG(result);
   if (result == 0)
   {
      if (timeout->tv_sec > 0)
         sprintf(str_error,"Socket status timeout: %d seconds\n", timeout->tv_sec);
      else
         sprintf(str_error,"Socket status timeout: %d microseconds\n", timeout->tv_usec);
      result = ELOG(str_error,ETCP_CONNECT_TIMEOUT);
   }
   else
   {
      // Socket ready for writing
      if (FD_ISSET(socket_id, &write_flags) == 0)
      {
         sprintf(str_error,"Socket is not ready for writing.");
         result = ELOG(str_error,ETCP_CONNECT);
      }
   }
//   FD_CLR(socket_id, &write_flags); //clear set
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_closesocket(SOCKET* socket_id)
{
    char str_error[255];
    int result;

    if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
    if (socket_id == NULL || *socket_id == INVALID_SOCKET) return ETCP_NONE;
    if (closesocket(*socket_id) != 0)
    {
        result = WSAGetLastError();
        switch (result)
        {
        case WSANOTINITIALISED:
            result = ELOG("WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.", ETCP_CONNECT);
            break;
        case WSAENETDOWN:
            result = ELOG("WSAENETDOWN: The network subsystem has failed.", ETCP_CONNECT);
            break;
        case WSAEINPROGRESS:
            result = ELOG("WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.", ETCP_CONNECT);
            break;
        case WSAENOTSOCK:
            result = ELOG("WSAENOTSOCK: The descriptor s is not a socket.", ETCP_CONNECT);
            break;
        case WSAEINTR:
            result = ELOG("WSAEINTR: The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.", ETCP_CONNECT);
            break;
        case WSAEWOULDBLOCK:
            result = ELOG("WSAEWOULDBLOCK: The socket is marked as nonblocking, but the l_onoff member of the linger structure is set to nonzero and the l_linger member of the linger structure is set to a nonzero timeout value.", ETCP_CONNECT);
            break;
        case WSAEFAULT:
            result = ELOG("WSAEFAULT: The argp parameter is not a valid part of the user address space.", ETCP_CONNECT);
            break;
        default:
            sprintf(str_error, "Socket function failed with error: %ld\n", result);
            result = ELOG(str_error, ETCP_CONNECT);
            break;
        }
    }
    else
    {
        *socket_id = INVALID_SOCKET;
        result = ETCP_NONE;
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_getsockname (struct sockaddr *local_addr, SOCKET socket_id)
{
char str_error[255];
int result, local_addr_len;

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   local_addr_len = sizeof(struct sockaddr);
   if (getsockname(socket_id, local_addr, &local_addr_len) == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      switch(result)
      {
      case WSAEFAULT:
         result = ELOG("WSAEFAULT: The name parameter is a NULL pointer or is not a valid part of the user address space. This error is also returned if the buffer size specified by namelen parameter is too small to hold the complete host name.",ETCP_CONNECT);
         break;
      case WSANOTINITIALISED:
         result = ELOG("WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("WSAENETDOWN: The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      default:
         sprintf(str_error,"Socket function failed with error: %ld\n", result);
         result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
      ws_closesocket(&socket_id);
   }
   else result = ETCP_NONE;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_recv(char *data, int n_data, SOCKET socket_id)
{
char str_error[255];
int result;

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   data[0] = 0;
   if ((result = recv(socket_id, data, n_data, 0)) == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("The buf parameter is not completely contained in a valid part of the user address space.",ETCP_CONNECT);
         break;
      case WSAENOTCONN:
         result = ELOG("The socket is not connected.",ETCP_CONNECT);
         break;
      case WSAEINTR:
         result = ELOG("The (blocking) call was canceled through WSACancelBlockingCall.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAENETRESET:
         result = ELOG("For a connection-oriented socket, this error indicates that the connection has been broken due to keep-alive activity that detected a failure while the operation was in progress. For a datagram socket, this error indicates that the time to live has expired.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("The descriptor is not a socket.",ETCP_CONNECT);
         break;
      case WSAEOPNOTSUPP:
         result = ELOG("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.",ETCP_CONNECT);
         break;
      case WSAESHUTDOWN:
         result = ELOG("The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.",ETCP_CONNECT);
         break;
      case WSAEWOULDBLOCK:
         result = ELOG("The socket is marked as nonblocking and the receive operation would block.",ETCP_CONNECT);
         break;
      case WSAEMSGSIZE:
         result = ELOG("The message was too large to fit into the specified buffer and was truncated.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.",ETCP_CONNECT);
         break;
      case WSAECONNABORTED:
         result = ELOG("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.",ETCP_CONNECT);
         break;
      case WSAETIMEDOUT:
         result = ELOG("The connection has been dropped because of a network failure or because the peer system failed to respond.",ETCP_CONNECT);
         break;
      case WSAECONNRESET:
         result = ELOG("The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable.",ETCP_CONNECT);
         break;
      default:
         sprintf(str_error,"Socket function failed with error: %ld\n", result);
         result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_send(char *data, int n_data, SOCKET socket_id)
{
char str_error[255];
int result;

   if (!gb_wsa_startup) return ELOG(charETCP_WSAStartup, ETCP_WSAStartup);
   if (data == NULL) return ELOG(charETCP_RANGO, ETCP_RANGO);
   if ((result = send(socket_id, data, n_data, 0)) == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      switch(result)
      {
      case WSANOTINITIALISED:
         result = ELOG("WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.",ETCP_CONNECT);
         break;
      case WSAENETDOWN:
         result = ELOG("WSAENETDOWN: The network subsystem has failed.",ETCP_CONNECT);
         break;
      case WSAEACCES:
         result = ELOG("WSAEACCES: The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST socket option to enable use of the broadcast address.",ETCP_CONNECT);
         break;
      case WSAEINTR:
         result = ELOG("WSAEINTR: A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.",ETCP_CONNECT);
         break;
      case WSAEINPROGRESS:
         result = ELOG("WSAEINPROGRESS: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.",ETCP_CONNECT);
         break;
      case WSAEFAULT:
         result = ELOG("WSAEFAULT: The buf parameter is not completely contained in a valid part of the user address space.",ETCP_CONNECT);
         break;
      case WSAENETRESET:
         result = ELOG("WSAENETRESET: The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.",ETCP_CONNECT);
         break;
      case WSAENOBUFS:
         result = ELOG("WSAENOBUFS: No buffer space is available.",ETCP_CONNECT);
         break;
      case WSAENOTCONN:
         result = ELOG("WSAENOTCONN: The socket is not connected.",ETCP_CONNECT);
         break;
      case WSAENOTSOCK:
         result = ELOG("WSAENOTSOCK: The descriptor is not a socket.",ETCP_CONNECT);
         break;
      case WSAEOPNOTSUPP:
         result = ELOG("WSAEOPNOTSUPP: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.",ETCP_CONNECT);
         break;
      case WSAESHUTDOWN:
         result = ELOG("WSAESHUTDOWN: The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.",ETCP_CONNECT);
         break;
      case WSAEWOULDBLOCK:
         result = ELOG("WSAEWOULDBLOCK: The socket is marked as nonblocking and the requested operation would block.",ETCP_CONNECT);
         break;
      case WSAEMSGSIZE:
         result = ELOG("WSAEMSGSIZE: TThe socket is message oriented, and the message is larger than the maximum supported by the underlying transport.",ETCP_CONNECT);
         break;
      case WSAEHOSTUNREACH:
         result = ELOG("WSAEHOSTUNREACH: The remote host cannot be reached from this host at this time.",ETCP_CONNECT);
         break;
      case WSAEINVAL:
         result = ELOG("WSAEINVAL: The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.",ETCP_CONNECT);
         break;
      case WSAECONNABORTED:
         result = ELOG("WSAECONNABORTED: The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.",ETCP_CONNECT);
         break;
      case WSAECONNRESET:
         result = ELOG("WSAECONNRESET: The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a 'Port Unreachable' ICMP packet. The application should close the socket as it is no longer usable.",ETCP_CONNECT);
         break;
      case WSAETIMEDOUT:
         result = ELOG("WSAETIMEDOUT: The connection has been dropped, because of a network failure or because the system on the other end went down without notice.",ETCP_CONNECT);
         break;
      default:
         sprintf(str_error,"Socket function failed with error: %ld\n", result);
         result = ELOG(str_error,ETCP_CONNECT);
         break;
      }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ws_send_msg(void *data, int data_len, SOCKET socket_id)
{
/*usnd_msg *msg;
int result, n_bytes;

   msg = (usnd_msg *) gb_send_buffer;
   msg->header = _endian16(USND_MSG_HEADER);
   msg->data_len = _endian16(data_len);
   memcpy(msg->data, data, data_len);
   *((usnd_footer *) &msg->data[data_len]) = _endian16(USND_MSG_FOOTER);
   n_bytes = sizeof(usnd_header) + sizeof(usnd_length) + data_len + sizeof(usnd_footer);

   if ((result = ws_send(gb_send_buffer, n_bytes, socket_id)) < 0)
      return RLOG(result);
   return result;*/
   return 0;
}


