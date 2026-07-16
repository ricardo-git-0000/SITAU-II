#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
#endif

#include "st_calc.h"
#include "st_reg.h"
#include "st_get.h"
#include "stlib2.h"
#include "stlib_dev.h"
#include "st_info.h"
#include "utils.h"
#include "st_error_code.h"
#include "st_vch_get.h"
#include "c_virtual_channel.h"

#include <math.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
   #include <mem.h>
#endif
#include <dos.h>
#include <float.h>
#include <limits>

#include <algorithm>

using namespace std;
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
// OJO - CUANDO SE UTILIZA LA CURVA DE CALIBRACION NO ES 255 ES EL VALOR MAXIMO DEL CONVERSOR
WORD dBToDA (float data) {return Round16u((data + (float)2.) * (float)255. / (float)50.);}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float STPA_ADDataToPercentage (int data, int id_vch)
{
float result;
//int acq_mode;
int range_max_signal = ST_RangePA_MaxSignalAmplitude(id_vch);
int range_min_signal = ST_RangePA_MinSignalAmplitude(id_vch);
short int rf_data;

   rf_data = (short int)data;
   if (data > 0) result = (float)rf_data * 50. / (float)range_max_signal;
   else result = (float)rf_data * -50. / (float)range_min_signal;
   if (result > 50) result = 50;
   if (result < -50) result = -50;
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float STFP_ADDataToPercentage (int data)
{
float result;
//int acq_mode;
int range_max_signal = ST_RangeFP_MaxSignalAmplitude();
int range_min_signal = ST_RangeFP_MinSignalAmplitude();
short int rf_data;

   rf_data = (short int)data;
   if (data > 0) result = (float)rf_data * 50. / (float)range_max_signal;
   else result = (float)rf_data * -50. / (float)range_min_signal;
   if (result > 50) result = 50;
   if (result < -50) result = -50;
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STPA_PercentageToADData (float data, int id_vch)
{
int result;
int max_range = ST_RangePA_MaxSignalAmplitude(id_vch);

   result = Round32(data * max_range / 50.);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int STFP_PercentageToADData (float data)
{
int result;
int max_range = ST_RangeFP_MaxSignalAmplitude();

   result = Round32(data * max_range / 50.);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_TimeToDistance (float data)
{
float result;
float material_velocity = ST_GetMaterialVelocity();

    if (ST_GetAcquisitionTimeMode() == ACQ_Transmission) result = data * (material_velocity / 1000.);
    else result = data * (material_velocity / 2000.);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_DistanceToTime (float data)
{
float result;
float material_velocity = ST_GetMaterialVelocity();

    if (ST_GetAcquisitionTimeMode() == ACQ_Transmission) result = data / (material_velocity / 1000.);
    else	result = data / (material_velocity / 2000.);
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_SamplesToTime (int data)
{
float result;

   if ((result = ST_VCH_SamplesToTime(data)) < 0) return (float)RLOG(result);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_TimeToSamples (float data)
{
unsigned long result;
float sampling_frequency;
int id_virtual_channel = 0;

	if ((sampling_frequency = ST_VCH_GET_SamplingFrequency(id_virtual_channel)) < 0)  return RLOG(Round32(sampling_frequency));
	result = Round32u(data * sampling_frequency);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_SamplesToDistance (int data)
{
	return ST_TimeToDistance(ST_SamplesToTime(data));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_PrintPinAssignment (void)
{
FILE *fd_pinCh = fopen ("pin_assignment.txt","wt");
time_t t;

	if(fd_pinCh == NULL) return ELOG(charES2_FILE,ES2_FILE);

   time(&t);
	fprintf (fd_pinCh, "\r\nPIN ASSIGNMENT - %s\n", ctime(&t));

	fprintf(fd_pinCh, "  Element Number");
	fprintf(fd_pinCh, "  Channel Number");
	fprintf(fd_pinCh, "\n\n");

   //for (int j=0; j<PA.user_pin_assignment.size(); j++)
   //{
   //   fprintf(fd_pinCh, "      %3d  ", j+1);
   //   fprintf(fd_pinCh, "----------->");
   //   fprintf(fd_pinCh, " %3d", PA.user_pin_assignment[j]);
   //   fprintf(fd_pinCh, "\n");
   //}
	fclose(fd_pinCh);
   return ES2_NONE;
}
//------------------------------------------------------------------------------
/*int Maximo(short int *Ascan, int gateIni, int gateFin)
{
    short int max=-MAXSHORT;
    //Recorro todos los puntos del Ascan
    for(int k=gateIni; k<gateFin; k++)
    {
        //Calculo el mÃ¡ximo del Ascan
        if( max < *(Ascan+k) )
            max = *(Ascan+k);
    }//end for
    return max;
}//end Max
*/
//------------------------------------------------------------------------------
float Min(float *array, int n, int *ind)
{
    float min = array[0];
    *ind = 0;

    for(int i=0; i<n; i++)
    {
        if( array[i] < min )
        {
            *ind = i;
            min = array[i];
        }//end if
    }//end for

    return min;
}
// -----------------------------------------------------------------------------
/**
FunciÃ³n Gan_dB2reg
vGan  ->  Vector de valores de ganancia
vTime  ->  Vector de valores de tiempo
ck    ->  Tiempo de actualizaciÃ³n del cat
*/
// -----------------------------------------------------------------------------
int Gan_dB2reg(vector<float> &vGan, vector<float> &vTime, WORD *ck_cat,int )
{
int n_ck=4; //%100 ns
float tcat;
int nmu_cat;

	tcat=vTime.back()-vTime[0];
	nmu_cat=tcat*FCLK/(float)n_ck;//n_ck no puede ser 0 porque empieza en 4 y voy incrementando.
	while (nmu_cat>2048 && n_ck<16)
	{
		n_ck++;
		nmu_cat=tcat*FCLK/(float)n_ck;
	}
	if(nmu_cat>2048 || n_ck>16 ||nmu_cat<1)
	{
		return ELOG(charES2_RANGO,ES2_RANGO);
	}
	vector<float> vGanTemp(nmu_cat+1,0);
	vector<WORD> vMuesTemp(2*nmu_cat,0);
	//Divido mi vector de tiempos por su primer elemento
	for(int i=vTime.size()-1;i>-1;i--)
	{
		vTime[i]=(vTime[i]-vTime[0]);//Ya comprobÃ© que el divisor no es nulo
	}
	//
	vMuesTemp[0]=1;
	for(int i=1;i<(int)vTime.size();i++)
	{
		if(vTime[vTime.size()-1]==0)
		{
			return ELOG(charES2_ZERO_DIVISION,ES2_ZERO_DIVISION);
		}
		vMuesTemp[i]=(WORD)((vTime[i]/vTime[vTime.size()-1])*(nmu_cat));
	}

	//Inicializo como correspondo el vector temporal de ganancia vGanTemp
	for(int i=0;i<(int)vGan.size();i++)
	{
		vGanTemp[vMuesTemp[i]]=vGan[i];
	}

	//Realizo la verdadera interpolaciÃ³n lineal.
	float m=0;//la pendiente
	float b=0;//la ordenada en el origen
	for(int k=1;k<(int)vTime.size();k++)
	{
		for(int i=vMuesTemp[k-1];i<vMuesTemp[k];i++)
		{
			if(i==vMuesTemp[k-1])
			{
				if(vMuesTemp[k]==vMuesTemp[k-1])
				{
				   return ELOG(charES2_ZERO_DIVISION,ES2_ZERO_DIVISION);
				}
				m=(vGan[k]-vGan[k-1])/(vMuesTemp[k]-vMuesTemp[k-1]);
				b=vGan[k]-m*vMuesTemp[k];
			}
			vGanTemp[i]=m*(float)(i)+b;
		}
	}
	vGan.assign(vGanTemp.begin(),vGanTemp.end());
	*ck_cat=(WORD)n_ck;

	//Meto el valor inicial, que es igual al segundo valor
	//(porque durante INICAT la curva ha de ser plana)
	vGan[0]=vGan[1];

	//Tengo vGan en dB, lo tengo que convertir a registro.
	for(int i=0; i<(int)vGan.size(); i++)
	{
		//Ganancia analÃ³gica: 8 bits
		if (vGan[i] <= 48)
		{
			vGan[i] = dBToDA(vGan[i]);
		}
		//Ganancia digital: formato Q9.6
		else
		{
			//ConversiÃ³n de dB's a unidades naturales:
			vGan[i] = pow(10,(vGan[i]-48)/20);
			//ConversiÃ³n de nÃºmero decimal a formato Q9.6
			vGan[i] = Round32(vGan[i] * pow (2.,6));
			//El bit 15 debe ser 1:
			vGan[i] = vGan[i] + pow(2.,15);
		}
	}

   return ES2_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//Calculo los lÃ­mites del rango de adquisiciÃ³n y de la velocidad sin tener en cuenta la restricciÃ³n por suela,
//SÃ³lo estoy considerando la restricciÃ³n por nÃºmero de lÃ­neas
float ST_CalRangeAcqMax (void)
{
float sampling_frequency;//tmax_img
float range_nmu;//range_img
float result;
int acq_mode, id_virtual_channel = 0;;
float velocity = ST_GetMaterialVelocity() / 1000.;

   if ((acq_mode = ST_GetAcquisitionTimeMode()) < 0) return RLOG(acq_mode);
   if ((sampling_frequency = ST_VCH_GET_SamplingFrequency(id_virtual_channel)) < 0) 
      return RLOG(sampling_frequency);

	//Comprobar que el nÃºmero de muestras despuÃ©s del diezmado no es mayor de 20478
   // Aplicamos un factro de seguridad de 400 muestras
   range_nmu = velocity * ((float)MAX_NMU_OUT - 400) / sampling_frequency;

    result = range_nmu;
    if (result < 1) result = 1;
    if (acq_mode == ACQ_PulseEcho || acq_mode == ACQ_PitchCatch) result = result / 2.;
    return Round32(result - 0.5);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float ST_CalRangeAcqMin (void)
{
float tmin_nmu;// = (MIN_NMU_OUT*d) / FCLK; //Min nmu que se puede obtener tras el diezmado
float result, sampling_frequency;
float velocity = ST_GetMaterialVelocity() / 1000.;
//float sampling_frequency = ST_GetSamplingFrequency(-1);
int acq_mode, id_virtual_channel = 0;

   if ((sampling_frequency = ST_VCH_GET_SamplingFrequency(id_virtual_channel)) < 0) 
      return RLOG(sampling_frequency);

   tmin_nmu = (float)MIN_NMU_OUT / sampling_frequency; //Min nmu que se puede obtener tras el diezmado
   result = (float)velocity * tmin_nmu / 1000.;

	if (result < 1) result = 1;
   acq_mode = ST_GetAcquisitionTimeMode();
   if (acq_mode == ACQ_PulseEcho || acq_mode == ACQ_PitchCatch) result = result / 2.;
   return Round32(result + 0.5);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ST_CalMinDownSamplingFactor (float *fs_max)
{
   if (gb_virtual_channel.size() > 0 && gb_virtual_channel[0] != NULL)
      return gb_virtual_channel[0]->MinDownSamplingFactor(fs_max);
   else return ELOG(charES2_IdVirtualChannel, ES2_IdVirtualChannel);
}
//========================================================
/*                        fpbajo
                        ==========

Descripcion: Calcula filtro paso-bajo de orden N,
             frecuencia corte fc (relativa a Nyquist=fs/2)

Parametros:
    N= orden del filtro. Nota: numero de coeficientes = N+1
    fc = frecuencia de corte relativa a la de Nyquist fs/2
    *B = puntero a vector de coeficientes resultantes

*/
//----------------------------------------------------
void fpbajo(int N, float fc, float* B)
{
   float pi = 3.141592;
   float win[255];
   int k;
   float sB;

   for (k = 0; k < 255; k++)
      win[k] = 0;
   sB = 0;

   for (k = 0; k <= N; k++)          // Calcular ventana de Hamming
      win[k] = 0.54 - 0.46 * cos(2 * pi * k / N);

   for (k = 0; k < N / 2; k++)
   {
      B[k] = win[k] * sin(pi * fc * (k - N / 2)) / (k - N / 2);
      sB = sB + B[k];
   }//end for

   B[N / 2] = win[N / 2] * pi * fc;
   sB = sB + B[N / 2];

   for (k = N / 2 + 1; k <= N; k++)
   {
      if ((k - N / 2) != 0)
         B[k] = win[k] * sin(pi * fc * (k - N / 2)) / (k - N / 2);
      sB = sB + B[k];
   }//end for

   // NormalizaciÃ³n para ganancia unidad a frecuencia cero:
   for (k = 0; k <= N && sB != 0; k++)
      B[k] = B[k] / sB;
}// fin fpbajo

//========================================================
/*                       invspec
                        ==========

Descripcion: Realiza la inversion espectral de N coeficientes pasados en *B

Parametros:
      N= numero de coeficientes
      B= array de coeficientes de entrada/salida

Efectos laterales:
      Modifica los coeficientes pasados por referencia
*/
//----------------------------------------------------
void invspec(int N, float* B)
{
   int i, m;

   m = (N + 1) / 2;
   for (i = 0; i <= N; i++)
      B[i] = -B[i];
   B[m] = 1 + B[m];
}// fin invspec

//========================================================
/*                       fpbanda
                        ==========

Descripcion: Calcula filtro paso-banda  de orden N

Parametros:
    N = orden del filtro. Nota: numero coeficientes = N+1
    fb = frecuencia baja relativa a la de Nyquist = fmuestreo/2
    fa = frecuencia alta relativa a la de Nyquist = fmuestreo/2
    *BA = puntero a vector de coeficientes

*/
//----------------------------------------------------
void fpbanda(int N, float fb, float fa, float* BA)
{
   float BB[255];
   int k;

   fpbajo(N, fb, BB);            // Paso-bajo en fb
   fpbajo(N, fa, BA);            // Paso-bajo en fa
   invspec(N, BA);               // Paso-alto en fa
   for (k = 0; k <= N; k++)
      BA[k] = BB[k] + BA[k];    // Notch entre fb y fa
   invspec(N, BA);               // Paso-banda entre fb y fa
}// fin fpbanda




//========================================================
/*                      discretiza
                        ==========

Descripcion: Discretiza los coeficientes a b bits, con maxima ganancia
             esto es, el coeficiente mayor se iguala a 2.^(b-1)-1

Parametros:
    N = orden del filtro (numero coeficientes = N+1)
    *B = puntero a vector de coeficientes de entrada
    *Bd = puntero a vector de coeficientes discretizados (salida)

*/
//----------------------------------------------------
void discretiza(int N, int b, float* B, short* Bd)
{
   float aux, mxB;
   int i;
   int mxK;

   mxK = (1 << (b - 1)) - 1;
   mxB = -1E10;    //un valor exageradamente bajo...

   for (i = 0; i <= N; i++)
   {
      if (fabs(B[i]) > mxB)
         mxB = fabs(B[i]);
   }//end for

  //for (i=0; mxB>0 && i<=N; i++)
   for (i = 0; mxB > 0 && i < N; i++)
   {
      // Normalizar a 1 (todos los coeficientes son < 1)
      aux = B[i]; // / mxB;  // SOLUCION PARA SEÃ‘ALES SATURADAS DE 1 MHZ
      // Convierte a 10 bits
      Bd[i] = Round16(mxK * aux);
   }//end for
} // fin discretiza


//========================================================
/*                       espectro
                        ==========

Descripcion: Calcula el espectro del vector x, de N muestras, N=par
// El espectro se compone de N/2 puntos, desde frecuencia 0 a la de Nyquist
// Para un vector con un numero impar de muestras, aÃ±adir una a 0.
// Para obtener un espectro de 100 puntos, aÂ¤adir los ceros necesarios al
// vector de entrada x hasta completar N=200.

{

Parametros:
    N = numero de muestras del vector de entrada
    *x = puntero a vector de muestras de entrada
    *y = puntero a vector de espectro de salida

Devuelve:
      Devuelve el valor maximo del espectro (util para normalizar)
*/
//----------------------------------------------------
float espectro(int N, float* x, float* y)
{
   int i, k;
   float Re, Im;
   float maxspec = 0.0;
   float aux;

   for (k = 0; k < N / 2 - 1; k++)
   {
      Re = 0.0;
      Im = 0.0;
      aux = 2 * M_PI * (float)k;
      for (i = 0; i <= N; i++)
      {
         Re = Re + x[i] * cos(aux * i / (N + 1.0));
         Im = Im + x[i] * sin(aux * i / (N + 1.0));
      }//fin bucle i
      y[k] = sqrt(Re * Re + Im * Im);
      if (y[k] > maxspec)
         maxspec = y[k];
   }// fin bucle k
   return maxspec;
}// fin espectro


//========================================================
/*                     funit
                      =======

Descripcion: Calcula un filtro unidad

Parametros:
    N = orden del filtro (numero coefs = N+1)
    *B = puntero a vector de coeficientes

*/
//----------------------------------------------------
void funit(int N, float* B)
{
   WORD k;
   for (k = 0; k <= N; k++)
      B[k] = 0.0;
   B[N / 2] = 1.0;
}//end funit
//========================================================
/*                      CalculaFiltro
         =============

Descripcion: Obtiene los coeficientes discretizados para un filtro FIR
        de orden NFIL (global), con NBITCOE bits cada uno (global)

Parametros:
     fb: frecuencia baja (kHz)
     fa: frecuencia alta (kHz)
     fs: frecuencia de muestreo (KHz)

Notas: si fb <= fs/NFIL crea un paso-bajo con frecuencia de corte fa
       si, ademÂ s, fa >= fs/2, crea un paso-todo (filtro unidad)

Devuelve: el valor teÂ¢rico del escalado Â¢ptimo para ecualizar la amplitud
     de la respuesta a la del filtro unidad.

*/
//----------------------------------------------------
#define MAX_COEF 64
// OJO: NFIL debe ser menor que MAX_COEF
unsigned short CalculaFiltro(float fb,
   float fa,
   float fs,
   int NFIL,
   int NBITCOE,
   short* Bd,
   vector<float>& fft)
{
   float  spec[2 * MAX_COEF];               // espectro del filtro
   WORD k = 0;
   float max;
   float B[MAX_COEF + 1];

   //if (fs > 40000) fs = 40000.;
   float fa_max = fs / 2. - fs / NFIL;

   fa /= 1000.;
   fb /= 1000.;
   fs /= 1000.;
   if (fs > 0)
   {
      if (fa > fa_max)
         fa = fs / 2.;

      fpbanda(NFIL, 2. * fb / fs, 2. * fa / fs, B);
      discretiza(NFIL, NBITCOE, B, Bd);
      for (k = 0; k < NFIL; k++)
         B[k] = (float)Bd[k];
      max = espectro(NFIL, B, spec);            //maximo de espectro
   }
   //if (fir_print_file_)
   {
      FILE* fd = fopen("mcfir.txt", "w+");
      for (int j = 0; fd != NULL && j < NFIL; j++)
         fprintf(fd, "%d ", (short int)Bd[j]);
      if (fd != NULL) fclose(fd);
   }

   //	k = 32767; // SOLUCION PARA SEÃ‘ALES SATURADAS DE 1 MHZ
   k = 16384;//0x4000
   return k;
}
