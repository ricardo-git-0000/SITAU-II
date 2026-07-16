#ifndef fp_c_xml_sitau_hwH
#define fp_c_xml_sitau_hwH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <vcl.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pugixml.hpp"
#include "cxml.h"
#include "c_pa_system.h"
#include "t_sitau_hw.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define XML_MAX_N_PASystems          4
#define XML_DEF_N_BASES              4
#define XML_MAX_N_MODULES_BASE       4
#define XML_MAX_N_FRONTEND           4
#define XML_MAX_N_FRONTEND           4

#define XML_VERSION              "1.0"

#define DEPTH_TREE               10

#define XMLHW_eSITAU                   "SITAU2"
#define XMLHW_eRegistersConfiguration   "Registers_Configuration"
#define XMLHW_eHardwareConfiguration   "Hardware_Configuration"

#define XMLHW_eUserConfiguration "User_Configuration"
#define pNameSTATUS_ENABLED "STATUS_ENABLED"
#define pNameSTATUS_PERIOD_MS "STATUS_PERIOD_MS"

#define XMLHW_eBufferConfiguration "Buffer_Configuration"
#define pNamePCIE_USER_BUFER_BURST_NUMBER   "PCIE_USER_BUFER_BURST_NUMBER"
#define pNamePCIE_FIFO_POINTERS_NUMBER   "PCIE_FIFO_POINTERS_NUMBER"
#define pNamePCIE_DMA_BLOCKS_NUMBER   "PCIE_DMA_BLOCKS_NUMBER"

#define XMLHW_eDataloggerConfiguration "Datalogger_Configuration"
#define pNameLOG_FILE_ENABLED   "LOG_FILE_ENABLED"
#define pNameLOG_FILE_PCIE_USER_BUFFER   "LOG_FILE_PCIE_USER_BUFFER"
#define pNameLOG_FILE_CONFIG   "LOG_FILE_CONFIG"
#define pNameLOG_FILE_PA_BEAMFORMER  "LOG_FILE_PA_BEAMFORMER"
#define pNameLOG_FILE_TFM_BEAMFORMER   "LOG_FILE_TFM_BEAMFORMER"

#define ePASystemConfiguration   "PASystem_Configuration"
#define eSITAUHardware           "SITAU_Hardware"
#define eEquipmentInformation    "Equipment_Information"
#define ePowerSupply             "Power_Supply"
#define eFRONTEND_Connector      "CONNECTOR"

#define eParameter               "eParameter"
#define pName                    "pName"
#define pType                    "pType"
#define pStartArrayElement       "pStartArrayElement"
#define pValue                   "pValue"

#define pNameModel                  "Model"
#define pNameSerialNumber           "Serial Number"
#define pNameManufactureDate        "Manufacture Date"
#define pNameCalibrationDate        "Calibration Date"
#define pNameStartChannel           "Start Channel"
#define pNameTotalChannels          "Total Channels"

#define pNameIdBASE                 "BASE Id"

#define pNameId                "Id"
#define pNameName                "Name"

#define pNameVersion                 "Version"
#define pNameVersionXML          "XML File Version"
#define pNameVersionApp          "MotorMotion Application Version"
#define pNameVersionAPIMC        "APIMC Library Version"
#define pNameVersionFirmware     "Firmware Version"

#define ePASystem              "PASystem"
#define eBASE                  "BASE"
#define eFRONTEND                "FRONTEND"
#define eFUAT_BASE               "FUAT_BASE"
#define eUCI                     "UCI"
#define eFUAT_BIPO_TARMA         "FUAT_BIPO_TARMA"
#define eEncoder                 "Encoder"
#define eBEAMFORMER         "BEAMFORMER"


#define pNameTemperatureWarning     "Temperature Warning"
#define pNameTemperatureShutdown      "Temperature Shutdown"
#define pNameTemperatureHysteresis      "Temperature Hysteresis"
#define pNameLowFrequency     "Low Frequency"

#define pNameMultiplexedChannels   "Multiplexed Channels"
#define pNameActiveChannels   "Active Channels"
#define pNamePitchCatch   "Pitch-Catch"
#define pNameBASENumber   "BASE Number"
#define pNameIdConnector                "Connector Id"
#define pNameIdFRONTEND                "FRONTEND Id"

#define pNameVoltage             "Voltage"

#define pNamePowerSupplyType                  "Power Supply Type"

#define pNameBeamformer_Number_1   "Beamformer Number"
#define pNameBeamformer_Number_2   "BEAMFORMER_NUMBER"

#define pNameBASES_NUMBER   "BASES_NUMBER"

#define XMLHW_ePULSER                    "PULSER"
#define XMLHW_eAFE                       "AFE"
#define XMLHW_eBUSSAR                    "BUSSAR"
#define XMLHW_eSPI                       "SPI"
#define XMLHW_eCONTROL                   "CONTROL"
#define XMLHW_eDELAY_MEM                 "DELAY_MEM"
#define XMLHW_eREG21                     "REG21"
#define XMLHW_eREG33                     "REG33"
#define XMLHW_eREG51                     "REG51"
#define XMLHW_eREG52                     "REG52"
#define XMLHW_eREG59                     "REG59"
#define XMLHW_eREG61                     "REG61"

#define XMLHW_eRegister                  "Register"
#define XMLHW_eMemory                    "Memory"

#define XMLHW_pNamePULSER_MAIN_DELAY     "PULSER_MAIN_DELAY"
#define XMLHW_pNameDELAY_MEM_PTR         "DELAY_MEM_PTR"

#define XMLHW_pNameGND0_HIZ1             "GND0_HIZ1"
#define XMLHW_pNamePULSER_LOAD           "PULSER_LOAD"
#define XMLHW_pNamePULSER_AUTO_LOAD      "PULSER_AUTO_LOAD"
#define XMLHW_pNamePULSER_MODE           "PULSER_MODE"
#define XMLHW_pNamePULSER_STATUS         "PULSER_STATUS"
#define XMLHW_pNameSTOP                  "STOP"
#define XMLHW_pNameEXTERNAL_TRIGGER      "EXTERNAL_TRIGGER"
#define XMLHW_pNameNOT_BUSY              "NOT_BUSY"
#define XMLHW_pNameSOFTWARE_TRIGGER      "SOFTWARE_TRIGGER"

#define XMLHW_pNameDELAY_MEM_INI         "DELAY_MEM_INI"
#define XMLHW_pNameDELAY_MEM_END         "DELAY_MEM_END"

#define XMLHW_pNameDELAY_OFFSET			"DELAY_OFFSET"

#define XMLHW_pNameDIGITAL_HPF_FILTER_ENABLE_1_4  "DIGITAL_HPF_FILTER_ENABLE_1_4"
#define XMLHW_pNameDIGITAL_HPF_FILTER_K_CH_1_4    "DIGITAL_HPF_FILTER_K_CH_1_4"

#define XMLHW_pNameDIGITAL_HPF_FILTER_ENABLE_5_8  "DIGITAL_HPF_FILTER_ENABLE_5_8"
#define XMLHW_pNameDIGITAL_HPF_FILTER_K_CH_5_8    "DIGITAL_HPF_FILTER_K_CH_5_8"

#define XMLHW_pNameLPF_PROGRAMMABILITY      "LPF_PROGRAMMABILITY"
#define XMLHW_pNamePGA_INTEGRATOR_DISABLE   "PGA_INTEGRATOR_DISABLE"
#define XMLHW_pNamePGA_CLAMP_LEVEL          "PGA_CLAMP_LEVEL"

#define XMLHW_pNameACTIVE_TERMINATION          "ACTIVE_TERMINATION"
#define XMLHW_pNameACT_TER_ENA_CTRL            "ACT_TER_ENA_CTRL"
#define XMLHW_pNamePRESET_ACTIVE_TERMINATIONS  "PRESET_ACTIVE_TERMINATIONS"
#define XMLHW_pNameACTIVE_TERMINATION_ENABLE   "ACTIVE_TERMINATION_ENABLE"
#define XMLHW_pNameLNA_INPUT_CLAMP_SETTING     "LNA_INPUT_CLAMP_SETTING"
#define XMLHW_pNameLNA_INTEGRATOR_DISABLE      "LNA_INTEGRATOR_DISABLE"

#define XMLHW_pNameHPF_LNA                     "HPF_LNA"
#define XMLHW_pNameCW_SUM_AMP_PDN              "CW_SUM_AMP_PDN"
#define XMLHW_pNamePGA_TEST_MODE               "PGA_TEST_MODE"

#define XMLHW_pNameV2I_CLAMP                   "V2I_CLAMP"
#define XMLHW_pNameLPF_5MHz                    "LPF_5MHz"
#define XMLHW_pNamePGA_CLAMP_MINUS6DBFS        "PGA_CLAMP_MINUS6DBFS"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   T I P O S
// =============================================================================

// _____________________________________________________________________________
// ------ CXML_SITAU_HW class
// _____________________________________________________________________________
// =============================================================================
class CXML_SITAU_HW : public CXML
{
private:

   vector<CPASystem*> pa_system_;
   THW_Encoder encoder_;
   int error_;
   AnsiString file_name_;

   int GetData_PULSER_CONTROL(pugi::xml_node node_data, TXMLHW_PULSER_CONTROL* data);
   int GetData_PULSER_DELAY_MEM(pugi::xml_node node_data, TXMLHW_DELAY_MEM* data);
   int GetData_PULSER(void);
   int GetData_AFE_BUSSAR(pugi::xml_node node_data, TXMLHW_AFE_BUSSAR* data);
   int GetData_AFE_SPI_REG21(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG21* data);
   int GetData_AFE_SPI_REG33(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG33* data);
   int GetData_AFE_SPI_REG51(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG51* data);
   int GetData_AFE_SPI_REG52(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG52* data);
   int GetData_AFE_SPI_REG59(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG59* data);
   int GetData_AFE_SPI_REG61(pugi::xml_node node_data, TXMLHW_AFE_SPI_REG61* data);
   int GetData_AFE_SPI(pugi::xml_node node_data, TXMLHW_AFE_SPI* data);
	int GetData_AFE(void);
   int GetData_PASystemConfiguration(void);
   //int GetData_BASE (void);
   int GetData_FRONTEND_Connector (pugi::xml_node nodes, vector<TXMLHW_CONNECTOR> &connector);
   int GetData_FRONTEND (void);
   int GetData_Encoder (pugi::xml_node nodes, vector<TXMLHW_Encoder> &encoder);
   int GetData_UCI (void);
   int GetData_BEAMFORMER(void);
   int GetData_HardwareConfiguration(void);
   int GetData_DataLogger(void);
   int GetData_Buffer(void);
   int GetData_UserConfig(void);

public:

   TXMLHW_Data data_;
   
   CXML_SITAU_HW(AnsiString file_name);
   ~CXML_SITAU_HW() {};
   int Update(void);

   TXMLHW_Data *GetData(void) {return &data_;}
   int ResetData(void);
   int GetError(void) { return error_; }

   int GetPASystemNumber(void) { return (int)pa_system_.size(); }
   int GetBasesNumber(void) { return (int)data_.sitau_hardware.base.size(); }
   int GetPASysBasesNumber(int id_pa_sys);
   int GetPASysChannelsNumber(int id_pa_sys);
   int GetSITAUChannelsNumber(void);
   int GetSITAUConnectorsNumber(void);
   int GetSITAUActiveChannelsNumber(void);
   int GetSITAUMultiplexedChannelsNumber(void);
   int GetPASysActiveChannelsNumber(int id_pa_sys);
   int GetPASysMultiplexedChannelsNumber(int id_pa_sys);
   int GetPASysFirstBASE(int id_pa_sys);
   int GetPASysLastBASE(int id_pa_sys);
   int GetConnectorStartChannel(int id_pa_sys, int id_connector);
   int GetPinToChannel(int id_pa_sys, int id_connector, vector<unsigned short>& pin_to_channel);
   int GetPinToChannelSize(int id_pa_sys, int id_connector);
   int ChannelToPin(int id_pa_sys, int id_connector, int id_channel);
   CPASystem* GetPtrPASystem(int id_pa_sys);
   CPASystem* GetPtrPASystem_BASEFirst(int id_base_first);

   int GetEncodersNumber(void) { return encoder_.number; }
   AnsiString GetEncoderName(int id_encoder);
   int GetBeamformersNumber(void);
};

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern CXML_SITAU_HW* gb_xml_sitau_hw;

#endif
