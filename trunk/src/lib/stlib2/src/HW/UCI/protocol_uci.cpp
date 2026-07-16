// -----------------------------------------------------------------------------
/**
@file protocol.c

@brief Este archivo contiene la implementaciÃ³n de las funciones de la clase \c FL_Aperture.<br>

Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
un barrido.

@author (rg) Ricardo GonzÃ¡lez

<pre>
MODIFICATION HISTORY:

Ver   Who  Date       Changes
----- ---- ---------- -----------------------------------------------------------
1.00a (rg) 16/01/2017 First release
</pre>
*/

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <math.h>
#include <winsock2.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_reg.h"
#include "protocol_uci.h"
#include "protocol_base.h"
#include "tcp_error_code.h"
#include "c_log_file.h"
#include "tcp_comm.h"
#include "api_tcp.h"
#include "ctimer.h"
#include "utils.h"
#include "c_log_protocol.h"
#include "shared_mem.h"
#include "api_tcp.h"
#include "st_error_code.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

AnsiString gb_file_log_protocol = "";
int gb_log_protocol_enabled = 0;
bool gb_load_config_enabled = true;
TMSG_TerminalLog gb_terminal_log;
u32 gb_id_frame_rd = 0;

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del nï¿½mero de datos de la trama.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
unsigned short ChecksumFletcher16(char *data, int n_bytes)
{
   return 0;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
void LOGProtocol(char *msg)
{
FILE *fd = NULL;
char mode[5];
int print_time = 0;
time_t t;

   if (!gb_log_protocol_enabled) return;
   if (msg != NULL)
   {
      if (gb_file_log_protocol == "")
      {
         if (gb_log_file_path == "") gb_file_log_protocol = "log_protocol.txt";
         else gb_file_log_protocol = gb_log_file_path + "\\log_protocol.txt";
         strcpy(mode,"wt");
         print_time = 1;
      }
      else strcpy(mode,"at");
      if ((fd = fopen (gb_file_log_protocol.c_str(), mode)) != NULL)
      {
			if (print_time)
         {
            time(&t);
            fprintf (fd, "\r\n%s", ctime(&t));
         }
         fprintf(fd, "%s", msg);
         fclose(fd);
      }
   }
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_RESET_ACQ_COUNTER(void)
{
T_AMPLIACommand cmd;
int result;

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_RESET_ACQ_COUNTER;
   cmd.BIT.DAT = 0;
   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
       {
           LOGProtocol("\r\nCMD_RESET_ACQ_COUNTER... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
   }
   LOGProtocol("\r\nCMD_RESET_ACQ_COUNTER... OK");
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_RESET_ENCODER(int id_encoder)
{
T_AMPLIACommand cmd;
int result;

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_RESET_ENCODER;
   if (id_encoder < 0) cmd.BIT.DAT = 0xFFFF;
   else cmd.BIT.DAT = id_encoder;
   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
       {
           LOGProtocol("\r\nCMD_RESET_ENCODER... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
   }
   LOGProtocol("\r\nCMD_RESET_ENCODER... OK");
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_CACHE_ENABLED(void)
{
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if (!gb_no_hw) cache_enabled(uci_tcp_cfg);
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_CACHE_DISABLED(void)
{
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if (!gb_no_hw) cache_disabled(uci_tcp_cfg);
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_IMAGE_ENABLED(void)
{
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if (!gb_no_hw) image_enabled(uci_tcp_cfg);
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_IMAGE_DISABLED(void)
{
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if (!gb_no_hw) image_disabled(uci_tcp_cfg);
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_STOP(void)
{
T_AMPLIACommand cmd;
int result;

    if (gb_no_hw) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_STOP;
    cmd.BIT.DAT = 0;
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
        {
            LOGProtocol("\r\nCMD_STOP... ERROR");
            return RLOG(result);
        }
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    LOGProtocol("\r\nCMD_STOP... OK");
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PAUSE(TMSG_Pause *msg, bool wait)
{
    T_AMPLIACommand cmd;
    TMSG_SITAUStatus msg_status;
    int result, n_data_recv;
    CTimeout* timeout = NULL;
    char* ptr_msg = NULL;
    unsigned long size_msg;

    if (gb_no_hw) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    if ((ptr_msg = (char*)malloc(sizeof(TMSG_Pause) + sizeof(T_AMPLIACommand))) == NULL)
        return ELOG(charETCP_MEM, ETCP_MEM);

    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_PAUSE;
    cmd.BIT.DAT = sizeof(TMSG_Pause);

    memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
    memcpy(ptr_msg + sizeof(T_AMPLIACommand), (char*)msg, sizeof(TMSG_Pause));
    size_msg = sizeof(T_AMPLIACommand) + sizeof(TMSG_Pause);

    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_msg)) < 0)
        {
            LOGProtocol("\r\nCMD_PAUSE... ERROR");
            return RLOG(result);
        }
        LOGProtocol("\r\nCMD_PAUSE... OK");
        n_data_recv = 0;

        if (!gb_no_hw && wait)
        {
            if ((timeout = new CTimeout(1000, MILISECONDS)) == NULL) return result;
            timeout->Start();
            while (n_data_recv == 0 && !timeout->Timeout())
            {
                if ((n_data_recv = TCP_Receive_ConfigPort(&msg_status, sizeof(TMSG_SITAUStatus))) < 0)
                    return RLOG(n_data_recv);
            }

            if (n_data_recv == 0)
            {
                ELOG("No PAUSE", -1);
            }
        }
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
        if (ptr_msg != NULL) free(ptr_msg);
        if (timeout != NULL) delete timeout;
    }
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_CONTINUE(void)
{
    T_AMPLIACommand cmd;
    int result;

    if (gb_no_hw) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_CONTINUE;
    cmd.BIT.DAT = 0;
    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
        {
            LOGProtocol("\r\nCMD_CONTINUE... ERROR");
            return RLOG(result);
        }
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    LOGProtocol("\r\nCMD_CONTINUE... OK");
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_ACQUIRE(unsigned short n_acq)
{
T_AMPLIACommand cmd;
int result;
char str[255];

    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_FP_ACQUIRE;
    cmd.BIT.DAT = n_acq;

    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
        {
            sprintf(str, "\r\nCMD_FP_ACQUIRE (%d)... ERROR", n_acq);
            LOGProtocol(str);
            return RLOG(result);
        }
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
    }
    if (log_protocol != NULL) log_protocol->LastMessage(n_acq);
    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PrintPASystem(void)
{
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_TERMINAL_LOG(unsigned short data)
{
T_AMPLIACommand cmd;
int result;

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_TERMINAL_LOG;
   cmd.BIT.DAT = data;
   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0) return RLOG(result);
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
   }
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PrintVirtualChannel_Config(int id_vch)
{
T_AMPLIACommand cmd;
int result;
char str[255];

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_PRINT_VCH_CONFIG;
   if (id_vch < 0) cmd.BIT.DAT = 0xFFFF;
   else cmd.BIT.DAT = id_vch;
   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
       {
           sprintf(str, "\r\nCMD_FP_PRINT_VCH_CONFIG (%d)... ERROR", id_vch);
           LOGProtocol(str);
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
   }
   sprintf(str, "\r\nCMD_FP_PRINT_VCH_CONFIG (%d)... OK", id_vch);
   LOGProtocol(str);
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PrintVirtualChannel_BASERegisters(int id_vch)
{
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PrintConfig(void)
{
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_PulseAmplitude(unsigned char data)
{
TFP_MSG_PulseAmplitude *msg = NULL;
T_AMPLIACommand cmd;
int result = ETCP_NONE;
unsigned long size_msg;
char *ptr_msg = NULL;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
//try
//{
   if ((msg = (TFP_MSG_PulseAmplitude *)malloc(sizeof(TFP_MSG_PulseAmplitude))) == NULL)
      return ELOG(charETCP_MEM, ETCP_MEM);
   if ((ptr_msg = (char *)malloc(sizeof(TFP_MSG_PulseAmplitude) + sizeof(T_AMPLIACommand))) == NULL)
      return ELOG(charETCP_MEM, ETCP_MEM);

   msg->data = data;

   size_msg = sizeof(TFP_MSG_PulseAmplitude);
   if (size_msg > 65535)
      ELOG("Overflow MSG", -1);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_PULSE_AMPLITUDE;
   cmd.BIT.DAT = size_msg;

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   memcpy(ptr_msg + sizeof(T_AMPLIACommand), (char *)msg, sizeof(TFP_MSG_PulseAmplitude));
   size_msg = sizeof(T_AMPLIACommand) + sizeof(TFP_MSG_PulseAmplitude);

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_msg)) < 0)
       {
           LOGProtocol("\r\nCMD_FP_PULSE_AMPLITUDE... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (msg != NULL) free(msg);
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_FP_PULSE_AMPLITUDE... OK");
   if (log_protocol != NULL) log_protocol->LastMessage(data);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_TriggerSource(TMSG_TriggerSource *msg)
{
T_AMPLIACommand cmd;
unsigned long size_msg;
char *ptr_msg = NULL;
int result;

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   if ((ptr_msg = (char *)malloc(sizeof(TMSG_TriggerSource) + sizeof(T_AMPLIACommand))) == NULL)
      return ELOG(charETCP_MEM, ETCP_MEM);

   size_msg = sizeof(TMSG_TriggerSource);
   if (size_msg > 65535)
      ELOG("Overflow MSG", -1);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_TRIGGER_SOURCE;
   cmd.BIT.DAT = size_msg;

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   memcpy(ptr_msg + sizeof(T_AMPLIACommand), (char *)msg, sizeof(TMSG_TriggerSource));
   size_msg = sizeof(T_AMPLIACommand) + sizeof(TMSG_TriggerSource);

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_msg)) < 0)
       {
           LOGProtocol("\r\nCMD_FP_TRIGGER_SOURCE... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_FP_TRIGGER_SOURCE... OK");
   if (log_protocol != NULL) log_protocol->LastMessage(msg);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_StatusConfig(TMSG_StatusConfig* msg)
{
    T_AMPLIACommand cmd;
    unsigned long size_msg;
    char* ptr_msg = NULL;
    int result;

    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    if ((ptr_msg = (char*)malloc(sizeof(TMSG_StatusConfig) + sizeof(T_AMPLIACommand))) == NULL)
        return ELOG(charETCP_MEM, ETCP_MEM);

    size_msg = sizeof(TMSG_StatusConfig);
    if (size_msg > 65535)
        ELOG("Overflow MSG", -1);
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_STATUS_CONFIG;
    cmd.BIT.DAT = size_msg;

    memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
    memcpy(ptr_msg + sizeof(T_AMPLIACommand), (char*)msg, sizeof(TMSG_StatusConfig));
    size_msg = sizeof(T_AMPLIACommand) + sizeof(TMSG_StatusConfig);

    try
    {
        if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
        {
            ELOG(charES2_Acquire, ES2_Acquire);
            return false;
        }
        if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_msg)) < 0)
        {
            LOGProtocol("\r\nCMD_STATUS_CONFIG... ERROR");
            return RLOG(result);
        }
    }
    __finally
    {
        if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
        if (ptr_msg != NULL) free(ptr_msg);
    }
    LOGProtocol("\r\nCMD_STATUS_CONFIG... OK");
    if (log_protocol != NULL) log_protocol->LastMessage(msg);

    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VCH_Config(TMSG_Config *msg)
{
T_AMPLIACommand cmd, cmd_aux;
unsigned long size_msg, size_frame;
char *ptr_msg = NULL;
int result, ptr_msg_offset;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   size_msg = sizeof(TMSG_Config);
   if (size_msg >= 0xFFFF)
   {
      cmd.BIT.DAT = 0xFFFF;
      cmd_aux.CMD = size_msg;
   }
   else cmd.BIT.DAT = size_msg;
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_VIRTUAL_CHANNEL_CONFIG;

   size_frame = sizeof(TMSG_Config) + sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

   if ((ptr_msg = (char *)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   ptr_msg_offset = sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF)
   {
      memcpy(ptr_msg + ptr_msg_offset, (char *)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset += sizeof(T_AMPLIACommand);
   }
   memcpy(ptr_msg + ptr_msg_offset, (char *)msg, sizeof(TMSG_Config));

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
       {
           LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_CONFIG... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_CONFIG... OK");
   if (log_protocol != NULL) log_protocol->LastMessage(msg);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VCH_FIR(TMSG_FIR * msg)
{
    T_AMPLIACommand cmd, cmd_aux;
    unsigned long size_msg, size_frame;
    char* ptr_msg = NULL;
    int result, ptr_msg_offset;

    if (!gb_load_config_enabled) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    size_msg = sizeof(TMSG_FIR);
    if (size_msg >= 0xFFFF)
    {
        cmd.BIT.DAT = 0xFFFF;
        cmd_aux.CMD = size_msg;
    }
    else cmd.BIT.DAT = size_msg;
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_FP_VIRTUAL_CHANNEL_FIR;

    size_frame = sizeof(TMSG_FIR) + sizeof(T_AMPLIACommand);
    if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

      if ((ptr_msg = (char*)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

      memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset = sizeof(T_AMPLIACommand);
      if (cmd.BIT.DAT == 0xFFFF)
      {
         memcpy(ptr_msg + ptr_msg_offset, (char*)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
         ptr_msg_offset += sizeof(T_AMPLIACommand);
      }
      memcpy(ptr_msg + ptr_msg_offset, (char*)msg, sizeof(TMSG_FIR));

      try
      {
          if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
          {
              ELOG(charES2_Acquire, ES2_Acquire);
              return false;
          }
          if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
          {
              LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_FIR... ERROR");
              return RLOG(result);
          }
      }
      __finally
      {
          if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
          if (ptr_msg != NULL) free(ptr_msg);
      }
      LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_FIR... OK");
      if (log_protocol != NULL) log_protocol->LastMessage(msg);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VCH_EmissionFocallaws(TMSG_EmissionFocalLaws *msg)
{
T_AMPLIACommand cmd, cmd_aux;
unsigned long size_msg, size_frame;
char *ptr_msg = NULL;
int result, ptr_msg_offset;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   size_msg = sizeof(TMSG_EmissionFocalLaws);
   if (size_msg >= 0xFFFF)
   {
      cmd.BIT.DAT = 0xFFFF;
      cmd_aux.CMD = size_msg;
   }
   else cmd.BIT.DAT = size_msg;
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_VCH_EMISSION_FOCAL_LAW;

   size_frame = sizeof(TMSG_EmissionFocalLaws) + sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

   if ((ptr_msg = (char *)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   ptr_msg_offset = sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF)
   {
      memcpy(ptr_msg + ptr_msg_offset, (char *)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset += sizeof(T_AMPLIACommand);
   }
   memcpy(ptr_msg + ptr_msg_offset, (char *)msg, sizeof(TMSG_EmissionFocalLaws));

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
       {
           LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_FOCAL_LAW... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_FOCAL_LAW... OK");
   if (log_protocol != NULL) log_protocol->LastMessage(msg);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VCH_EmissionFocallawsExtended(TMSG_EmissionFocalLawsExtended* msg)
{
    T_AMPLIACommand cmd, cmd_aux;
    unsigned long size_msg, size_frame;
    char* ptr_msg = NULL;
    int result, ptr_msg_offset;

    if (!gb_load_config_enabled) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    size_msg = sizeof(TMSG_EmissionFocalLawsExtended);
    if (size_msg >= 0xFFFF)
    {
        cmd.BIT.DAT = 0xFFFF;
        cmd_aux.CMD = size_msg;
    }
    else cmd.BIT.DAT = size_msg;
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_VCH_EMISSION_FOCAL_LAW_EXT;

    size_frame = sizeof(TMSG_EmissionFocalLawsExtended) + sizeof(T_AMPLIACommand);
    if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

        if ((ptr_msg = (char*)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

        memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
        ptr_msg_offset = sizeof(T_AMPLIACommand);
        if (cmd.BIT.DAT == 0xFFFF)
        {
            memcpy(ptr_msg + ptr_msg_offset, (char*)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
            ptr_msg_offset += sizeof(T_AMPLIACommand);
        }
        memcpy(ptr_msg + ptr_msg_offset, (char*)msg, sizeof(TMSG_EmissionFocalLawsExtended));

        try
        {
            if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
            {
                ELOG(charES2_Acquire, ES2_Acquire);
                return false;
            }
            if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
            {
                LOGProtocol("\r\nCMD_VCH_FOCAL_LAW_EXT... ERROR");
                return RLOG(result);
            }
        }
        __finally
        {
            if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
            if (ptr_msg != NULL) free(ptr_msg);
        }
        LOGProtocol("\r\nCMD_VCH_FOCAL_LAW_EXT... OK");
        if (log_protocol != NULL) log_protocol->LastMessage(msg);

    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VirtualChannel_Beamformer(TMSG_Beamformer *msg)
{
T_AMPLIACommand cmd, cmd_aux;
unsigned long size_msg, size_frame;
char *ptr_msg = NULL;
int result, ptr_msg_offset;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   size_msg = sizeof(TMSG_Beamformer);
   if (size_msg >= 0xFFFF)
   {
      cmd.BIT.DAT = 0xFFFF;
      cmd_aux.CMD = size_msg;
   }
   else cmd.BIT.DAT = size_msg;
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_VIRTUAL_CHANNEL_BEAMFORMER;

   size_frame = sizeof(TMSG_Beamformer) + sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

   if ((ptr_msg = (char *)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   ptr_msg_offset = sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF)
   {
      memcpy(ptr_msg + ptr_msg_offset, (char *)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset += sizeof(T_AMPLIACommand);
   }
   memcpy(ptr_msg + ptr_msg_offset, (char *)msg, sizeof(TMSG_Beamformer));

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
       {
           LOGProtocol("\r\nCMD_VIRTUAL_CHANNEL_BEAMFORMER (PA)... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_VIRTUAL_CHANNEL_BEAMFORMER (PA)... OK");

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VirtualChannel_BeamformerExtended(TMSG_BeamformerExtended* msg)
{
    T_AMPLIACommand cmd, cmd_aux;
    unsigned long size_msg, size_frame;
    char* ptr_msg = NULL;
    int result, ptr_msg_offset;

    if (!gb_load_config_enabled) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    size_msg = sizeof(TMSG_BeamformerExtended);
    if (size_msg >= 0xFFFF)
    {
        cmd.BIT.DAT = 0xFFFF;
        cmd_aux.CMD = size_msg;
    }
    else cmd.BIT.DAT = size_msg;
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT;

    size_frame = sizeof(TMSG_BeamformerExtended) + sizeof(T_AMPLIACommand);
    if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

        if ((ptr_msg = (char*)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

        memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
        ptr_msg_offset = sizeof(T_AMPLIACommand);
        if (cmd.BIT.DAT == 0xFFFF)
        {
            memcpy(ptr_msg + ptr_msg_offset, (char*)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
            ptr_msg_offset += sizeof(T_AMPLIACommand);
        }
        memcpy(ptr_msg + ptr_msg_offset, (char*)msg, sizeof(TMSG_BeamformerExtended));

        try
        {
            if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
            {
                ELOG(charES2_Acquire, ES2_Acquire);
                return false;
            }
            if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
            {
                LOGProtocol("\r\nCMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (PA)... ERROR");
                return RLOG(result);
            }
        }
        __finally
        {
            if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
            if (ptr_msg != NULL) free(ptr_msg);
        }
        LOGProtocol("\r\nCMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (PA)... OK");

    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_TFM_FMC_Forward(TMSG_TFM_FMC_Forward* msg)
{
    T_AMPLIACommand cmd, cmd_aux;
    unsigned long size_msg, size_frame;
    char* ptr_msg = NULL;
    int result, ptr_msg_offset;

    if (!gb_load_config_enabled) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    size_msg = sizeof(TMSG_TFM_FMC_Forward);
    if (size_msg >= 0xFFFF)
    {
        cmd.BIT.DAT = 0xFFFF;
        cmd_aux.CMD = size_msg;
    }
    else cmd.BIT.DAT = size_msg;
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_TFM_FMC_FORWARD;

    size_frame = sizeof(TMSG_TFM_FMC_Forward) + sizeof(T_AMPLIACommand);
    if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

        if ((ptr_msg = (char*)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

        memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
        ptr_msg_offset = sizeof(T_AMPLIACommand);
        if (cmd.BIT.DAT == 0xFFFF)
        {
            memcpy(ptr_msg + ptr_msg_offset, (char*)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
            ptr_msg_offset += sizeof(T_AMPLIACommand);
        }
        memcpy(ptr_msg + ptr_msg_offset, (char*)msg, sizeof(TMSG_TFM_FMC_Forward));

        try
        {
            if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
            {
                ELOG(charES2_Acquire, ES2_Acquire);
                return false;
            }
            if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
            {
                LOGProtocol("\r\nCMD_TFM_FMC_FORWARD... ERROR");
                return RLOG(result);
            }
        }
        __finally
        {
            if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
            if (ptr_msg != NULL) free(ptr_msg);
        }
        LOGProtocol("\r\nCMD_TFM_FMC_FORWARD... OK");

    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_TFM_PWI_Forward(TMSG_TFM_PWI_Forward* msg)
{
    T_AMPLIACommand cmd, cmd_aux;
    unsigned long size_msg, size_frame;
    char* ptr_msg = NULL;
    int result, ptr_msg_offset;

    if (!gb_load_config_enabled) return ETCP_NONE;
    if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

    size_msg = sizeof(TMSG_TFM_PWI_Forward);
    if (size_msg >= 0xFFFF)
    {
        cmd.BIT.DAT = 0xFFFF;
        cmd_aux.CMD = size_msg;
    }
    else cmd.BIT.DAT = size_msg;
    cmd.BIT.REG = UCI_REG_UCIv2;
    cmd.BIT.TAC = 0;
    cmd.BIT.DIR = CMD_TFM_PWI_FORWARD;

    size_frame = sizeof(TMSG_TFM_PWI_Forward) + sizeof(T_AMPLIACommand);
    if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

        if ((ptr_msg = (char*)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

        memcpy(ptr_msg, (char*)&cmd.CMD, sizeof(T_AMPLIACommand));
        ptr_msg_offset = sizeof(T_AMPLIACommand);
        if (cmd.BIT.DAT == 0xFFFF)
        {
            memcpy(ptr_msg + ptr_msg_offset, (char*)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
            ptr_msg_offset += sizeof(T_AMPLIACommand);
        }
        memcpy(ptr_msg + ptr_msg_offset, (char*)msg, sizeof(TMSG_TFM_PWI_Forward));

        try
        {
            if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
            {
                ELOG(charES2_Acquire, ES2_Acquire);
                return false;
            }
            if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
            {
                LOGProtocol("\r\nCMD_TFM_PWI_FORWARD... ERROR");
                return RLOG(result);
            }
        }
        __finally
        {
            if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
            if (ptr_msg != NULL) free(ptr_msg);
        }
        LOGProtocol("\r\nCMD_TFM_PWI_FORWARD... OK");

    return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de la TGC a una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_VirtualChannel_TGC(TMSG_TGC *msg)
{
T_AMPLIACommand cmd, cmd_aux;
unsigned long size_msg, size_frame;
char *ptr_msg = NULL;
int result, ptr_msg_offset;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   size_msg = sizeof(TMSG_TGC);
   if (size_msg >= 0xFFFF)
   {
      cmd.BIT.DAT = 0xFFFF;
      cmd_aux.CMD = size_msg;
   }
   else cmd.BIT.DAT = size_msg;
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_VIRTUAL_CHANNEL_TGC;

   size_frame = sizeof(TMSG_TGC) + sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

   if ((ptr_msg = (char *)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   ptr_msg_offset = sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF)
   {
      memcpy(ptr_msg + ptr_msg_offset, (char *)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset += sizeof(T_AMPLIACommand);
   }
   memcpy(ptr_msg + ptr_msg_offset, (char *)msg, sizeof(TMSG_TGC));

   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)ptr_msg, size_frame)) < 0)
       {
           LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_TGC... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
       if (ptr_msg != NULL) free(ptr_msg);
   }
   LOGProtocol("\r\nCMD_FP_VIRTUAL_CHANNEL_TGC... OK");
   if (log_protocol != NULL) log_protocol->LastMessage(msg);

   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**
EnvÃ­a el mensaje de configuraciÃ³n de los registros de una BASE.

@return Ver cÃ³digos de error.
*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_SITAUHardwareEnabled(int data)
{
T_AMPLIACommand cmd;
int result;

   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_HW_SITAU_ENABLED;
   cmd.BIT.DAT = data;
   try
   {
       if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
       {
           ELOG(charES2_Acquire, ES2_Acquire);
           return false;
       }
       if ((result = uci_send(uci_tcp_cfg, (const char*)&cmd.CMD, sizeof(T_AMPLIACommand))) < 0)
       {
           LOGProtocol("\r\nCMD_HW_SITAU_ENABLED... ERROR");
           return RLOG(result);
       }
   }
   __finally
   {
       if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
   }
   LOGProtocol("\r\nCMD_HW_SITAU_ENABLED... OK");
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int TCPSend_MSG_LoadFirmware(char *buffer, unsigned long n_data)
{
T_AMPLIACommand cmd, cmd_aux;
unsigned long size_msg, size_frame;
char *ptr_msg = NULL, *ptr_status = NULL;
int result, ptr_msg_offset;
TMSG_LoadFirmware msg;
CTimeout *timeout = NULL;
bool firmware_loaded;

   if (!gb_load_config_enabled) return ETCP_NONE;
   if (TCP_Connected() == 0) return ELOG(charETCP_CONNECT, ETCP_CONNECT);

   msg.size_file = n_data;
   size_msg = sizeof(TMSG_LoadFirmware);
   if (size_msg >= 0xFFFF)
   {
      cmd.BIT.DAT = 0xFFFF;
      cmd_aux.CMD = size_msg;
   }
   else cmd.BIT.DAT = size_msg;
   cmd.BIT.REG = UCI_REG_UCIv2;
   cmd.BIT.TAC = 0;
   cmd.BIT.DIR = CMD_FP_LOAD_FIRMWARE;

   size_frame = sizeof(TMSG_LoadFirmware) + sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF) size_frame += sizeof(unsigned long);

try
{
   if ((ptr_msg = (char *)malloc(size_frame)) == NULL) return ELOG(charETCP_MEM, ETCP_MEM);

   memcpy(ptr_msg, (char *)&cmd.CMD, sizeof(T_AMPLIACommand));
   ptr_msg_offset = sizeof(T_AMPLIACommand);
   if (cmd.BIT.DAT == 0xFFFF)
   {
      memcpy(ptr_msg + ptr_msg_offset, (char *)&cmd_aux.CMD, sizeof(T_AMPLIACommand));
      ptr_msg_offset += sizeof(T_AMPLIACommand);
   }
   memcpy(ptr_msg + ptr_msg_offset, (char *)&msg, sizeof(TMSG_LoadFirmware));

   if ((result = uci_send(uci_tcp_cfg, (const char *)ptr_msg, size_frame)) < 0)
   {
      LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE... ERROR");
      return RLOG(result);
   }
   LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE... OK");

   if ((result = uci_send(uci_tcp_cfg, (const char *)buffer, n_data)) < 0)
   {
      LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE (File)... ERROR");
      return RLOG(result);
   }

   Delay(2000, MILISECONDS);

   if ((timeout = new CTimeout(10000, MILISECONDS)) == NULL) return result;
   firmware_loaded = false;
   timeout->Start();
   while (!firmware_loaded && !timeout->Timeout())
   {
       ST_UpdateStatus();
       if (gb_shared.cpu1.flags.BIT.MINI_BASE_LOADED == 1)
           firmware_loaded = true;
       else Delay(500, MILISECONDS);
   }

    if (firmware_loaded)
    {
        LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE (File)... OK");
        return ETCP_NONE;
    }
    else
    {
        ST_UpdateStatus();
        {
            if (gb_shared.cpu1.flags.BIT.MINI_BASE_LOADED == 1)
            {
                LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE (File)... OK");
                return ETCP_NONE;
            }
            else
            {
                LOGProtocol("\r\nCMD_FP_LOAD_FIRMWARE (File)... Error");
                return ELOG(charETCP_CONNECT_TIMEOUT, ETCP_CONNECT_TIMEOUT);
            }
        }
    }
}
__finally
{
   if (ptr_msg != NULL) free(ptr_msg);
   if (timeout != NULL) delete timeout;
}
   return ETCP_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool TCPSend_MSG_SHARED_MEM(void)
{
    u32 comand[6];
    int result, n_data_recv, idx_data_recv = 0;
    CTimeout* timeout = NULL;

    if (TCP_Connected() == 0) return false;
    if (!gb_no_hw)
    {
        comand[0] = CMD_HEADER;
        comand[1] = 16;
        comand[2] = GET_SHARED_MEM;
        comand[3] = ++gb_id_frame_rd;

        try
        {
            if (gb_plock_tcp_cfg != NULL && !gb_plock_tcp_cfg->Acquire(CS_WR))
            {
                ELOG(charES2_Acquire, ES2_Acquire);
                return false;
            }

            if (!gb_no_hw && (result = send(uci_tcp_cfg, (const char*)comand, comand[1], 0)) < 0)
            {
                LOGProtocol("\r\nGET_SHARED_MEM... ERROR");
                return false;
            }
            LOGProtocol("\r\nGET_SHARED_MEM... OK");

            if ((timeout = new CTimeout(5000, MILISECONDS)) == NULL) return false;
            timeout->Start();

            idx_data_recv = 0;
            while (idx_data_recv < MEM_FRAME_SIZE && !timeout->Timeout())
            {
                if ((n_data_recv = TCP_Receive_ConfigPort(&gb_shared_mem[idx_data_recv], MEM_FRAME_SIZE)) < 0)
                    return false;
                idx_data_recv += n_data_recv;
            }
            if (timeout->Timeout())
                n_data_recv = TCP_Receive_ConfigPort(&gb_shared_mem[0], MEM_FRAME_SIZE);
        }
        __finally
        {
            if (timeout != NULL) delete timeout;
            if (gb_plock_tcp_cfg != NULL) gb_plock_tcp_cfg->Release();
        }

        if (idx_data_recv > LAST_SHRD_KB_SIZE)
        {
            SHRD_Update(&gb_shared_mem[0]);
            return true;
        }
    }
    return false;
}
