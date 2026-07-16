#ifndef t_base_tadH
#define t_base_tadH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

#include "tad.h"

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "xil_types.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define MAX_N_BASES           8     //!< Número máximo de módulos
#define MOD_MAX_CH            32    //!< Número máximo de canales por módulo
#define MOD_MAX_FOCAL_LAW     1024  //!< Número máximo de leyes focales por módulo
#define TGC_MAX_SIZE          2048  //!< Número máximo de puntos d ela curva TGC

//Número de coeficientes del filtro pasa banda
#define FIR_N_COEFFICIENTS							63
//Número de bits de los coeficientes del filtro pasa banda
#define BASE_BITS_CPB 16

#define FP_BASE_MAX_FOCAL_LAW          1024
#define FP_BASE_MAX_NPUL               255
//#define FP_MAX_N_CHANNELS              MOD_MAX_CH * MAX_N_BASES

#define CEILING(x,y) (((x) + (y) - 1) / (y))

#define LOG_2_NU	2

typedef float delta_r_inv_t;

typedef float freq_t;

#define C_T_INT_WIDTH	14
#define C_A_INT_WIDTH	(C_T_INT_WIDTH - 1)
#define C_A_D_PRECISION	6
#define C_Z_WIDTH		11
#define C_EXTRA			3
#define C_F_WIDTH		16


#define D_0_WIDTH	(C_T_INT_WIDTH+C_A_D_PRECISION)
#define A_0_WIDTH	(C_A_INT_WIDTH+C_A_D_PRECISION)
#define	R_0_WIDTH	(C_T_INT_WIDTH+LOG_2_NU-1)
#define Z_0_WIDTH	(C_Z_WIDTH)
#define T_0_WIDTH	(C_T_INT_WIDTH+LOG_2_NU)
#define E_0_WIDTH	(C_EXTRA)

#define D_0_OFFSET	0
#define A_0_OFFSET	(D_0_WIDTH)
#define	R_0_OFFSET	(A_0_WIDTH+D_0_WIDTH)
#define Z_0_OFFSET	(R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)
#define T_0_OFFSET	(Z_0_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)
#define E_0_OFFSET	(T_0_WIDTH+Z_0_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)


#define C_INIT_WORD_WIDTH			(E_0_WIDTH+T_0_WIDTH+C_Z_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)

#define C_PARAM_BRAM_DATA_WIDTH		36
#define C_WRITE_INTERFACE_WIDTH		32

#define C_MINIMUM_BRAM_DEEP			512
#define C_NUM_BEAMFORMERS			1

#define MAX_PROCESSING_ELEM			32
#define MAX_LINE					512
#define C_N_CHANNELS				(MAX_PROCESSING_ELEM)
#define C_TOTAL_LINES				(MAX_LINE)
#define C_LINES_PER_BEAMFORMER		(C_TOTAL_LINES/C_NUM_BEAMFORMERS)
#define C_N_INIT_SHARED_CH			(C_MINIMUM_BRAM_DEEP/C_LINES_PER_BEAMFORMER)
#define C_N_PARALLEL_INIT			(C_N_CHANNELS/C_N_INIT_SHARED_CH)
#define C_USED_BITS					(C_N_PARALLEL_INIT*C_INIT_WORD_WIDTH)
#define	FLOOR_N_BRAMS_BEAMFORMER	((int)(C_USED_BITS/C_PARAM_BRAM_DATA_WIDTH))
#define ZERO_BRAM_WASTE_BITS		((FLOOR_N_BRAMS_BEAMFORMER*C_PARAM_BRAM_DATA_WIDTH)==C_USED_BITS?1:0)
#define C_N_BRAMS_BEAMFORMER		(ZERO_BRAM_WASTE_BITS?FLOOR_N_BRAMS_BEAMFORMER:FLOOR_N_BRAMS_BEAMFORMER+1)
#define C_WASTED_BITS				((C_N_BRAMS_BEAMFORMER*C_PARAM_BRAM_DATA_WIDTH)-C_USED_BITS)
#define FLOOR_TRANSF_PER_USED_BITS	((int)(C_USED_BITS/C_WRITE_INTERFACE_WIDTH))
#define ZERO_TRANSF_WASTE_BITS		((FLOOR_TRANSF_PER_USED_BITS*C_WRITE_INTERFACE_WIDTH)==C_USED_BITS?1:0)
#define C_N_TRANSF					(ZERO_TRANSF_WASTE_BITS?FLOOR_TRANSF_PER_USED_BITS:FLOOR_TRANSF_PER_USED_BITS+1)
#define C_TRANSF_BITS				(C_N_TRANSF*C_WRITE_INTERFACE_WIDTH)
#define C_WASTED_TRANSF				((C_TRANSF_BITS)-C_USED_BITS)
#define C_TRANSF_WORDS_BASE			(C_N_TRANSF*C_TOTAL_LINES)
//#define C_TOTAL_TRANSF_WORDS		(C_TRANSF_WORDS_BASE*MAX_N_BASES)
#define MAX_BITS_SEND_PER_WORD		((C_INIT_WORD_WIDTH+C_WRITE_INTERFACE_WIDTH-1))
#define FLOOR_MAX_TRANSF_PER_WORD	((int)MAX_BITS_SEND_PER_WORD/C_WRITE_INTERFACE_WIDTH)
#define CEIL_MAX_TRANSF_PER_WORD	((FLOOR_MAX_TRANSF_PER_WORD*C_WRITE_INTERFACE_WIDTH)==MAX_BITS_SEND_PER_WORD?FLOOR_MAX_TRANSF_PER_WORD:FLOOR_MAX_TRANSF_PER_WORD+1)
#define MAX_PARAM_WORDS				(CEILING(C_INIT_WORD_WIDTH,32)*MAX_LINE*MOD_MAX_CH)

#define INIT_WORD_32BIT_WORDS_NUMBER	CEILING(C_INIT_WORD_WIDTH,32)

#define FP_BASE_BEAMFORMER_MEMORY_SIZE C_TRANSF_WORDS_BASE//84 * FP_BASE_MAX_FOCAL_LAW//016 * 4//2700 * FP_BASE_MAX_FOCAL_LAW / 32;

//En teoria es max(4,INIT_WORD_32BIT_WORDS_NUMBER), pero INIT_WORD_32BIT_WORDS_NUMBER es 3 y está lejos de aumentar
#define INIT_WORD_TFM_FORWARD_32B_BEATS	4
// ---------------------------------------------------

#define BEAMFORMER_SIZE_FMC_FWD_WORD32	4
#define BEAMFORMER_SIZE_PWI_FWD_WORD32	1
#define MAX_PWI_FL	128
#define TFM_FMC_FORWARD_MEMORY_SIZE (BEAMFORMER_SIZE_FMC_FWD_WORD32*MAX_LINE*MAX_PROCESSING_ELEM*MAX_N_BASES)
#define TFM_PWI_FORWARD_MEMORY_SIZE	(BEAMFORMER_SIZE_PWI_FWD_WORD32*MAX_PWI_FL*MAX_LINE)
#define TFM_PWI_COS_ANG_MEMORY_SIZE (MAX_PWI_FL)

// ---------------------------------------------------

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TFP_VCH_BASESysConfig {
   int id_base_sitau;
   unsigned char base_amplia_address;
   bool multiplexed;
} FP_VCH_BASESysConfig;

#define TGC_MAX_SIZE 2048
typedef struct TFP_VCH_TGC
{
	unsigned short n_points;
   unsigned char curve[TGC_MAX_SIZE];
} TFP_VCH_TGC;

typedef struct T_VCH_BASE_PA_EmissionFocalLaws {
   unsigned short n_fl;
   vector<vector<unsigned short> > delay;
} T_VCH_BASE_PA_EmissionFocalLaws;

typedef struct T_VCH_BASE_FP_EmissionFocalLaws {
   unsigned short n_fl;
   vector<unsigned short> delay;
} T_VCH_BASE_FP_EmissionFocalLaws;

typedef struct TFP_VCH_BASE_BeamformerParameters
{
	long D_0;
	long A_0;
	unsigned long R_0;
	unsigned long Z_0;
	unsigned long T_0;
	unsigned long E_0;
} TFP_VCH_BASE_BeamformerParameters;

typedef struct TTFM_PWI_ForwardBeamformerParameters
{	
	short F_0; 
	unsigned short T_0;
} TTFM_PWI_ForwardBeamformerParameters;

typedef struct TTFM_PWI_AnglesBeamformerParameters
{
	unsigned short COS_PWI;
} TTFM_PWI_AnglesBeamformerParameters;


typedef struct T_VCH_BASE_Beamformer {
   unsigned short n_fl;
   unsigned short n_fl_parallel; // 8, 16, 32, 64
   //unsigned short n_fl_hw;
   unsigned long t0_max;
   int n_lword;
	int shift_bits;
   vector<vector<TFP_VCH_BASE_BeamformerParameters> > bf_parameters; // size = n_fl * 32
   u32 bf_memory[FP_BASE_BEAMFORMER_MEMORY_SIZE];
} T_VCH_BASE_Beamformer;

typedef struct T_VCH_BASE_FP_FocalLaws {
   T_VCH_BASE_FP_EmissionFocalLaws efl;
} T_VCH_BASE_FP_FocalLaws;

typedef struct T_VCH_BASE_TFMParameters
{
	long D_0;
	long A_0;
	unsigned long R_0;
	unsigned long Z_0;
	unsigned long T_0;
	unsigned long E_0;
} T_VCH_BASE_TFMParameters;

typedef struct T_VCH_BASE_TFM {
	int n_lword;
	//vector<vector<T_VCH_BASE_TFMParameters> > forward_parameters;
	vector<vector<T_VCH_BASE_TFMParameters> > backward_parameters;
	u32 backward_memory[FP_BASE_BEAMFORMER_MEMORY_SIZE];
	//u32 forward_memory[FP_BASE_BEAMFORMER_MEMORY_SIZE];
} T_VCH_BASE_TFM;

typedef struct TFP_VCH_BASE {
	T_VCH_BASE_FP_FocalLaws fp_fl;
	T_VCH_BASE_Beamformer beamformer;
	T_VCH_BASE_Beamformer tfm_backward;
	//T_VCH_BASE_TFM tfm;
} TFP_VCH_BASE;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ P R O T O T I P O   D E   F U N C I O N E S
// =============================================================================

#endif
