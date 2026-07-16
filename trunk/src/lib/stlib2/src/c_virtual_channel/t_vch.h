#ifndef t_vchH
#define t_vchH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_base.h"
#include "st_info.h"
#include "c_pa_system.h"
#include "fl_scan_ui.h"
#include "fl_pa_ui.h"
#include "fl_specimen_ui.h"
#include "c_pulser.h"
#include "c_afe.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// Acquisition Types
#define ACQUISITION_TYPE_MC		0
#define ACQUISITION_TYPE_PA		1
#define ACQUISITION_TYPE_TFM		2
#define ACQUISITION_TYPE_PA_MC	3

// Beamforming Types
#define BEAMFORMING_PA		0
#define BEAMFORMING_TFM		1
#define BEAMFORMING_PWI		2

#define VCH_FP_FL_TYPE_FMC    0
#define VCH_FP_FL_TYPE_PWI    1
#define VCH_FP_FL_TYPE_USER   2

#define VCH_PA_FL_TYPE_LINEAR    0
#define VCH_PA_FL_TYPE_ANGULAR   1

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct T_VCH_TFM_FMC_FORWARD {
	int n_lword;
	vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
	u32 memory[TFM_FMC_FORWARD_MEMORY_SIZE];
} T_VCH_TFM_FMC_FORWARD;

typedef struct T_VCH_TFM_PWI_FORWARD {
	int n_lword;
	vector<vector<TTFM_PWI_ForwardBeamformerParameters> > forwardBeamformer;
	vector<TTFM_PWI_AnglesBeamformerParameters> anglesBeamformer;
	u32 forwardMemory[TFM_PWI_FORWARD_MEMORY_SIZE];
	u16 anglesMemory[MAX_PWI_FL];
} T_VCH_TFM_PWI_FORWARD;

typedef struct TFP_VCH_SysConfig {
   int id_pa_sys;
   CPASystem* pa_sys;
   int id_connector;

   C_PULSER* pulser;
   C_AFE* afe;
   TFP_VCH_TGC tgc;

   T_VCH_TFM_FMC_FORWARD tfm_fmc_forward;
   T_VCH_TFM_PWI_FORWARD tfm_pwi_forward;
   vector<CBASE_VirtualChannel*> base;

} TFP_VCH_SysConfig; 

typedef struct TFP_VCH_FIRFilter {
   bool update;
   float low_frequency;
   float high_frequency;
   float enabled;
   unsigned short factor;
   vector<short int> hw_coefficients;
   vector<float> coefficients_fft;
} TFP_VCH_FIRFilter;

typedef struct TFP_VCH_HWGate {
	bool enabled;
   int type;
	float start_distance; //distancia inicial de la puerta fijada por el usuario (mm)
	float end_distance; //distancia final de la puerta fijada por el usuario (mm)
	float width_distance;
	float start_time; //tiempo inicial de la puerta fijada por el usuario (µs)
	float end_time; //tiempo final de la puerta fijada por el usuario (µs)
	float width_time;
	float threshold_percent; //valor del umbral en porcentaje
   int threshold_ad;
	int start_sample; //muestra inicial de la puerta fijada por el usuario
	int end_sample; //muestra final de la puerta fijada por el usuario
	int width_samples;
} TFP_VCH_HWGate;

typedef struct TFP_VCH_ConfigData_SWGate {
	float start_distance; //distancia inicial de la puerta fijada por el usuario (µs)
	float end_distance; //distancia final de la puerta fijada por el usuario (µs)
	float start_time; //tiempo inicial de la puerta fijada por el usuario (µs)
	float end_time; //tiempo final de la puerta fijada por el usuario (µs)
	float threshold; //valor del umbral en porcentaje
	int start_sample; //muestra inicial de la puerta fijada por el usuario
	int end_sample; //muestra final de la puerta fijada por el usuario
	int threshold_reg;  //umbral de rechazo de la puerta expresado como entero entre
					//-MAXSHORT a MAXSHORT si estamos en RF o
					//0 a MAXSHORT si estamos en Video
	float threshold_position; //valor del umbral comenzando siempre en 0 a MAXSHORT
	int enabled;
	int type; //detección de máximo (maximo=1), minimo (maximo=0),
				//flanco positivo (maximo=2) ó flanco negativo (maximo=3)
	int distance_cte; // Indica que distancia permanece constante:
							// 0 --> Distancia de tiempo vuelo constante
							// 0 --> Profundidad constante
} TFP_VCH_ConfigData_SWGate;

typedef struct TFP_VCH_ConfigData_HWGate {
	float start_distance; //distancia inicial de la puerta fijada por el usuario (mm)
	float end_distance; //distancia final de la puerta fijada por el usuario (mm)
	float start_time; //tiempo inicial de la puerta fijada por el usuario (µs)
	float end_time; //tiempo final de la puerta fijada por el usuario (µs)
	float threshold; //valor del umbral en porcentaje
	int start_sample; //muestra inicial de la puerta fijada por el usuario
	int end_sample; //muestra final de la puerta fijada por el usuario
	int threshold_reg;  //umbral de rechazo de la puerta expresado como entero entre
					//-MAXSHORT a MAXSHORT si estamos en RF o
					//0 a MAXSHORT si estamos en Video
	float threshold_position; //valor del umbral comenzando siempre en 0 a MAXSHORT
	int enabled;
	int ctrl_linked;
	int ctrl_type;
	int ctrl_wpos;
	int ctrl_hwin;
	int ctrl_linked_gate;
	int ctrl_flk;
} TFP_VCH_ConfigData_HWGate;

typedef struct TFP_VCH_ConfigData_Encoder {
	int sign;
	int steps_mm;
	int steps_scan;
} TFP_VCH_ConfigData_Encoder;

typedef struct TFP_VCH_LoadConfigData {
   unsigned short n_element;
   float pitch;
   float frequency;
   float first_element_dist;
   float radius;

   int reduction_factor;
   int down_sampling_frequency;
} TFP_VCH_LoadConfigData;

typedef struct TFP_VCH_AcquisitionConfig {
   float acq_start_time;
   float acq_end_time;
   float acq_start_distance;
   float acq_end_distance;
   unsigned short n_acq_start_samples;
   unsigned short n_acq_samples;
   unsigned short n_beamformed_samples;
   unsigned short increased_acq_samples;

   vector<double> ascan_samples_position_time_double;
   vector<double> ascan_samples_position_distance_double;
   vector<float> ascan_samples_position_time_float;
   vector<float> ascan_samples_position_distance_float;

   vector<float> acq_samples_time_;
   vector<float> acq_samples_distance_;

   vector<float> beamformed_samples_time_;
   vector<float> beamformed_samples_distance_;
} TFP_VCH_AcquisitionConfig;

typedef struct T_VCH_EmitterFocalLaw {
   int emitter_enabled;
   float emitter_delay;
   unsigned short emitter_delay_ticks;
} T_VCH_EmitterFocalLaw;

typedef struct TFP_VCH_FocalLaw {
   int n_ascan;
   unsigned long image_size;
   unsigned long frame_size;
} TFP_VCH_FocalLaw;

typedef struct TPA_VCH_FocalLaw {
   unsigned long image_size_user;
   unsigned long image_size_hw;
   unsigned long frame_size;
   int parallel_lines;
   vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
} TPA_VCH_FocalLaw;

typedef struct TTFM_VCH {
	float start_x_distance;
	float end_x_distance;
	float start_x_time;
	float end_x_time;
	float angle;
	unsigned long image_size_user;
	unsigned long image_size_hw;
	unsigned long frame_size;
	int n_lines;
	int emi_start_element;
	int emi_end_element;
	int emi_step_element;
	vector<int> emi_elements;
	vector<int> rec_elements;
	int focal_laws_type;
	vector<float> pwic_angles;
	float pwic_start_angle;
	float pwic_step_angle;
	float pwic_end_angle;
} TTFM_VCH;


typedef struct TTFM_VCH_FocalLaw {
	unsigned long image_size_user;
	unsigned long image_size_hw;
	unsigned long frame_size;
	int parallel_lines;
	vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer;
} TTFM_VCH_FocalLaw;

typedef struct T_VCH_FIRFilter {
   bool enabled;
   float low_frequency;
   float high_frequency;
   unsigned short factor;
   vector<short int> hw_coefficients;
   vector<float> coefficients_fft;
} T_VCH_FIRFilter;

typedef struct TFP_VCH_ConfigData {
   AnsiString name;
   int id;
   bool enabled;
   bool config_update;

   TFP_VCH_AcquisitionConfig acq;

   T_VCH_FIRFilter fir;

   int probe_aperture_samples;

   unsigned long fp_acquisition_size;

   // Load Config
   TFP_VCH_LoadConfigData load_config;

   // Parámetros de adquisición - Pulser
   bool auto_pulse_width;

   // Parámetros de adquisición
   int acq_mode;
   int acq_time_mode;
   float prf_time;
   float prf_burst_time;
   int prf_enabled;
   int prf_burst_enabled;

   // Parámetros de adquisición - Ganancia
   float gain_att_db;
   float gain_pga_db;
   float gain_lna_db;
   float gain_db;

   // Parámetros de adquisición - DSP
   unsigned char processing_type;
   bool wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short parameter_processing_avr;
   unsigned short n_signals_processing_emi;

   // Parámetros de adquisición - TGC
   int digital_tgc_attenuator;
   int tgc_enabled;
   int tgc_type;
   float tgc_attn_coefficient;

   // Parámetros de adquisición - DSP
   float sampling_frequency;

   int acquisition_type;
   int signal_mode;
   unsigned long gtx_nfl_blocks;
   unsigned long gtx_n_blocks_burst;

   // (Matrix Capture) Focal laws
   int fp_focal_laws_type;
   vector<float> fp_pwic_angles;
   float fp_pwic_start_angle;
   float fp_pwic_step_angle;
   float fp_pwic_end_angle;
   vector<TFP_VCH_FocalLaw> fp_focal_law;
   vector <vector< T_VCH_EmitterFocalLaw > > mc_emission_focal_law;
   AnsiString fp_focal_law_file;
   unsigned char fp_processing_type;

   // (Phased Array) Leyes Focales
   CFL_ScanUI *scan;
   CFL_PAUI *pa_probe;
   CFL_SpecimenUI *specimen;
   TPA_VCH_FocalLaw pa_focal_law;
   vector <vector< T_VCH_EmitterFocalLaw > > pa_emission_focal_law;

   // (TFM)
   TTFM_VCH tfm;
   TPA_VCH_FocalLaw tfm_backward;
   vector <vector< T_VCH_EmitterFocalLaw > > tfm_emission_focal_law;

   vector<double> image_entry_point_double_y;
   vector<double> image_entry_point_double_z;
   vector<double> image_entry_point_double_angle;

   vector<float> image_entry_point_float_y;
   vector<float> image_entry_point_float_z;
   vector<float> image_entry_point_float_angle;

} TFP_VCH_ConfigData;

#endif
