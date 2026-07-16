#ifndef stfp_loadH
#define stfp_loadH
// ______________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ======================================================================
#include "tad.h"  //Definición de los tipos de datos

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <Math.hpp>
#endif

using namespace std;

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ======================================================================

// CONFIGURACION POR DEFECTO
#define DCFG_GAIN       36.     // dB
#define DCFG_VEL        1540    // m/s
#define DCFG_DINI       0.      // mm
#define DCFG_DFIN       40.    // mm
#define DCFG_BL_EINI    0
#define DCFG_BL_EFIN    127
#define DCFG_BL_STP     1
#define DCFG_BL_APT     32
#define DCFG_PA_NE      128
#define DCFG_PA_FRQ     5.      // MHz
#define DCFG_PA_D       0.      // mm
#define DCFG_PA_PITCH   0.5     // mm
#define DCFG_PA_RADIUS	0			// mm
#define DCFG_AUTO_FREQ  0       // mm
#define DCFG_AUTO_PULSE 0       // mm
#define DCFG_NWEDGE     0
#define DCFG_WEDGE_EN	0
#define DCFG_WEDGE_HEI  10      // mm
#define DCFG_WEDGE_WID  100     // mm
#define DCFG_WEDGE_DIS  0       // mm
#define DCFG_WEDGE_ANG  0       // º
#define DCFG_WEDGE_RAD  0       // mm
#define DCFG_WEDGE_X_CENTER  0       // mm
#define DCFG_WEDGE_Z_CENTER  0       // mm
#define DCFG_WEDGE_VEL  2700	// m/s
#define DCFG_DF_EMI     18.     // mm
#define DCFG_DF_REC     18.     // mm
#define DCFG_DF_EN      1
#define DCFG_PRF        3500    // Hz
#define DCFG_OFFTIME    0.      // µs
#define DCFG_PUL_N      1
#define DCFG_PUL_W      50.    // µs
#define DCFG_ADQ_MODE   1       // VIDEO
#define DCFG_SGN_TST    0
#define DCFG_FIR_LOW    0.      // KHz
#define DCFG_FIR_HIGH   20000.  // KHz
#define DCFG_FIR_EN     0
#define DCFG_DSP_TYPE   0
#define DCFG_DSP_NREP   1
#define DCFG_REDU       0
#define DCFG_ENC        50      // pulsos
#define DCFG_GATE_DINI  5.     // mm
#define DCFG_GATE_DFIN  15.     // mm
#define DCFG_GATE_THR   20.     // %
#define DCFG_GATE_EN    0
#define DCFG_SAD_Velocity  1490 // m/s
#define DCFG_SAD_StartRange  0  // mm
#define DCFG_SAD_EndRange     40  // mm

// ______________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ======================================================================

// ______________________________________________________________________
// ------ P R O T O T I P O S   D E   F U N C I O N E S -----------------
// ======================================================================

int ST_LoadPulser (int npulses, float wpulse, int auto_width=0);

int ST_LoadPRF (float data, int id_virtual_channel);

int ST_LoadPRFBurst(float data, int id_virtual_channel);

int ST_LoadSignalTest(int data);

int ST_LoadDisabledChannel (int disabled, int channel);

#endif
