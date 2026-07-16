
#define dbg_printf printf
//#define dbg_printf //

//#include "streg.h"
#include "tcp_comm.h"
#include "api_tcp.h"
#include "c_log_file.h"
#include "tcp_error_code.h"
#include "winsock_api.h"
#include "tad.h"
#include "st_reg.h"

static u32 ID=0;
bool gb_no_hw = false;
bool gb_cancel_connect = false;
int gb_connect_delay = 1000;
int gb_connect_try = 10;

__inline unsigned short endian16(unsigned short i)
{
    return ((i>>8)& 0xff)|((i << 8)& 0xff00);
}

__inline unsigned int endian32(unsigned int i)
{
    return((i&0xff)<<24)|((i&0xff00)<<8)|((i&0xff0000)>>8)|((i>>24)&0xff);
}

int tcpcom_connect(char* ip_str_dst, unsigned short port, SOCKET *socket_id)
{
    unsigned long non_blocking = 1;
    int result;//i, 
    //u_long block;
    bool connected;// , error;
    int counter = gb_connect_try;

    int get_buffer_length, set_buffer_length = 1 * 1024 * 1024;
    int get_buffer_length_size = sizeof(int);

    DWORD get_timeout, set_timeout = 3000;
    int get_timeout_size = sizeof(get_timeout);

    bool get_bool, set_bool;
    int get_bool_size = sizeof(get_bool);

    if (socket_id == NULL) return ELOG(charETCP_RANGO, ETCP_RANGO);
    *socket_id = INVALID_SOCKET;

    if (!gb_no_hw && !gb_cancel_connect)
    {
        do
        {
            connected = false;
            // Initialize Winsock
            if ((result = ws_WSAStartup(2, 2)) != 0) return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            // Create a SOCKET for connecting to server
            if ((*socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
            {
                ws_WSACleanup();
                if ((result = ws_WSAStartup(2, 2)) != 0) return ELOG(charETCP_SOCKET, ETCP_SOCKET);
                if ((*socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return ELOG(charETCP_SOCKET, ETCP_SOCKET);
            }

            if ((result = setsockopt(*socket_id, SOL_SOCKET, SO_SNDBUF, (char*)&set_buffer_length, sizeof(set_buffer_length))) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = getsockopt(*socket_id, SOL_SOCKET, SO_SNDBUF, (char*)&get_buffer_length, &get_buffer_length_size)) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if (get_buffer_length_size != sizeof(set_buffer_length) || set_buffer_length != get_buffer_length)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = setsockopt(*socket_id, SOL_SOCKET, SO_RCVBUF, (char*)&set_buffer_length, sizeof(set_buffer_length))) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = getsockopt(*socket_id, SOL_SOCKET, SO_RCVBUF, (char*)&get_buffer_length, &get_buffer_length_size)) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if (get_buffer_length_size != sizeof(set_buffer_length) || set_buffer_length != get_buffer_length)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = setsockopt(*socket_id, SOL_SOCKET, SO_RCVTIMEO, (char*)&set_timeout, sizeof(set_timeout))) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = getsockopt(*socket_id, SOL_SOCKET, SO_RCVTIMEO, (char*)&get_timeout, &get_timeout_size)) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if (get_timeout_size != sizeof(set_timeout) || set_timeout != get_timeout)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            // Indicates that out-of-bound data should be returned in-line with regular data.
            // This option is only valid for connection-oriented protocols that support out-of-band data
            set_bool = true;
            if ((result = setsockopt(*socket_id, SOL_SOCKET, SO_OOBINLINE, (char*)&set_bool, sizeof(set_bool))) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = getsockopt(*socket_id, SOL_SOCKET, SO_OOBINLINE, (char*)&get_bool, &get_bool_size)) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if (get_bool_size != sizeof(set_bool) || set_bool != get_bool)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            // Disables the Nagle algorithm for send coalescing.
            // This socket option is included for backward compatibility with Windows Sockets 1.1
            set_bool = true;
            if ((result = setsockopt(*socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&set_bool, sizeof(set_bool))) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if ((result = getsockopt(*socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&get_bool, &get_bool_size)) == SOCKET_ERROR)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            if (get_bool_size != sizeof(set_bool) || set_bool != get_bool)
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);

            // put socked in non-blocking mode...
            //block = 1;
            //if (ioctlsocket(tcpcom_socket, FIONBIO, &block) == SOCKET_ERROR)
            //{
            //    ELOG(charETCP_SOCKET, ETCP_SOCKET);
            //    ws_closesocket(&tcpcom_socket);
            //    return INVALID_SOCKET;
            //}
            if (counter == gb_connect_try) _snprintf_s(gb_msg_progress_open_sys, 255, _TRUNCATE, "Connecting '%s:%d...", ip_str_dst, port);
            else _snprintf_s(gb_msg_progress_open_sys, 255, _TRUNCATE, "Connecting '%s:%d...(Try %d/%d)", ip_str_dst, port, gb_connect_try - counter, gb_connect_try);

            if ((result = TCP_Ping(ip_str_dst)) < 0) return RLOG(result);

            if ((result = ws_connect(ip_str_dst, port, gb_connect_timeout, *socket_id)) < 0)
            {
                ws_closesocket(socket_id);
                counter--;
            }
            else
            {
                connected = true;
            }
        } while (!gb_cancel_connect && !connected && counter >= 0);

        if (!connected)
        {
            ELOG(charETCP_SOCKET, ETCP_SOCKET);
            ws_closesocket(socket_id);
            return INVALID_SOCKET;
        }

        // put socked in blocking mode...
        //block = 0;
        //if (ioctlsocket(tcpcom_socket, FIONBIO, &block) == SOCKET_ERROR)
        //{
        //    ELOG(charETCP_SOCKET, ETCP_SOCKET);
        //    ws_closesocket(&tcpcom_socket);
        //    return INVALID_SOCKET;
        //}
        return ETCP_NONE;
    }
    else return 1;
}

SOCKET tcpimage_connect_img_opts(char* ip_str_dst,unsigned short port)
{
	//unsigned long ip_addr;
	WSADATA wsaData;
	SOCKET tcpimage_socket = INVALID_SOCKET;
	struct sockaddr_in server_addr;
	unsigned long non_blocking = 1;
	int ret;

	int buffer_lenght = 1*1024*1024;
    int output_len = sizeof (int);
    int output_value = 0;
	
	DWORD timeout = 3000,timeout_out;
	int timeout_len=sizeof(timeout);
	bool tcp_nodelay=1;
	int tcp_nodelay_len=sizeof(tcp_nodelay);

	ret = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (ret != 0) return tcpimage_socket;

	tcpimage_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcpimage_socket == INVALID_SOCKET) 
	{
		WSACleanup();
		return tcpimage_socket;
	}
	// buffer_length
	ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);

    ret = setsockopt(tcpimage_socket, SOL_SOCKET, SO_SNDBUF, (char *) &buffer_lenght, sizeof(buffer_lenght));
    if (ret == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"Set SO_RCVBUF: %d\n",buffer_lenght);
	
    ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_SNDBUF, (char *) &output_value, &output_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_SNDBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
	
	// buffer_length
	ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVBUF, (char *) &output_value, &output_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);

    ret = setsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVBUF, (char *) &buffer_lenght, sizeof(buffer_lenght));
    if (ret == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"Set SO_RCVBUF: %d\n",buffer_lenght);
	
    ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVBUF, (char *) &output_value, &output_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_RCVBUF failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVBUF Value: %ld\n", output_value);
	
	// recv_timeout
	ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout_out, &timeout_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVTIMEO Value: %ld\n", timeout_out);

    ret = setsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, timeout_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"Set SO_RCVTIMEO: %d\n",timeout_out);

    ret = getsockopt(tcpimage_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout_out, &timeout_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"SO_RCVTIMEO Value: %ld\n", timeout_out);

/*
	// NO DELAY
    ret = setsockopt(tcpimage_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, tcp_nodelay_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"setsockopt for TCP_NODELAY failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"Set TCP_NODELAY: %d\n",tcp_nodelay_len);

    ret = getsockopt(tcpimage_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_nodelay, &tcp_nodelay_len);
    if (ret == SOCKET_ERROR) {
        wprintf(L"getsockopt for TCP_NODELAY failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"TCP_NODELAY Value: %ld\n", tcp_nodelay);
*/
	// Connect to server.
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_str_dst);
	server_addr.sin_port = htons(port);     

	ret = connect(tcpimage_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

	if (ret == SOCKET_ERROR)
	{
		closesocket(tcpimage_socket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	//dbg_printf("Conected to %s:%d\n",ip_str_dst,port);
	return tcpimage_socket;
}

int tcpcom_recv_no_block(SOCKET tcpcom_socket, void *data)
{
  int bytes=0;
  bytes += recv(tcpcom_socket,(char *)data, RX_BUFFER_SIZE, 0);

  return bytes;
}

int tcpcom_large_send(SOCKET tcpcom_socket, const char * data, u32 data_len, u32 dst_addr)
{
    u32 total_bytes = 0;// , bytes;
u32 comando[5];
int result;

    comando[0] = CMD_HEADER;
    comando[1] = 20;
    comando[2] = PUT_MEM;
    comando[3] = dst_addr;
    comando[4] = data_len;

    if (!gb_no_hw)
    {
        send(tcpcom_socket, (const char*)comando, 20, 0);
        do
        {
            if ((result = send(tcpcom_socket, (const char*)&data[total_bytes], data_len - total_bytes, 0)) == SOCKET_ERROR)
            {
                LOG_Message("ERROR Code %d: Sending data to socket", ETCP_SOCKET);
                return ELOG(charETCP_SOCKET, ETCP_SOCKET);
            }
            if (result < 0)
            {
                LOG_Message("ERROR Code %d: Sending data to socket", ETCP_SOCKET);
                return total_bytes;
            }
            total_bytes += result;
        } while (total_bytes < data_len);
    }
    return total_bytes;
}


int tcpcom_rcv_response(SOCKET tcpcom_socket,char *data,u32 data_len,u32 ID)
{
u32 respuesta[4];
u32 total_bytes=0, bytes;
int ret;

  // Primero recibo la cabecera, bloqueando...
  ret=recv(tcpcom_socket,(char *)respuesta, 16, MSG_WAITALL);

  if(respuesta[0]!=CMD_HEADER)
  {
	  dbg_printf("tcpcom_rcv_response(): Error recibiendo cabecera de la respuesta respuesta[0] = %08x\n",respuesta[0]);
	  return -1;
  }
  if(respuesta[2]!=ID)
  {
	  dbg_printf("tcpcom_rcv_response(): Error recibiendo ID de la respuesta\n");
	  return -1;
  }
  if(respuesta[3]!=data_len)
  {
	  dbg_printf("tcpcom_rcv_response(): Error recibiendo longitud de la respuesta\n");
	  return -1;
  }
  do {
    bytes = recv(tcpcom_socket,(char *)&data[total_bytes], data_len-total_bytes, 0);
	//dbg_printf("%d bytes recibidos\n",bytes);
	if(bytes==0) return total_bytes;
	if(bytes<0)
	{
		dbg_printf("tcpcom_rcv_response(): Error en el socket\n");
		return -1;
	}
	total_bytes += bytes;
  }while(total_bytes<data_len);
  return total_bytes;
}

int tcpcom_large_recv(SOCKET tcpcom_socket,char *data, u32 data_len,u32 src_addr)
{
  int ret;
  u32 comando[6];

  //u32 data_len_rcv;

  comando[0] = CMD_HEADER;
  comando[1] = 24;
  comando[2] = GET_MEM;
  comando[3] = src_addr;
  comando[4] = data_len;
  comando[5] = ++ID;
  send(tcpcom_socket,(const char *)comando, comando[1], 0);
  
  ret=tcpcom_rcv_response(tcpcom_socket,data,data_len,ID);
  if(ret<0) return ret;
  return data_len;
}

int tcpcom_dbg_function(SOCKET tcpcom_socket)
{
  int bytes=0;
  u32 comando[3];
  comando[0] = CMD_HEADER;
  comando[1] = 12;
  comando[2] = DBG_FUNCTION;

  bytes=send(tcpcom_socket,(const char *)comando, comando[1], 0);

  return bytes;
}

int tcpcom_load_file(unsigned char* dst_addr,const char *file_route,int max_bytes)
{
FILE *f = NULL;
int bytes,total_bytes=0;

   if (dst_addr == NULL) return ELOG(charETCP_RANGO, ETCP_RANGO);
   if ((f = fopen(file_route,"rb")) == NULL)
   {
       LOG_Message("ERROR Code %d: Can not open file: %s", ETCP_FILE, file_route);
       return ELOG(charETCP_FILE, ETCP_FILE);
   }
   while(!feof(f))
   {
	   bytes = (int)fread (&dst_addr[total_bytes], sizeof(unsigned char), max_bytes - total_bytes, f);
	   if (bytes <= 0)
	   {
           LOG_Message("ERROR Code %d: Format File", ETCP_FILE);
		   return total_bytes;
	   }
	   total_bytes += bytes;
	   if (total_bytes >= max_bytes)
	   {
           LOG_Message("ERROR Code %d: Format File", ETCP_FILE);
           _snprintf_s(gb_msg_progress_open_sys, 255, _TRUNCATE, "ERROR Code %d: Format File", ETCP_FILE);
           total_bytes = -1;
		   break;
	   }
   }
   fclose(f);
   return total_bytes;

}

int tcpcom_load_file2(vector<unsigned char> &dst_addr, const char *file_route, int max_size)
{
FILE *f = NULL;
int bytes,total_bytes=0;
//unsigned char data;

   dst_addr.resize(max_size);
   if ((f = fopen(file_route,"rb")) == NULL)
   {
	   dbg_printf("tcpcom_load_file: Cant open file: %s\n",file_route);
	   return -1;
   }
   while(!feof(f))
   {
	   bytes = (int)fread ( &dst_addr[total_bytes],
                           sizeof(unsigned char),
                           max_size - total_bytes,
                           f);
	   if (bytes <= 0)
	   {
		   dbg_printf("tcpcom_load_file: Error en el archivo\n");
		   return total_bytes;
	   }
	   total_bytes += bytes;
	   if (total_bytes >= max_size)
	   {
		   total_bytes = -1;
		   break;
	   }
   }
   fclose(f);
   return total_bytes;

}

int tcpcom_save_file(const unsigned char* src_addr,const char *file_route,int bytes)
{
#if defined(_MSV_VER)
  FILE* pFile;
  errno_t err;
  pFile  = fopen(file_route,"w+b" );

  if(pFile==NULL)
  {
	dbg_printf("tcpcom_save_file: Cant open file: %s\n",file_route);
	return -1;
  }
  bytes = fwrite(src_addr,sizeof(unsigned char),bytes,pFile);

  fclose(pFile);
  return bytes;
#else
	return 0;
#endif
}

int tcpcom_load_n_run_image(SOCKET tcpcom_socket, u32 image_addr)
{
  int bytes=0;
  u32 comando[4];
  comando[0] = CMD_HEADER;
  comando[1] = 16;
  comando[2] = LOAD_N_RUN;
  comando[3] = image_addr;

  bytes=send(tcpcom_socket,(const char *)comando, comando[1], 0);

  return bytes;
}

int tcpcom_write_qspi(SOCKET tcpcom_socket,u32 flash_addr, u32 image_addr,u32 image_len)
{
  int bytes=0;
  u32 comando[6];
  comando[0] = CMD_HEADER;
  comando[1] = 24;
  comando[2] = WRITE_QSPI;
  comando[3] = image_addr;
  comando[4] = flash_addr;
  comando[5] = image_len;
  bytes=send(tcpcom_socket,(const char *)comando, comando[1], 0);

  return bytes;
}

int tcpcom_read_qspi(SOCKET tcpcom_socket,u32 flash_addr, u32 image_addr,u32 image_len)
{
  int bytes=0;
  u32 comando[6];
  comando[0] = CMD_HEADER;
  comando[1] = 24;
  comando[2] = READ_QSPI;
  comando[3] = image_addr;
  comando[4] = flash_addr;
  comando[5] = image_len;
  bytes=send(tcpcom_socket,(const char *)comando, comando[1], 0);

  return bytes;
}


int tcpcom_rst_sys(SOCKET tcpcom_socket)
{
  int bytes=0;
  u32 comando[3];
  comando[0] = CMD_HEADER;
  comando[1] = 12;
  comando[2] = RST_SYS;

  bytes=send(tcpcom_socket,(const char *)comando, comando[1], 0);

  return bytes;
}

int tcpcom_read_current_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p)
{
  int ret;
  u32 comando[4];

  comando[0] = CMD_HEADER;
  comando[1] = 16;
  comando[2] = CURR_NET_PARAM;
  comando[3] = ++ID;
  send(tcpcom_socket,(const char *)comando, comando[1], 0);

  ret=tcpcom_rcv_response(tcpcom_socket,(char *)net_params_p,sizeof(struct network_parameters_s),ID);

  return ret;
}

int tcpcom_read_flash_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p)
{
  int ret;
  u32 comando[4];

  comando[0] = CMD_HEADER;
  comando[1] = 16;
  comando[2] = FLSH_NET_PARAM;
  comando[3] = ++ID;
  send(tcpcom_socket,(const char *)comando, comando[1], 0);

  ret=tcpcom_rcv_response(tcpcom_socket,(char *)net_params_p,sizeof(struct network_parameters_s),ID);

  return ret;
}

int tcpcom_write_flash_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p)
{
  int ret,total_bytes=0;
  u32 comando[4+sizeof(struct network_parameters_s)/4+1];

  comando[0] = CMD_HEADER;
  comando[1] = 16;
  comando[2] = SAVE_NET_PARAM;
  comando[3] = sizeof(struct network_parameters_s);

  memcpy(&comando[4],(void*)net_params_p,sizeof(struct network_parameters_s));

  ret=send(tcpcom_socket,(const char *)comando, 16+sizeof(struct network_parameters_s), 0);

  return ret;
}

int sprint_net_param(char* str_dst,const struct network_parameters_s *p_net_params)
{
	char* str_dst_org;
	str_dst_org=str_dst;
	str_dst+=sprintf(str_dst,"\n\r");
	str_dst+=sprintf(str_dst,"HEADER:\t\t\t%04X\n\r",p_net_params->signature_head);

	str_dst+=sprintf(str_dst,"MAC:\t\t\t%02x:%02x:%02x:%02x:%02x:%02x\n\r",p_net_params->mac_addr[0],
										  	  	  	  	  	 p_net_params->mac_addr[1],
										  	  	  	  	  	 p_net_params->mac_addr[2],
										  	  	  	  	  	 p_net_params->mac_addr[3],
										  	  	  	  		 p_net_params->mac_addr[4],
										  	  	  	  	  	 p_net_params->mac_addr[5]);
	str_dst+=sprintf(str_dst,"Options:\t\t%04X\n\r",p_net_params->options);

	str_dst+=sprintf(str_dst,"IP:\t\t\t%3u.%3u.%3u.%3u\n\r",p_net_params->ip_addr[0],
										  	  p_net_params->ip_addr[1],
										  	  p_net_params->ip_addr[2],
										  	  p_net_params->ip_addr[3]);

	str_dst+=sprintf(str_dst,"Gateway:\t\t%3u.%3u.%3u.%3u\n\r",p_net_params->gateway[0],
										     	 p_net_params->gateway[1],
										     	 p_net_params->gateway[2],
										     	 p_net_params->gateway[3]);

	str_dst+=sprintf(str_dst,"Mask:\t\t\t%3u.%3u.%3u.%3u\n\r",p_net_params->mask[0],
										    	p_net_params->mask[1],
										    	p_net_params->mask[2],
										    	p_net_params->mask[3]);

	str_dst+=sprintf(str_dst,"CONF TCP port:\t\t%u\n\r",p_net_params->conf_port);
	str_dst+=sprintf(str_dst,"CMMD TCP port:\t\t%u\n\r",p_net_params->cmmd_port);
	str_dst+=sprintf(str_dst,"DATA TCP port:\t\t%u\n\r",p_net_params->data_port);

	str_dst+=sprintf(str_dst,"TAIL:\t\t\t%04X\n\r",p_net_params->signature_tail);
	str_dst+=sprintf(str_dst,"\n\r");
	return (int)(str_dst - str_dst_org);
}

int tcpimage_clear_image_buffer(SOCKET tcpcom_socket)
{
  int ret,total_bytes=0;
  u32 comando[3];

  comando[0] = CMD_HEADER;
  comando[1] = 12;
  comando[2] = IMAGE_CLEAR_BUF;

  ret=send(tcpcom_socket,(const char *)comando, 12, 0);
  return ret;
}

int cache_enabled(SOCKET tcpcom_socket)
{
  int ret,total_bytes=0;
  u32 comando[3];

  comando[0] = CMD_HEADER;
  comando[1] = 12;
  comando[2] = CACHE_ENABLED;

  ret = send(tcpcom_socket,(const char *)comando, 12, 0);
  return ret;
}

int cache_disabled(SOCKET tcpcom_socket)
{
  int ret,total_bytes=0;
  u32 comando[3];

  comando[0] = CMD_HEADER;
  comando[1] = 12;
  comando[2] = CACHE_DISABLED;

  ret = send(tcpcom_socket,(const char *)comando, 12, 0);
  return ret;
}

int image_enabled(SOCKET tcpcom_socket)
{
    int ret, total_bytes = 0;
    u32 comando[3];

    comando[0] = CMD_HEADER;
    comando[1] = 12;
    comando[2] = IMAGE_ENABLED;

    ret = send(tcpcom_socket, (const char*)comando, 12, 0);
    return ret;
}

int image_disabled(SOCKET tcpcom_socket)
{
    int ret, total_bytes = 0;
    u32 comando[3];

    comando[0] = CMD_HEADER;
    comando[1] = 12;
    comando[2] = IMAGE_DISABLED;

    ret = send(tcpcom_socket, (const char*)comando, 12, 0);
    return ret;
}

int uci_send(SOCKET tcpcom_socket,const char *host_2_uci, u32 n_data)
{
u32 comando[4];
unsigned long size_msg, n_data_pending, n_data_sent;
char *ptr_msg = NULL;
int i_packet, bytes, data_len, split_frame_size, total_bytes = 0, split_bytes = 0;
bool split_frame = false;
int slit_size = 1024 * 1024;

   if (!gb_no_hw)
   {
      try
      {
         size_msg = sizeof(u32) * 4 + n_data;
         if (split_frame && size_msg > slit_size)
         {
            n_data_pending = n_data;
            split_frame_size = sizeof(u32) * 4 + slit_size;
            if ((ptr_msg = (char *)malloc(split_frame_size)) == NULL) 
                  return ELOG(charETCP_MEM, ETCP_MEM);
            i_packet = 0;
            do
            {
               if (n_data_pending > slit_size) n_data_sent = slit_size; 
               else n_data_sent = n_data_pending;

               comando[0] = CMD_HEADER;
               comando[1] = 16;
               comando[2] = UCI_SEND;
               comando[3] = n_data_sent;

               memcpy(ptr_msg, (char *)&comando, sizeof(u32) * 4);
               memcpy(ptr_msg + sizeof(u32) * 4, (char *)(host_2_uci + slit_size * i_packet), n_data_sent);
               //n_data_sent += sizeof(u32) * 4;

               split_bytes = 0;
               do
               {
                  if ((bytes = send(tcpcom_socket,(const char *)&ptr_msg[split_bytes], n_data_sent + sizeof(u32) * 4 - split_bytes, 0)) < 0)
                     return RLOG(bytes);
                  split_bytes += bytes;
                  if (split_bytes < n_data_sent  + sizeof(u32) * 4) 
                        ELOG("Warnning", -1);
               } while (split_bytes < n_data_sent  + sizeof(u32) * 4);
               total_bytes += n_data_sent;
               i_packet++;
               n_data_pending = size_msg - total_bytes;
            } while (n_data_pending > 0);
         }
         else
         {
            if ((ptr_msg = (char *)malloc(size_msg)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

            comando[0] = CMD_HEADER;
            comando[1] = 16;
            comando[2] = UCI_SEND;
            comando[3] = n_data;

            memcpy(ptr_msg, (char *)&comando, sizeof(u32) * 4);
            memcpy(ptr_msg + sizeof(u32) * 4, (char *)host_2_uci, n_data);

            data_len = size_msg;
            do
            {
               bytes = send(tcpcom_socket,(const char *)&ptr_msg[total_bytes], data_len - total_bytes, 0);
               if (bytes < 0)
                  return RLOG(bytes);
               total_bytes += bytes;
               if (total_bytes < data_len) 
                     ELOG("Warnning", -1);
            } while (total_bytes < data_len);
         }
      }
      __finally
      {
         if (ptr_msg != NULL) free(ptr_msg);
      }
   }
   return total_bytes;
}

int uci_recv(SOCKET tcpcom_socket, u8* uci_2_host, u32 n_data)
{
  int ret=0;
  u32 comando[4];
  comando[0] = CMD_HEADER;
  comando[1] = 20;
  comando[2] = UCI_RECV;
  comando[3] = n_data;

  send(tcpcom_socket,(const char *)comando, comando[1], 0);

  tcpcom_large_recv(tcpcom_socket,(char *)uci_2_host, n_data,-1);

  return ret;
}

int uci_clear_send_fast_recv(SOCKET tcpcom_socket,const u8 *host_2_uci, u32 n_data_host_2_uci,u8* uci_2_host, u32 n_data_uci_2_host)
{
  int ret=0;
  int total_bytes=0;
  const char *data=(const char *)host_2_uci;
  u32 comando[6];

  tcpcom_large_send(tcpcom_socket,data, n_data_host_2_uci,-1);

  comando[0] = CMD_HEADER;
  comando[1] = sizeof(comando);
  comando[2] = UCI_C_S_F_R;
  comando[3] = n_data_host_2_uci;
  comando[4] = n_data_uci_2_host;
  comando[5] = ++ID;

  send(tcpcom_socket,(const char *)comando, comando[1], 0);

  ret=tcpcom_rcv_response(tcpcom_socket,(char *)uci_2_host,n_data_uci_2_host,ID);

  return ret;
}

int uci_clear_send_block_recv(SOCKET tcpcom_socket,const u8 *host_2_uci, u32 n_data_host_2_uci,u8* uci_2_host, u32 n_data_uci_2_host)
{
  int result=0;
  int total_bytes=0,bytes,data_len;
  const char *data=(const char *)host_2_uci;
  u32 comando[4];

  comando[0] = CMD_HEADER;
  comando[1] = sizeof(comando);
  comando[2] = UCI_C_S_B_R;
  comando[3] = n_data_host_2_uci;

  send(tcpcom_socket,(const char *)comando, comando[1], 0);
  
  data_len=n_data_host_2_uci;
  do {
    bytes = send(tcpcom_socket,(const char *)&host_2_uci[total_bytes], data_len-total_bytes, 0);
	if(bytes<0)
		return total_bytes;
	total_bytes += bytes;
  }while(total_bytes<data_len);

   if ((result = ws_recv((char *)uci_2_host, n_data_uci_2_host, tcpcom_socket)) < 0)
      return RLOG(result);

  return ETCP_NONE;
}

int uci_uci2host_n_data(SOCKET tcpcom_socket, u32 *n_data_ready)
{
  int ret=0;
  u32 comando[4];
  comando[0] = CMD_HEADER;
  comando[1] = 16;
  comando[2] = UCI_UCI2HOST_N;
  comando[3] = ++ID;

  send(tcpcom_socket,(const char *)comando, comando[1], 0);

  ret=tcpcom_rcv_response(tcpcom_socket,(char *)n_data_ready,4,ID);

  return ret;
}

void image_exec(SOCKET tcpcom_socket, const char *archivo_imagen)
{
int ret,ndatos;

	ndatos = tcpcom_load_file(datos_env, archivo_imagen, TAMANIO);
	ret = tcpcom_large_send(tcpcom_socket, (char *)datos_env, ndatos * sizeof(unsigned char),-1);
	tcpcom_load_n_run_image(tcpcom_socket,-1);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int HW_RDActualNetConfig(struct network_parameters_s* p_net_param)
{
   int result = ETCP_NONE;

   if (!gb_no_hw)
   {
      if (gb_ethernet_connected == 1)
         result = tcpcom_read_current_net_conf(uci_tcp_cfg, p_net_param);
   }
   return RLOG(result);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int HW_RDUserFlashNetConfig(struct network_parameters_s* p_net_param)
{
   int result = ETCP_NONE;

   if (!gb_no_hw)
   {
      if (gb_ethernet_connected == 1)
         result = tcpcom_read_flash_net_conf(uci_tcp_cfg, p_net_param);
   }
   return RLOG(result);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int HW_WRUserFlashNetConfig(struct network_parameters_s* p_net_param)
{
   int result = ETCP_NONE;

   if (!gb_no_hw)
   {
      if (gb_ethernet_connected == 1)
      {
         if ((result = tcpcom_write_flash_net_conf(uci_tcp_cfg, p_net_param)) < 0)
            return RLOG(result);
      }
   }
   return RLOG(result);
}

