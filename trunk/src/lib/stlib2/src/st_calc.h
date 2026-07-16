#ifndef stfp_calcH
#define stfp_calcH

#include "tad.h"
#include "c_log_file.h"
#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

WORD dBToDA (float data);

float Min(float *array, int n, int *ind);

int Gan_dB2reg(vector<float> &vGan,vector<float> &vTime, WORD *ck_cat,int hilo);

int ST_PrintPinAssignment (void);

float ST_CalRangeAcqMax (void);

float ST_CalRangeAcqMin (void);

unsigned short CalculaFiltro(float fb, float fa, float fs, int NFIL, int NBITCOE, short* Bd, vector<float>& fft);

#endif
