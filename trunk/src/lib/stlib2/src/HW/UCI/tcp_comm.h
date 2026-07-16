#ifndef fp_tcp_commH
#define fp_tcp_commH

#if defined(_MSC_VER)
   #define NOMINMAX
#endif

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>
#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

#include "tad.h"
#include "xil_types.h"

#define ERROR_PRINT				dbg_printf

#define RX_BUFFER_SIZE	128*1024


#pragma comment(lib, "Ws2_32.lib")

#define PUT_MEM			0x0100
#define DBG_FUNCTION	   0x0101
#define GET_MEM			0x0102
#define LOAD_N_RUN		0x0103
#define WRITE_QSPI		0x0104
#define READ_QSPI		   0x0105
#define RST_SYS			0x0106
#define CURR_NET_PARAM	0x0107
#define SAVE_NET_PARAM	0x0108
#define FLSH_NET_PARAM	0x0109
#define UCI_SEND		   0x010A
#define UCI_RECV		   0x010B
#define UCI_C_S_F_R		0x010C
#define UCI_C_S_B_R		0x010D
#define UCI_UCI2HOST_N	0x010E
#define IMAGE_CLEAR_BUF	0x010F
#define CACHE_ENABLED	0x0110
#define CACHE_DISABLED	0x0111
#define IMAGE_DISABLED	0x0112
#define IMAGE_ENABLED	0x0113
#define GET_SHARED_MEM	0x0114

#define CMD_HEADER		0XC51CDA5E
#define COM_MIN_SIZE	12

#define __htons					endian16
#define __htonl					endian32

struct network_parameters_s
{
	// Signature
	u16 signature_head;
	// MAC_PARAMETERS
	u8 mac_addr[6];

	//IP_PARAMETERS
	u16 options;
	u8 ip_addr[4];
	u8 gateway[4];
	u8 mask[4];

	// TCP_PARAMETERS
	u16 conf_port;
	u16 cmmd_port;
	u16 data_port;
	// Signature
	u16 signature_tail;
};

extern bool gb_no_hw;
extern bool gb_cancel_connect;

__inline unsigned short endian16(unsigned short i);
__inline unsigned int endian32(unsigned int i);

int tcpcom_connect(char* ip_str_dst, unsigned short port, SOCKET* socket_id);

int tcpcom_save_file(const unsigned char* src_addr,const char *file_route,int bytes);
int tcpcom_load_file(unsigned char* dst_addr,const char *file_route,int max_size);
int tcpcom_load_file2(vector<unsigned char> &dst_addr, const char *file_route,int max_size);
int tcpcom_load_n_run_image(SOCKET tcpcom_socket, u32 image_addr);

int sprint_net_param(char* str_dst,const struct network_parameters_s *p_net_params);
int tcpcom_write_flash_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p);
int tcpcom_read_flash_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p);
int tcpcom_read_current_net_conf(SOCKET tcpcom_socket, struct network_parameters_s *net_params_p);

int tcpcom_write_qspi(SOCKET tcpcom_socket,u32 flash_addr, u32 image_addr,u32 image_len);
int tcpcom_read_qspi(SOCKET tcpcom_socket,u32 flash_addr, u32 image_addr,u32 image_len);
int tcpcom_rst_sys(SOCKET tcpcom_socket);

int tcpcom_recv_no_block(SOCKET tcpcom_socket, const char *data);
int tcpcom_large_send(SOCKET tcpcom_socket, const char *data, u32 data_len,u32 dst_addr);
int tcpcom_large_recv(SOCKET tcpcom_socket,       char *data, u32 data_len,u32 src_addr);

int tcpcom_dbg_function(SOCKET tcpcom_socket);

int tcpimage_clear_image_buffer(SOCKET tcpcom_socket);

int uci_send(SOCKET tcpcom_socket,const char * host_2_uci, u32 n_data);
int uci_recv(SOCKET tcpcom_socket, u8* uci_2_host, u32 n_data);
int uci_clear_send_block_recv(SOCKET tcpcom_socket,const u8 *host_2_uci, u32 n_data_host_2_uci,u8* uci_2_host, u32 n_data_uci_2_host);
int uci_clear_send_fast_recv(SOCKET tcpcom_socket,const u8 *host_2_uci, u32 n_data_host_2_uci,u8* uci_2_host, u32 n_data_uci_2_host);
int uci_uci2host_n_data(SOCKET tcpcom_socket, u32 *n_data_ready);
//int BASE_MOD_LoadFirmware(const char *filename);
SOCKET tcpimage_connect_img_opts(char* ip_str_dst,unsigned short port);
void image_flash(SOCKET tcpcom_socket, const char *archivo_imagen);
void image_exec(SOCKET tcpcom_socket, const char *archivo_imagen);

int cache_enabled(SOCKET tcpcom_socket);
int cache_disabled(SOCKET tcpcom_socket);

int image_enabled(SOCKET tcpcom_socket);
int image_disabled(SOCKET tcpcom_socket);

int HW_RDActualNetConfig(struct network_parameters_s* p_net_param);
int HW_RDUserFlashNetConfig(struct network_parameters_s* p_net_param);
int HW_WRUserFlashNetConfig(struct network_parameters_s* p_net_param);

#endif //_TCPCOM_H_
