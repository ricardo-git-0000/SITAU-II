//---------------------------------------------------------------------------

#include "sitau_hw.h"
#include "st_reg.h"
#include "sitau_error_code.h"
#include "protocol_uci.h"
#include <time.h>
#include "stlib2.h"
#include "c_xml_sitau_hw.h"
#include "t_sitau_hw.h"

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void STHW_DeleteXMLData (void)
{
   if (gb_xml_sitau_hw != NULL) delete gb_xml_sitau_hw;
   gb_xml_sitau_hw = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_CreateXMLData (char *path, char *file)
{
AnsiString str;
char str_error[255];
System::String ^file_string = nullptr;

    if (file == NULL) str = AnsiString(path) + AnsiString(FILENAME_CONFIG_SYS_XML);
	else str = AnsiString(path) + AnsiString(file);

    file_string = gcnew System::String(str.c_str());
    if (!System::IO::File::Exists(file_string))
    {
        _snprintf_s(str_error, sizeof(str_error), _TRUNCATE, "Error, '%s' doesn't exist", str.c_str());
        return ELOG(str_error, EHW_File);
    }
    if (gb_xml_sitau_hw != NULL) delete gb_xml_sitau_hw;
    if ((gb_xml_sitau_hw = new CXML_SITAU_HW(str)) == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return EHW_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASystemNumber(void)
{
   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetPASystemNumber();
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem *STHW_GetPtrPASystem(int id_pa_sys)
{
CPASystem *result = NULL;

   if (gb_xml_sitau_hw != NULL) result = gb_xml_sitau_hw->GetPtrPASystem(id_pa_sys);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASystemName(char *name, int id_pa_sys)
{
   CPASystem* pa_sys = NULL;
   AnsiString str;

   if (gb_xml_sitau_hw != NULL && (pa_sys = gb_xml_sitau_hw->GetPtrPASystem(id_pa_sys)) != NULL)
   {
      str = pa_sys->GetName();
      strcpy(name, str.c_str());
   }
   return EHW_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysConnectorsNumber(int id_pa_sys)
{
   CPASystem* pa_sys = NULL;
   int result = 0;

   if (gb_xml_sitau_hw != NULL && (pa_sys = gb_xml_sitau_hw->GetPtrPASystem(id_pa_sys)) != NULL)
      result = pa_sys->GetConnectorsNumber();
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysConnectorName(char *name, int id_connector, int id_pa_sys)
{
   CPASystem* pa_sys = NULL;
   int result = 0;
   AnsiString str = "";

   if (gb_xml_sitau_hw != NULL && (pa_sys = gb_xml_sitau_hw->GetPtrPASystem(id_pa_sys)) != NULL)
      str = pa_sys->GetConnectorName(id_connector);
   strcpy(name, str.c_str());
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysConnectorChannelsNumber(int id_connector, int id_pa_sys)
{
   CPASystem* pa_sys = NULL;
   int result = 0;

   if (gb_xml_sitau_hw != NULL && (pa_sys = gb_xml_sitau_hw->GetPtrPASystem(id_pa_sys)) != NULL)
      result = pa_sys->GetConnectorChannelsNumber(id_connector);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CPASystem *STHW_GetPtrPASystem_BASEFirst (int id_base_first)
{
CPASystem *result = NULL;

   if (gb_xml_sitau_hw != NULL) result = gb_xml_sitau_hw->GetPtrPASystem_BASEFirst(id_base_first);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysChannelsNumber (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysChannelsNumber(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysActiveChannelsNumber (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysActiveChannelsNumber(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysMultiplexedChannelsNumber (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysMultiplexedChannelsNumber(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetSITAUActiveChannelsNumber (void)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetSITAUActiveChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetSITAUMultiplexedChannelsNumber (void)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetSITAUMultiplexedChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetSITAUChannelsNumber (void)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetSITAUChannelsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetSITAUConnectorsNumber (void)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetSITAUConnectorsNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetHWBASENumber (void)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetBasesNumber();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysBASENumber (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysBasesNumber(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetConnectorStartChannel (int id_pa_sys, int id_conector)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetConnectorStartChannel(id_pa_sys, id_conector);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysFirstBASE (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysFirstBASE(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetPASysLastBASE (int id_pa_sys)
{
   if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_NoXMLData, EHW_NoXMLData);
   return gb_xml_sitau_hw->GetPASysLastBASE(id_pa_sys);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString STHW_GetEncoderName(int id_encoder)
{
   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetEncoderName(id_encoder);
   else return "";
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetEncodersNumber(void)
{
   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetEncodersNumber();
   else return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STHW_GetBeamformersNumber(void)
{
   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetBeamformersNumber();
   else return 1;
}
//// -----------------------------------------------------------------------------
///**
//
//*/
//// -----------------------------------------------------------------------------
//AnsiString STHW_GetSerialNumber(void)
//{
//   if (gb_xml_sitau_hw != NULL) return gb_xml_sitau_hw->GetSerialNumber();
//   else return "";
//}
// _____________________________________________________________________________
// ------ XML Hardware Config
// =============================================================================

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetConfig_StatusPeriod(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.user_config.status_period_ms;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetConfig_StatusEnabled(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.user_config.status_enabled;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetBuffer_PCIeUserBufferBurstsNumber(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.buffer.pcie_user_buffer_burst_number;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetBuffer_PCIeDMABlocksNumber(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.buffer.pcie_dma_blocks_number;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetBuffer_PCIeFIFOPointersNumber(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.buffer.pcie_fifo_pointers_number;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetDataLogger_PCIeUserBuffer(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.data_logger.pcie_user_buffer;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_XML_GetDataLogger_Enabled(void)
{
    if (gb_xml_sitau_hw == NULL) return ELOG(charEHW_PtrAccess_NULL, EHW_PtrAccess_NULL);
    return gb_xml_sitau_hw->data_.data_logger.enabled;
}
