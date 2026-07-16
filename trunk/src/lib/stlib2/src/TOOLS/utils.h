#ifndef utilsH
#define utilsH

#include "tad.h"

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
   #include <System.hpp>
#endif

using namespace std;

AnsiString ChangeFileExtension (AnsiString file_path, AnsiString extension);

bool StrContains (AnsiString str, AnsiString sub_str);

AnsiString ToUpper (AnsiString str);

int StrLength (AnsiString str);

float dB2Linear (float db);

float Linear2dB (float linear);

unsigned int Log2 (unsigned int data);

void* Realloc(void* ptr, size_t n_data);

void* Malloc(size_t n_data);

int Round32 (float data);

unsigned long Round32u (float data);

short int Round16 (float data);

unsigned short Round16u (float data);

unsigned short Round16u_d (double data);

unsigned char Round8u (double data);

unsigned char Round8u (float data);

char Round8 (double data);

int RoundUp32 (float f);

unsigned short RoundUp16u (float f);

int RoundDown (float f);

float Media(float *array, int n);

int FindLowerData (vector<float> &distance, int ind_ref, float data);

int FindLowerData_Int32 (vector<int> &distance, int ind_ref, float data);

int FindHigherData (vector<float> &distance, int ind_ref, float data);

int FileCopy(char *src, char *dest);

void polyfit_2do_grado (vector<float> y, vector<float> &coef);

void polyfitval_2do_grado (vector<float> y, vector<float> &y_filt);

void polyfitval_2do_grado (vector<float> y, vector<float> x, vector<float> &y_out);

void polyval (float *coef, int n_coef, vector<float> x, vector<float> &y_out);

float polyval (float *coef, int n_coef, float x);

int polyval (vector<float> &coefficient, vector<float> &x, vector<float> &y_out);

float polyval_y (vector<float> &coefficients, float x);

int polyval ( float *coef,
               int n_coef,
               float x_min,
               float x_resolution,
               int n_data,
               float *x,
               float *y);

int polyval ( float *coef,
               int n_coef,
               float x_min,
               float x_resolution,
               int n_data,
               double *x,
               double *y);

float polyderval (float *coef, int n_coef, float x);

int cirval (   float x_center,
               float z_center,
               float radius,
               vector<float> &x,
               vector<float> &y);

int cirval (   float x_center,
               float z_center,
               float radius,
               float x_min,
               float x_resolution,
               int n_data,
               float *x,
               float *y);

int cirval (   float x_center,
               float z_center,
               float radius,
               float x_min,
               float x_resolution,
               int n_data,
               double *x,
               double *y);

int cirval (float x_center, float z_center, float radius, float x, float *z);

float cirderval (float x_center, float z_center, float radius, float x);

int polyfitval_n_grado  (  float *y,
                           float *x,
                           float *coefficients_ec,
                           float *y_fit,
                           float *desviation,
                           int *deviation_points,
                           float *desviation_mean,
                           float deviation_threshold,
                           int n_points,
                           int degree);

int EstimaCirc (float *xp, float *zp, float *coef, int n_points);


#endif
