//---------------------------------------------------------------------------
#include "st_info.h"
#include "st_reg.h"
#include "stlib2.h"
#include "stlib_dev.h"
//#include "c_sitau_hw.h"
#include "protocol_uci.h"
#include "st_error_code.h"
#include "c_xml_sitau_hw.h"

//---------------------------------------------------------------------------

vector<TInfoChannel> gb_element_to_channel;
vector<unsigned short> gb_pin_to_channel;

//=============================================================================
// FUNCIONES EXCLUSIVAS PARA PHASED ARRAY
//=============================================================================


//=============================================================================
// FUNCIONES COMUNES PARA PHASED ARRAY Y MULTICANAL
//=============================================================================

// -----------------------------------------------------------------------------
/**
 Devuelve la versión de la librería \c stlib

 @param[out]   data
               Puntero a una cadena de caracteres donde se devuelve la versión.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.
*/
// -----------------------------------------------------------------------------
void ST_InfoLibraryVersion (char *data)
{
	strcpy(data,_VERSION_DLL_MAJOR);
	strcat(data,".");
	strcat(data,_VERSION_DLL_MINOR);
	strcat(data,".");
	strcat(data,_VERSION_DLL_RELEASE);
	strcat(data,".");
	strcat(data,_VERSION_DLL_BUILD);
	strcat(data," ");
	strcat(data,_VERSION_DLL_DATE);
}
// -----------------------------------------------------------------------------
/**
 Devuelve la versión del firmware del sistema SITAU.

 @param[out]   firmware_boot
               Puntero a una cadena de caracteres donde se devuelve la versión
               del dispositivo de arranque.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @param[out]   firmware_ethernet
               Puntero a una cadena de caracteres donde se devuelve la versión
               del dispositivo que gestiona las comunicaciones.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @param[out]   firmware_uci_sw
               Puntero a una cadena de caracteres donde se devuelve la versión
               del software de la unidad de control.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @param[out]   firmware_uci_hw
               Puntero a una cadena de caracteres donde se devuelve la versión
               del hardware de la unidad de control.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @param[out]   firmware_base
               Puntero a una cadena de caracteres donde se devuelve la versión
               de la BASE 32Ch.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @param[out]   firmware_module
               Puntero a una cadena de caracteres donde se devuelve la versión
               del MODULO 8Ch.\n
               La memoria que es necesario reservar para este puntero es de 255 caracteres.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int ST_InfoFirmwareVersion (char *firmware_ethernet, char *firmware_uci_sw, char *firmware_uci_hw, char *firmware_module)
{
int result;
TMSG_Version version;

#if defined(_GETHERNET)
   if (!gb_no_hw)
   {
       if ((result = ST_UpdateStatus()) < 0) return RLOG(result);
       version.cpu0.branch = gb_shared.sw_version_cpu0.branch_sw;
       version.cpu0.major = gb_shared.sw_version_cpu0.major_sw;
       version.cpu0.minor = gb_shared.sw_version_cpu0.minor_sw;
       strcpy(version.cpu0.date, gb_shared.sw_version_cpu0.date);
       strcpy(version.cpu0.device, gb_shared.sw_version_cpu0.description);

       version.cpu1.branch = gb_shared.sw_version_cpu1.branch_sw;
       version.cpu1.major = gb_shared.sw_version_cpu1.major_sw;
       version.cpu1.minor = gb_shared.sw_version_cpu1.minor_sw;
       strcpy(version.cpu1.date, gb_shared.sw_version_cpu1.date);
       strcpy(version.cpu1.device, gb_shared.sw_version_cpu1.description);

       version.hw_uci = gb_shared.hw_version;
       version.hw_mk32 = gb_shared.mk32_version;
//       //if ((result = TCPSend_MSG_VERSION (&version)) < 0) return RLOG(result);
   }
   else
   {   
      version.cpu0.branch = 0;
      version.cpu0.major = 0;
      version.cpu0.minor = 0;
      strcpy(version.cpu0.date, "00/00/0000");
      strcpy(version.cpu0.device, "NONE");

      version.cpu1.branch = 0;
      version.cpu1.major = 0;
      version.cpu1.minor = 0;
      strcpy(version.cpu1.date, "00/00/0000");
      strcpy(version.cpu1.device, "NONE");
   }
#else
   version.fslb.branch = 0;
   version.fslb.major = 0;
   version.fslb.minor = 0;
   strcpy(version.fslb.date, "00/00/0000");
   strcpy(version.fslb.device, "NONE");

   version.cpu0.branch = 0;
   version.cpu0.major = 0;
   version.cpu0.minor = 0;
   strcpy(version.cpu0.date, "00/00/0000");
   strcpy(version.cpu0.device, "NONE");

   version.cpu1.branch = 0;
   version.cpu1.major = 0;
   version.cpu1.minor = 0;
   strcpy(version.cpu1.date, "00/00/0000");
   strcpy(version.cpu1.device, "NONE");
#endif

   if (firmware_ethernet != NULL)
   {
      sprintf(firmware_ethernet, "%d.%d.%d (%s)",
         version.cpu0.branch,
         version.cpu0.major,
         version.cpu0.minor,
         version.cpu0.date);
   }
   if (firmware_uci_sw != NULL)
   {
      sprintf(firmware_uci_sw, "%d.%d.%d (%s)",
         version.cpu1.branch,
         version.cpu1.major,
         version.cpu1.minor,
         version.cpu1.date);
   }
   if (firmware_uci_hw != NULL)
   {
       sprintf(firmware_uci_hw, "%d (%x/%x/%x)", version.hw_uci & 0xFF, (version.hw_uci >> 24) & 0xFF, (version.hw_uci >> 16) & 0xFF, (version.hw_uci >> 8) & 0xFF);
   }
   if (firmware_module != NULL)
   {
       sprintf(firmware_module, "%d (%x/%x/%x)", version.hw_mk32 & 0xFF, (version.hw_mk32 >> 24) & 0xFF, (version.hw_mk32 >> 16) & 0xFF, (version.hw_mk32 >> 8) & 0xFF);
   }
   return ES2_NONE;
}
/* ----------------------------------------------------------------------
ST_InfoSWGatesNumber
====================
	Devuelve el número de puertas hardware de las que dispone cada base.
---------------------------------------------------------------------- */
int ST_InfoSWGatesNumber (void) {return SW_NUM_GATES;}

/* ----------------------------------------------------------------------
ST_InfoEncodersNumber
=====================
	Devuelve el número de encoders configurados en el equipo mediante la
aplicación SITAUConfig.
---------------------------------------------------------------------- */
unsigned short ST_InfoEncodersNumber (void)
{
   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetEncodersNumber();
	return 0;
}

