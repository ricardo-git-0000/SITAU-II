#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
   #define NOMINMAX
#endif

// -----------------------------------------------------------------------------
/**
 @file fl_calc.cpp

 @brief Este archivo contiene la implementación de las funciones genéricas de cálculo.

 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 11/29/10 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <limits>
#include <math.h>
#include <vector>
#include <time.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_calc.h"
#include "fl_error_code.h"
#include "c_log_file.h"
#include "utils.h"
#include "foc_params.h"

using namespace std;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short MinArrayUSHORT (unsigned short *ptr, int size)
{
int i;
unsigned short min = numeric_limits<unsigned short>::max();

   try
   {
      for (i=0; i<size; i++)
      {
         if (min > ptr[i]) min = ptr[i];
      }
   }
#if !defined(_MSC_VER)
   catch (Exception &E)
   {
      EELOG(&E);
      throw;
   }
   catch (exception &e)
   {
      EeLOG(&e);
      throw;
   }
#endif
   catch (...)
   {
      NEELOG();
      throw;
   }
   return min;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short MaxArrayUSHORT (unsigned short *ptr, int size)
{
int i;
unsigned short max = numeric_limits<unsigned short>::min();

   try
   {
      for (i=0; i<size; i++)
      {
         if (max < ptr[i]) max = ptr[i];
      }
   }
#if !defined(_MSC_VER)
   catch (Exception &E)
   {
      EELOG(&E);
      throw;
   }
   catch (exception &e)
   {
      EeLOG(&e);
      throw;
   }
#endif
   catch (...)
   {
      NEELOG();
      throw;
   }
   return max;
}
// -----------------------------------------------------------------------------
/**
 Devuelve el valor más elevado de un vector de número reales.
 
 @param[in] *ptr     Puntero al vector que contiene los número de los que queremos
                     obtener el mayor.
 
 @param[in] size     Tamaño del vector pasado como parámetro \c ptr.
 
 @param[out] *index  Puntero al parámetro donde se devuelve la posición del elemento
                     mayor del vector \c ptr.

 @return Devuelve el mayor de los elementos del vector \c ptr.
*/
// -----------------------------------------------------------------------------
float MaxArrayFLOAT (float *ptr, int size, int *index)
{
int i;
float max = numeric_limits<float>::max() * -1;

   if (index != NULL) *index = 0;

   try
   {
      for (i=0; i<size; i++)
      {
         if (max < ptr[i])
         {
         	max = ptr[i];
            if (index != NULL) *index = i;
         }
      }
   }
#if !defined(_MSC_VER)
   catch (Exception &E)
   {
      EELOG(&E);
      throw;
   }
   catch (exception &e)
   {
      EeLOG(&e);
      throw;
   }
#endif
   catch (...)
   {
      NEELOG();
      throw;
   }
   return max;
}
// -----------------------------------------------------------------------------
/**
Devuelve la distancia entre punto A (xa,za) y B (xb,zb)
A o B pueden ser vectores de la misma longitud. A o B puede ser escalar y
el otro vector.
*/
// -----------------------------------------------------------------------------
float FL_Distance (  float xa,
                     float za,
                     float xb,
                     float zb)
{
   return sqrt((xa - xb) * (xa - xb) + (za - zb) * (za - zb));
}
// -----------------------------------------------------------------------------
/**
Devuelve la distancia entre punto A (xa,za) y B (xb,zb)
A o B pueden ser vectores de la misma longitud. A o B puede ser escalar y
el otro vector.
*/
// -----------------------------------------------------------------------------
int FL_Distance ( float xa,
                  float za,
                  float *xb,
                  float *zb,
                  int v_size,
                  vector<float> &distance)
{
int i;

   if (distance.size() != v_size) distance.resize(v_size);
   for (i=0; i<v_size; i++)
      distance[i] = sqrt((xa - xb[i]) * (xa - xb[i]) + (za - zb[i]) * (za - zb[i]));
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**
Devuelve la distancia entre punto A (xa,za) y B (xb,zb)
A o B pueden ser vectores de la misma longitud. A o B puede ser escalar y
el otro vector.
*/
// -----------------------------------------------------------------------------
int FL_Distance ( float xa,
                  float za,
                  double *xb,
                  double *zb,
                  int v_size,
                  vector<float> &distance)
{
int i;

   if (distance.size() != v_size) distance.resize(v_size);
   for (i=0; i<v_size; i++)
      distance[i] = sqrt((xa - xb[i]) * (xa - xb[i]) + (za - zb[i]) * (za - zb[i]));
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**
Devuelve la distancia entre punto A (xa,za) y B (xb,zb)
A o B pueden ser vectores de la misma longitud. A o B puede ser escalar y
el otro vector.
*/
// -----------------------------------------------------------------------------
int FL_Distance ( float *xa,
                  float *za,
                  float *xb,
                  float *zb,
                  int v_size,
                  vector<float> &distance)
{
int i;

   if (distance.size() != v_size) distance.resize(v_size);
   for (i=0; i<v_size; i++)
      distance[i] = sqrt((xa[i] - xb[i]) * (xa[i] - xb[i]) + (za[i] - zb[i]) * (za[i] - zb[i]));
   return EFL_NONE;
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
int DDF_RADParameters ( int n_elements,
                        float *adjust_time,
								float Tec,
								float start_range,
								float *x_aperture,
								float *z_aperture,
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
								//vector<int> &rad_R0,
        //                vector<int> &rad_A0,
        //                vector<int> &rad_D0,
                        //float *reception_focal_law,
                        //vector<unsigned short> &n_null_samples,
                        //float *x_focus_rfl,
                        //float *z_focus_rfl,
                        vector<TFP_VCH_BASE_BeamformerParameters> &beamformer,
                        //vector<float> &tida,
                        //vector<float> &tvue,
                        //vector<float> &tk,
                        float sampling_frequency)
{
int i;
unsigned short n_null_samples;
float fck = sampling_frequency * 1000000;//40e6; // Frecuencia del reloj de procesamiento;
float tick = 1 / fck; // Periodo maestro
float tick4 = tick / 4.;  // Periodo reloj x4

// Definiciones del circuito focalizador
float v = 4;                  // Factor de sobremuestreo (potencia de 2, please)
float f = 2. * fck / material_velocity;   // Factor de conversión distancias a periodos Ts

// Parte fraccional de los registros del focalizador
float frac_r = 2.; // 1 bit de parte fraccional
float frac_a = pow(2.,6); // 6 bit de parte fraccional
float frac_d = pow(2.,6); // 6 bit de parte fraccional

vector<float> tk;
vector<float> xv;
vector<float> zv;
vector<float> t0;

   //if (beamformer == NULL) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);

   tk.resize(n_elements);
   xv.resize(n_elements);
   zv.resize(n_elements);

// Convierto a segundos y metros
   for (i=0; i<n_elements; i++)
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

float dv = FL_Distance(xvc, zvc, xec, zec);

float xvL; // elementos del array virtual (coord. Locales)
float zvL; // elementos del array virtual (coord. Locales)

float angle = atan2(zu_ray, xu_ray) + M_PI / 2.;
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
vector<float> tida;
float tida_min, tdif=0, aux_f, min_value;
vector<float> tvue;
vector<float> DLR;
int retardo_muestras = Round32((start_range / material_velocity) * 2 * fck) - 10;

   tvue.resize(n_elements);
   xf0.resize(n_elements);
   zf0.resize(n_elements);
   //rad_R0.resize(n_elements);
   //rad_A0.resize(n_elements);
   //rad_D0.resize(n_elements);
   tida.resize(n_elements);
   t0.resize(n_elements);
   DLR.resize(n_elements);

   n0.resize(n_elements);
   r0.resize(n_elements);
   A.resize(n_elements);
   D.resize(n_elements);

   beamformer.resize(n_elements);

   for (i=0; i<n_elements; i++)
   {
      xvL = xv[i] - xvc;
      zvL = zv[i] - zvc;

      R0min_mas   = xvL * sin_angle - zvL * cos_angle + (xvL * cos_angle + zvL * sin_angle) * (v / 2. - 1.) / sqrt(float(v - 1));
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
      r0_rnd = Round32(r0[i] * frac_r) / frac_r;

      // constante para elemento y deflexión
      K = (float)1. - (float)2. * (nvx * sin_angle - nvz * cos_angle);

      // primer valor A(n0) para cada elemento
      A[i] = (float)2. * (n0[i] - r0_rnd) + K;
         
      // Constante D
      D[i] = A[i] + r0[i] * r0[i] - r0_rnd * r0_rnd + (float)2. / v * r0_rnd - (float)1. + (float)2. / v - (float)1. / (v * v);

      xf0[i] = xvc + n0x / f;  // coords ext 1er foco (diferente para cada elmto)
      zf0[i] = zvc + n0z / f;

      tvue[i] = FL_Distance(xv[i], zv[i], xf0[i], zf0[i]) / material_velocity;

      if (!consuela)
      {
         tida[i] = FL_Distance(xec, zec, xf0[i], zf0[i]) / material_velocity;
         tida_min =  start_range / material_velocity;   // Es tiempo de ida por el rayo central hasta la primer muestra
                                          // Se usa para inicializar el CAT
         tdif = 0;
      }
   }

   if (consuela && wedge_velocity > 0)
   {
      min_value = numeric_limits<float>::max();
      for (i=0; i<n_elements; i++)
      {
         tida[i] =   FL_Distance(xc, zc, xec, zec) / wedge_velocity +
                     FL_Distance(xec, zec, xf0[i], zf0[i]) / material_velocity;
// Es tiempo de ida por el rayo central hasta la primer muestra.
// Se usa para inicializar el CAT
         tida_min =  FL_Distance(xc, zc, xec, zec) / wedge_velocity +
                     start_range / material_velocity;
         aux_f = FL_Distance(xf0[i], zf0[i], xec, zec);
         if (aux_f < min_value) min_value = aux_f;
      }
      tdif = 2 * (min_value - start_range) / material_velocity;
   }

   for (i=0; i<n_elements; i++)
   {
      t0[i] = tida[i] + tvue[i] + tk[i] + Tec - tdif;
      DLR[i] = Round32(t0[i] * (v * fck)) - (retardo_muestras * v);
      if (n0[i] > nri) n_null_samples = n0[i] - nri;
      else n_null_samples = 0;
      //reception_focal_law[i] = (DLR[i] - n_null_samples[i] * v) * 1000000. / (v * fck);
      //rad_R0[i] = Round32(r0[i] * frac_r);
      //rad_A0[i] = Round32(A[i] * frac_a);
      //rad_D0[i] = Round32(D[i] * frac_d);

      beamformer[i].R_0 = Round32(r0[i] * frac_r);
      beamformer[i].A_0 = Round32(A[i] * frac_a);
      beamformer[i].D_0 = Round32(D[i] * frac_d);
      beamformer[i].T_0 = DLR[i];
      beamformer[i].Z_0 = n_null_samples;
      beamformer[i].E_0 = 0;

      //if (x_focus_rfl != NULL) x_focus_rfl[i] = xf0[i] * 1000.;
      //if (z_focus_rfl != NULL) z_focus_rfl[i] = zf0[i] * 1000.;
   }
   //for (i=0; i<tida.size(); i++) tida[i] *= 1000000;
   //for (i=0; i<tvue.size(); i++) tvue[i] *= 1000000;
   //for (i=0; i<tk.size(); i++) tk[i] *= 1000000;
   return EFL_NONE;
}



