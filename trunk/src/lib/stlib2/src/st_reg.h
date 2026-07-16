#ifndef stfp_regH
#define stfp_regH

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

#include <time.h>

// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ======================================================================

#if defined(_MSC_VER)
   #include "c_critical_section.h"
#else
   #include <SyncObjs.hpp>
#endif
//#include "c_sitau_hw.h"
#include "c_xml_config.h"
#include "c_log_message.h"

#define FILENAME_BASE         "MK32CH_basics_wrapper.bin"
#define FILENAME_CPU1         "BOOT.bin"//"boot_cpu1.bin"

using namespace std;

// ______________________________________________________________________
// ------ T I P O S   D E   D A T O S
// ======================================================================

#define m_HIGH_VOLTAGE_TO_DA  1.5785
#define b_HIGH_VOLTAGE_TO_DA  -1.7223
#define HIGH_VOLTAGE_DA_MAX   255

#define SIZE_BUFFER_SWAP	  1024*1024*10

#define DLL_SW_GATE_ID	4
#define SW_NUM_GATES		3

#define SCAN_BLIN   0
#define SCAN_BSEC   1

#define ACQ_PulseEcho      0
#define ACQ_Transmission   1
#define ACQ_PitchCatch     2

#if !defined(_GETHERNET)
   #define DELAY_LOADFIRMWARE    100
#endif
#define N_CONTROL_WORD_READ   10

#define UT_THREAD_STATUS_FILE "ThreadStatus.TXT"

// First Gate Linked Type
#define FGL_StartWidthFixed   0
#define FGL_StartEndFixed     1
#define FGL_EndWidthFixed     2
#define FGL_EndStartFixed     3

//Visualizaciones de onda
#define RF    0
#define VIDEO 1
#define HWp   2
#define HWn   3
#define FW    4

//Tipo de fuentes de disparo
#define TRG_SCAN_PRF    0
#define TRG_SCAN_EXT    1
#define TRG_SCAN_ENC    2

//Tipos de procesamiento
#define PROC_NONE 		0	//Se opera normalmente
#define PROC_AVERAGE    1	//Se hace el promediado
#define PROC_EMI        2	//Se aplica el filtro EMI

//SISTEMA.-
#define NWEDGE_MAX 2
#define STARTELEMENT_MAX 96

//Relojes
#define FCLK      62.5//50.   // MHz
#define PCLK      0.016//0.02  // us
#define FCLK4_MHz 250.//200.  // MHz
#define PCLK4_ns  4.//5.    // ns

//Límites en el número de muestras
#define MAX_NMU_OUT 20478
#define MIN_NMU_OUT 31

// Acquisition Modes
#define ACQUISITION_MODE_FIXED		0
#define ACQUISITION_MODE_CONTINOUS		1
#define MIN_ACQUISITIONS_MODE_CONTINOUS		10

// Data Link Modes
#define DATA_LINK_MODE_GE		0
#define DATA_LINK_MODE_GTX		1
#define DATA_LINK_MODE_GE_GTX	2

#define GTX_SIZE_BLOCK  ((1<<23) - 32)
#define GTX_SIZE_HEADER	32


#define N_ENCODERS	4

typedef struct T_FUS {
	float central_frequency;
	float rapport_focal_aperture;
	float apodization_coefficient;
	float lateral_position_x;
	float pixel_size_x;
	int pixels_x;
	float depth_z;
	int pixels_z;
	float pixel_size_z;
	int single_plane;
	int samples_wavelength;
	int interpolation_coefficients;
} T_FUS; 

typedef struct TUCIP {
	int trigger_source;   //Fuente de disparo (TRG_SW, TRG_SCAN_ENC, TRG_SCAN_SW)
	int user_pause_after_burst;
	int pause_after_burst;
	int auto_continue_mode;
	unsigned long trigger_prf_time;
	unsigned long trigger_acq_number;
	unsigned long n_bursts;
	unsigned long n_images_burst;
	float delay_sync_time;
	int delay_sync_n_acq;
	unsigned short acquisition_mode;
	int status_enabled;
	int status_period_ms;
	int status_pcie;
} TUCIP;

#define ST_MIN_PRFLIN   200//1000000. / (65535. * 0.05) // con el reloj de la UCI de 50 ns
#define ST_MAX_PRFLIN   20000 // 20 HKz


#if defined(_NO_HW)
   #define ST_MIN_PA_Frequency      0.001 // MHz
#else
   #define ST_MIN_PA_Frequency      0.25 // MHz
#endif

typedef struct TstVAR {
	bool hw_iniciado;  //ST_OpenSys terminada OK y no se ha llamado a ST_CloseSys
	bool hw_conectado; //ST_ConfigSys terminada OK y no se ha llamado a ST_CloseSys
	int high_voltage;
} TstVAR;

#define MIN_GAIN	0
#define MAX_GAIN	100	//Más ganancia al incluir el registro WGD

extern vector<unsigned char> gb_high_voltage_to_da;

extern bool gb_no_hw;
extern AnsiString gb_st_file_path;
extern char gb_msg_progress_open_sys[255];
extern int gb_progress_open_sys;
extern int gb_progress_load_config;
extern char gb_msg_progress_load_acquisition[255];
extern int gb_progress_load_acquisition;
extern int gb_selected_hw_gate;
extern int gb_hw_sitau_enabled;

extern TUCIP UCIP;
extern TstVAR stVAR;

extern int gb_opened_system;

extern int gb_log_config_enabled;
extern int gb_log_system_info_enabled;
extern int gb_load_uci_firmware;
extern bool gb_no_hardware_waiting_continue;
extern bool gb_no_hardware_waiting_continue_previous;

extern int ErrorLoadFirmware;

extern bool gb_acquisition_process_stoped;
extern int gb_save_data_log;
extern T_FUS gb_fus;
extern CXML_CONFIG *gb_xml_file;
extern bool gb_hw_format;

int ST_OpenHW (char* path, char *ip_address, int tcp_port_cfg, int tcp_port_acq);

int ST_PRG (bool update=false);

void ST_Release (void);

int ST_StarAcquireProcess (void);

int ST_LoadFirmware(char *file_name);

int ST_UpdateStatus(void);

#endif
