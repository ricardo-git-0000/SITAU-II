#ifndef fl_apertureH
#define fl_apertureH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_tad.h"

class CFL_Aperture
{
public:

	CFL_Aperture();

   ~CFL_Aperture();

   //int Resize (unsigned short n_fl, unsigned short *ap_size);

   int Resize (unsigned short n_fl, unsigned short ap_size);

   int GetEndElement (vector<unsigned short> &end_element);

   int GetApertureCoordinates (float *x_pa, float *z_pa, int n_elements, float pitch);

   float GetNearFieldLimit (int ind_fl, float frequency, float material_velocity);

   int *ap_; /**< Puntero que contiene el índice de los elementos
                             que forman cada una de las aperturas del barrido.<br>
                             El índice del primer elemento es el 0. */

   unsigned short *ap_size_; /**< Puntero que contiene el número de elementos de
                                  cada una de las aperturas del barrido. */

   unsigned short *start_element_; /**< Puntero que contiene el índice del elemento
                                        inicial de cada una de las aperturas del
                                        barrido. */

   float *x_; /**< Puntero que contiene las coordenas X (en mm) de los elementos
                   de cada una de las aperturas del barrido. */

   float *z_; /**< Puntero que contiene las coordenas Y (en mm) de los elementos
                    de cada una de las aperturas del barrido. */

   float *x_center_; /**< Puntero que contiene las coordenas X (en mm) de los
                          centros de cada una de las aperturas del barrido. */

   float *z_center_; /**< Puntero que contiene las coordenas Y (en mm) de los
                          centros de cada una de las aperturas del barrido. */

   unsigned short n_fl_; /**< Número de leyes focales que forman el barrido. */

   unsigned short n_delay_; /**< Número total de retardos que componen todas las
                                 leyes focales que forman el barrido.<br>
                                 Este número coincide con la suma de los tamaños
                                 de todas las aperturas, es decir, con la suma
                                 de los elementos del vector \c 'ap_size_' */

   unsigned short min_element_; /**< Es el índice menor de los elementos que
                                     forman parte de todas las aperturas del barrido. */

   unsigned short max_element_; /**< Es el índice mayor de los elementos que
                                     forman parte de todas las aperturas del barrido. */

   float *ap_dimension2_; /**< Puntero que contiene la dimensión^2 (en mm) de cada
                              una de las aperturas del barrido. */

	float *x_ap_size_; /**< Puntero que contiene la dimensión (en mm) en el eje X
                           de cada una de las aperturas del barrido. */

   float *z_ap_size_; /**< Puntero que contiene la dimensión (en mm) en el eje Z
                           de cada una de las aperturas del barrido. */

private:

};

#endif

