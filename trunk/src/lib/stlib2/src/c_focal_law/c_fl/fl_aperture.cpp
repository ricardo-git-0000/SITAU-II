// -----------------------------------------------------------------------------
/**
 @file fl_aperture.cpp

 @brief Este archivo contiene la implementación de las funciones de la clase \c FL_Aperture.<br>
 
 Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
 un barrido.
 
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

#include <math.h>
#include <limits>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_aperture.h"
#include "fl_calc.h"
#include "fl_error_code.h"
#include "c_log_file.h"

template <class T>
static int resize_array(T** ptr, size_t count)
{
   T* resized_ptr = (T*)realloc(*ptr, count * sizeof(T));
   if (resized_ptr == NULL)
      return ELOG(charEFL_NO_MEM, EFL_NO_MEM);

   *ptr = resized_ptr;
   return EFL_NONE;
}

// -----------------------------------------------------------------------------
/**
 Constructor de la clase \c CFL_Aperture. 
*/
// -----------------------------------------------------------------------------
CFL_Aperture::CFL_Aperture ()
{
	n_fl_ = 0;
	n_delay_ = 0;
   min_element_ = 0;
   max_element_ = 0;
	ap_size_ = NULL;
   ap_ = NULL;
   x_center_ = NULL;
   z_center_ = NULL;
   x_ap_size_ = NULL;
   z_ap_size_ = NULL;
   start_element_ = NULL;
	x_ = NULL;
   z_ = NULL;
   ap_dimension2_ = NULL;
}
// -----------------------------------------------------------------------------
/**
 Destructor de la clase \c CFL_Aperture. 
*/
// -----------------------------------------------------------------------------
CFL_Aperture::~CFL_Aperture()
{
	if (ap_size_ != NULL) free(ap_size_);
	if (x_ap_size_ != NULL) free(x_ap_size_);
	 if (z_ap_size_ != NULL) free(z_ap_size_);
   if (ap_ != NULL) free(ap_);
	if (start_element_ != NULL) free(start_element_);
	if (x_ != NULL) free(x_);
	if (z_ != NULL) free(z_);
   if (x_center_ != NULL) free(x_center_);
   if (z_center_ != NULL) free(z_center_);
   if (ap_dimension2_ != NULL) free(ap_dimension2_);

	ap_size_ = NULL;
	x_ap_size_ = NULL;
   z_ap_size_ = NULL;
   ap_ = NULL;
	start_element_ = NULL;
	x_ = NULL;
	z_ = NULL;
   x_center_ = NULL;
   z_center_ = NULL;
   ap_dimension2_ = NULL;

   n_fl_ = 0;
   n_delay_ = 0;
   min_element_ = 0;
   max_element_ = 0;
}
// -----------------------------------------------------------------------------
/**
 Re-dimeniona los vectores y el tamaño de la memoria asignada a las distintos
 parámetros internos de la clase \c CFL_Aperture, para un barrido en el que las
 aperuras tienen el mismo tamaño.

 @param[in] n_fl    Número de leyes focales que forman el barrido.

 @param[in] ap_size Tamaño de las aperuras del barrido.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int CFL_Aperture::Resize (unsigned short n_fl, unsigned short ap_size)
{
int i, n_delay, result;

	if (n_fl_ != n_fl)
   {
		if ((result = resize_array(&ap_size_, n_fl)) < 0)
			return RLOG(result);

		if ((result = resize_array(&start_element_, n_fl)) < 0)
			return RLOG(result);

		if ((result = resize_array(&x_ap_size_, n_fl)) < 0)
			return RLOG(result);

		if ((result = resize_array(&z_ap_size_, n_fl)) < 0)
			return RLOG(result);

      if ((result = resize_array(&ap_dimension2_, n_fl)) < 0)
			return RLOG(result);

      if ((result = resize_array(&x_center_, n_fl)) < 0)
			return RLOG(result);

      if ((result = resize_array(&z_center_, n_fl)) < 0)
			return RLOG(result);
   }
   n_fl_ = n_fl;

	for (i=0; i<n_fl; i++) {ap_size_[i] = ap_size;}

   n_delay = n_fl * ap_size;
   if (n_delay != n_delay_)
   {
		if ((result = resize_array(&ap_, n_delay)) < 0)
			return RLOG(result);

      if ((result = resize_array(&x_, n_delay)) < 0)
			return RLOG(result);

      if ((result = resize_array(&z_, n_delay)) < 0)
			return RLOG(result);
	}
   n_delay_ = n_delay;
	return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**
 Devuelve los índices de los elementos finales de cada una de las aperturas que
 forman el barrido barrido.

 @param[out] end_element Vector en el que se devuelven los índices de los últimos
                         elementos de cada una de las aperturas que forman el barrido.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
int CFL_Aperture::GetEndElement (vector<unsigned short> &end_element)
{
int i, ind;

   end_element.clear();
   end_element.resize(n_fl_);
	for (i=ind=0; i<n_fl_; i++)
   {
      if (ind >= n_delay_)
         return ELOG(charEFL_PARAMETER, EFL_PARAMETER);

		if (ind < 0)
			return ELOG(charEFL_PARAMETER, EFL_PARAMETER);

		end_element[i] = ap_[ind];      //no parece que este bien deberia ser ap_[ind + ap_size_[i]]
		ind += ap_size_[i];
	}
	return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Aperture::GetApertureCoordinates (float *x_pa, float *z_pa, int n_elements, float pitch)
{
int i, j, ind_e;
float x_min, x_max, z_min, z_max;

	// Focal laws number
	for (ind_e=i=0; i<n_fl_; i++)
	{
		x_min = numeric_limits<float>::max();
		x_max = numeric_limits<float>::max() * -1;
		z_min = numeric_limits<float>::max();
		z_max = numeric_limits<float>::max() * -1;

		// Coordinates of aperture elements
		for (j=0; j<ap_size_[i]; j++)
		{
			if (ind_e >= n_delay_)
            return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);

         if (ap_[ind_e] > n_elements)
            return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);

			x_[ind_e] = x_pa[ap_[ind_e]];
			if (x_min > x_[ind_e]) x_min = x_[ind_e];
			if (x_max < x_[ind_e]) x_max = x_[ind_e];

			z_[ind_e] = z_pa[ap_[ind_e]];
			if (z_min > z_[ind_e]) z_min = z_[ind_e];
			if (z_max < z_[ind_e]) z_max = z_[ind_e];

			ind_e++;
		}
		x_ap_size_[i] = fabs(x_max - x_min) + pitch;
		z_ap_size_[i] = fabs(z_max - z_min);

		ap_dimension2_[i] = x_ap_size_[i] * x_ap_size_[i] + z_ap_size_[i] * z_ap_size_[i];

		x_center_[i] = x_min + (x_max - x_min) / 2.;
		z_center_[i] = z_min + (z_max - z_min) / 2.;
	}
	return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

 @param[in] n_fl    Número de leyes focales que forman el barrido.

 @param[in] ap_size Vector que contiene los tamaños de cada una de las aperturas
                    que forman el barrido.

 @return Ver códigos de error.
*/
// -----------------------------------------------------------------------------
float CFL_Aperture::GetNearFieldLimit (int ind_fl, float frequency, float material_velocity)
{
float result = 1;

   if (material_velocity > 0 && frequency > 0)
      result = 1000. * frequency * ap_dimension2_[ind_fl] / (4. * material_velocity);
   return result;
}
