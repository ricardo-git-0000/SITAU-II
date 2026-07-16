#ifndef _FOC_PARAMS_H_
#define _FOC_PARAMS_H_

#include "tad.h"
#include "t_base_tad.h"


typedef int D_0_t;
typedef int A_0_t;
typedef unsigned int R_0_t;
typedef unsigned int Z_0_t;
typedef unsigned int T_0_t;
typedef unsigned int E_0_t;

#define DEFINED_FLOAT_MUL_POW2(x, y) (x * pow(2,y))
#define DEFINED_SQRT(data) sqrt(data)


typedef struct init_word_s
{
	unsigned int words_32bit[INIT_WORD_32BIT_WORDS_NUMBER];
}init_word_t;

typedef struct foc_params_s
{
	D_0_t D_0;
	A_0_t A_0;
	R_0_t R_0;
	Z_0_t Z_0;
	T_0_t T_0;
	E_0_t E_0;
}foc_params_t;

int copy_init_word_to_buffer(u32 *buffer, init_word_t init_word, int offset, bool last, bool reset);

//2
void copy_foc_line_to_init_word(init_word_t *init_word,TFP_VCH_BASE_BeamformerParameters *foc_line);
int copy_init_word_to_buffer_tfm_forward(u32* buffer, init_word_t init_word, int offset);

void copy_foc_tfm_pwi_line_to_init_word(unsigned long* init_word, TTFM_PWI_ForwardBeamformerParameters* foc_line);
int copy_init_word_to_buffer_tfm_pwi_forward(u32* buffer, unsigned long init_word, int offset);

//1
int tfm_param_arrays(int n_rec_elements, int n_emi_elements, int n_lines, int acq_delay_samples, float* rx_mm, float* rz_mm, float* ex_mm, float* ez_mm, float* r_tk_us, float* e_tk_us, float* pix_mm, float* piz_mm, float* sin_ang, float* cos_ang, float c_mm_us, float fs_mhz, int* rec_disable, int* emi_disable, vector<vector<foc_params_s> >& emi_fl, vector<vector<foc_params_s> >& rec_fl);
int tfm_fmc_forward_parameters(vector<int> elements, int acq_delay_samples, vector<float>& ex_mm, vector<float>& ez_mm, vector<float>& pix_mm, vector<float>& piz_mm, vector<float>& sin_ang, vector<float>& cos_ang, float c_mm_us, float fs_mhz, vector<int>& disable, vector<vector<TFP_VCH_BASE_BeamformerParameters> >& beamformer);
int tfm_pwi_forward_parameters(float first_element_x, float first_element_z, float last_element_x, float last_element_z,
	vector<float> pwi_angles,
	int acq_delay_samples,
	vector<float>& pix_mm,
	vector<float>& piz_mm,
	float line_angle,
	float c_mm_us,
	float fs_mhz,
	vector<vector<TTFM_PWI_ForwardBeamformerParameters> >& forwardBeamformer,
	vector<TTFM_PWI_AnglesBeamformerParameters >& anglesBeamformer);
int tfm_backward_parameters(int n_elements, int acq_delay_samples, vector<float>& rx_mm, vector<float>& rz_mm, vector<float>& pix_mm, vector<float>& piz_mm, vector<float>& sin_ang, vector<float>& cos_ang, float c_mm_us, float fs_mhz, vector<int>& disable, vector<vector<TFP_VCH_BASE_BeamformerParameters> >& beamformer);

#endif //_FOC_PARAMS_H_
