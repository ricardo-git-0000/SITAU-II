#ifndef stfp_configH
#define stfp_configH

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

#include "st_reg.h"
#include "st_info.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define STCFG_iCAMPO          '['
#define STCFG_fCAMPO          ']'
#define STCFG_iCFG            '('
#define STCFG_fCFG            ')'
#define STCFG_SIZE            2
#define STCFG                 "ST"
#define STCFG_VERSION         "0000000000000004"
#define STCFG_VERSION_SIZE    16

#define STCFG_VERSION_HW      0
#define STCFG_VERSION_HW_PA  '1'

#define STCFG_SIZE_DATE       20

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TConfigData_Scan {
	float efocus;
	float rfocus;
	int en_focus;
	int en_emission_autofocus;

	int emission_scan_aperture;
	int reception_scan_aperture;
	int emission_start_element;
	int reception_start_element;
	int type;
	// Linear Scan
	int lin_end_element;
	int lin_step;
	float lin_angle;
	// Sectorial Scan
	float sec_start_angle;
	float sec_end_angle;
	float sec_step;
	int optimum_step;

	// Linear Sensitivity calibration
	vector<float> linear_sensitivity_gain;

	// Angular Sensitivity calibration
	vector<float> angular_sensitivity_gain;
	vector<float> angular_sensitivity_delay;

	vector<vector<int> > emission_aperture;
	vector<vector<int> > reception_aperture;
} TConfigData_Scan;

typedef struct TConfigData_SWGate {
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
} TConfigData_SWGate;

typedef struct TConfigData_HWGate {
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
} TConfigData_HWGate;

typedef struct TConfigData_Encoder {
	int sign;
	int steps_mm;
	int steps_scan;
} TConfigData_Encoder;

typedef struct TConfigData {
// Parámetros de la adquisición
	float gain;
	float acq_start_time;
	float acq_start_distance;
	float acq_end_time;
	float acq_end_distance;
	int acq_mode;
	float material_entrance_time;
	float offset_user_time;
	int prf_line;
	int interp_lagrange;
	int pulse_amplitude_code;
	int trigger_source;
	int cscan_type;
	int time_stamp_timer;

// Phased array transducer
	int pa_nelement;      //Número de elementos del array
	float pa_pitch;      //Distancia entre elementos contiguos del array medido
						//de centro a centro (mm)
	float pa_freq;      //Frecuencia central del array (MHz)
	float pa_fdist;   // Distancia desde el borde del array hasta el elemento inicial.
	float pa_radius;   // Radio de curvatura del transductor phased array (mm).
	vector<unsigned short> pa_disabled_element;
	vector<unsigned short> pa_pin_assignment;

// Filtro FIR
	float fir_low_frequency;
	float fir_high_frequency;
	int fir_enabled;

// DSP
	int dsp_type;
	int dsp_average_number;
	int dsp_emi_number;

// Wedge
	float wdg_velocity;
	float wdg_height;
	float wdg_pa_dist;
	float wdg_width;
	float wdg_circular_radius;
	float wdg_circular_x_center;
	float wdg_circular_z_center;
	float wdg_angle;
	int wdg_enabled;

// Specimen
	float spc_velocity;

// Scan
	int ddf_enabled;
   int pci_enabled;
   float pci_factor;
	vector<TConfigData_Scan> scan;

// SW Gate
	vector<TConfigData_SWGate> sw_gate;

// HW Gate
	vector<TConfigData_HWGate> hw_gate;

// Coeficientes de apodización (calibración sensibilidad de los elementos)
	vector<float> ap_element;

// Parámetros de la curva CAT
	int cat_enabled;
	vector<float> cat_gain;
	vector<float> cat_time;

// Parámetros relaccionados con los encoders
	int enc_trigger;
	vector<TConfigData_Encoder> enc;

} TConfigData;

void ST_UpdateConfig (void);

int ST_SetConfig(void);

int ST_LoadXMLBufferConfig(vector<char>& file_buffer);

//Estructuras de memoria temporales para la carga de parámetros desde fichero:
extern bool gb_cfg_change_info;

#endif
