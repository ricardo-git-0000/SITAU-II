// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <string>  
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "st_config.h"
#include "utils.h"
#include "st_get.h"
#include "st_error_code.h"
#include "c_log_file.h"
#include "st_calc.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_vch_config.h"
#include "st_vch_get.h"
#include "st_vch_config.h"
#include "st_vch_set.h"
#include "protocol_uci.h"
#include "fileformat.h"
#include "c_virtual_channel.h"
#include "c_datalogger.h"
#include "c_xml_config.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

//Estructuras de memoria temporales para la carga de parámetros desde fichero:
bool gb_cfg_change_info = false;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_DecoFileVersion (   AnsiString version_file,
                           unsigned short *field_1,
                           unsigned short *field_2,
                           unsigned short *field_3,
                           unsigned short *field_4)
{
int idx;
AnsiString str;
#if defined(_MSC_VER)
int n_data = (int)version_file.length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.insert(str.length(), 1, version_file[idx]);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#else
int n_data = version_file.Length();

   for (str="", idx=1; idx<=n_data && version_file[idx] != '-'; idx++);

   for (str="", idx=1; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_1 != NULL && str != "") *field_1 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_2 != NULL && str != "") *field_2 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_3 != NULL && str != "") *field_3 = atoi(str.c_str());

   for (idx++, str=""; idx<=n_data && version_file[idx] != '.'; idx++)
      str.Insert(version_file[idx], str.Length()+1);
   if (field_4 != NULL && str != "") *field_4 = atoi(str.c_str());
#endif
   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ST_UpdateConfig (void)
{
int i;

   for (i=0; i<gb_virtual_channel.size(); i++)
      gb_virtual_channel[i]->UpdateConfig();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_SetConfig (void)
{
	int i, id_channel, result;
	CVirtualChannel* virtual_channel_node = NULL;

	if (gb_vch_load_config_update && gb_vch_load_config.size() > 0)
	{
		gb_vch_load_config_update = false;
		ST_VirtualChannelsDestroy();

		for (id_channel = i = 0; i < gb_vch_load_config.size(); i++)
		{
			if (gb_vch_load_config[i] != NULL)
			{
				virtual_channel_node = new CVirtualChannel(id_channel, gb_vch_load_config[i]->sys_.id_pa_sys, gb_vch_load_config[i]->sys_.id_connector, gb_vch_load_config[i]->cfg_.name, gb_hw_format);
				gb_virtual_channel.push_back(virtual_channel_node);
				if ((result = virtual_channel_node->CONFIG_Copy(gb_vch_load_config[i])) < 0) return RLOG(result);
				id_channel++;
			}
		}
	}
	for (i = 0; i < gb_vch_load_config.size(); i++)
	{
		if (gb_vch_load_config[i] != NULL) delete gb_vch_load_config[i];
		gb_vch_load_config[i] = NULL;
	}
	if (gb_vch_load_config.size() > 0) gb_vch_load_config.clear();
	if ((result = ST_VirtualChannel_Set()) < 0) return RLOG(result);
	if (STDL_GetConfig())
	{
		ST_PrintFileConfig("CONFIG_ACQ.TXT", 1);
	}
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_LoadXMLFileConfigData(TXMLCFG_Config* xml_cfg)
{
	TXMLCFG_VirtualChannel* xml_vch = NULL;
	int i, n_vch, result;
	CVirtualChannel* vch = NULL;

	if (xml_cfg == NULL) return ELOG(charES2_FILE, ES2_FILE);

	for (i = 0; i < gb_vch_load_config.size(); i++)
	{
		if (gb_vch_load_config[i] != NULL) delete gb_vch_load_config[i];
		gb_vch_load_config[i] = NULL;
	}
	if (gb_vch_load_config.size() > 0) gb_vch_load_config.clear();

	UCIP.acquisition_mode = xml_cfg->acq.acquisition_mode.Value();
	UCIP.trigger_source = xml_cfg->acq.trigger_source.Value();
	UCIP.user_pause_after_burst = xml_cfg->acq.pause_after_burst.Value();

	ST_SetDelaySync(xml_cfg->acq.delay_sync_time.Value());
	ST_SetAcquisitionMode(xml_cfg->acq.acquisition_mode.Value());

	gb_fus.central_frequency = xml_cfg->fus.central_frequency.Value();
	gb_fus.rapport_focal_aperture = xml_cfg->fus.rapport_focal_aperture.Value();
	gb_fus.apodization_coefficient = xml_cfg->fus.apodization_coefficient.Value();
	gb_fus.lateral_position_x = xml_cfg->fus.lateral_position_x.Value();
	gb_fus.pixel_size_x = xml_cfg->fus.pixel_size_x.Value();
	gb_fus.pixels_x = xml_cfg->fus.pixels_x.Value();
	gb_fus.depth_z = xml_cfg->fus.depth_z.Value();
	gb_fus.pixels_z = xml_cfg->fus.pixels_z.Value();
	gb_fus.pixel_size_z = xml_cfg->fus.pixel_size_z.Value();
	gb_fus.single_plane = xml_cfg->fus.single_plane.Value();
	gb_fus.samples_wavelength = xml_cfg->fus.samples_wavelength.Value();
	gb_fus.interpolation_coefficients = xml_cfg->fus.interpolation_coefficients.Value();

	if ((n_vch = gb_xml_file->GetVirtualChannelNumber()) > 0)
	{
		for (i = 0; i < n_vch; i++)
		{
			if ((xml_vch = gb_xml_file->GetVCHConfig(i)) != NULL)
			{
				if ((vch = new CVirtualChannel(0, 0, 0, "Default", gb_hw_format)) != NULL)
				{
					if ((result = vch->LoadXMLConfigData(xml_vch, xml_cfg->acq.n_bursts.Value(), xml_cfg->acq.n_images_per_burst.Value(), i)) < 0) { delete vch; return RLOG(result); } // Libera el canal rechazado y propaga el codigo original.
					gb_vch_load_config.push_back(vch);
					gb_vch_load_config_update = true;
				}
			}
		}
	}
	else return ELOG(charES2_FILE, ES2_FILE);

	if ((result = ST_SetConfig()) < 0) return RLOG(result);

	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Carga la configuración guardada en un archivo .PAC

 @param[in]    file
               Nombre, junto con la ruta, del archivo de configuración .PAC

 @return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ST_LoadXMLFileConfig (char *file)
{
	TXMLCFG_Config* xml_cfg = NULL;
	int result;

	if (!FS_FileReadAccess(file)) return ELOG(charES2_FILE, ES2_FILE);
	if ((gb_xml_file = new CXML_CONFIG(AnsiString(file))) != NULL && (xml_cfg = gb_xml_file->GetConfig()) != NULL)
	{
		if ((result = ST_LoadXMLFileConfigData(xml_cfg)) < 0) return RLOG(result);
	}
	else return ELOG(charES2_FILE, ES2_FILE);
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Carga la configuración guardada en un archivo .PAC

 @param[in]    file
			   Nombre, junto con la ruta, del archivo de configuración .PAC

 @return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ST_LoadXMLBufferConfig(vector<char> &file_buffer)
{
	TXMLCFG_Config* xml_cfg = NULL;
	int result;

	if (file_buffer.size() == 0) return ELOG(charES2_FILE, ES2_FILE);

	if ((gb_xml_file = new CXML_CONFIG(file_buffer)) != NULL && (xml_cfg = gb_xml_file->GetConfig()) != NULL)
	{
		if ((result = ST_LoadXMLFileConfigData(xml_cfg)) < 0) return RLOG(result);
	}
	else return ELOG(charES2_FILE, ES2_FILE);

	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**
 Carga la configuración guardada en un archivo .PAC

 @param[in]    file
               Nombre, junto con la ruta, del archivo de configuración .PAC

 @return Ver códigos de error
*/
// -----------------------------------------------------------------------------
int ST_SaveXMLFileConfig (char *file)
{
int i, result;
CXML_CONFIG *xml_file = NULL;
TXMLCFG_VirtualChannel vch_struct;
TXMLCFG_Config cfg, *ptr_cfg = NULL;

	cfg.acq.acquisition_mode.Set(UCIP.acquisition_mode);
	cfg.acq.delay_sync_time.Set(UCIP.delay_sync_time);
	cfg.acq.trigger_source.Set(UCIP.trigger_source);
	cfg.acq.pause_after_burst.Set(UCIP.user_pause_after_burst);
	cfg.acq.n_bursts.Set(UCIP.n_bursts);
	cfg.acq.n_images_per_burst.Set(UCIP.n_images_burst);

	cfg.fus.central_frequency.Set(gb_fus.central_frequency);
	cfg.fus.rapport_focal_aperture.Set(gb_fus.central_frequency);
	cfg.fus.apodization_coefficient.Set(gb_fus.central_frequency);
	cfg.fus.lateral_position_x.Set(gb_fus.central_frequency);
	cfg.fus.pixel_size_x.Set(gb_fus.central_frequency);
	cfg.fus.pixels_x.Set(gb_fus.central_frequency);
	cfg.fus.depth_z.Set(gb_fus.central_frequency);
	cfg.fus.pixels_z.Set(gb_fus.central_frequency);
	cfg.fus.pixel_size_z.Set(gb_fus.central_frequency);
	cfg.fus.single_plane.Set(gb_fus.central_frequency);
	cfg.fus.samples_wavelength.Set(gb_fus.central_frequency);
	cfg.fus.interpolation_coefficients.Set(gb_fus.central_frequency);

	for (i=0; i<gb_virtual_channel.size(); i++)
	{
		if ((result = gb_virtual_channel[i]->GetXMLConfigData(&vch_struct)) < 0) return RLOG(result);
		cfg.vch.push_back(vch_struct);
	}

	ptr_cfg = &cfg;
	if (gb_xml_file != NULL)
	{
		if ((result = gb_xml_file->Copy(&cfg)) < 0) return RLOG(result);
		ptr_cfg = gb_xml_file->GetConfig();
	}
	if ((xml_file = new CXML_CONFIG(ptr_cfg)) != NULL)
	{
		if ((result = xml_file->CreateXMLFile(AnsiString(file))) < 0) return RLOG(result);
	}
	else return ELOG(charES2_PtrAccess_NULL, ES2_PtrAccess_NULL);
	return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int ST_PrintFileConfig(char *file)
//{
//int result;
//
//   if ((result = ST_VCH_PrintFileConfig(file)) < 0) return RLOG(result);
//   return ES2_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PrintFileConfig(char* file, int hardware_config)
{
	int i, data_int;
	FILE* f = NULL;
	time_t t;
	char date[255];

	if ((f = fopen(file, "wt")) == NULL) return ELOG(charES2_FILE, ES2_FILE);

	time(&t);
	strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", localtime(&t));
	fprintf(f, "[%s] ---- ACQUISITION CONFIGURATION ----", date);
	
	fprintf(f, "\n____________________________________________________________________________________________________");
	fprintf(f, "\n--> User Configuration");
	fprintf(f, "\n    ------------------------------------------------------------------------------------------------");
	fprintf(f, "\n    Status Enabled:\t");
	if ((data_int = ST_XML_GetConfig_StatusEnabled()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    Status Period:\t");
	if ((data_int = ST_XML_GetConfig_StatusPeriod()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d ms", data_int);
	fprintf(f, "\n____________________________________________________________________________________________________");
	fprintf(f, "\n--> PCIe Configuration");
	fprintf(f, "\n    ------------------------------------------------------------------------------------------------");
	fprintf(f, "\n    PCIe User Buffer (Bursts Number):\t");
	if ((data_int = ST_XML_GetBuffer_PCIeUserBufferBurstsNumber()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    PCIe FIFO Pointers Number:\t\t\t");
	if ((data_int = ST_XML_GetBuffer_PCIeFIFOPointersNumber()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    PCIe DMA Blocks Number:\t\t\t\t");
	if ((data_int = ST_XML_GetBuffer_PCIeDMABlocksNumber()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    PCIe Blocks Burst Number:\t\t\t");
	if ((data_int = ST_PCIe_GetBlocksBurstNumber()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    PCIe Focal Laws Blocks Number:\t\t");
	if ((data_int = ST_PCIe_GetFocalLawsBlockNumber()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);

	fprintf(f, "\n\n____________________________________________________________________________________________________");
	fprintf(f, "\n--> Data Logger Configuration");
	fprintf(f, "\n    ------------------------------------------------------------------------------------------------");
	fprintf(f, "\n    Enabled:\t\t\t");
	if ((data_int = ST_XML_GetDataLogger_Enabled()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);
	fprintf(f, "\n    PCIe User Buffer:\t");
	if ((data_int = ST_XML_GetDataLogger_PCIeUserBuffer()) < 0) fprintf(f, "NULL");
	else fprintf(f, "%d", data_int);

	for (i = 0; i < gb_virtual_channel.size(); i++)
		if (gb_virtual_channel[i] != NULL) gb_virtual_channel[i]->PrintFileConfig(f, hardware_config == 1);
	fclose(f);
	return ES2_NONE;
}
