// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <math.h>
#include <algorithm>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "fl_pa_ui.h"
#include "fl_scan_ui.h"
#include "fl_error_code.h"
#include "c_log_file.h"

using namespace std;

CFL_PAUI::CFL_PAUI ( unsigned short n_elements,
                     float pitch,
                     float felement_dist,
                     float frequency,
                     float radius) : CFL_PA (n_elements, frequency)
{
	if (  felement_dist < 0 || pitch < 0)
		throw ELOG(charEFL_PARAMETER,EFL_PARAMETER);

	if (radius != 0 && (n_elements * pitch) > radius)
      throw ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    pitch_ = pitch;
   felement_dist_ = felement_dist;
   radius_ = radius;
   n_elements_ = n_elements;
   pin_to_channel_assignment_ = 0;
   PACoordinates();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_PAUI::SetConfig (unsigned short n_elements, float pitch, float frequency, float felement_dist, float radius)
{
    int i;// , n_scan = scan_.size();

    if (frequency < 0.01 || felement_dist < 0 || pitch < 0 || n_elements < 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    // Comprueba si el radio es mayor que el valor máximo para el tamaño del PA
	if (radius != 0 && (n_elements - 1) * pitch / (2 * radius) > 1) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    pitch_ = pitch;
    frequency_ = frequency;
    felement_dist_ = felement_dist;
    n_elements_ = n_elements;
    SetElementNumber(n_elements);
    radius_ = radius;
    PACoordinates();
    if (pin_to_channel_.size() != n_elements)
    {
        pin_to_channel_.resize(n_elements);
        for (i = 0; i < n_elements; i++) pin_to_channel_[i] = i;
    }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_PAUI::FirstElementCoordinates(float* x, float* z)
{
    if (x_.size() > 0 && z_.size() > 0)
    {
        if (x != NULL) *x = x_.front();
        if (z != NULL) *z = z_.front();
    }
    else return ELOG(charEFL_DataInconsistency, EFL_DataInconsistency);
    return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_PAUI::LastElementCoordinates(float* x,  float* z)
{
    if (x_.size() > 0 && z_.size() > 0)
    {
        if (x != NULL) *x = x_.back();
        if (z != NULL) *z = z_.back();
    }
    else return ELOG(charEFL_DataInconsistency, EFL_DataInconsistency);
    return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_PAUI::PACoordinates (void)
{
int j, i_aux;

    if (x_.size() != n_elements_)
    {
       	x_.resize(n_elements_);
       	z_.resize(n_elements_);
    }
    for (j=0; j<n_elements_; j++)
   	{
        x_[j] = felement_dist_ + j * pitch_;
        z_[j] = 0;
    }
    i_aux = floor((float)n_elements_ / 2.);
    if (n_elements_ % 2 == 0) // Si es un número par
    {
        x_center_ = (x_[i_aux-1] + x_[i_aux]) / 2.;
        z_center_ = (z_[i_aux-1] + z_[i_aux]) / 2.;
    }
    else // Si es un numero impar
    {
        x_center_ = x_[i_aux];
        z_center_ = z_[i_aux];
    }
    return EFL_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CFL_PAUI::CONFIG_Copy (CFL_PAUI *pa_probe)
{
   if (pa_probe == NULL) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);
   n_elements_ = pa_probe->GetElementNumber();
   pitch_ = pa_probe->GetPitch();
   felement_dist_ = pa_probe->GetFirstElementDist();
   radius_ = pa_probe->GetRadius();
   frequency_ = pa_probe->GetFrequency();
   pin_to_channel_assignment_ = pa_probe->pin_to_channel_assignment_;
   pin_to_channel_.assign(pa_probe->pin_to_channel_.begin(), pa_probe->pin_to_channel_.end());
   PACoordinates();
   return EFL_NONE;
}



