// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <math.h>
#include <stdlib.h>
//#include <mem.h>
#include <dos.h>
#include <float.h>
#include <vector>
#include <exception>
#include <algorithm>
#include <limits>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "utils.h"
#include "c_log_file.h"
#include "common_error_code.h"
#include "fileformat.h"

typedef std::vector<int> vectori;
typedef std::vector<double> vectord;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString ChangeFileExtension (AnsiString file_path, AnsiString extension)
{
int i, idx_dot;
AnsiString result = "";// = file_path;
bool flag;

   for (flag=false, i=StrLength(file_path)-1; i>0 && !flag; i--)
   {
      if (file_path[i] == '.') {flag = true; i++;}
      else if (file_path[i] == '\\') i = 0;
   }
   idx_dot = i;
   if (idx_dot > 0)
   {
      for (i=1; i<idx_dot && i<StrLength(file_path); i++)
#if defined(_MSC_VER)
         result.insert(StrLength(result), i, file_path[i]);
#else
         result.Insert(file_path[i], StrLength(result)+1);
#endif
      result += extension;
   }
   else result = file_path + extension;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool StrContains (AnsiString str, AnsiString sub_str)
{
bool result = false;

#if defined(_MSC_VER)
   result = str.find(sub_str);
#else
   if (str.Pos(sub_str) > 0) result = true;
#endif
   return result;

}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int StrLength (AnsiString str)
{
#if defined(_MSC_VER)
   return (int)str.size();
#else
   return (int)str.Length();
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
AnsiString ToUpper (AnsiString data)
{
#if defined(_MSC_VER)
int i;
AnsiString result = data;

	for (i=0; i<data.size(); i++)
	   result[i] = toupper(result[i]);
   return result;
#else
   return data.UpperCase();
#endif
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float dB2Linear (float db)
{
   return (float)pow((float)10., db/(float)20.);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float Linear2dB (float linear)
{
   return (float)20. * (float)log10(linear);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned int Log2 (unsigned int data)
{
unsigned int result = 0;

   while (data >>= 1) result++;
   //result++;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void *Realloc(void *ptr, size_t n_data)
{
void* ptr_realloc = NULL;

   if ((ptr_realloc = realloc(ptr, n_data)) == NULL)
   {
      if (ptr != NULL) free(ptr);
      ELOG(charECOM_NoMemory, ECOM_NoMemory);
   }
   return ptr_realloc;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void* Malloc(size_t n_data)
{
   void* ptr_malloc = NULL;

   if ((ptr_malloc = malloc(n_data)) == NULL) {ELOG(charECOM_NoMemory, ECOM_NoMemory);}
   return ptr_malloc;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
template<typename C> void shrinkContainer(C& container) 
{
   if (container.size() != container.capacity()) 
   {
      C tmp = container;
      swap(container, tmp);
   }
   //container.size() == container.capacity()
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int Round32 (float data)
{
int result, sgn = 1;

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5 )
		result = (int)((data + 1) * sgn);
	else
		result = (int)(data * sgn);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned long Round32u (float data)
{
	if (data < 0) return 0;
	if (data - (int)data > 0.5 )
		return (unsigned long)(data + 1);
	else
		return (unsigned long)data;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
short int Round16 (float data)
{
int data_int, sgn = 1;
short int result;
int min_value = numeric_limits<short int>::min();
int max_value = numeric_limits<short int>::max();

	if (data < 0) sgn = -1;
	data *= sgn;

	if (data - (int)data > 0.5 )
		data_int = (int)((data + 1) * sgn);
	else
		data_int = (int)(data * sgn);
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (short int)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short Round16u (float data)
{
int data_int;
unsigned short result;
int max_value = numeric_limits<unsigned short>::max();

	if (data < 0) return 0;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1);
	else
		data_int = (int)data;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned short)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short Round16u_d (double data)
{
int data_int;
unsigned short result;
int max_value = numeric_limits<unsigned short>::max();

	if (data < 0) return 0;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1);
	else
		data_int = (int)data;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned short)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned char Round8u (double data)
{
int data_int, sgn = 1;
unsigned char result;
unsigned char min_value = numeric_limits<unsigned char>::min();
unsigned char max_value = numeric_limits<unsigned char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned char Round8u (float data)
{
int data_int, sgn = 1;
unsigned char result;
unsigned char min_value = numeric_limits<unsigned char>::min();
unsigned char max_value = numeric_limits<unsigned char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (unsigned char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
char Round8 (double data)
{
int data_int, sgn = 1;
char result;
char min_value = numeric_limits<char>::min();
char max_value = numeric_limits<char>::max();

	if (data < 0) sgn = -1;
	data *= sgn;
	if (data - (int)data > 0.5)
		data_int = (int)(data + 1) * sgn;
	else
		data_int = (int)data * sgn;
   if (data_int < min_value) data_int = min_value;
   if (data_int > max_value) data_int = max_value;
   result = (char)data_int;
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
// int RoundSup (float f)
// {
// int sgn = 1;
// float aux;


	// if (f < 0)
	// {
		// aux = f * (-1);
		// if (aux - (int)aux > 0)
			// return (int)(f-1);
		// else
			// return (int)f;
	// }
	// else
	// {
		// if (f-(int)f > 0)
			// return (int)((f + 1) * sgn);
		// else
			// return (int)(f * sgn);
    // }
// }
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int RoundUp32 (float f)
{
	if (f-(int)f > 0)
		return (int)(f+1);
	else
		return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
unsigned short RoundUp16u (float f)
{
	if (f-(int)f > 0)
		return (unsigned short)(f+1);
	else
		return (unsigned short)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int RoundDown (float f)
{
	if (f < 0)
	{
		if (f-(int)f > 0)
			return (int)f;
		else
			return (int)(f-1);
	}
	else
		return (int)f;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float Media(float *array, int n)
{
    float suma=0;
    for (int i=0; i<n; i++)
        suma += array[i];

    return (float)(suma/(1.0 * n));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FindLowerData (vector<float> &distance, int ind_ref, float data)
{
int i, size = (int)distance.size();
bool fin, dec = true;
int result = 0;

   // Busca el primer valor mayor
	for (fin=false, i=ind_ref; !fin && i<size;)
   {
   	if (distance[i] > data) fin = true;
      else i++;
   }

   if (!fin) // Ningún valor del vector es mayor
      return size-1;

   else
   {
      // Busca el primer valor menor
      for (fin=false; !fin && i>=0 && i<size;)
      {
         if (distance[i] <= data)
         {
            fin = true;
            result = i;
         }
         else i--;
      }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FindLowerData_Int32 (vector<int> &distance, int ind_ref, float data)
{
int i, size = (int)distance.size();
bool fin, dec = true;
int result = 0;

   // Busca el primer valor mayor
	for (fin=false, i=ind_ref; !fin && i<size;)
   {
   	if (distance[i] > data) fin = true;
      else i++;
   }

   if (!fin) // Ningún valor del vector es mayor
      return size-1;

   else
   {
      // Busca el primer valor menor
      for (fin=false; !fin && i>=0 && i<size;)
      {
         if (distance[i] <= data)
         {
            fin = true;
            result = i;
         }
         else i--;
      }
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FindHigherData (vector<float> &distance, int ind_ref, float data)
{
int i, size = (int)distance.size();
bool fin, dec = true;
int result = 0;

   // Busca el primer valor menor
	for (fin=false, i=ind_ref; !fin && i>0;)
   {
   	if (distance[i] < data) fin = true;
      else i--;
	}

   // Busca el primer valor mayor
	for (fin=false, i=ind_ref; !fin && i<=size;)
   {
   	if (distance[i] >= data)
      {
      	fin = true;
         result = i;
      }
      else i++;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int FileCopy(char *src, char *dest)
{
FILE *fe = NULL;
FILE *fs = NULL;
unsigned char buffer[2048]; // Buffer de 2 Kbytes
int n_bytes;

   // Abrir el fichero de entrada en lectura y binario
   if (src == NULL) return ELOG(charECOM_PathFile, ECOM_PathFile);
#if !defined(_MSC_VER)
   if (!FileExists(AnsiString(src))) return ELOG(charECOM_PathFile, ECOM_PathFile);
#else
   if (!FS_FileReadAccess(src)) return ELOG(charECOM_PathFile, ECOM_PathFile);
#endif
   if ((fe = fopen(src, "rb")) == NULL) return ELOG(charECOM_PathFile, ECOM_PathFile);

   // Crear o sobreescribir el fichero de salida en binario
   if ((fs = fopen(dest, "wb")) == NULL)
      return ELOG(charECOM_PathFile, ECOM_PathFile);

   // Bucle de copia:
   while ((n_bytes = (int)fread(buffer, 1, 2048, fe)) > 0)
      fwrite(buffer, 1, n_bytes, fs);
   
   // Cerrar ficheros:
   fclose(fe);
   fclose(fs);
   return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
void polyfitval_2do_grado (vector<float> y, vector<float> &y_out)
{
vector<float> x;
int i, n = (int)y.size();
vector<vector<float> > A;
vector<float> B;
vector<float> coef;
float sum_x, sum_x2, sum_x3, sum_x4, sum_y, mult_xy, mult_x2y;
float a21, a22, a23, a31, a32, a33, b2, b3;

   x.resize(n);
   for (i=0; i<n; i++) x[i] = (float)i + (float)1.;

   // armo las matrices del sistema de ecuaciones
   A.resize(3);
   for (i=0; i<3; i++) A[i].resize(3);
   B.resize(3);
   coef.resize(3);

   sum_x = sum_x2 = sum_x3 = sum_x4 = sum_y = mult_xy = mult_x2y = 0;
   for (i=0; i<n; i++)
   {
      sum_x += x[i];
      sum_x2 += (float)pow(x[i],2);
      sum_x3 += (float)pow(x[i],3);
      sum_x4 += (float)pow(x[i],4);
      sum_y += y[i];
      mult_xy += x[i] * y[i];
      mult_x2y += x[i] * x[i] * y[i];
   }
   A[0][0] = (float)n;
   A[0][1] = sum_x;
   A[0][2] = sum_x2;

   A[1][0] = sum_x;
   A[1][1] = sum_x2;
   A[1][2] = sum_x3;

   A[2][0] = sum_x2;
   A[2][1] = sum_x3;
   A[2][2] = sum_x4;

   B[0] = sum_y;
   B[1] = mult_xy;
   B[2] = mult_x2y;

   // Resuelvo el sistema de ecuaciones Ax=B por el método de eliminación Gausiana
   // NOTA: a partir de aquí se puede usar cualquier código en C que resuelva un
   // sistema de ecuaciones de 3x3.

   if (A[1][0] == 0 && A[1][2] == 0)
   {
      // Las x son simetricas y la matriz es singular
      coef[1] = B[1] / A[1][1];
      coef[0] = (B[0] - A[0][0] / A[2][0] * B[2]) / (A[0][2] - A[0][0] / A[2][0] * A[2][2]);
      coef[2] = (B[0] - A[0][2] * coef[0]) / A[0][0];
   }
   else if (A[1][0] == 0 && A[1][2] != 0)
   {
      // La suma de las x da cero pero no son simetricas
      a21 = A[0][0];
      a22 = -A[0][2] / A[1][2] * A[1][1];
      a31 = A[0][0] - A[0][2] / A[2][2] * A[2][0];
      a32 = -A[0][2] / A[2][2] * A[2][1];
      b2  = B[0] - A[0][2] / A[1][2] * B[1];
      b3  = B[0] - A[0][2] / A[2][2] * B[2];

      coef[1] = (b2 - a21 / a31 * b3) / (a22 - a21 / a31 * a32);
      coef[2] = (b2 - a22 * coef[1]) / a21;
      coef[0] = (B[0] - A[0][0] * coef[2]) / A[0][2];
   }
   else
   {
      // Las x no son simetricas y la matriz es no singular
      a22 = A[0][1] - A[0][0] / A[1][0] * A[1][1];
      a23 = A[0][2] - A[0][0] / A[1][0] * A[1][2];
      a32 = A[0][1] - A[0][0] / A[2][0] * A[2][1];
      a33 = A[0][2] - A[0][0] / A[2][0] * A[2][2];
      b2  = B[0] - A[0][0] / A[1][0] * B[1];
      b3  = B[0] - A[0][0] / A[2][0] * B[2];

      coef[0] = (b2 - a22 / a32 * b3) / (a23 - a22 / a32 * a33);
      coef[1] = (b2 - a23 * coef[0]) / a22;
      coef[2] = (B[0] - A[0][2] * coef[0] - A[0][1] * coef[1]) / A[0][0];
   }
   y_out.resize(n);
   for (i=0; i<n; i++)
      y_out[i] = coef[0] * x[i] * x[i] + coef[1] * x[i] + coef[2];
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
void polyfit_2do_grado (vector<float> y, vector<float> &coef)
{
vector<float> x;
int i, n = (int)y.size();
vector<vector<float> > A;
vector<float> B;
//vector<float> coef;
float sum_x, sum_x2, sum_x3, sum_x4, sum_y, mult_xy, mult_x2y;
float a21, a22, a23, a31, a32, a33, b2, b3;

   x.resize(n);
   for (i=0; i<n; i++) x[i] =  (float)i + (float)1.;

   // armo las matrices del sistema de ecuaciones
   A.resize(3);
   for (i=0; i<3; i++) A[i].resize(3);
   B.resize(3);
   coef.resize(3);

   sum_x = sum_x2 = sum_x3 = sum_x4 = sum_y = mult_xy = mult_x2y = 0;
   for (i=0; i<n; i++)
   {
      sum_x += x[i];
      sum_x2 += (float)pow(x[i],2);
      sum_x3 += (float)pow(x[i],3);
      sum_x4 += (float)pow(x[i],4);
      sum_y += y[i];
      mult_xy += x[i] * y[i];
      mult_x2y += x[i] * x[i] * y[i];
   }
   A[0][0] = (float)n;
   A[0][1] = sum_x;
   A[0][2] = sum_x2;

   A[1][0] = sum_x;
   A[1][1] = sum_x2;
   A[1][2] = sum_x3;

   A[2][0] = sum_x2;
   A[2][1] = sum_x3;
   A[2][2] = sum_x4;

   B[0] = sum_y;
   B[1] = mult_xy;
   B[2] = mult_x2y;

   // Resuelvo el sistema de ecuaciones Ax=B por el método de eliminación Gausiana
   // NOTA: a partir de aquí se puede usar cualquier código en C que resuelva un
   // sistema de ecuaciones de 3x3.

   if (A[1][0] == 0 && A[1][2] == 0)
   {
      // Las x son simetricas y la matriz es singular
      coef[1] = B[1] / A[1][1];
      coef[0] = (B[0] - A[0][0] / A[2][0] * B[2]) / (A[0][2] - A[0][0] / A[2][0] * A[2][2]);
      coef[2] = (B[0] - A[0][2] * coef[0]) / A[0][0];
   }
   else if (A[1][0] == 0 && A[1][2] != 0)
   {
      // La suma de las x da cero pero no son simetricas
      a21 = A[0][0];
      a22 = -A[0][2] / A[1][2] * A[1][1];
      a31 = A[0][0] - A[0][2] / A[2][2] * A[2][0];
      a32 = -A[0][2] / A[2][2] * A[2][1];
      b2  = B[0] - A[0][2] / A[1][2] * B[1];
      b3  = B[0] - A[0][2] / A[2][2] * B[2];

      coef[1] = (b2 - a21 / a31 * b3) / (a22 - a21 / a31 * a32);
      coef[2] = (b2 - a22 * coef[1]) / a21;
      coef[0] = (B[0] - A[0][0] * coef[2]) / A[0][2];
   }
   else
   {
      // Las x no son simetricas y la matriz es no singular
      a22 = A[0][1] - A[0][0] / A[1][0] * A[1][1];
      a23 = A[0][2] - A[0][0] / A[1][0] * A[1][2];
      a32 = A[0][1] - A[0][0] / A[2][0] * A[2][1];
      a33 = A[0][2] - A[0][0] / A[2][0] * A[2][2];
      b2  = B[0] - A[0][0] / A[1][0] * B[1];
      b3  = B[0] - A[0][0] / A[2][0] * B[2];

      coef[0] = (b2 - a22 / a32 * b3) / (a23 - a22 / a32 * a33);
      coef[1] = (b2 - a23 * coef[0]) / a22;
      coef[2] = (B[0] - A[0][2] * coef[0] - A[0][1] * coef[1]) / A[0][0];
   }
//   y_out.resize(n);
//   for (i=0; i<n; i++)
//      y_out[i] = coef[0] * x[i] * x[i] + coef[1] * x[i] + coef[2];
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
void polyfitval_2do_grado (vector<float> y, vector<float> x, vector<float> &y_out)
{
//vector<float> x;
int i, n = (int)y.size();
vector<vector<float> > A;
vector<float> B;
vector<float> coef;
float sum_x, sum_x2, sum_x3, sum_x4, sum_y, mult_xy, mult_x2y;
float a21, a22, a23, a31, a32, a33, b2, b3;

//   x.resize(n);
//   for (i=0; i<n; i++) x[i] = i+1;

   // armo las matrices del sistema de ecuaciones
   A.resize(3);
   for (i=0; i<3; i++) A[i].resize(3);
   B.resize(3);
   coef.resize(3);

   sum_x = sum_x2 = sum_x3 = sum_x4 = sum_y = mult_xy = mult_x2y = 0;
   for (i=0; i<n; i++)
   {
	  sum_x += x[i];
	  sum_x2 += (float)pow(x[i],2);
	  sum_x3 += (float)pow(x[i],3);
	  sum_x4 += (float)pow(x[i],4);
	  sum_y += y[i];
	  mult_xy += x[i] * y[i];
	  mult_x2y += x[i] * x[i] * y[i];
   }
   A[0][0] = (float)n;
   A[0][1] = sum_x;
   A[0][2] = sum_x2;

   A[1][0] = sum_x;
   A[1][1] = sum_x2;
   A[1][2] = sum_x3;

   A[2][0] = sum_x2;
   A[2][1] = sum_x3;
   A[2][2] = sum_x4;

   B[0] = sum_y;
   B[1] = mult_xy;
   B[2] = mult_x2y;

   // Resuelvo el sistema de ecuaciones Ax=B por el método de eliminación Gausiana
   // NOTA: a partir de aquí se puede usar cualquier código en C que resuelva un
   // sistema de ecuaciones de 3x3.

   if (A[1][0] == 0 && A[1][2] == 0)
   {
	  // Las x son simetricas y la matriz es singular
	  coef[1] = B[1] / A[1][1];
	  coef[0] = (B[0] - A[0][0] / A[2][0] * B[2]) / (A[0][2] - A[0][0] / A[2][0] * A[2][2]);
	  coef[2] = (B[0] - A[0][2] * coef[0]) / A[0][0];
   }
   else if (A[1][0] == 0 && A[1][2] != 0)
   {
	  // La suma de las x da cero pero no son simetricas
	  a21 = A[0][0];
	  a22 = -A[0][2] / A[1][2] * A[1][1];
	  a31 = A[0][0] - A[0][2] / A[2][2] * A[2][0];
	  a32 = -A[0][2] / A[2][2] * A[2][1];
	  b2  = B[0] - A[0][2] / A[1][2] * B[1];
	  b3  = B[0] - A[0][2] / A[2][2] * B[2];

	  coef[1] = (b2 - a21 / a31 * b3) / (a22 - a21 / a31 * a32);
	  coef[2] = (b2 - a22 * coef[1]) / a21;
	  coef[0] = (B[0] - A[0][0] * coef[2]) / A[0][2];
   }
   else
   {
	  // Las x no son simetricas y la matriz es no singular
	  a22 = A[0][1] - A[0][0] / A[1][0] * A[1][1];
	  a23 = A[0][2] - A[0][0] / A[1][0] * A[1][2];
	  a32 = A[0][1] - A[0][0] / A[2][0] * A[2][1];
	  a33 = A[0][2] - A[0][0] / A[2][0] * A[2][2];
	  b2  = B[0] - A[0][0] / A[1][0] * B[1];
	  b3  = B[0] - A[0][0] / A[2][0] * B[2];

	  coef[0] = (b2 - a22 / a32 * b3) / (a23 - a22 / a32 * a33);
	  coef[1] = (b2 - a23 * coef[0]) / a22;
	  coef[2] = (B[0] - A[0][2] * coef[0] - A[0][1] * coef[1]) / A[0][0];
   }
   y_out.resize(n);
   for (i=0; i<n; i++)
	  y_out[i] = coef[0] * x[i] * x[i] + coef[1] * x[i] + coef[2];
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
void polyval (float *coef, int n_coef, vector<float> x, vector<float> &y_out)
{
int i, n_data = (int)x.size();
float x2;

   if (n_coef == 2)
   {
      y_out.resize(n_data);
      for (i=0; i<n_data; i++)
	      y_out[i] = coef[1] * x[i] + coef[0];;
   }
   else if (n_coef == 3)
   {
      y_out.resize(n_data);
      for (i=0; i<n_data; i++)
	      y_out[i] = coef[2] * x[i] * x[i] + coef[1] * x[i] + coef[0];
   }
   else if (n_coef == 4)
   {
      y_out.resize(n_data);
      for (i=0; i<n_data; i++)
      {
         x2 = x[i] * x[i];
	      y_out[i] = coef[3] * x[i] * x2 + coef[2] * x2 + coef[1] * x[i] + coef[0];
      }
   }
//   y_out.resize(n_data);
//   for (i=0; i<n_data; i++)
//   {
//      y_out[i] = coef[n_coef-1];
//      for (j=0,k=n_coef-1; j<n_coef-1; k--,j++)
//         y_out[i] += coef[j] * pow(x[i],k);
//   }
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int polyval (vector<float> &coefficient, vector<float> &x, vector<float> &y_out)
{
int i, j;
float data_float;

   y_out.clear();
   for (i=0; i<x.size(); i++)
   {
      for (j=0, data_float=0; j<coefficient.size(); j++)
      {
         if (j == 0) data_float += coefficient[j];
         else data_float += coefficient[j] * (float)pow(x[i], j);
      }
      y_out.push_back(data_float);
   }
   return 0;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
float polyval_y (vector<float> &coefficients, float x)
{
int j;
float data_float = 0;

   if (coefficients.size() > 0)
   {
      data_float = coefficients.front();
      for (j=1; j<coefficients.size(); j++)
         data_float += coefficients[j] * (float)pow(x, j);
   }
   return data_float;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int polyval ( float *coef,
               int n_coef,
               float x_min,
               float x_resolution,
               int n_data,
               float *x,
               float *y)
{
int i = 0;
float x_value, x2;
float A, B, C, D;

   if (n_coef == 2)
   {
      A = coef[1];
      B = coef[0];
      for (x_value=x_min, i=0; i<n_data; i++, x_value += x_resolution)
      {
	      x[i] = x_value;
         y[i] = A * x_value + B;
      }
   }
   else if (n_coef == 3)
   {
      A = coef[2];
      B = coef[1];
      C = coef[0];
      for (x_value=x_min, i=0; i<n_data; i++, x_value += x_resolution)
      {
	      x[i] = x_value;
         y[i] = A * x_value * x_value + B * x_value + C;
      }
   }
   else if (n_coef == 4)
   {
      A = coef[3];
      B = coef[2];
      C = coef[1];
      D = coef[0];
      for (x_value=x_min, i=0; i<n_data; i++, x_value += x_resolution)
      {
	      x[i] = x_value;
         x2 = x_value * x_value;
         y[i] = A * x_value * x2 + B * x2 + C * x_value + D;
      }
   }
   return i;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int polyval ( float *coef,
               int n_coef,
               float x_min,
               float x_resolution,
               int n_data,
               double *x,
               double *y)
{
int i = 0;
double A, B, C, D, x2, x_value;

   if (n_coef == 2)
   {
      A = (double)coef[1];
      B = (double)coef[0];
      for (x_value=(double)x_min, i=0; i<n_data; i++, x_value += (double)x_resolution)
      {
	      x[i] = x_value;
         y[i] = A * x_value + B;
      }
   }
   else if (n_coef == 3)
   {
      A = (double)coef[2];
      B = (double)coef[1];
      C = (double)coef[0];
      for (x_value=(double)x_min, i=0; i<n_data; i++, x_value += (double)x_resolution)
      {
	      x[i] = x_value;
         y[i] = A * x_value * x_value + B * x_value + C;
      }
   }
   else if (n_coef == 4)
   {
      A = (double)coef[3];
      B = (double)coef[2];
      C = (double)coef[1];
      D = (double)coef[0];
      for (x_value=(double)x_min, i=0; i<n_data; i++, x_value += (double)x_resolution)
      {
	      x[i] = x_value;
         x2 = x_value * x_value;
         y[i] = A * x_value * x2 + B * x2 + C * x_value + D;
      }
   }
   return i;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int cirval (   float x_center,
               float z_center,
               float radius,
               vector<float> &x,
               vector<float> &y)
{
int j, i = 0;
float x_value, data_float;
int n_data = (int)x.size();

   y.resize(n_data);
   for (i=j=0; i<n_data; i++)
   {
      x_value = x[i];
      data_float = radius * radius - (x_value - x_center) * (x_value - x_center);
      if (data_float > 0)
      {
         x[j] = x_value;
         y[j] = (float)sqrt(data_float) + z_center;
         j++;
      }
   }
   x.resize(j);
   y.resize(j);
   return j;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int cirval (   float x_center,
               float z_center,
               float radius,
               float x_min,
               float x_resolution,
               int n_data,
               float *x,
               float *y)
{
int i = 0;
float x_value, data_float;
bool fin;

   data_float = (float)fabs(x_min - x_center);
   if (data_float > radius)
      x_min = radius - x_center + (float)0.001;

   for (fin=false, x_value=x_min, i=0; !fin && i<n_data; i++, x_value += x_resolution)
   {
      data_float = radius * radius - (x_value - x_center) * (x_value - x_center);
      if (data_float > 0)
      {
         x[i] = x_value;
         y[i] = (float)sqrt(data_float) + z_center;
      }
      else fin = true;
   }
   return i;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int cirval (   float x_center,
               float z_center,
               float radius,
               float x_min,
               float x_resolution,
               int n_data,
               double *x,
               double *y)
{
int i = 0;
double x_value, data_double;
bool fin;

   data_double = fabs((double)x_min - (double)x_center);
   if (data_double > (double)radius)
      x_min = radius - x_center + (float)0.001;

   for (fin=false, x_value=(double)x_min, i=0; !fin && i<n_data; i++, x_value += (double)x_resolution)
   {
      data_double = (double)radius * (double)radius - (x_value - (double)x_center) * (x_value - (double)x_center);
      if (data_double > 0)
      {
         x[i] = x_value;
         y[i] = sqrt(data_double) + (double)z_center;
      }
      else// fin = true;
      {
         x[i] = x_value;
         y[i] = (double)z_center;
      }
   }
   return i;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
int cirval (float x_center, float z_center, float radius, float x, float *z)
{
float data_float;

   if (z == NULL) return ELOG(charECOM_Parameter, ECOM_Parameter);
   *z = z_center;
   data_float = radius * radius - (x - x_center) * (x - x_center);
   if (data_float < 0)
      return ECOM_MathFunction;
   else *z = (float)sqrt(data_float) + z_center;
   return ECOM_NONE;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
float cirderval (float x_center, float z_center, float radius, float x)
{
float data_float, result = 0;

   data_float = radius * radius - (x - x_center) * (x - x_center);
   if (data_float < 0)
      {ELOG("Error en el cálculo de una función matemática", -1);}
   else
   {
      data_float = (float)sqrt(data_float);
      if (data_float == 0)
      {
         ELOG("Error en el cálculo de una función matemática", -1);
         if (x > x_center) result = 1e6;
         else result = -1e6;
      }
      else result = (x - x_center) / data_float;
   }
   return result;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
float polyval (float *coef, int n_coef, float x)
{
float x2, result = 0;

   if (n_coef == 2)
      result = coef[1] * x + coef[0];
   else if (n_coef == 3)
      result = coef[2] * x * x + coef[1] * x + coef[0];
   else if (n_coef == 4)
   {
      x2 = x * x;
      result = coef[3] * x * x2 + coef[2] * x2 + coef[1] * x + coef[0];
   }
   return result;
}
// -----------------------------------------------------------------------------
/**
Ajuste de los datos en x,y a una curva de segundo grado:

y = coef(1) * x^2 + coef(2) * x + coef(3)
*/
// -----------------------------------------------------------------------------
float polyderval (float *coef, int n_coef, float x)
{
float result = 0;

   if (n_coef == 2)
      result = coef[1];
   else if (n_coef == 3)
      result = 2 * coef[2] * x + coef[1];
   else if (n_coef == 4)
      result = 3 * coef[3] * x * x + 2 * coef[2] * x + coef[1];
   return result;
}
/*! polyfitval_n_grado.- funcion que genera los coeficientes y los valores de los puntos de nuestra funcion.
estos puntos estan en funcion a los parametros de entrada. El grado del polinomio a resolver es introducido
por el usuario.

El tiempo de calculo sera mayor siempre y cuando el polinomio sea elevado

@param[in] Y.-  puntos en y
@param[in] X.-  Puntos en X
@param[out] Y_out.- coeficientes de salida
@param[ount] puntos.-  puntos evaluados respecto (X,Y)
@param[in] degree.- Grado del polyfit

@retorna[out].- un valor de salida igual al error o a la ejecucción satisfactoria
*/
int polyfitval_n_grado  (  float *y,
                           float *x,
                           float *coefficients_ec,
                           float *y_fit,
                           float *deviation,
                           int *deviation_points,
                           float *deviation_mean,
                           float deviation_threshold,
                           int n_points,
                           int degree)
{
int solution;
int i,j,k,contador;
double temp,z;
vector<float> current_x, current_y;
double *a = NULL, *t = NULL, **r = NULL;
int n_deviation_points = 0;

   if (  n_points <= 0 ||
         x == NULL ||
         y == NULL ||
         coefficients_ec == NULL)
      return ELOG(charECOM_Parameter, ECOM_Parameter);

	/************************************************/
	/* Reservo la memoria necesaria para el calculo */
	/************************************************/
	if ((a = (double *)calloc(2*degree+1,sizeof(double)))== NULL)
      return ELOG(charECOM_NoMemory, ECOM_NoMemory);
	if ((t = (double *)calloc(degree+2,sizeof(double))) ==NULL)
   {
      if (a != NULL) free(a);
		return ELOG(charECOM_NoMemory, ECOM_NoMemory);
   }
	if ((r = (double **)calloc(degree+2,sizeof(double *))) == NULL )
   {
      if (a != NULL) free(a);
      if (t != NULL) free(t);
		return ELOG(charECOM_NoMemory, ECOM_NoMemory);
   }
	for (i=0 ; i <= degree+1 ; i++) r[i] = NULL;
	for (i=0 ; i <= degree+1 ; i++)
	{
		if ((r[i] = (double *)calloc(degree + 2, sizeof(double))) == NULL )
      {
         if (a != NULL) free(a);
         if (t != NULL) free(t);
         for (j=0; j < i; j++)
         {
            if (r[j] != NULL) free(r[j]);
         }
         if (r != NULL) free(r);
         return ELOG(charECOM_NoMemory, ECOM_NoMemory);
      }
   }
	/******************************************************/
	/* Rellenar las matrices con un sistema de ecuaciones */
	/******************************************************/
	a[0] = (double)n_points;
	contador=0;
	do
	{
		for (j=1; j <= 2.0*degree; j++)
			a[j] += (double)pow(x[contador],(double)j);
		for (k=0; k <= degree; k++)
		{
			if (x[contador] == 0) temp = 0;
         else temp = y[contador] * pow( x[contador],(double)k);
			r[k][degree+1] = t[k] + temp;
			t[k] += temp;
		}
		t[degree+1] += pow( y[contador], (double)2.0);
		contador++;
	}
	while (contador < n_points);

	for (j=0; j<= degree; j++)
	{
		for (k=0; k<= degree; k++) r[j][k] = a[j+k];
	}

	/*****************************************************/
	/* Resuelve el sistema de ecuaciones en las matrices */
	/*****************************************************/
	for (j=0; j<= degree; j++)
	{
		k = j;
		solution = 0;
		do
		{
			if (k > (degree))
				return ELOG("Error en el cálculo de una función matemática", -1);
			if (r[k][j] != 0.0)
			{
            solution = 1;
				for (i=0; i<= degree+1; i++)
				{
					temp = r[j][i];
               r[j][i] = r[k][i];
               r[k][i] = temp;
				}
				z = (double)1.0 / r[j][j];
				for (i=0; i<=degree+1; i++)
				{
					r[j][i] = z * r[j][i];
				}
				for (k=0; k<=degree; k++)
				{
					if (k!=j)
					{
   					z = -r[k][j];
	   				for (i=0; i<=degree+1; i++) r[k][i] += z * r[j][i];
					}
				}
			}
			else k = k+1;
		}
		while (!solution);
	}
	/************************************************/
	/* Pasamos los coeficientes                     */
	/************************************************/
	for (j=0; j<= degree; j++)
		coefficients_ec[j] = (float)r[j][degree+1];

   if (y_fit != NULL || deviation != NULL)
   {
      for (i=0; i<n_points; i++)
      {
         temp = coefficients_ec[0];
         for (j=1; j<=degree; j++)
         {
            temp = temp + (coefficients_ec[j] * pow(x[i], (float)j) )  ;
         }

         if (y_fit != NULL) y_fit[i] = (float)temp;
         if (deviation != NULL) deviation[i] = (float)fabs((float)temp - y[i]);
      }
   }

   if (  deviation_mean != NULL &&
         deviation != NULL)
   {
      for (*deviation_mean=(float)0,i=0; i<n_points; i++)
         *deviation_mean += deviation[i];
      *deviation_mean /= n_points;
   }

   if (  deviation_mean != NULL &&
         deviation != NULL &&
         deviation_points != NULL)
   {
      memset(deviation_points, -1, n_points * sizeof(int));
      for (j=i=0; i<n_points; i++)
      {
         if (fabs(deviation[i] - *deviation_mean) > deviation_threshold)
         {
            deviation_points[j] = i;
            n_deviation_points++;
            j++;
         }
      }
   }

	/*  liberamos la memoria   */
	if (a != NULL) free(a);
	if (t != NULL) free(t);
	for (i=0 ; i <= degree+1 ; i++)
   {
      if (r[i] != NULL) free(r[i]);
      r[i] = NULL;
   }
	if (r != NULL) free(r);
   a = NULL;
   t = NULL;
   r = NULL;
	return n_deviation_points;
}
/*! Función de ejuste de una circunferencia a un conjunto de puntos.

@param[in] xp.- Coordenadas de los puntos a los cuales ajustar en X
@param[in] zp.- Coordenadas de los puntos a los cuales ajustar en Z
@param[out] coef.- parametros de la circuinferencia
			 [0] .-  Coordenadas del centro de la circunferencia en X
			 [1] .-  Coordenadas del centro de la circunferencia en X
			 [2] .-  Radio de la circunferencia.
*/
int EstimaCirc (float *xp, float *zp, float *coef, int n_points)
{
float A, B, C, D, E;
int i;
float tempA1=0.0; float tempA2=0.0; float tempB2=0.0; float tempB1=0.0; float tempC1=0.0;
float tempD1=0.0; float tempD3=0.0; float tempE1=0.0; float tempE3=0.0; float Radio=0.0;

	/* Variables intermedias */
	for (i=0; i<n_points; i++)
	{
		tempA2 += xp[i];
		tempA1 += xp[i] * xp[i];
		tempB2 += zp[i];
		tempC1 += zp[i] * zp[i];
		tempB1 += xp[i] * zp[i];
		tempD1 += xp[i] * zp[i] * zp[i];
		tempD3 += xp[i] * xp[i] * xp[i];
		tempE1 += zp[i] * xp[i] * xp[i];
		tempE3 += zp[i] * zp[i] * zp[i];
	}
	A = (n_points * tempA1) - (tempA2 * tempA2);
	B = (n_points * tempB1) - (tempA2 * tempB2);
	C = (n_points * tempC1) - (tempB2 * tempB2);
	D = (float)0.5 * ( (n_points * tempD1) - (tempA2 * tempC1) + (n_points * tempD3) - (tempA2 * tempA1));
	E = (float)0.5 * ( (n_points * tempE1) - (tempB2 * tempA1) + (n_points * tempE3) - (tempB2 * tempC1));

	/* coordenadas centro estimado */
	coef[0] = ( (D*C) - (B*E) ) / ( (A*C) - (B*B) );
	coef[1] = ( (A*E) - (B*D) ) / ( (A*C) - (B*B) );

	/* Radio Estimado */
	for (i=0; i < n_points; i++)
	{
		Radio += (float)sqrt( (xp[i] - coef[0])*(xp[i] - coef[0])+ (zp[i] - coef[1])*(zp[i] - coef[1]));
	}
	coef[2] = Radio / n_points;
	return (1);
}
