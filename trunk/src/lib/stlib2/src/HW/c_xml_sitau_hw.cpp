// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <time.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_xml_sitau_hw.h"
#include "c_log_file.h"
#include "xml_err.h"
#include "sitau_error_code.h"
#include "fileformat.h"
#include "tad.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

CXML_SITAU_HW* gb_xml_sitau_hw = NULL;

// _____________________________________________________________________________
// ------ CXML_SITAU_HW class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML_SITAU_HW::CXML_SITAU_HW(AnsiString file_name)
	   : CXML()
{
    error_ = EXML_NONE;
    file_name_ = file_name;
    error_ = Update();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::Update(void)
{
int result = EXML_NONE, i, j, k, n_pa_sys, id_frontend = 0, n_bases, id = 0;
bool is_frontend = false, multiplexed = false;

    ResetData();

    if (FS_FileReadAccess((char *)file_name_.c_str()))
    {
        CXML::LoadFile(file_name_);
        GetData_AFE();
        GetData_PULSER();
        GetData_BEAMFORMER();
        if ((result = GetData_HardwareConfiguration()) < 0) return RLOG(result);
        GetData_FRONTEND();
        GetData_UCI();
        GetData_PASystemConfiguration();
        GetData_DataLogger();
        GetData_Buffer();
        GetData_UserConfig();

        if (data_.pa_system.size() == 0) data_.pa_system.resize(1);
        for (i = 0; i < data_.pa_system.size(); i++)
        {
            data_.pa_system[i].id_base.resize(data_.sitau_hardware.base.size());
            for (j = 0; j < data_.pa_system[i].id_base.size(); j++)
                data_.pa_system[i].id_base[j] = j;
            data_.pa_system[i].id_base_first = 0;
        }

        //if (data_.sitau_hardware.base.size() == 0) data_.sitau_hardware.base.resize(4);
        if (data_.sitau_hardware.frontend.size() == 0)
        {
            // En modo paralelo, cada base FPGA aporta MOD_MAX_CH canales.
            int n_total_channels = data_.sitau_hardware.bases_number * MOD_MAX_CH;

            data_.sitau_hardware.frontend.resize(1);
            // n_active_channels representa los canales de una base; el total se calcula con n_total_channels.
            data_.sitau_hardware.frontend[0].n_active_channels = MOD_MAX_CH;
            // Un valor cero identifica el modo paralelo; el multiplexado se tratará en una fase posterior.
            data_.sitau_hardware.frontend[0].n_multiplexed_channels = 0;
            data_.sitau_hardware.frontend[0].n_channels = n_total_channels;
            data_.sitau_hardware.frontend[0].connector.resize(1);
            // El frontend debe referenciar exactamente las bases indicadas por BASES_NUMBER.
            data_.sitau_hardware.frontend[0].id_base.resize(data_.sitau_hardware.bases_number);
            for (j = 0; j < data_.sitau_hardware.frontend[0].id_base.size(); j++)
                data_.sitau_hardware.frontend[0].id_base[j] = j;
            // El conector y sus mapas deben cubrir todos los canales de las bases seleccionadas.
            data_.sitau_hardware.frontend[0].connector[0].n_total_channels = n_total_channels;
            data_.sitau_hardware.frontend[0].connector[0].pin_to_channel.resize(n_total_channels);
            data_.sitau_hardware.frontend[0].connector[0].pin_disabled.resize(n_total_channels);
            for (j = 0; j < n_total_channels; j++)
                data_.sitau_hardware.frontend[0].connector[0].pin_to_channel[j] = j;
        }

        if (pa_system_.size() != data_.pa_system.size()) pa_system_.resize(data_.pa_system.size());
        pa_system_[0] = new CPASystem(
            "Default", 
            0, 
            0, 
            (int)data_.sitau_hardware.base.size(), 
            data_.sitau_hardware.frontend[0].n_multiplexed_channels, 
            data_.sitau_hardware.frontend[0].connector[0].start_channel, 
            &data_.sitau_hardware.frontend[0]);

        if ((n_pa_sys = (int)data_.pa_system.size()) > 0)
        {
            pa_system_.resize(n_pa_sys);
            for (i = 0; i < n_pa_sys; i++)
            {
                is_frontend = false;
                for (j = 0; !is_frontend && j < data_.sitau_hardware.frontend.size(); j++)
                {
                    if (data_.pa_system[i].id_base.size() == data_.sitau_hardware.frontend[j].id_base.size())
                    {
                        is_frontend = data_.sitau_hardware.frontend.size() > 0 ? true : false;
                        for (k = 0; k < data_.pa_system[i].id_base.size(); k++)
                        {
                            if (data_.pa_system[i].id_base[k] != data_.sitau_hardware.frontend[j].id_base[k])
                            is_frontend = false;
                        }
                    }
                    if (is_frontend) id_frontend = j;
                }

                data_.pa_system[i].id_frontend = is_frontend ? id_frontend : -1;
                if (is_frontend)
                {
                    if (id_frontend < data_.sitau_hardware.frontend.size())
                        multiplexed = data_.sitau_hardware.frontend[id_frontend].n_multiplexed_channels > 0 ? true : false;
                    else result = ELOG(charEHW_XML_Format, EHW_XML_Format);
                }
                else
                {
                    multiplexed = false;
                    result = ELOG(charEHW_XML_Format, EHW_XML_Format);
                }

                if ((n_bases = data_.pa_system[i].id_base.size()) == 0)
                {
                    result = ELOG(charEHW_XML_Format, EHW_XML_Format);
                }
                if (id_frontend < data_.sitau_hardware.frontend.size() && (int)data_.sitau_hardware.frontend[id_frontend].connector.size() > 0 && data_.pa_system[i].id_base.size() > 0)
                    pa_system_[i] = new CPASystem(data_.pa_system[i].name,
                        i,
                        data_.pa_system[i].id_base.front(),
                        (int)data_.pa_system[i].id_base.size(),
                        data_.sitau_hardware.frontend[id_frontend].n_multiplexed_channels,
                        data_.sitau_hardware.frontend[id_frontend].connector[0].start_channel,
                        &data_.sitau_hardware.frontend[id_frontend]);
                else result = ELOG(charEHW_XML_Format, EHW_XML_Format);
            }
        }
        else
        {
            result = ELOG(charEHW_XML_Format, EHW_XML_Format);
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_PULSER_CONTROL(pugi::xml_node node_data, TXMLHW_PULSER_CONTROL* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eCONTROL) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameGND0_HIZ1, &data->gnd0_hiz1)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePULSER_LOAD, &data->pulser_load)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePULSER_AUTO_LOAD, &data->pulser_auto_load)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePULSER_MODE, &data->pulser_current)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePULSER_STATUS, &data->pulser_status)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameSTOP, &data->stop)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameEXTERNAL_TRIGGER, &data->external_trigger)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameNOT_BUSY, &data->not_busy)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameSOFTWARE_TRIGGER, &data->software_trigger)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_PULSER_DELAY_MEM(pugi::xml_node node_data, TXMLHW_DELAY_MEM* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eDELAY_MEM) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameDELAY_MEM_INI, &data->delay_mem_ini)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameDELAY_MEM_END, &data->delay_mem_end)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_PULSER(void)
{
    pugi::xml_node node, nodes;
    int result;

    if ((nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eRegistersConfiguration).child(XMLHW_ePULSER)) != NULL)
    {
        for (node = nodes.first_child(); node; node = node.next_sibling())
        {
            if (GetAttribute(&node.first_attribute(), XMLHW_pNamePULSER_MAIN_DELAY, &data_.registers.pulser.main_delay)) {}
            else if (GetAttribute(&node.first_attribute(), XMLHW_pNameDELAY_MEM_PTR, &data_.registers.pulser.delay_mem_ptr)) {}
        }
        if ((result = GetData_PULSER_CONTROL(nodes.child(XMLHW_eCONTROL), &data_.registers.pulser.control)) < 0) return RLOG(result);
        if ((result = GetData_PULSER_DELAY_MEM(nodes.child(XMLHW_eDELAY_MEM), &data_.registers.pulser.delay_mem_ini_end)) < 0) return RLOG(result);
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_BUSSAR(pugi::xml_node node_data, TXMLHW_AFE_BUSSAR* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eBUSSAR) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameEXTERNAL_TRIGGER, &data->external_trigger)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG21(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG21* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG21) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameDIGITAL_HPF_FILTER_ENABLE_1_4, &data->DIGITAL_HPF_FILTER_ENABLE_1_4)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameDIGITAL_HPF_FILTER_K_CH_1_4, &data->DIGITAL_HPF_FILTER_K_CH_1_4)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG33(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG33* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG33) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameDIGITAL_HPF_FILTER_ENABLE_5_8, &data->DIGITAL_HPF_FILTER_ENABLE_5_8)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameDIGITAL_HPF_FILTER_K_CH_5_8, &data->DIGITAL_HPF_FILTER_K_CH_5_8)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG51(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG51* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG51) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameLPF_PROGRAMMABILITY, &data->LPF_PROGRAMMABILITY)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePGA_INTEGRATOR_DISABLE, &data->PGA_INTEGRATOR_DISABLE)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePGA_CLAMP_LEVEL, &data->PGA_CLAMP_LEVEL)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG52(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG52* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG52) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameACTIVE_TERMINATION, &data->ACTIVE_TERMINATION)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameACT_TER_ENA_CTRL, &data->ACT_TER_ENA_CTRL)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePRESET_ACTIVE_TERMINATIONS, &data->PRESET_ACTIVE_TERMINATIONS)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameACTIVE_TERMINATION_ENABLE, &data->ACTIVE_TERMINATION_ENABLE)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameLNA_INPUT_CLAMP_SETTING, &data->LNA_INPUT_CLAMP_SETTING)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameLNA_INTEGRATOR_DISABLE, &data->LNA_INTEGRATOR_DISABLE)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG59(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG59* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG59) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameHPF_LNA, &data->HPF_LNA)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameCW_SUM_AMP_PDN, &data->CW_SUM_AMP_PDN)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePGA_TEST_MODE, &data->PGA_TEST_MODE)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI_REG61(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG61* data)
{
    pugi::xml_node node;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eREG61) == 0)
    {
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLHW_eRegister) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLHW_pNameV2I_CLAMP, &data->V2I_CLAMP)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNameLPF_5MHz, &data->LPF_5MHz)) {}
                else if (GetAttribute(&node.first_attribute(), XMLHW_pNamePGA_CLAMP_MINUS6DBFS, &data->PGA_CLAMP_minus6dBFS)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE_SPI(pugi::xml_node node_data, TXMLHW_AFE_SPI* data)
{
    pugi::xml_node node;
    int result;

    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLHW_eSPI) == 0)
    {
        if ((result = GetData_AFE_SPI_REG21(node_data.child(XMLHW_eREG21), &data->reg21)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI_REG33(node_data.child(XMLHW_eREG33), &data->reg33)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI_REG51(node_data.child(XMLHW_eREG51), &data->reg51)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI_REG52(node_data.child(XMLHW_eREG52), &data->reg52)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI_REG59(node_data.child(XMLHW_eREG59), &data->reg59)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI_REG61(node_data.child(XMLHW_eREG61), &data->reg61)) < 0) return RLOG(result);
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_AFE(void)
{
    int result;
    pugi::xml_node node, nodes;
    bool flag;

    if ((nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eRegistersConfiguration).child(XMLHW_eAFE)) != NULL)
    {
        for (node = nodes.first_child(); node; node = node.next_sibling())
        {
            if (GetAttribute(&node.first_attribute(), XMLHW_pNameDELAY_OFFSET, &data_.registers.afe.delay_offset_))
                flag = true;
        }
        if ((result = GetData_AFE_BUSSAR(nodes.child(XMLHW_eBUSSAR), &data_.registers.afe.bussar)) < 0) return RLOG(result);
        if ((result = GetData_AFE_SPI(nodes.child(XMLHW_eSPI), &data_.registers.afe.spi)) < 0) return RLOG(result);
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_BEAMFORMER(void)
{
   pugi::xml_node node, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration).child(eSITAUHardware);

    for (node = nodes.child(eBEAMFORMER).first_child(); node; node = node.next_sibling())
    {
        if (GetAttribute(&node.first_attribute(), pNameBeamformer_Number_1, &data_.sitau_hardware.beamformer.number)) {}
        else if (GetAttribute(&node.first_attribute(), pNameBeamformer_Number_2, &data_.sitau_hardware.beamformer.number)) {}
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_HardwareConfiguration(void)
{
    int i, n_bases = XML_DEF_N_BASES;
    pugi::xml_node node, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration);

    for (node = nodes.first_child(); node; node = node.next_sibling())
    {
        if (GetAttribute(&node.first_attribute(), pNameBASES_NUMBER, &n_bases)) {}
    }
    // En esta fase solo se soportan configuraciones paralelas de 4 u 8 bases.
    if (n_bases != 4 && n_bases != 8) return ELOG(charEHW_BASE_Number, EHW_BASE_Number);
    // Conservamos el valor leído para que el resto de la configuración lo herede.
    data_.sitau_hardware.bases_number = n_bases;
    if (data_.sitau_hardware.base.size() != n_bases) data_.sitau_hardware.base.resize(n_bases);
    for (i = 0; i < data_.sitau_hardware.base.size(); i++)
    {
        data_.sitau_hardware.base[i].model = "BASE-32C";
        data_.sitau_hardware.base[i].version = "1";
        data_.sitau_hardware.base[i].serial_number = "";
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_Buffer(void)
{
    pugi::xml_node node, nodes = CXML::doc_.child(XMLHW_eSITAU);

    for (node = nodes.child(XMLHW_eBufferConfiguration).first_child(); node; node = node.next_sibling())
    {
        if (GetAttribute(&node.first_attribute(), pNamePCIE_USER_BUFER_BURST_NUMBER, &data_.buffer.pcie_user_buffer_burst_number)) {}
        else if (GetAttribute(&node.first_attribute(), pNamePCIE_DMA_BLOCKS_NUMBER, &data_.buffer.pcie_dma_blocks_number)) {}
        else if (GetAttribute(&node.first_attribute(), pNamePCIE_FIFO_POINTERS_NUMBER, &data_.buffer.pcie_fifo_pointers_number)) {}
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_UserConfig(void)
{
    pugi::xml_node node, nodes = CXML::doc_.child(XMLHW_eSITAU);

    for (node = nodes.child(XMLHW_eUserConfiguration).first_child(); node; node = node.next_sibling())
    {
        if (GetAttribute(&node.first_attribute(), pNameSTATUS_PERIOD_MS, &data_.user_config.status_period_ms)) {}
        else if (GetAttribute(&node.first_attribute(), pNameSTATUS_ENABLED, &data_.user_config.status_enabled)) {}
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_DataLogger(void)
{
    pugi::xml_node node, nodes = CXML::doc_.child(XMLHW_eSITAU);

    for (node = nodes.child(XMLHW_eDataloggerConfiguration).first_child(); node; node = node.next_sibling())
    {
        if (GetAttribute(&node.first_attribute(), pNameLOG_FILE_ENABLED, &data_.data_logger.enabled)) {}
        else if (GetAttribute(&node.first_attribute(), pNameLOG_FILE_PCIE_USER_BUFFER, &data_.data_logger.pcie_user_buffer)) {}
        else if (GetAttribute(&node.first_attribute(), pNameLOG_FILE_CONFIG, &data_.data_logger.config)) {}
        else if (GetAttribute(&node.first_attribute(), pNameLOG_FILE_PA_BEAMFORMER, &data_.data_logger.pa_beamformer)) {}
        else if (GetAttribute(&node.first_attribute(), pNameLOG_FILE_TFM_BEAMFORMER, &data_.data_logger.tfm_beamformer)) {}
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_PASystemConfiguration (void)
{
pugi::xml_node node, node_pa_system, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration).child(ePASystemConfiguration);
TXMLHW_PASystem pa_system_struct;

   data_.pa_system.clear();
   for (node_pa_system = nodes.child(ePASystem); node_pa_system != NULL; node_pa_system = node_pa_system.next_sibling())
   {
      if (strcmp(node_pa_system.name(), ePASystem) == 0)
      {
         for (node = node_pa_system.first_child(); node; node = node.next_sibling())
         {
            if (GetAttribute(&node.first_attribute(), pNameName, &pa_system_struct.name)) {}
            else if (GetAttribute(&node.first_attribute(), pNameActiveChannels, &pa_system_struct.n_active_channels)) {}
            else if (GetAttribute(&node.first_attribute(), pNameMultiplexedChannels, &pa_system_struct.n_multiplexed_channels)) {}
            else if (GetAttribute(&node.first_attribute(), pNameIdBASE, pa_system_struct.id_base)) {}
            else if (GetAttribute(&node.first_attribute(), pNameIdFRONTEND, &pa_system_struct.id_frontend)) {}
            else if (GetAttribute(&node.first_attribute(), pNameIdBASE, pa_system_struct.id_base)) {}
         }
         data_.pa_system.push_back(pa_system_struct);
      }
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//int CXML_SITAU_HW::GetData_BASE (void)
//{
//pugi::xml_node node, node_base, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration).child(eSITAUHardware);;
//TXMLHW_BASE base_struct;
//
//   data_.sitau_hardware.base.clear();
//   for (node_base = nodes.child(eBASE); node_base != NULL; node_base = node_base.next_sibling())
//   {
//      if (strcmp(node_base.name(), eBASE) == 0)
//      {
//         for (node = node_base.first_child(); node; node = node.next_sibling())
//         {
//            if (GetAttribute(&node.first_attribute(), pNameModel, &base_struct.model)) {}
//            else if (GetAttribute(&node.first_attribute(), pNameVersion, &base_struct.version)) {}
//            else if (GetAttribute(&node.first_attribute(), pNameSerialNumber, &base_struct.serial_number)) {}
//         }
//         data_.sitau_hardware.base.push_back(base_struct);
//      }
//   }
//   return EXML_NONE;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_FRONTEND_Connector (pugi::xml_node nodes, vector<TXMLHW_CONNECTOR> &connector)
{
pugi::xml_node node, node_connector;
TXMLHW_CONNECTOR connector_struct;
CXMLParameter < vector<float>> v_float;
CXMLParameter < vector<int>> v_int32;
CXMLParameter < vector<unsigned short>> v_uint16;

   connector.clear();
   for (node_connector = nodes.child(eFRONTEND_Connector); node_connector != NULL; node_connector = node_connector.next_sibling())
   {
      if (strcmp(node_connector.name(), eFRONTEND_Connector) == 0)
      {
         for (node = node_connector.first_child(); node; node = node.next_sibling())
         {
            if (GetAttribute(&node.first_attribute(), pNameName, &connector_struct.name)) {}
            else if (GetAttribute(&node.first_attribute(), pNameStartChannel, &connector_struct.start_channel)) {}
            else if (GetAttribute(&node.first_attribute(), pNameTotalChannels, &connector_struct.n_total_channels)) {}
         }
         connector.push_back(connector_struct);
      }
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/** TODO: Habria que actualizar estos comentarios para 256 canales
FRONTEND-V1 --> 32 canales
FRONTEND-MUX128-32 --> 32:128
FRONTEND-32-128X4 --> 32:512
FRONTEND-4X128-32TR_V2 --> 32:128(32:32x4) (Danorail)
FRONTEND_128x128 --> 128 canales
Frontend_32_HP -->
*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_FRONTEND (void)
{
pugi::xml_node node, node_frontend, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration).child(eSITAUHardware);
TXMLHW_FRONTEND frontend_struct;
int result;

   data_.sitau_hardware.frontend.clear();
   for (node_frontend = nodes.child(eFRONTEND); node_frontend != NULL; node_frontend = node_frontend.next_sibling())
   {
      if (strcmp(node_frontend.name(), eFRONTEND) == 0)
      {
         for (node = node_frontend.first_child(); node; node = node.next_sibling())
         {
            if (GetAttribute(&node.first_attribute(), pNameModel, &frontend_struct.model)) {}
            else if (GetAttribute(&node.first_attribute(), pNameVersion, &frontend_struct.version)) {}
            else if (GetAttribute(&node.first_attribute(), pNameSerialNumber, &frontend_struct.serial_number)) {}
            else if (GetAttribute(&node.first_attribute(), pNameActiveChannels, &frontend_struct.n_active_channels)) {}
            else if (GetAttribute(&node.first_attribute(), pNameMultiplexedChannels, &frontend_struct.n_multiplexed_channels)) {}
            else if (GetAttribute(&node.first_attribute(), pNameIdBASE, frontend_struct.id_base)) {}
         }
         if ((result = GetData_FRONTEND_Connector(node_frontend, frontend_struct.connector)) < 0) return RLOG(result);
         data_.sitau_hardware.frontend.push_back(frontend_struct);
      }
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_Encoder (pugi::xml_node nodes, vector<TXMLHW_Encoder> &encoder)
{
pugi::xml_node node, node_encoder;
TXMLHW_Encoder encoder_struct;

   encoder.clear();
   for (node_encoder = nodes.child(eEncoder); node_encoder != NULL; node_encoder = node_encoder.next_sibling())
   {
      if (strcmp(node_encoder.name(), eEncoder) == 0)
      {
         for (node = node_encoder.first_child(); node; node = node.next_sibling())
         {
            if (GetAttribute(&node.first_attribute(), pNameId, &encoder_struct.id)) {}
            else if (GetAttribute(&node.first_attribute(), pNameName, &encoder_struct.name)) {}
         }
         encoder.push_back(encoder_struct);
      }
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetData_UCI (void)
{
pugi::xml_node node, node_uci, nodes = CXML::doc_.child(XMLHW_eSITAU).child(XMLHW_eHardwareConfiguration).child(eSITAUHardware);
int result;

   if ((node_uci = nodes.child(eUCI)) != NULL)
   {
      for (node = node_uci.first_child(); node; node = node.next_sibling())
      {
         if (GetAttribute(&node.first_attribute(), pNameModel, &data_.sitau_hardware.uci.model)) {}
         else if (GetAttribute(&node.first_attribute(), pNameVersion, &data_.sitau_hardware.uci.version)) {}
         else if (GetAttribute(&node.first_attribute(), pNameSerialNumber, &data_.sitau_hardware.uci.serial_number)) {}
      }
      if ((result = GetData_Encoder(node_uci, data_.sitau_hardware.uci.encoder)) < 0) return RLOG(result);
      data_.sitau_hardware.uci.enabled = true;
   }
   else data_.sitau_hardware.uci.enabled = false;
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::ResetData(void)
{
   int i;

   data_.registers.afe.bussar.external_trigger.Set(0);
   data_.registers.afe.delay_offset_.Set(0);
   
   data_.registers.afe.spi.reg21.DIGITAL_HPF_FILTER_ENABLE_1_4.Set(0);
   data_.registers.afe.spi.reg21.DIGITAL_HPF_FILTER_K_CH_1_4.Set(0);
   
   data_.registers.afe.spi.reg33.DIGITAL_HPF_FILTER_ENABLE_5_8.Set(0);
   data_.registers.afe.spi.reg33.DIGITAL_HPF_FILTER_K_CH_5_8.Set(0);
   
   data_.registers.afe.spi.reg51.LPF_PROGRAMMABILITY.Set(0);
   data_.registers.afe.spi.reg51.PGA_CLAMP_LEVEL.Set(0);
   data_.registers.afe.spi.reg51.PGA_INTEGRATOR_DISABLE.Set(0);
   
   data_.registers.afe.spi.reg52.ACTIVE_TERMINATION.Set(0);
   data_.registers.afe.spi.reg52.ACTIVE_TERMINATION_ENABLE.Set(0);
   data_.registers.afe.spi.reg52.ACT_TER_ENA_CTRL.Set(0);
   data_.registers.afe.spi.reg52.LNA_INPUT_CLAMP_SETTING.Set(0);
   data_.registers.afe.spi.reg52.LNA_INTEGRATOR_DISABLE.Set(0);
   data_.registers.afe.spi.reg52.PRESET_ACTIVE_TERMINATIONS.Set(0);
   
   data_.registers.afe.spi.reg59.CW_SUM_AMP_PDN.Set(0);
   data_.registers.afe.spi.reg59.HPF_LNA.Set(0);
   data_.registers.afe.spi.reg59.PGA_TEST_MODE.Set(0);

   data_.registers.afe.spi.reg61.LPF_5MHz.Set(0);
   data_.registers.afe.spi.reg61.PGA_CLAMP_minus6dBFS.Set(0);
   data_.registers.afe.spi.reg61.V2I_CLAMP.Set(0);

   data_.registers.pulser.control.external_trigger.Set(0);
   data_.registers.pulser.control.gnd0_hiz1.Set(0);
   data_.registers.pulser.control.not_busy.Set(0);
   data_.registers.pulser.control.pulser_auto_load.Set(0);
   data_.registers.pulser.control.pulser_load.Set(0);
   data_.registers.pulser.control.pulser_current.Set(0);
   data_.registers.pulser.control.pulser_status.Set(0);
   data_.registers.pulser.control.software_trigger.Set(0);
   data_.registers.pulser.control.stop.Set(0);

   data_.registers.pulser.delay_mem_ini_end.delay_mem_end.Set(0);
   data_.registers.pulser.delay_mem_ini_end.delay_mem_ini.Set(0);
   data_.registers.pulser.delay_mem_ptr.Set(0);
   data_.registers.pulser.high_z.Set(0);
   data_.registers.pulser.main_delay.Set(0);

   data_.pa_system.resize(1);
   data_.pa_system[0].name = "Default";
   data_.pa_system[0].id_frontend = 0;

   data_.pa_system[0].id_base.resize(1);
   data_.pa_system[0].id_base[0] = 0;

   data_.sitau_hardware.beamformer.number = 1;

   // Valor inicial usado antes de leer BASES_NUMBER desde SITAU.XML.
   data_.sitau_hardware.bases_number = XML_DEF_N_BASES;
   data_.sitau_hardware.base.resize(XML_DEF_N_BASES);
   for (i = 0; i < data_.sitau_hardware.base.size(); i++)
   {
       data_.sitau_hardware.base[i].model = "BASE-32C";
       data_.sitau_hardware.base[i].version = "1";
       data_.sitau_hardware.base[i].serial_number = "";
   }

   data_.sitau_hardware.frontend.resize(1);
   data_.sitau_hardware.frontend[0].model = "FORNTEND-V1";
   data_.sitau_hardware.frontend[0].version = "0";
   data_.sitau_hardware.frontend[0].serial_number = "";
   data_.sitau_hardware.frontend[0].n_active_channels = 32;
   // El valor predeterminado corresponde al modo paralelo, no al multiplexado.
   data_.sitau_hardware.frontend[0].n_multiplexed_channels = 0;
   data_.sitau_hardware.frontend[0].n_channels = XML_DEF_N_BASES * MOD_MAX_CH;
   data_.sitau_hardware.frontend[0].n_active_channels = 32;

   data_.sitau_hardware.frontend[0].id_base.resize(1);
   data_.sitau_hardware.frontend[0].id_base[0] = 0;

   data_.sitau_hardware.frontend[0].connector.resize(1);
   data_.sitau_hardware.frontend[0].connector[0].name = "PA PROBE";
   data_.sitau_hardware.frontend[0].connector[0].pin_to_channel_assignment = true;
   data_.sitau_hardware.frontend[0].connector[0].start_channel = 0;
   data_.sitau_hardware.frontend[0].connector[0].n_total_channels = XML_DEF_N_BASES * MOD_MAX_CH;

   data_.sitau_hardware.frontend[0].connector[0].pin_to_channel.resize(XML_DEF_N_BASES * MOD_MAX_CH);
   for (i = 0; i < data_.sitau_hardware.frontend[0].connector[0].pin_to_channel.size(); i++)
      data_.sitau_hardware.frontend[0].connector[0].pin_to_channel[i] = i;

   data_.sitau_hardware.frontend[0].connector[0].pin_disabled.resize(XML_DEF_N_BASES * MOD_MAX_CH);
   for (i = 0; i < data_.sitau_hardware.frontend[0].connector[0].pin_disabled.size(); i++)
      data_.sitau_hardware.frontend[0].connector[0].pin_disabled[i] = 0;

   data_.sitau_hardware.uci.enabled = true;
   data_.sitau_hardware.uci.model = "UCI-II - GEthernet";
   data_.sitau_hardware.uci.version = "1";
   data_.sitau_hardware.uci.serial_number = "";

   data_.sitau_hardware.uci.encoder.resize(4);
   for (i = 0; i < data_.sitau_hardware.uci.encoder.size(); i++)
   {
      data_.sitau_hardware.uci.encoder[i].id = i;
      switch (i)
      {
      case 0: data_.sitau_hardware.uci.encoder[i].name = "A"; break;
      case 1: data_.sitau_hardware.uci.encoder[i].name = "B"; break;
      case 2: data_.sitau_hardware.uci.encoder[i].name = "C"; break;
      case 3: data_.sitau_hardware.uci.encoder[i].name = "D"; break;
      default: data_.sitau_hardware.uci.encoder[i].name = "None"; break;
      }
   }

   pa_system_.resize(1);
   pa_system_[0] = new CPASystem("Default", 0, 0, (int)data_.sitau_hardware.base.size(), data_.sitau_hardware.frontend[0].n_multiplexed_channels, data_.sitau_hardware.frontend[0].connector[0].start_channel, &data_.sitau_hardware.frontend[0]); //pa_system_[0] = new CPASystem(pa_system[i].name, 0,

   encoder_.number = 4;
   encoder_.id_2_name.resize(encoder_.number);
   for (i = 0; i < encoder_.id_2_name.size(); i++)
   {
      switch (i)
      {
      case 0: encoder_.id_2_name[i] = "A"; break;
      case 1: encoder_.id_2_name[i] = "B"; break;
      case 2: encoder_.id_2_name[i] = "C"; break;
      case 3: encoder_.id_2_name[i] = "D"; break;
      default: encoder_.id_2_name[i] = "None"; break;
      }
   }

   data_.buffer.pcie_dma_blocks_number = 500;
   data_.buffer.pcie_fifo_pointers_number = 512;
   data_.buffer.pcie_user_buffer_burst_number = 20;

   data_.user_config.status_period_ms = 500;
   data_.user_config.status_enabled = 1;

   data_.data_logger.enabled = 1; 
   data_.data_logger.pcie_user_buffer = 0;

   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
AnsiString CXML_SITAU_HW::GetEncoderName(int id_encoder)
{
   if (id_encoder < encoder_.id_2_name.size()) return encoder_.id_2_name[id_encoder];
   else return "";
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetBeamformersNumber(void)
{
   return data_.sitau_hardware.beamformer.number;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetConnectorStartChannel(int id_pa_sys, int id_connector)
{
   int id_frontend;

   if (id_pa_sys < data_.pa_system.size() &&
      (id_frontend = data_.pa_system[id_pa_sys].id_frontend) < data_.sitau_hardware.frontend.size() &&
      id_connector < data_.sitau_hardware.frontend[id_frontend].connector.size())
   {
      return data_.sitau_hardware.frontend[id_frontend].connector[id_connector].start_channel;
   }
   else return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPinToChannel(int id_pa_sys, int id_connector, vector<unsigned short>& pin_to_channel)
{
   int id_frontend;//, i, result = 100000;

   if (id_pa_sys < data_.pa_system.size() &&
      (id_frontend = data_.pa_system[id_pa_sys].id_frontend) < data_.sitau_hardware.frontend.size() &&
      id_connector < data_.sitau_hardware.frontend[id_frontend].connector.size())
   {
      pin_to_channel.assign(data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel.begin(),
         data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel.end());
      return (int)data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel.size();
   }
   else return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::ChannelToPin(int id_pa_sys, int id_connector, int id_channel)
{
   int result = -1, id_frontend;

   if (id_pa_sys < data_.pa_system.size() &&
      (id_frontend = data_.pa_system[id_pa_sys].id_frontend) < data_.sitau_hardware.frontend.size() &&
      id_connector < data_.sitau_hardware.frontend[id_frontend].connector.size() &&
      id_channel < data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel.size())
   {
      return data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel[id_channel];
   }
   else return ELOG(charEHW_DataInconsistency, EHW_DataInconsistency);
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPinToChannelSize(int id_pa_sys, int id_connector)
{
   int result = 0, id_frontend;

   if (id_pa_sys < data_.pa_system.size() &&
      (id_frontend = data_.pa_system[id_pa_sys].id_frontend) < data_.sitau_hardware.frontend.size() &&
      id_connector < data_.sitau_hardware.frontend[id_frontend].connector.size())
   {
      result = (int)data_.sitau_hardware.frontend[id_frontend].connector[id_connector].pin_to_channel.size();
   }
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
CPASystem* CXML_SITAU_HW::GetPtrPASystem(int id_pa_sys)
{
   CPASystem* result = NULL;

   if (id_pa_sys < pa_system_.size()) result = pa_system_[id_pa_sys];
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
CPASystem* CXML_SITAU_HW::GetPtrPASystem_BASEFirst(int id_base_first)
{
   CPASystem* result = NULL;
   int i;

   for (i = 0; result == NULL && i < pa_system_.size(); i++)
   {
      if (pa_system_[i] != NULL && id_base_first == pa_system_[i]->GetBASEFirst())
         result = pa_system_[i];
   }
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysChannelsNumber(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size() && data_.pa_system[id_pa_sys].id_frontend < data_.sitau_hardware.frontend.size())
      return data_.sitau_hardware.frontend[data_.pa_system[id_pa_sys].id_frontend].n_channels;
   return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetSITAUChannelsNumber(void)
{
   int i, result = 0;

   for (i = 0; i < data_.pa_system.size(); i++)
   {
      if (data_.pa_system[i].id_frontend < data_.sitau_hardware.frontend.size())
         result += data_.sitau_hardware.frontend[data_.pa_system[i].id_frontend].n_channels;
   }
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetSITAUConnectorsNumber(void)
{
   int i, result = 0;

   for (i = 0; i < data_.pa_system.size(); i++)
   {
      if (data_.pa_system[i].id_frontend < data_.sitau_hardware.frontend.size())
         result += (int)data_.sitau_hardware.frontend[data_.pa_system[i].id_frontend].connector.size();
   }
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysActiveChannelsNumber(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size() && data_.pa_system[id_pa_sys].id_frontend < data_.sitau_hardware.frontend.size())
      return data_.sitau_hardware.frontend[data_.pa_system[id_pa_sys].id_frontend].n_active_channels;
   return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetSITAUActiveChannelsNumber(void)
{
   int f, result = 0;

   for (f = 0; f < data_.sitau_hardware.frontend.size(); f++)
      result += data_.sitau_hardware.frontend[f].n_active_channels;
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetSITAUMultiplexedChannelsNumber(void)
{
   int f, result = 0;

   for (f = 0; f < data_.sitau_hardware.frontend.size(); f++)
      result += data_.sitau_hardware.frontend[f].n_multiplexed_channels;
   return result;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysMultiplexedChannelsNumber(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size() && data_.pa_system[id_pa_sys].id_frontend < data_.sitau_hardware.frontend.size())
      return data_.sitau_hardware.frontend[data_.pa_system[id_pa_sys].id_frontend].n_multiplexed_channels;
   return 0;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysFirstBASE(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size() && data_.pa_system[id_pa_sys].id_base.size() > 0)
      return data_.pa_system[id_pa_sys].id_base.front();
   return -1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysLastBASE(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size() && data_.pa_system[id_pa_sys].id_base.size() > 0)
      return data_.pa_system[id_pa_sys].id_base.back();
   return -1;
}
// -----------------------------------------------------------------------------
/*

*/
// -----------------------------------------------------------------------------
int CXML_SITAU_HW::GetPASysBasesNumber(int id_pa_sys)
{
   if (id_pa_sys < data_.pa_system.size()) return (int)data_.pa_system[id_pa_sys].id_base.size();
   return 0;
}










