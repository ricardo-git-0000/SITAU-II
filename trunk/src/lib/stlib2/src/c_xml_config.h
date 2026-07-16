#ifndef c_xml_configH
#define c_xml_configH

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

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define LOG_XML_FILE                    "LOG_XMLFile.txt"

#define XMLCFG_eSITAU2                    "SITAU2"
#define XMLCFG_pNameVersionXML            "XML File Version"
#define XMLCFG_VERSION                    "2.0"

#define XMLCFG_eVersion                   "Version"
#define XMLCFG_eDateTime                  "DateTime"
#define XMLCFG_pNameDateTime              "Date and Time"

//#define XMLCFG_tString                    "tString"
//#define XMLCFG_tInt32                     "tInt32"
//#define XMLCFG_tUInt32                    "tUInt32"
//#define XMLCFG_tFloat                     "tFloat"
//#define XMLCFG_tBoolean                   "tBoolean"
//#define XMLCFG_tfalse                     "false"
//#define XMLCFG_ttrue                      "true"
//#define XMLCFG_tArrayUInt16               "tArrayUInt16"

#define XMLCFG_eSTFPLIB2                  "STLIB2"
#define XMLCFG_eVirtualChannel            "VirtualChannel"
//#define XMLCFG_eDSP                       "DSP"
//#define XMLCFG_eBASERegisters             "BASE_Registers"

//#define XMLCFG_eAFE                       "AFE"
//#define XMLCFG_eBUSSAR                    "BUSSAR"
//#define XMLCFG_eSPI                       "SPI"
//#define XMLCFG_eCONTROL                   "CONTROL"
//#define XMLCFG_eDELAY_MEM                 "DELAY_MEM"
//#define XMLCFG_eREG21                     "REG21"
//#define XMLCFG_eREG33                     "REG33"
//#define XMLCFG_eREG51                     "REG51"
//#define XMLCFG_eREG52                     "REG52"
//#define XMLCFG_eREG59                     "REG59"
//#define XMLCFG_eREG61                     "REG61"

#define XMLCFG_eParameter                 "Parameter"
#define XMLCFG_eRegister                  "Register"
#define XMLCFG_eMemory                    "Memory"
#define XMLCFG_pName                      "pName"
//#define XMLCFG_pType                      "pType"
//#define XMLCFG_pStartArrayElement         "pStartArrayElement"
#define XMLCFG_pValue                     "pValue"

//#define XMLCFG_pNamePULSE_WIDTH           "PULSE_WIDTH"
//#define XMLCFG_pNamePULSER_MAIN_DELAY     "PULSER_MAIN_DELAY"
//#define XMLCFG_pNameDELAY_MEM_PTR         "DELAY_MEM_PTR"
//
//#define XMLCFG_pNamePUL_ENABLE            "PUL_ENABLE"
//#define XMLCFG_pNameGND0_HIZ1             "GND0_HIZ1"
//#define XMLCFG_pNamePULSER_LOAD           "PULSER_LOAD"
//#define XMLCFG_pNamePULSER_AUTO_LOAD      "PULSER_AUTO_LOAD"
//#define XMLCFG_pNamePULSER_MODE           "PULSER_MODE"
//#define XMLCFG_pNamePULSER_STATUS         "PULSER_STATUS"
//#define XMLCFG_pNameN_PULSES              "N_PULSES"
//#define XMLCFG_pNameSTOP                  "STOP"
//#define XMLCFG_pNameEXTERNAL_TRIGGER      "EXTERNAL_TRIGGER"
//#define XMLCFG_pNameNOT_BUSY              "NOT_BUSY"
//#define XMLCFG_pNameSOFTWARE_TRIGGER      "SOFTWARE_TRIGGER"
//
//#define XMLCFG_pNameDELAY_MEM_INI         "DELAY_MEM_INI"
//#define XMLCFG_pNameDELAY_MEM_END         "DELAY_MEM_END"
//
//#define XMLCFG_pNameDELAY_OFFSET                "DELAY_OFFSET"
//#define XMLCFG_pNameDEC_RATIO                "DEC_RATIO"
//#define XMLCFG_pNameNUM_SAMPLES              "NUM_SAMPLES"
//#define XMLCFG_pNameWATER_DELAY              "WATER_DELAY"
//#define XMLCFG_pNameLOG2_AVERAGE             "LOG2_AVERAGE"
//
//#define XMLCFG_pNameDIGITAL_HPF_FILTER_ENABLE_1_4  "DIGITAL_HPF_FILTER_ENABLE_1_4"
//#define XMLCFG_pNameDIGITAL_HPF_FILTER_K_CH_1_4    "DIGITAL_HPF_FILTER_K_CH_1_4"
//
//#define XMLCFG_pNameDIGITAL_HPF_FILTER_ENABLE_5_8  "DIGITAL_HPF_FILTER_ENABLE_5_8"
//#define XMLCFG_pNameDIGITAL_HPF_FILTER_K_CH_5_8    "DIGITAL_HPF_FILTER_K_CH_5_8"
//
//#define XMLCFG_pNameLPF_PROGRAMMABILITY      "LPF_PROGRAMMABILITY"
//#define XMLCFG_pNamePGA_INTEGRATOR_DISABLE   "PGA_INTEGRATOR_DISABLE"
//#define XMLCFG_pNamePGA_CLAMP_LEVEL          "PGA_CLAMP_LEVEL"
//#define XMLCFG_pNamePGA_GAIN_CONTROL         "PGA_GAIN_CONTROL"
//
//#define XMLCFG_pNameACTIVE_TERMINATION          "ACTIVE_TERMINATION"
//#define XMLCFG_pNameACT_TER_ENA_CTRL            "ACT_TER_ENA_CTRL"
//#define XMLCFG_pNamePRESET_ACTIVE_TERMINATIONS  "PRESET_ACTIVE_TERMINATIONS"
//#define XMLCFG_pNameACTIVE_TERMINATION_ENABLE   "ACTIVE_TERMINATION_ENABLE"
//#define XMLCFG_pNameLNA_INPUT_CLAMP_SETTING     "LNA_INPUT_CLAMP_SETTING"
//#define XMLCFG_pNameLNA_INTEGRATOR_DISABLE      "LNA_INTEGRATOR_DISABLE"
//#define XMLCFG_pNameLNA_GAIN                    "LNA_GAIN"
//#define XMLCFG_pNameLNA_INDIVIDUAL_CH_CNTL      "LNA_INDIVIDUAL_CH_CNTL"
//
//#define XMLCFG_pNameHPF_LNA                     "HPF_LNA"
//#define XMLCFG_pNameDIG_TGC_ATT_GAIN            "DIG_TGC_ATT_GAIN"
//#define XMLCFG_pNameDIG_TGC_ATT                 "DIG_TGC_ATT"
//#define XMLCFG_pNameCW_SUM_AMP_PDN              "CW_SUM_AMP_PDN"
//#define XMLCFG_pNamePGA_TEST_MODE               "PGA_TEST_MODE"
//
//#define XMLCFG_pNameV2I_CLAMP                   "V2I_CLAMP"
//#define XMLCFG_pNameLPF_5MHz                    "LPF_5MHz"
//#define XMLCFG_pNamePGA_CLAMP_MINUS6DBFS        "PGA_CLAMP_MINUS6DBFS"

// <FUS>
#define XMLCFG_eFUS									"FUS"

#define XMLCFG_pName_FUS_CentralFrequency			"central_frequency"
#define XMLCFG_pName_FUS_RapportFocalAperture		"rapport_focal_aperture"
#define XMLCFG_pName_FUS_ApodizationCoefficient		"apodization_coefficient"
#define XMLCFG_pName_FUS_LateralPositionX			"lateral_position_x"
#define XMLCFG_pName_FUS_PixelSizeX					"pixel_size_x"
#define XMLCFG_pName_FUS_PixelsX					"pixels_x"
#define XMLCFG_pName_FUS_DepthZ						"depth_z"
#define XMLCFG_pName_FUS_PixelsZ					"pixels_z"
#define XMLCFG_pName_FUS_PixelSizeZ					"pixel_size_z"
#define XMLCFG_pName_FUS_SinglePlane				"single_plane"
#define XMLCFG_pName_FUS_SamplesWavelength			"samples_wavelength"
#define XMLCFG_pName_FUS_InterpolationCoefficients	"interpolation_coefficients"

// <ACQUISITION>
#define XMLCFG_eACQUISITION					"ACQUISITION"

//#define XMLCFG_pName_ACQ_DataLinkMode		"data_link_mode"
#define XMLCFG_pName_ACQ_AcquisitionMode	"acquisition_mode"
#define XMLCFG_pName_ACQ_BurstsNumber		"n_bursts"
#define XMLCFG_pName_ACQ_ImagesBurstNumber	"n_images_burst"
#define XMLCFG_pName_DelaySyncTime_1		"Delay Sync Time [ms]"
#define XMLCFG_pName_DelaySyncTime_2		"delay_sync_time"
#define XMLCFG_pName_TriggerSource			"trigger_source"
#define XMLCFG_pName_PauseAfterBurst		"pause_after_burst"
#define XMLCFG_pName_BurstNumber			"burst_number"
#define XMLCFG_pName_ImagesPerBurstNumber	"images_per_burst_number"


// <UT_Parameters>
// <UT_PARAMETERS>
//#define XMLCFG_eUTParameters_1              "UT_Parameters"
#define XMLCFG_eUT_PARAMETERS				"UT_PARAMETERS"

//#define XMLCFG_pName_Gain_1					"Gain [dB]"
#define XMLCFG_pName_Gain_2					"gain"

#define XMLCFG_pName_TGC_Type_1				"TGC Type"
#define XMLCFG_pName_TGC_Type_2				"tgc_type"

#define XMLCFG_pName_TGC_Enabled_1			"TGC Enabled"
#define XMLCFG_pName_TGC_Enabled_2			"tgc_enabled"

#define XMLCFG_pName_TGC_Slope				"tgc_slope"

#define XMLCFG_pName_TGC_Points_dB			"tgc_points_mm"

#define XMLCFG_pName_TGC_Points_mm			"tgc_points_db"

//#define XMLCFG_pName_PRF_Frequency_1		"PRF Frequency [Hz]"
#define XMLCFG_pName_PRF_Frequency_2		"prf_frequency"

#define XMLCFG_pName_PRF_Burst_Frequency		"prf_burst_frequency"

//#define XMLCFG_pName_SamplingDecimation_1	"Sampling Frequency Decimation"
#define XMLCFG_pName_SamplingDecimation_2	"sampling_freq_decimation"

//#define XMLCFG_pName_MaterialVelocity_1		"Material Velocity [m/s]"
#define XMLCFG_pName_MaterialVelocity_2		"material_velocity"

//#define XMLCFG_pName_AcqStartDistance_1		"Acq. Start Distance [mm]"
#define XMLCFG_pName_AcqStartDistance_2		"acq_start_distance"

//#define XMLCFG_pName_AcqEndDistance_1		"Acq. End Distance [mm]"
#define XMLCFG_pName_AcqEndDistance_2		"acq_end_distance"

#define XMLCFG_pName_AcqTimeMode			"acq_time_mode"

#define XMLCFG_pName_AcqStartSamples		"acq_start_samples"

#define XMLCFG_pName_AcqSamplesNumber		"acq_n_samples"

#define XMLCFG_pName_AcquisitionType		"acquisition_type"

#define XMLCFG_pName_SignalMode		"signal_mode"

// <TGC>
#define XMLCFG_eTGC							"TGC"

#define XMLCFG_pName_TGC_Curve				"TGC Curve"

#define XMLCFG_pName_TGC_Attenuation		"TGC Attenuation"

// <PULSER>
#define XMLCFG_ePULSER						"PULSER"

//#define XMLCFG_pName_PulserEnabled_1		"Pulser Enabled"
#define XMLCFG_pName_PulserEnabled_2		"pulser_enabled"

//#define XMLCFG_pName_PulsesNumber_1			"Pulses Number"
#define XMLCFG_pName_PulsesNumber_2			"pulses_number"

//#define XMLCFG_pName_PulseWidth_1			"Pulse Width [ns]"
#define XMLCFG_pName_PulseWidth_2			"pulse_width"

//#define XMLCFG_pName_PulseAmplitude_1		"Pulse Amplitude [V]"
#define XMLCFG_pName_PulseAmplitude_2		"pulse_amplitude"

// <PROBE>
#define XMLCFG_ePROBE						"PROBE"

//#define XMLCFG_pName_ProbeElementsNumber_1	"Probe Elements Number"
#define XMLCFG_pName_ProbeElementsNumber_2	"probe_elements_number"

//#define XMLCFG_pName_ProbePitch_1			"Probe Pitch [mm]"
#define XMLCFG_pName_ProbePitch_2			"probe_pitch"

//#define XMLCFG_pName_ProbeFrequency_1		"Probe Frequency [MHz]"
#define XMLCFG_pName_ProbeFrequency_2		"probe_frequency"

//#define XMLCFG_pName_HighVoltageMaximum_1	"High Voltage Maximum"
#define XMLCFG_pName_HighVoltageMaximum_2	"high_voltage_maximum"

#define XMLCFG_pName_PinToChannel			"pin_to_channel"
#define XMLCFG_pName_PinToChannelAssignment	"pin_to_channel_assignment"

// <FP_FOCAL_LAW>
#define XMLCFG_eMC_FOCAL_LAW				"MC_FOCAL_LAW"

//#define XMLCFG_pName_Acquisition_Type		"Acquisition Type"
#define XMLCFG_pName_Acquisition_Mode		"fp_acquisition_mode"

//#define XMLCFG_pName_FP_FocalLawType_1		"(FP) Focal Law type"
#define XMLCFG_pName_FP_FocalLawType_2		"fp_focal_law_type"
#define XMLCFG_pName_MC_FocalLawType		"mc_focal_law_type"

//#define XMLCFG_pName_FP_PWICStartAngle_1		"(FP) PWIC Start Angle [deg]"
#define XMLCFG_pName_FP_PWICStartAngle_2		"fp_pwic_start_angle"
#define XMLCFG_pName_MC_PWICStartAngle		"mc_pwic_start_angle"

//#define XMLCFG_pName_FP_PWICEndAngle_1		"(FP) PWIC End Angle [deg]"
#define XMLCFG_pName_FP_PWICEndAngle_2		"fp_pwic_end_angle"
#define XMLCFG_pName_MC_PWICEndAngle		"mc_pwic_end_angle"

//#define XMLCFG_pName_FP_PWICStepAngle_1		"(FP) PWIC Step Angle [deg]"
#define XMLCFG_pName_FP_PWICStepAngle_2		"fp_pwic_step_angle"
#define XMLCFG_pName_MC_PWICStepAngle		"mc_pwic_step_angle"

#define XMLCFG_pName_FP_PWICAngles			"fp_pwic_angles"
#define XMLCFG_pName_MC_PWICAngles			"mc_pwic_angles"

//#define XMLCFG_pName_FP_AcqNumber			"(FP) Acquisitions Number"

#define XMLCFG_pName_FP_BurstNumber			"fp_burst_number"

#define XMLCFG_pName_FP_BurstImagesNumber	"fp_burst_images_number"

#define XMLCFG_pName_FP_FocalLawFile		"fp_focal_law_file"
#define XMLCFG_pName_MC_FocalLawFile		"mc_focal_law_file"

// <DSP>
#define XMLCFG_eDSP							"DSP"
#define XMLCFG_pName_DSP_FIREnabled			"fir_enabled"
#define XMLCFG_pName_DSP_FIRLowFrequency	"fir_low_frequency"
#define XMLCFG_pName_DSP_FIRHighFrequency	"fir_high_frequency"

// <FP_DSP>
#define XMLCFG_eMC_DSP							"MC_DSP"

#define XMLCFG_pName_MC_DSP_FIREnabled			"mc_fir_enabled"
#define XMLCFG_pName_MC_DSP_FIRLowFrequency		"mc_fir_low_frequency"
#define XMLCFG_pName_MC_DSP_FIRHighFrequency	"mc_fir_high_frequency"
#define XMLCFG_pName_MC_DSP_ProcessingType		"mc_processing_type"
#define XMLCFG_pName_MC_AverageSignalsNumber	"mc_average_signals_number"

// <PA_FocalLaw>
// <FP_FOCAL_LAW>
//#define XMLCFG_ePAFocalLaw					"PA_FocalLaw"
#define XMLCFG_ePA_FOCAL_LAW				"PA_FOCAL_LAW"

//#define XMLCFG_pName_PA_Aperture_1			"(PA) Aperture Size"
#define XMLCFG_pName_PA_Aperture_2			"pa_aperture_size"

//#define XMLCFG_pName_PA_ScanType_1			"(PA) Scan Type"
#define XMLCFG_pName_PA_ScanType_2			"pa_scan_type"

//#define XMLCFG_pName_PA_StartElement_1		"(PA) Start Element"
#define XMLCFG_pName_PA_StartElement_2		"pa_start_element"

//#define XMLCFG_pName_PA_LinearStep_1		"(PA) Linear Step"
#define XMLCFG_pName_PA_LinearStep_2		"pa_linear_step"

//#define XMLCFG_pName_PA_LinearEndElement_1	"(PA) Linear End Element"
#define XMLCFG_pName_PA_LinearEndElement_2	"pa_linear_end_element"

//#define XMLCFG_pName_PA_LinearAngle_1		"(PA) Linear Angle [deg]"
#define XMLCFG_pName_PA_LinearAngle_2		"pa_linear_angle"

//#define XMLCFG_pName_PA_AngularStartAngle_1	"(PA) Angular Start Angle [deg]"
#define XMLCFG_pName_PA_AngularStartAngle_2	"pa_angular_start_angle"

//#define XMLCFG_pName_PA_AngularEndAngle_1	"(PA) Angular End Angle [deg]"
#define XMLCFG_pName_PA_AngularEndAngle_2	"pa_angular_end_angle"

//#define XMLCFG_pName_PA_AngularStep_1		"(PA) Angular Step [deg]"
#define XMLCFG_pName_PA_AngularStep_2		"pa_angular_step"

#define XMLCFG_pName_PA_FocusEnabled		"pa_focus_enabled"
#define XMLCFG_pName_PA_ReceptionAutoFocus	"pa_reception_auto_focus"

//#define XMLCFG_pName_PA_EmissionFocus_1		"(PA) Emission Focal Point [mm]"
#define XMLCFG_pName_PA_EmissionFocus_2		"pa_emission_focal_point"

//#define XMLCFG_pName_PA_ReceptionFocus_1	"(PA) Reception Focal Point [mm]"
#define XMLCFG_pName_PA_ReceptionFocus_2	"pa_reception_focal_point"

#define XMLCFG_pName_PA_ParallelLines_1		"(PA) Global Parallel Lines"
#define XMLCFG_pName_PA_ParallelLines_2		"pa_global_parallel_lines"

#define XMLCFG_pName_PA_IncreasedAcquiredSamples_1	"(PA) Increased Acquired Samples"
#define XMLCFG_pName_PA_IncreasedAcquiredSamples_2	"pa_increased_acquired_samples"

// <PA_DSP>
#define XMLCFG_ePA_DSP							"PA_DSP"

//#define XMLCFG_pName_DSP_FIREnabled				"FIR Enabled"
#define XMLCFG_pName_PA_DSP_FIREnabled			"pa_fir_enabled"

//#define XMLCFG_pName_DSP_FIRLowFrequency		"FIR Low Frequency [MHz]"
#define XMLCFG_pName_PA_DSP_FIRLowFrequency		"pa_fir_low_frequency"

//#define XMLCFG_pName_DSP_FIRHighFrequency		"FIR High Frequency [MHz]"
#define XMLCFG_pName_PA_DSP_FIRHighFrequency	"pa_fir_high_frequency"

//#define XMLCFG_pName_DSP_ProcessingType			"Processing Type"
#define XMLCFG_pName_PA_DSP_ProcessingType		"pa_processing_type"

//#define XMLCFG_pName_DSP_AVRSignalsNumber		"Average Signals Number"
#define XMLCFG_pName_PA_DSP_AVRSignalsNumber	"pa_average_signals_number"

//#define XMLCFG_pName_DSP_EMISignalsNumber		"EMI Signals Number"
#define XMLCFG_pName_PA_DSP_EMISignalsNumber	"pa_emi_signals_number"

// <TFM_FOCAL_LAW>
#define XMLCFG_eTFM_FOCAL_LAW					"TFM_FOCAL_LAW"
#define XMLCFG_pName_TFM_EmissionStartElement	"tfm_emission_start_element"
#define XMLCFG_pName_TFM_EmissionEndElement		"tfm_emission_end_element"
#define XMLCFG_pName_TFM_EmissionStepElement	"tfm_emission_step_element"
#define XMLCFG_pName_TFM_ReceiverLinesNumber	"tfm_receiver_lines_number"
#define XMLCFG_pName_TFM_ReceiverAngle			"tfm_receiver_angle"
#define XMLCFG_pName_TFM_ReceiverStartDistance	"tfm_receiver_start_distance"
#define XMLCFG_pName_TFM_ReceiverEndDistance	"tfm_receiver_end_distance"

#define XMLCFG_pName_TFM_ParallelLines		"tfm_global_parallel_lines"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   T I P O S
// =============================================================================

typedef struct TXMLCFG_PAFocalLaw
{
   CXMLParameter<int> pa_aperture;
   CXMLParameter<int> pa_scan_type;
   CXMLParameter<int> pa_start_element;
   CXMLParameter<int> pa_linear_step;
   CXMLParameter<int> pa_linear_end_element;
   CXMLParameter<float> pa_linear_angle;
   CXMLParameter<float> pa_angular_step;
   CXMLParameter<float> pa_angular_start_angle;
   CXMLParameter<float> pa_angular_end_angle;
   CXMLParameter<int> pa_focus_enabled;
   CXMLParameter<int> pa_reception_auto_focus_ddf;
   CXMLParameter<float> pa_emission_focus;
   CXMLParameter<float> pa_reception_focus;
   CXMLParameter<int> n_parallel_beamformed_lines;
   CXMLParameter<int> increased_acquired_samples;
} TXMLCFG_PAFocalLaw;

typedef struct TXMLCFG_UT_PARAMETERS
{
	CXMLParameter<float> gain;
	CXMLParameter<int> tgc_enabled;
	CXMLParameter<int> tgc_type;
	CXMLParameter<float> tgc_attenuation;
	CXMLParameter<vector<float>> tgc_points_mm;
	CXMLParameter<vector<float>> tgc_points_db;
	CXMLParameter<float> prf_frequency;
	CXMLParameter<float> prf_burst_frequency;
	CXMLParameter<int> sampling_freq_decimation;
	CXMLParameter<float> material_velocity;
	CXMLParameter<float> acq_start_distance;
	CXMLParameter<float> acq_end_distance;
	CXMLParameter<int> acq_time_mode;
	CXMLParameter<int> acq_start_samples;
	CXMLParameter<int> acq_n_samples;
	CXMLParameter<int> acquisition_type;
	CXMLParameter<int> signal_mode;
} TXMLCFG_UT_PARAMETERS;

typedef struct TXMLCFG_PULSER
{
	CXMLParameter<int> pulser_enabled;
	CXMLParameter<int> pulses_number;
	CXMLParameter<int> pulse_width;
	CXMLParameter<int> pulse_amplitude;
} TXMLCFG_PULSER;

typedef struct TXMLCFG_PROBE
{
	CXMLParameter<int> probe_elements_number;
	CXMLParameter<float> probe_pitch;
	CXMLParameter<float> probe_frequency;
	CXMLParameter<int> high_voltage_maximum;
	CXMLParameter<int> pin_to_channel_assignment;
	CXMLParameter<vector<unsigned short>> pin_to_channel;
} TXMLCFG_PROBE;

typedef struct TXMLCFG_FP_FOCAL_LAW
{
	CXMLParameter<int> fp_focal_law_type;
	CXMLParameter<float> fp_pwic_start_angle;
	CXMLParameter<float> fp_pwic_end_angle;
	CXMLParameter<float> fp_pwic_step_angle;
	CXMLParameter<vector<float>> fp_pwic_angles;
	//CXMLParameter<int> fp_burst_number;
	//CXMLParameter<int> fp_burst_images_number;
	CXMLParameter<string> fp_focal_law_file;
} TXMLCFG_FP_FOCAL_LAW;

typedef struct TXMLCFG_PA_FOCAL_LAW
{
	CXMLParameter<int> pa_scan_type;
	CXMLParameter<int> pa_aperture_size;
	CXMLParameter<int> pa_start_element;
	CXMLParameter<int> pa_linear_end_element;
	CXMLParameter<int> pa_linear_step;
	CXMLParameter<float> pa_linear_angle;
	CXMLParameter<float> pa_angular_start_angle;
	CXMLParameter<float> pa_angular_end_angle;
	CXMLParameter<float> pa_angular_step;
	CXMLParameter<int> pa_focus_enabled;
	CXMLParameter<int> pa_reception_auto_focus;
	CXMLParameter<float> pa_emission_focal_point;
	CXMLParameter<float> pa_reception_focal_point;
	CXMLParameter<int> pa_global_parallel_lines;
	CXMLParameter<int> pa_increased_acquired_samples;
} TXMLCFG_PA_FOCAL_LAW;

typedef struct TXMLCFG_TFM_FOCAL_LAW
{
	CXMLParameter<int> tfm_global_parallel_lines;
	CXMLParameter<int> tfm_emission_start_element;
	CXMLParameter<int> tfm_emission_end_element;
	CXMLParameter<int> tfm_emission_step_element;
	CXMLParameter<int> tfm_receiver_lines_number;
	CXMLParameter<float> tfm_receiver_angle;
	CXMLParameter<float> tfm_receiver_start_distance;
	CXMLParameter<float> tfm_receiver_end_distance;
} TXMLCFG_TFM_FOCAL_LAW;

typedef struct TXMLCFG_PA_DSP
{
	CXMLParameter<int> pa_processing_type;
	CXMLParameter<int> pa_average_signals_number;
	CXMLParameter<int> pa_emi_signals_number;
	//CXMLParameter<int> pa_fir_enabled;
	//CXMLParameter<float> pa_fir_low_frequency;
	//CXMLParameter<float> pa_fir_high_frequency;
} TXMLCFG_PA_DSP;

typedef struct TXMLCFG_MC_DSP
{
	CXMLParameter<int> mc_processing_type;
	CXMLParameter<int> mc_average_signals_number;
	//CXMLParameter<int> mc_fir_enabled;
	//CXMLParameter<float> mc_fir_low_frequency;
	//CXMLParameter<float> mc_fir_high_frequency;
} TXMLCFG_MC_DSP;

typedef struct TXMLCFG_DSP
{
	CXMLParameter<int> fir_enabled;
	CXMLParameter<int> fir_low_frequency;
	CXMLParameter<int> fir_high_frequency;
} TXMLCFG_DSP;

typedef struct TXMLCFG_UTParameters
{
   CXMLParameter<int> acquisition_type;

   CXMLParameter<int> high_voltage_maximum;
   CXMLParameter<int> pulser_enabled;
   CXMLParameter<int> n_pulses;
   CXMLParameter<float> pulse_width;
   CXMLParameter<int> pulse_amplitude;

   CXMLParameter<float> prf_frequency;
   CXMLParameter<float> prf_burst_frequency;
   CXMLParameter<float> material_velocity;
   CXMLParameter<float> acq_start_distance;
   CXMLParameter<float> acq_end_distance;
   CXMLParameter<float> gain;
   CXMLParameter<float> delay_sync_time;
   
   // Phased Array Probe
   CXMLParameter<int> probe_n_element;
   CXMLParameter<float> probe_pitch;
   CXMLParameter<float> probe_frequency;

   // (Full Parallel) Focal Law
   CXMLParameter<int> fp_focal_law_type;
   CXMLParameter<float> fp_pwic_start_angle;
   CXMLParameter<float> fp_pwic_end_angle;
   CXMLParameter<float> fp_pwic_step_angle;
   CXMLParameter<unsigned long> fp_acq_number;
} TXMLCFG_UTParameters;

typedef struct TXMLCFG_TGC
{
   CXMLParameter<int> enabled;
   CXMLParameter<int> type;
   CXMLParameter<float> attenuation;
   CXMLParameter<vector<unsigned char> > curve;
} TXMLCFG_TGC;






typedef struct TXMLCFG_ACQUISITION
{
	//CXMLParameter<int> data_link_mode;
	CXMLParameter<int> acquisition_mode;
	CXMLParameter<float> delay_sync_time;
	CXMLParameter<int> trigger_source;
	CXMLParameter<int> pause_after_burst;
	CXMLParameter<int> n_bursts;
	CXMLParameter<int> n_images_per_burst;
} TXMLCFG_ACQUISITION;

typedef struct TXMLCFG_FUS
{
	CXMLParameter<float> central_frequency;
	CXMLParameter<float> rapport_focal_aperture;
	CXMLParameter<float> apodization_coefficient;
	CXMLParameter<float> lateral_position_x;
	CXMLParameter<float> pixel_size_x;
	CXMLParameter<int> pixels_x;
	CXMLParameter<float> depth_z;
	CXMLParameter<int> pixels_z;
	CXMLParameter<float> pixel_size_z;
	CXMLParameter<int> single_plane;
	CXMLParameter<int> samples_wavelength;
	CXMLParameter<int> interpolation_coefficients;
} TXMLCFG_FUS;

typedef struct TXMLCFG_VirtualChannel
{
	TXMLCFG_UT_PARAMETERS ut_parameters;
	TXMLCFG_PULSER pulser;
	TXMLCFG_PROBE probe;
	TXMLCFG_DSP dsp;
	TXMLCFG_FP_FOCAL_LAW fp_focal_law;
	TXMLCFG_MC_DSP fp_dsp;
	TXMLCFG_PA_FOCAL_LAW pa_focal_law;
	TXMLCFG_PA_DSP pa_dsp;
	TXMLCFG_TFM_FOCAL_LAW tfm_focal_law;
} TXMLCFG_VirtualChannel;

typedef struct TXMLCFG_Config
{
	TXMLCFG_ACQUISITION acq;
	TXMLCFG_FUS fus;
	vector<TXMLCFG_VirtualChannel> vch;
} TXMLCFG_Config;

// _____________________________________________________________________________
// ------ CXML_CONFIG class
// _____________________________________________________________________________
// =============================================================================
class CXML_CONFIG : public CXML
{
private:

	TXMLCFG_Config cfg_;

	int GetData_FUS(pugi::xml_node node_data, TXMLCFG_FUS* data);

	int GetData_ACQUISITION(pugi::xml_node node_data, TXMLCFG_ACQUISITION* data);

	int GetData_UT_PARAMETERS(pugi::xml_node node_data, TXMLCFG_UT_PARAMETERS* data);

	int GetData_PULSER(pugi::xml_node node_data, TXMLCFG_PULSER* data);

	int GetData_PROBE(pugi::xml_node node_data, TXMLCFG_PROBE* data);

	int GetData_DSP(pugi::xml_node node_data, TXMLCFG_DSP* data);

	int GetData_MC_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_FP_FOCAL_LAW* data);

	int GetData_MC_DSP(pugi::xml_node node_data, TXMLCFG_MC_DSP* data);

	int GetData_PA_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_PA_FOCAL_LAW* data);

	int GetData_PA_DSP(pugi::xml_node node_data, TXMLCFG_PA_DSP* data);
	   
	int GetData_TFM_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_TFM_FOCAL_LAW* data);

	int GetData_TGC (pugi::xml_node node_data, TXMLCFG_VirtualChannel* vch);

   //int GetData_UTParameters (pugi::xml_node node_data, TXMLCFG_VirtualChannel* vch);

   int LoadData(void);

   int Create_FUS(pugi::xml_node node_root, TXMLCFG_FUS* data);

   int Create_ACQUISITION(pugi::xml_node node_root, TXMLCFG_ACQUISITION* data);

   int Create_UT_PARAMETERS(pugi::xml_node node_root, TXMLCFG_UT_PARAMETERS* data);

   int Create_PA_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_PA_FOCAL_LAW* data);

   int Create_PULSER(pugi::xml_node node_root, TXMLCFG_PULSER* data);

   int Create_PROBE(pugi::xml_node node_root, TXMLCFG_PROBE* data);

   int Create_MC_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_FP_FOCAL_LAW* data);

   int Create_MC_DSP(pugi::xml_node node_root, TXMLCFG_MC_DSP* data);

   int Create_PA_DSP(pugi::xml_node node_root, TXMLCFG_PA_DSP* data);

   int Create_DSP(pugi::xml_node node_root, TXMLCFG_DSP* data);
     
   int Create_TFM_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_TFM_FOCAL_LAW* data);

   int Create_Version (pugi::xml_node node_root);

   //int GetData_PAFocalLaw (pugi::xml_node node_data, TXMLCFG_VirtualChannel* vch);

   //int GetData_DSP(pugi::xml_node node_data, TXMLCFG_VirtualChannel* vch);

   void ResetData(void);

public:

	CXML_CONFIG::CXML_CONFIG();
	   
   CXML_CONFIG(AnsiString file_name);

   CXML_CONFIG(vector<char>& file_buffer);

   CXML_CONFIG(TXMLCFG_Config* cfg);
   
   ~CXML_CONFIG() {};

   int Copy(TXMLCFG_Config* cfg);

   int CreateXMLFile(AnsiString file_name);
   
   int GetVirtualChannelNumber(void) {return (int)cfg_.vch.size();}
    
   TXMLCFG_Config* GetConfig(void) { return &cfg_; };

   TXMLCFG_VirtualChannel* GetVCHConfig(int id_vch);

   int SaveToFile(AnsiString file);
};

void XMLCFG_ResetData(TXMLCFG_VirtualChannel *xml_vch);

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

#endif
