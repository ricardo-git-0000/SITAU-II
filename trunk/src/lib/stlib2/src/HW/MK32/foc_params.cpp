#include "foc_params.h"
#include "c_log_file.h"
#include "utils.h"

#define v_c ((float)(1<<LOG_2_NU))
#define C_Z0_CALC_CONST	(((float)v_c/2-1)/sqrt((float)v_c-1))
#define C_LOG2_D_2_BY_V	(1-LOG_2_NU)
#define PI       3.14159265358979323846   // pi

static const int 	v				= (int)v_c;
static const float	z0_const		= C_Z0_CALC_CONST;
static const int	log2_d_2_by_v	= 1-LOG_2_NU;
static const D_0_t	d_0_const_fp	= (D_0_t) (-1+2/v_c-1.0/v_c/v_c);
static const float	d_0_const_float	= (float) (-1+2/v_c-1.0/v_c/v_c);

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
foc_params_s tfm_param(float rx, float rz, float tk, float pix, float piz, float sin_ang, float cos_ang, float c_inv, int retardo_adq_muestras, float fs, bool disable)
{
	foc_params_t params;
	float trans_x, trans_z;
	//float v;
	float K, alfa;
	float R_0_f;
	float Z_0_f;
	float T_0_f;

	params.E_0 = (disable ? 1 : 0);
	if (disable)
	{
		params.D_0 = 0;
		params.A_0 = 0;
		params.R_0 = 0;
		params.Z_0 = 0;
		return params;
	}
	trans_x = (rx - pix) * 2 * c_inv * fs;
	trans_z = (rz - piz) * 2 * c_inv * fs;

	Z_0_f = (trans_x * sin_ang - trans_z * cos_ang + fabs(trans_x * cos_ang + trans_z * sin_ang) * z0_const);
	Z_0_f = (Z_0_f > 0 ? Z_0_f : 0);
	params.Z_0 = (Z_0_t)ceil(Z_0_f);
	// En caso de haber Z0, pi(pix,piz) se "avanza" z0, porque r0 no puede empezar tan pronto. Es equivalente a restar (z0*sin,-z0cos)
	trans_x = trans_x - ((int)params.Z_0) * sin_ang;
	trans_z = trans_z + ((int)params.Z_0) * cos_ang;

	alfa = trans_x * sin_ang - trans_z * cos_ang;
	K = 1 - DEFINED_FLOAT_MUL_POW2(alfa, 1);


	R_0_f = DEFINED_SQRT(trans_x * trans_x + trans_z * trans_z);
	params.R_0 = (R_0_t)roundf(DEFINED_FLOAT_MUL_POW2((float)R_0_f, LOG_2_NU - 1));
	params.A_0 = (A_0_t)(DEFINED_FLOAT_MUL_POW2((K - DEFINED_FLOAT_MUL_POW2(R_0_f, 1)), C_A_D_PRECISION));
	params.D_0 = (D_0_t)(params.A_0 + (DEFINED_FLOAT_MUL_POW2((R_0_f * R_0_f) - DEFINED_FLOAT_MUL_POW2((float)(params.R_0 * params.R_0), -2 * (LOG_2_NU - 1)) + (DEFINED_FLOAT_MUL_POW2(R_0_f, log2_d_2_by_v)) + d_0_const_float, C_A_D_PRECISION)));

	//params.T_0 = (T_0_u32_t)roundf(DEFINED_FLOAT_MUL_POW2((float)DEFINED_FLOAT_MUL_POW2((R_0_f + tk * c_inv), -1), LOG_2_NU));
	T_0_f = ((R_0_f / 2 + tk * fs) - retardo_adq_muestras);
	if (T_0_f < 0) { ELOG("WARNING: T0 < 0", -1); }

	params.T_0 = (T_0_t)roundf(DEFINED_FLOAT_MUL_POW2(T_0_f,LOG_2_NU)); // Aquí no estoy seguro si tk debería dividirse entre dos. acq_delay_samples he comprobado que no se divide entre dos, pero ahora mismo no entiendo porqué
	/*
	tmp1=(float)((R_0_f*R_0_f));
	tmp2=(float)((-DEFINED_FLOAT_MUL_POW2((float)(params.R_0*params.R_0),-2*(LOG_2_NU-1))));
	tmp3=(float)((DEFINED_FLOAT_MUL_POW2(R_0_f,log2_d_2_by_v)));
	tmp4=(float)((d_0_const_float));
	tmp5=(float)(R_0_f*R_0_f)-DEFINED_FLOAT_MUL_POW2(((float)params.R_0*(float)params.R_0),-2*(LOG_2_NU-1))+DEFINED_FLOAT_MUL_POW2(R_0_f,log2_d_2_by_v)+d_0_const_float;
	tmp55=(float)tmp1+tmp2;
	tmp6=(float)DEFINED_FLOAT_MUL_POW2((float)params.A_0,-C_A_D_PRECISION);
	tmp7=(float)DEFINED_FLOAT_MUL_POW2((float)params.D_0,-C_A_D_PRECISION);
	*/

	return params;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TFP_VCH_BASE_BeamformerParameters tfm_parameters(float rx, float rz, float tk, float pix, float piz, float sin_ang, float cos_ang, float c_inv, int retardo_adq_muestras, float fs, bool disable)
{
	TFP_VCH_BASE_BeamformerParameters p;
	float trans_x, trans_z;
	float K, alfa;
	float R_0_f;
	float Z_0_f;
	float T_0_f;

	p.E_0 = (disable ? 1 : 0);
	if (disable)
	{
		p.D_0 = 0;
		p.A_0 = 0;
		p.R_0 = 0;
		p.Z_0 = 0;
		return p;
	}
	trans_x = (rx - pix) * 2 * c_inv * fs;
	trans_z = (rz - piz) * 2 * c_inv * fs;

	Z_0_f = (trans_x * sin_ang - trans_z * cos_ang + fabs(trans_x * cos_ang + trans_z * sin_ang) * z0_const);
	Z_0_f = (Z_0_f > 0 ? Z_0_f : 0);
	p.Z_0 = (Z_0_t)ceil(Z_0_f);
	// En caso de haber Z0, pi(pix,piz) se "avanza" z0, porque r0 no puede empezar tan pronto. Es equivalente a restar (z0*sin,-z0cos)
	trans_x = trans_x - ((int)p.Z_0) * sin_ang;
	trans_z = trans_z + ((int)p.Z_0) * cos_ang;

	alfa = trans_x * sin_ang - trans_z * cos_ang;
	K = 1 - DEFINED_FLOAT_MUL_POW2(alfa, 1);


	R_0_f = DEFINED_SQRT(trans_x * trans_x + trans_z * trans_z);
	p.R_0 = (R_0_t)roundf(DEFINED_FLOAT_MUL_POW2((float)R_0_f, LOG_2_NU - 1));
	p.A_0 = (A_0_t)(DEFINED_FLOAT_MUL_POW2((K - DEFINED_FLOAT_MUL_POW2(R_0_f, 1)), C_A_D_PRECISION));
	p.D_0 = (D_0_t)(p.A_0 + (DEFINED_FLOAT_MUL_POW2((R_0_f * R_0_f) - DEFINED_FLOAT_MUL_POW2((float)(p.R_0 * p.R_0), -2 * (LOG_2_NU - 1)) + (DEFINED_FLOAT_MUL_POW2(R_0_f, log2_d_2_by_v)) + d_0_const_float, C_A_D_PRECISION)));

	T_0_f = ((R_0_f / 2 + tk * fs) - retardo_adq_muestras);
	if (T_0_f < 0) { ELOG("WARNING: T0 < 0", -1); }

	p.T_0 = (T_0_t)roundf(DEFINED_FLOAT_MUL_POW2(T_0_f, LOG_2_NU)); // Aquí no estoy seguro si tk debería dividirse entre dos. acq_delay_samples he comprobado que no se divide entre dos, pero ahora mismo no entiendo porqué
	return p;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TFP_VCH_BASE_BeamformerParameters tfm_pwi_parameters(float rx, float rz, float tk, float pix, float piz, float sin_ang, float cos_ang, float c_inv, int retardo_adq_muestras, float fs, bool disable)
{
	TFP_VCH_BASE_BeamformerParameters p;
	float trans_x, trans_z;
	float K, alfa;
	float R_0_f;
	float Z_0_f;
	float T_0_f;

	p.E_0 = (disable ? 1 : 0);
	if (disable)
	{
		p.D_0 = 0;
		p.A_0 = 0;
		p.R_0 = 0;
		p.Z_0 = 0;
		return p;
	}
	trans_x = (rx - pix) * 2 * c_inv * fs;
	trans_z = (rz - piz) * 2 * c_inv * fs;

	Z_0_f = (trans_x * sin_ang - trans_z * cos_ang + fabs(trans_x * cos_ang + trans_z * sin_ang) * z0_const);
	Z_0_f = (Z_0_f > 0 ? Z_0_f : 0);
	p.Z_0 = (Z_0_t)ceil(Z_0_f);
	// En caso de haber Z0, pi(pix,piz) se "avanza" z0, porque r0 no puede empezar tan pronto. Es equivalente a restar (z0*sin,-z0cos)
	trans_x = trans_x - ((int)p.Z_0) * sin_ang;
	trans_z = trans_z + ((int)p.Z_0) * cos_ang;

	alfa = trans_x * sin_ang - trans_z * cos_ang;
	K = 1 - DEFINED_FLOAT_MUL_POW2(alfa, 1);


	R_0_f = DEFINED_SQRT(trans_x * trans_x + trans_z * trans_z);
	p.R_0 = (R_0_t)roundf(DEFINED_FLOAT_MUL_POW2((float)R_0_f, LOG_2_NU - 1));
	p.A_0 = (A_0_t)(DEFINED_FLOAT_MUL_POW2((K - DEFINED_FLOAT_MUL_POW2(R_0_f, 1)), C_A_D_PRECISION));
	p.D_0 = (D_0_t)(p.A_0 + (DEFINED_FLOAT_MUL_POW2((R_0_f * R_0_f) - DEFINED_FLOAT_MUL_POW2((float)(p.R_0 * p.R_0), -2 * (LOG_2_NU - 1)) + (DEFINED_FLOAT_MUL_POW2(R_0_f, log2_d_2_by_v)) + d_0_const_float, C_A_D_PRECISION)));

	T_0_f = ((R_0_f / 2 + tk * fs) - retardo_adq_muestras);
	if (T_0_f < 0) { ELOG("WARNING: T0 < 0", -1); }

	p.T_0 = (T_0_t)roundf(DEFINED_FLOAT_MUL_POW2(T_0_f, LOG_2_NU)); // Aquí no estoy seguro si tk debería dividirse entre dos. acq_delay_samples he comprobado que no se divide entre dos, pero ahora mismo no entiendo porqué
	return p;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int tfm_param_arrays(int n_rec_elements, int n_emi_elements, int n_lines, int acq_delay_samples, float* rx_mm, float* rz_mm, float* ex_mm, float* ez_mm, float* r_tk_us, float* e_tk_us, float* pix_mm, float* piz_mm, float* sin_ang, float* cos_ang, float c_mm_us, float fs_mhz, int* rec_disable, int* emi_disable, vector<vector<foc_params_s> > & emi_fl, vector<vector<foc_params_s> >& rec_fl)
{
	int elem, line;
	float c_inv = 1 / c_mm_us;
	vector<foc_params_s> params;

	rec_fl.clear();
	for (line = 0; line < n_lines; line++)
	{
		params.clear();
		for (elem = 0; elem < n_rec_elements; elem++)
		{
			foc_params_s foc_param_tmp;
			foc_param_tmp = tfm_param(rx_mm[elem], rz_mm[elem], r_tk_us == NULL ? 0 : r_tk_us[line * n_rec_elements + elem], pix_mm[line], piz_mm[line], sin_ang[line], cos_ang[line], c_inv, acq_delay_samples, fs_mhz, rec_disable[elem]==1);
			params.push_back(foc_param_tmp);
		}
		rec_fl.push_back(params);
	}

	emi_fl.clear();
	for (elem = 0; elem < n_emi_elements; elem++)
	{
		params.clear();
		for (line = 0; line < n_lines; line++)
		{
			foc_params_s foc_param_tmp;
			foc_param_tmp = tfm_param(ex_mm[elem], ez_mm[elem], e_tk_us == NULL ? 0 : e_tk_us[line * n_emi_elements + elem], pix_mm[line], piz_mm[line], sin_ang[line], cos_ang[line], c_inv, acq_delay_samples, fs_mhz, emi_disable[elem]==1);
			params.push_back(foc_param_tmp);
		}
		emi_fl.push_back(params);
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int tfm_fmc_forward_parameters(vector<int> elements, int acq_delay_samples, vector<float> &ex_mm, vector<float> &ez_mm, vector<float> &pix_mm, vector<float> &piz_mm, vector<float> &sin_ang, vector<float> &cos_ang, float c_mm_us, float fs_mhz, vector<int> &disable, vector<vector<TFP_VCH_BASE_BeamformerParameters> > &beamformer)
{
	int i, e, l, n_elements = elements.size();
	int n_lines = pix_mm.size();
	float c_inv = 1 / c_mm_us;
	vector<TFP_VCH_BASE_BeamformerParameters> parameters;
	TFP_VCH_BASE_BeamformerParameters tmp;

	if (n_elements > 0 && n_elements == ex_mm.size() && n_elements == ez_mm.size() && n_lines == piz_mm.size() && n_lines == sin_ang.size() && n_lines == cos_ang.size() && n_elements == disable.size())
	{
		beamformer.clear();
		for (i = 0; i < n_elements; i++)
		{
			e = i;// elements[i];
			parameters.clear();
			for (l = 0; l < n_lines; l++)
			{				
				tmp = tfm_parameters(ex_mm[e], ez_mm[e], 0, pix_mm[l], piz_mm[l], sin_ang[l], cos_ang[l], c_inv, acq_delay_samples, fs_mhz, disable[e] == 1);
				parameters.push_back(tmp);
			}
			beamformer.push_back(parameters);
		}
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**
first_element_x: coordenada X del primer elemento del array, para los ángulos negativos dentro del material
first_element_z: coordenada Z del primer elemento del array, para los ángulos negativos dentro del material
last_element_x: coordenada X del último elemento del array, para los ángulos positivos dentro del material
last_element_z: coordenada Z del último elemento del array, para los ángulos positivos dentro del material
pwi_angles: ángulos de las ondas planas emitidas
*/
// -----------------------------------------------------------------------------
int tfm_pwi_forward_parameters(float first_element_x, float first_element_z, float last_element_x, float last_element_z, 
	vector<float> pwi_angles, 
	int acq_delay_samples, 
	vector<float>& pix_mm, 
	vector<float>& piz_mm, 
	float line_angle,
	float c_mm_us, 
	float fs_mhz, 
	vector<vector<TTFM_PWI_ForwardBeamformerParameters> >& forwardBeamformer,
	vector<TTFM_PWI_AnglesBeamformerParameters >& anglesBeamformer)
{
	int i, l;
	int n_lines = pix_mm.size();
	float T_0_exact, F_0_float, T_0_cuant, dist_ini_exacto, x_org, z_org, cos_pw, c_inv = 1 / c_mm_us;
	vector<TTFM_PWI_ForwardBeamformerParameters> tmp_forwardBeamformer;
	TTFM_PWI_ForwardBeamformerParameters tmp_forward;
	TTFM_PWI_AnglesBeamformerParameters tmp_angle;

	//if (n_elements > 0 && n_elements == ex_mm.size() && n_elements == ez_mm.size() && n_lines == piz_mm.size() && n_lines == sin_ang.size() && n_lines == cos_ang.size() && n_elements == disable.size())
	{
		anglesBeamformer.clear();
		forwardBeamformer.clear();
		for (i = 0; i < pwi_angles.size(); i++)
		{
			pwi_angles[i] *= (PI / 180);
			tmp_forwardBeamformer.clear();

			cos_pw = cos(pwi_angles[i] - line_angle);
			tmp_angle.COS_PWI = Round16u((0.5 * cos_pw - 0.25) * (float)pow(2,17));
			for (l = 0; l < n_lines; l++)
			{
				if (pwi_angles[i] < 0)
				{ 
					x_org = last_element_x;
					z_org = last_element_z;
				}
				else
				{
					x_org = first_element_x;
					z_org = first_element_z;
				}
				//T_0_f = ((R_0_f / 2 + tk * fs) - retardo_adq_muestras);
				float kk = sin(pwi_angles[i]);
				dist_ini_exacto = sin(pwi_angles[i]) * (pix_mm[l] - x_org) - cos(pwi_angles[i]) * (piz_mm[l] - z_org);
				//T_0_exact = dist_ini_exacto / c_mm_us * fs_mhz / 2 - acq_delay_samples;
				T_0_exact = dist_ini_exacto / c_mm_us * fs_mhz - acq_delay_samples;
				T_0_cuant = pow(2, -LOG_2_NU) * round(T_0_exact * pow(2, LOG_2_NU));
				F_0_float = (T_0_exact - T_0_cuant - 3 / 8 + 1 / 2 * cos_pw);

				tmp_forward.T_0 = T_0_cuant * pow(2,LOG_2_NU);
				tmp_forward.F_0 = Round16(F_0_float * pow(2,C_F_WIDTH+1));


				//TFP_VCH_BASE_BeamformerParameters tmp;
				//tmp = tfm_pwi_parameters(ex_mm[e], ez_mm[e], 0, pix_mm[l], piz_mm[l], sin_ang[l], cos_ang[l], c_inv, acq_delay_samples, fs_mhz, disable[e] == 1);
				//parameters.push_back(tmp);
				tmp_forwardBeamformer.push_back(tmp_forward);
			}
			anglesBeamformer.push_back(tmp_angle);
			forwardBeamformer.push_back(tmp_forwardBeamformer);
		}
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int tfm_backward_parameters(int n_elements, int acq_delay_samples, vector<float> &rx_mm, vector<float>& rz_mm, vector<float>& pix_mm, vector<float>& piz_mm, vector<float>& sin_ang, vector<float>& cos_ang, float c_mm_us, float fs_mhz, vector<int> &disable, vector<vector<TFP_VCH_BASE_BeamformerParameters> >& beamformer)
{
	int i, e, l;
	int n_lines = pix_mm.size();
	float c_inv = 1 / c_mm_us;
	vector<TFP_VCH_BASE_BeamformerParameters> parameters;

	if (n_elements > 0 && n_elements == rx_mm.size() && n_elements == rz_mm.size() && n_lines == piz_mm.size() && n_lines == sin_ang.size() && n_lines == cos_ang.size() && n_elements == disable.size())
	{
		beamformer.clear();
		for (l = 0; l < n_lines; l++)
		{
			parameters.clear();
			for (i = 0; i < n_elements; i++)
			{
				e = i;// elements[i];
				TFP_VCH_BASE_BeamformerParameters tmp;
				tmp = tfm_parameters(rx_mm[e], rz_mm[e], 0, pix_mm[l], piz_mm[l], sin_ang[l], cos_ang[l], c_inv, acq_delay_samples, fs_mhz, disable[e] == 1);
				parameters.push_back(tmp);
			}
			beamformer.push_back(parameters);
		}
	}
	return 0;
}
int copy_init_word_to_buffer(u32 *buffer, init_word_t init_word, int offset, bool last, bool reset)
{
	int i,send_words=0;
	unsigned int current_word,uint32_pass_word,msbs,lsbs,next_lsbs;
	unsigned int *buffer32;
	unsigned char bits_to_transfer;

	static unsigned char Number_bits_didnt_transfered=0;
	static unsigned int Bits_didnt_transfered=0;
	buffer32=(unsigned int*)buffer;
	u32 *axi4output_p;
	axi4output_p=(u32 *)&uint32_pass_word;

	if (reset)
	{
		Number_bits_didnt_transfered = 0;
		Bits_didnt_transfered = 0;
	}
	bits_to_transfer=Number_bits_didnt_transfered+C_INIT_WORD_WIDTH;

	lsbs=Bits_didnt_transfered&((1<<Number_bits_didnt_transfered)-1);
	for(i=0;i<INIT_WORD_32BIT_WORDS_NUMBER+1;i++)
	{
		current_word = (i < INIT_WORD_32BIT_WORDS_NUMBER) ? init_word.words_32bit[i] : 0;
		if(Number_bits_didnt_transfered)
			next_lsbs=current_word>>(32-Number_bits_didnt_transfered);
		else
			next_lsbs=0;
		msbs=current_word<<(Number_bits_didnt_transfered);
		uint32_pass_word=msbs|lsbs;
		if(bits_to_transfer<32) break;
		buffer[offset+send_words]=*axi4output_p;
		send_words++;
		lsbs=next_lsbs&((1<<Number_bits_didnt_transfered)-1);
		bits_to_transfer-=32;
	}
	if((bits_to_transfer!=0)&&(last==1))
	{
		msbs=0;
		uint32_pass_word=msbs|lsbs;
		buffer[offset+send_words]=*axi4output_p;
		send_words++;
		bits_to_transfer=0;
		lsbs=0;
	}
	Number_bits_didnt_transfered=bits_to_transfer;
	Bits_didnt_transfered=uint32_pass_word;

	return send_words;
}

int copy_init_word_to_buffer_tfm_forward(u32* buffer, init_word_t init_word, int offset)
{
	int sent_words = 0;
	unsigned int uint32_pass_word;
	unsigned int *buffer32;
	buffer32 = (unsigned int*)buffer;
	u32* axi4output_p;
	axi4output_p = (u32*)&uint32_pass_word;
	for (sent_words = 0; sent_words < INIT_WORD_TFM_FORWARD_32B_BEATS ; sent_words++)
	{
		if (sent_words < INIT_WORD_32BIT_WORDS_NUMBER)
			uint32_pass_word = init_word.words_32bit[sent_words];
		else
			uint32_pass_word = 0x00CACA00;

		buffer[offset + sent_words] = *axi4output_p;
	}

	return sent_words;
}

#if (A_0_OFFSET>32)
	#error Error, la configuración de anchos de bits de la palabra de inicialización (init_word) no está soportada, (A_0_OFFSET>32).
#endif
#if (Z_0_OFFSET>64)
	#error Error, la configuración de anchos de bits de la palabra de inicialización (init_word) no está soportada, (Z_0_OFFSET>64).
#endif
#if (T_0_OFFSET<64)
	#error Error, la configuración de anchos de bits de la palabra de inicialización (init_word) no está soportada, (T_0_OFFSET<64).
#endif
#if (E_0_OFFSET<64)
	#error Error, la configuración de anchos de bits de la palabra de inicialización (init_word) no está soportada, (E_0_OFFSET<64).
#endif
#if (INIT_WORD_32BIT_WORDS_NUMBER!=3)
	#error Error, la configuración de anchos de bits de la palabra de inicialización (init_word) no está soportada, (INIT_WORD_32BIT_WORDS_NUMBER!=3).
#endif

void copy_foc_line_to_init_word(init_word_t *init_word,TFP_VCH_BASE_BeamformerParameters *foc_line)
{
	unsigned int tmp1,tmp2,tmp3;

	tmp1=foc_line->D_0&((1<<D_0_WIDTH)-1);
	tmp2=(foc_line->A_0&((1<<A_0_WIDTH)-1))<<A_0_OFFSET;
	init_word->words_32bit[0]=tmp2|tmp1;

	tmp1=(foc_line->A_0&((1<<A_0_WIDTH)-1))>>(32-A_0_OFFSET);
	tmp2=(foc_line->R_0&((1<<R_0_WIDTH)-1))<<(R_0_OFFSET-32);
	tmp3=(foc_line->Z_0&((1<<Z_0_WIDTH)-1))<<(Z_0_OFFSET-32);
	init_word->words_32bit[1]=tmp3|tmp2|tmp1;

	tmp1=(foc_line->Z_0&((1<<Z_0_WIDTH)-1))>>(64-(Z_0_OFFSET));
	tmp2=(foc_line->T_0&((1<<T_0_WIDTH)-1))<<(T_0_OFFSET-64);
	tmp3=(foc_line->E_0&((1<<E_0_WIDTH)-1))<<(E_0_OFFSET-64);
	init_word->words_32bit[2]=tmp3|tmp2|tmp1;
}
void copy_foc_tfm_pwi_line_to_init_word(unsigned long* init_word, TTFM_PWI_ForwardBeamformerParameters* foc_line)
{
	unsigned int tmp1, tmp2;

	tmp1 = foc_line->F_0 & 0x0000FFFF;
	tmp2 = (foc_line->T_0 & ((1 << C_F_WIDTH) - 1)) << C_F_WIDTH;
	*init_word = tmp2 | tmp1;
}
int copy_init_word_to_buffer_tfm_pwi_forward(u32* buffer, unsigned long init_word, int offset)
{
	buffer[offset] = init_word;
	return 1;
}
// -----------------------------------------------------------------------------
/**
Devuelve la distancia entre punto A (xa,za) y B (xb,zb)
A o B pueden ser vectores de la misma longitud. A o B puede ser escalar y
el otro vector.
*/
// -----------------------------------------------------------------------------
float Distance(float xa,
	float za,
	float xb,
	float zb)
{
	return sqrt((xa - xb) * (xa - xb) + (za - zb) * (za - zb));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int round32(float data)
{
	int result, sgn = 1;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5)
		result = (int)((data + 1) * sgn);
	else
		result = (int)(data * sgn);
	return result;
}
// -----------------------------------------------------------------------------
/**
 Evalua los parámetros del circuito focalizador para un ángulo de deflexión dado por (ux,uz)
 Coordenadas Locales: hacer el centro del array virtual (xvc,zvc) el origen de las coords locales.

 @param[in] *tk    Puntero al array de tiempos tk para los N elementos.
 @param[in] Tec    Instante de emisión del elemento central.
 @param[in] *xv    Puntero al array con las coordenadas X de los elementos de la apertura virtual.
 @param[in] *zv    Puntero al array con las coordenadas Z de los elementos de la apertura virtual.
 @param[in] xvc    Coordenada X del centro de la apertura virtual.
 @param[in] zvc    Coordenada Z del centro de la apertura virtual.
 @param[in] xec    Coordenada X del punto de entrada del haz principal.
 @param[in] zec    Coordenada Z del punto de entrada del haz principal.
 @param[in] ux     Coordenada X del vector unitario en la dirección haz principal.
 @param[in] uz     Coordenada Z del vector unitario en la dirección haz principal.
 @param[in] dv     Distancia desde el centro de la apertura virtual al punto de entrada del haz principal.

 @param[out] n0   Rango inicial para cada elemento y ángulo deflexión actual.
 @param[out] r0   Parámetro inicial r0 para cada elemento y ángulo deflexión actual.
 @param[out] A    Parámetro inicial A para cada elemento y ángulo deflexión actual.
 @param[out] DLR  Retardo en recepción para cada elemento y ángulo deflexión actual.
 @param[out] NZ   Número muestras nulas para cada elemento y ángulo deflexión actual.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int RAD_Parameters(int n_elements,
	float* adjust_time,
	float Tec,
	float start_range,
	float* x_aperture,
	float* z_aperture,
	float xvc,
	float zvc,
	float xc,
	float zc,
	float xec,
	float zec,
	double xu_ray,
	double zu_ray,
	bool consuela,
	float material_velocity,
	float wedge_velocity,
	vector<int>& rad_R0,
	vector<int>& rad_A0,
	vector<int>& rad_D0,
	float* reception_focal_law,
	vector<unsigned short>& n_null_samples,
	float* x_focus_rfl,
	float* z_focus_rfl,
	vector<TFP_VCH_BASE_BeamformerParameters>& beamformer,
	vector<float>& tida,
	vector<float>& tvue,
	vector<float>& tk,
	float sampling_frequency)
{
	int i;
	float fck = sampling_frequency * 1000000;//40e6; // Frecuencia del reloj de procesamiento;
	float tick = 1 / fck; // Periodo maestro
	float tick4 = tick / 4.;  // Periodo reloj x4

	// Definiciones del circuito focalizador
	float v = 4;                  // Factor de sobremuestreo (potencia de 2, please)
	float f = 2. * fck / material_velocity;   // Factor de conversión distancias a periodos Ts

	// Parte fraccional de los registros del focalizador
	float frac_r = 2.; // 1 bit de parte fraccional
	float frac_a = pow(2., 6); // 6 bit de parte fraccional
	float frac_d = pow(2., 6); // 6 bit de parte fraccional

	//vector<float> tk;
	vector<float> xv;
	vector<float> zv;
	vector<float> t0;

	//if (beamformer == NULL) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);

	tk.resize(n_elements);
	xv.resize(n_elements);
	zv.resize(n_elements);

	// Convierto a segundos y metros
	for (i = 0; i < n_elements; i++)
	{
		if (adjust_time != NULL) tk[i] = adjust_time[i] / 1000000.;
		else tk[i] = 0;
		xv[i] = x_aperture[i] / 1000.;
		zv[i] = z_aperture[i] / 1000.;
	}
	Tec /= 1000000.;
	xvc /= 1000.;
	zvc /= 1000.;
	xc /= 1000.;
	zc /= 1000.;
	xec /= 1000.;
	zec /= 1000.;
	start_range /= 1000.;

	float dv = Distance(xvc, zvc, xec, zec);

	float xvL; // elementos del array virtual (coord. Locales)
	float zvL; // elementos del array virtual (coord. Locales)

	float angle = atan2(zu_ray, xu_ray) + PI / 2.;
	float sin_angle = sin(angle);
	float cos_angle = cos(angle);

	float R0min;
	float R0x;
	float R0min_mas, R0min_menos;
	float ri = start_range + dv; // Rango inicial desde el centro del array virtual
	float nvx, nvz;
	vector<float> n0;      // Rangos iniciales, tics, entero --> elegir siguiente
	float n0x;// = n0 * ux;          // (n0x,n0z) = coords locales 1er foco para cada elemto
	float n0z;// = n0 * uz;
	int nri;
	vector<float> r0;
	float r0_rnd;
	float K;
	vector<float> A;
	vector<float> D;
	vector<float> xf0;// = xvc + n0x / f;  // coords ext 1er foco (diferente para cada elmto)
	vector<float> zf0;// = zvc + n0z / f;
	//vector<float> tida;
	float tida_min, tdif = 0, aux_f, min_value;
	//vector<float> tvue;
	vector<float> DLR;

	tvue.resize(n_elements);
	xf0.resize(n_elements);
	zf0.resize(n_elements);
	rad_R0.resize(n_elements);
	rad_A0.resize(n_elements);
	rad_D0.resize(n_elements);
	tida.resize(n_elements);
	t0.resize(n_elements);
	DLR.resize(n_elements);

	n0.resize(n_elements);
	r0.resize(n_elements);
	A.resize(n_elements);
	D.resize(n_elements);

	beamformer.resize(n_elements);

	for (i = 0; i < n_elements; i++)
	{
		xvL = xv[i] - xvc;
		zvL = zv[i] - zvc;

		R0min_mas = xvL * sin_angle - zvL * cos_angle + (xvL * cos_angle + zvL * sin_angle) * (v / 2. - 1.) / sqrt(float(v - 1));
		R0min_menos = xvL * sin_angle - zvL * cos_angle - (xvL * cos_angle + zvL * sin_angle) * (v / 2. - 1.) / sqrt(float(v - 1));
		if (R0min_mas > R0min_menos) R0min = R0min_mas;
		else R0min = R0min_menos;
		if (ri > R0min) R0x = ri;
		else R0x = R0min;

		// Coordenadas locales elementos en tics Ts
		nvx = f * xvL;
		nvz = f * zvL;

		nri = ceil(f * ri);
		n0[i] = ceil(f * R0x);
		n0x = n0[i] * xu_ray;
		n0z = n0[i] * zu_ray;

		// dists elemtos a su rango inicial, tics
		r0[i] = sqrt((n0x - nvx) * (n0x - nvx) + (n0z - nvz) * (n0z - nvz));

		// valor discreto a programar
		r0_rnd = round32(r0[i] * frac_r) / frac_r;

		// constante para elemento y deflexión
		K = (float)1. - (float)2. * (nvx * sin_angle - nvz * cos_angle);

		// primer valor A(n0) para cada elemento
		A[i] = (float)2. * (n0[i] - r0_rnd) + K;

		// Constante D
		D[i] = A[i] + r0[i] * r0[i] - r0_rnd * r0_rnd + (float)2. / v * r0_rnd - (float)1. + (float)2. / v - (float)1. / (v * v);

		xf0[i] = xvc + n0x / f;  // coords ext 1er foco (diferente para cada elmto)
		zf0[i] = zvc + n0z / f;

		tvue[i] = Distance(xv[i], zv[i], xf0[i], zf0[i]) / material_velocity;

		if (!consuela)
		{
			tida[i] = Distance(xec, zec, xf0[i], zf0[i]) / material_velocity;
			tida_min = start_range / material_velocity;   // Es tiempo de ida por el rayo central hasta la primer muestra
			// Se usa para inicializar el CAT
			tdif = 0;
		}
	}

	if (consuela && wedge_velocity > 0)
	{
		min_value = numeric_limits<float>::max();
		for (i = 0; i < n_elements; i++)
		{
			tida[i] = Distance(xc, zc, xec, zec) / wedge_velocity +
				Distance(xec, zec, xf0[i], zf0[i]) / material_velocity;
			// Es tiempo de ida por el rayo central hasta la primer muestra.
			// Se usa para inicializar el CAT
			tida_min = Distance(xc, zc, xec, zec) / wedge_velocity +
				start_range / material_velocity;
			aux_f = Distance(xf0[i], zf0[i], xec, zec);
			if (aux_f < min_value) min_value = aux_f;
		}
		tdif = 2 * (min_value - start_range) / material_velocity;
	}
	for (i = 0; i < n_elements; i++)
	{
		t0[i] = tida[i] + tvue[i] + tk[i] + Tec - tdif;
		DLR[i] = round32(t0[i] * (v * fck));
		if (n0[i] > nri) n_null_samples[i] = n0[i] - nri;
		else n_null_samples[i] = 0;
		reception_focal_law[i] = (DLR[i] - n_null_samples[i] * v) * 1000000. / (v * fck);
		rad_R0[i] = round32(r0[i] * frac_r);
		rad_A0[i] = round32(A[i] * frac_a);
		rad_D0[i] = round32(D[i] * frac_d);

		beamformer[i].R_0 = round32(r0[i] * frac_r);
		beamformer[i].A_0 = round32(A[i] * frac_a);
		beamformer[i].D_0 = round32(D[i] * frac_d);
		beamformer[i].T_0 = DLR[i];
		beamformer[i].Z_0 = n_null_samples[i];
		beamformer[i].E_0 = 0;

		if (x_focus_rfl != NULL) x_focus_rfl[i] = xf0[i] * 1000.;
		if (z_focus_rfl != NULL) z_focus_rfl[i] = zf0[i] * 1000.;
	}
	for (i = 0; i < tida.size(); i++) tida[i] *= 1000000;
	for (i = 0; i < tvue.size(); i++) tvue[i] *= 1000000;
	for (i = 0; i < tk.size(); i++) tk[i] *= 1000000;
	return 0;
}
