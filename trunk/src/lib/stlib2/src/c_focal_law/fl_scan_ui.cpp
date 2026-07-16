#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
   #define WIN32_LEAN_AND_MEAN
   #define NOMINMAX
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================
#include <math.h>
#include <time.h>
#include <dos.h>
#include <algorithm>
#include <limits>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "fl_scan_ui.h"
#include "fl_error_code.h"
#include "c_log_file.h"
#include "fl_calc.h"
#include "utils.h"

template <class T>
static int resize_array(T** ptr, size_t count)
{
   T* resized_ptr = (T*)realloc(*ptr, count * sizeof(T));
   if (resized_ptr == NULL)
      return ELOG(charEFL_NO_MEM, EFL_NO_MEM);

   *ptr = resized_ptr;
   return EFL_NONE;
}

using namespace std;

CFL_ScanUI::CFL_ScanUI (CFL_PAUI *pa, CFL_SpecimenUI *specimen, int id) : 
   CFL_Scan (pa, specimen, id)
{
int result;

	pa_ = NULL;
   specimen_ = NULL;

   if (pa == NULL)
   		throw ELOG(charEFL_PA_NULL, EFL_PA_NULL);
   if (specimen == NULL)
   		throw ELOG(charEFL_PA_NULL, EFL_PA_NULL);

   n_parallel_beamformed_lines_ = 1;

	pa_ = pa;
   pa_->update_scan_ = &update_;
	
   specimen_ = specimen;
   specimen_->update_scan_ = &update_;


	emission_focus_ = 15;
	reception_focus_ = 50;

    x_reception_entry_point_th_ = NULL;
	x_emission_entry_point_th_ = NULL;

   scan_type_ = 0;
	scan_emission_start_element_ = 0;
	scan_reception_start_element_ = 0;
	scan_reception_ap_size_ = 8;
	scan_emission_ap_size_ = 8;
    // Linear Scan
	lin_scan_angle_ = 0.;
	lin_scan_end_element_ = 31;
	lin_scan_step_ = 1;
    // Sectorail Scan
   sec_scan_start_angle_ = -45.;
   sec_scan_end_angle_ = 45.;
	sec_scan_step_ = 1.;

   scan_type_ = FL_TYPE_LINEAR;
   scan_reception_start_element_ = 0;
   scan_reception_ap_size_ = 8;
   scan_emission_start_element_ = 0;
   scan_emission_ap_size_ = 8;
   lin_scan_end_element_ = 31;
   lin_scan_step_ = 1,
   lin_scan_angle_ = 0;
   if ((result = ConfigLinearScanStd (	scan_reception_start_element_,
                                       scan_reception_ap_size_,
                                       scan_emission_start_element_,
													scan_emission_ap_size_,
                                       1,
                                       lin_scan_end_element_,
                                       lin_scan_step_,
                                       lin_scan_angle_)) < 0)
      {RLOG(result);}

   fl_.focallaw_user  =0;;
	fl_.update = 1;
}

CFL_ScanUI::~CFL_ScanUI()
{
    if (x_emission_entry_point_th_ != NULL) free(x_emission_entry_point_th_);
    if (x_reception_entry_point_th_ != NULL) free(x_reception_entry_point_th_);
    x_emission_entry_point_th_ = NULL;
    x_reception_entry_point_th_ = NULL;

}
/* -----------------------------------------------------------------------------
Código para la discretización de la cuña y del array
   -------------------------------------------------------------------------- */
int CFL_ScanUI::CalPACoordinates_NoWedge (void)
{
int i;
int n_elements = pa_->GetElementNumber();

   x_pa_.resize(n_elements);
   z_pa_.resize(n_elements);

   for (i=0; i<n_elements; i++)
   {
      // Traslación del sistema de referencia
      // x, z son las coordenadas del origen sistema de referencia que queremos
      // trasladar
      x_pa_[i] = pa_->x_[i];
      z_pa_[i] = pa_->z_[i] * (-1);
   }
	return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigSectorialScan (  unsigned short *emission_ap,
													unsigned short *reception_ap,
                                    	unsigned short emission_ap_size,
                                    	unsigned short reception_ap_size,
	                              		float start_angle,
    	                          			float end_angle,
        	                      			float step_angle)
{
float pitch = pa_->GetPitch();
float lambda = specimen_->GetVelocity() / (1000. * pa_->GetFrequency());
float start_angle_rad, end_angle_rad, step_angle_rad;
int i, j, result, ind;
unsigned short emission_start_element, reception_start_element;
unsigned short n_efl, n_rfl;
unsigned short min_e_e = numeric_limits<unsigned short>::max();
unsigned short max_e_e = numeric_limits<unsigned short>::min();
unsigned short min_e_r = numeric_limits<unsigned short>::max();
unsigned short max_e_r = numeric_limits<unsigned short>::min();
unsigned short min_e_efl = 0, max_e_efl = 0, min_e_rfl = 0, max_e_rfl = 0;

	if (n_parallel_beamformed_lines_ <= 0) return ELOG(charEFL_DataInconsistency, EFL_DataInconsistency);

   try
    {
        //Me aseguro de que no se den los ángulos -90º y 90º porque acabarían dándome división por 0.
   	    if (start_angle <= -90.)	start_angle = -89.;
        if (end_angle >= 90.) end_angle = 89.;
        if (step_angle <= 0) end_angle = 1;

        start_angle_rad = start_angle * M_PI / 180.;
        end_angle_rad = end_angle * M_PI / 180.;
        step_angle_rad = step_angle * M_PI / 180.;

        emission_start_element = MinArrayUSHORT(emission_ap,emission_ap_size);
        reception_start_element = MinArrayUSHORT(reception_ap,reception_ap_size);

        n_rfl = Round16u((end_angle_rad - start_angle_rad) / step_angle_rad) + 1;
        if (n_rfl == 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

        reception_angle_.resize(n_rfl);
        //reception_angle_yz_.resize(n_rfl);
        //reception_angle_xy_.resize(n_rfl);

        if ((result = reception_ap_->Resize(n_rfl, reception_ap_size)) < 0)
        return RLOG(result);

        n_efl = RoundUp16u((float)n_rfl / (float)n_parallel_beamformed_lines_);
        if (n_efl == 0) n_efl = 1;//return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

        emission_angle_.resize(n_efl);
        //emission_angle_yz_.resize(n_efl);
        //emission_angle_xy_.resize(n_efl);

        if ((result = emission_ap_->Resize(n_efl, emission_ap_size)) < 0)
        return RLOG(result);

        if ((result = Resize()) < 0) return RLOG(result);

        for (i=0; i<n_efl; i++)
        {
        if (n_parallel_beamformed_lines_ == 1)
            emission_angle_[i] = start_angle_rad + step_angle_rad * (i * n_parallel_beamformed_lines_ + n_parallel_beamformed_lines_);
        else emission_angle_[i] = start_angle_rad + step_angle_rad * (i * n_parallel_beamformed_lines_ + n_parallel_beamformed_lines_ / 2);

        // Emission apertures of scan
        emission_ap_->start_element_[i] = emission_ap[0];
        for (j=0; j<emission_ap_size; j++)
        {
            ind = i * emission_ap_size + j;
            if (ind >= emission_ap_->n_delay_)
                return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);
            emission_ap_->ap_[ind] = emission_ap[j];
            if (min_e_e > emission_ap_->ap_[ind])
            {
                min_e_e = emission_ap_->ap_[ind];
                min_e_efl = i;
            }
            if (max_e_e <= emission_ap_->ap_[ind])
            {
                max_e_e = emission_ap_->ap_[ind];
                max_e_efl = i;
            }
        }
        }

        for (i=0; i<n_rfl; i++)
        {
        reception_angle_[i] = start_angle_rad + step_angle_rad * i;

	        // Reception apertures of scan
        reception_ap_->start_element_[i] = reception_ap[0];
        for (j=0; j<reception_ap_size; j++)
        {
            ind = i * reception_ap_size + j;
            if (ind >= reception_ap_->n_delay_)
                return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);
            reception_ap_->ap_[ind] = reception_ap[j];
            if (min_e_r > reception_ap_->ap_[ind])
            {
                min_e_r = reception_ap_->ap_[ind];
                min_e_rfl = i;
            }
            if (max_e_r <= reception_ap_->ap_[ind])
            {
                max_e_r = reception_ap_->ap_[ind];
                max_e_rfl = i;
            }
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
	scan_type_ = 1;
	scan_emission_start_element_ = emission_start_element;
	scan_reception_start_element_ = reception_start_element;
	scan_reception_ap_size_ = reception_ap_size;
   scan_emission_ap_size_ = emission_ap_size;
	sec_scan_start_angle_ = start_angle;
   sec_scan_end_angle_ = end_angle;
	sec_scan_step_ = step_angle;
   emission_ap_->min_element_ = min_e_e;
   emission_ap_->max_element_ = max_e_e;
   reception_ap_->min_element_ = min_e_r;
   reception_ap_->max_element_ = max_e_r;

   //DDF_Aperture(reception_ap_);

   //if ((result = BackupEmissionAperture(emission_start_element)) < 0) {RLOG(result);}

   if ((result = emission_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
      return RLOG(result);

   if ((result = reception_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
      return RLOG(result);

   update_ = 1;
   if ((result = ConfigFocus (emission_focus_, reception_focus_)) < 0)
      return RLOG(result);

   if ((result = GetFocalLaw (   fl_.emission_center_tof,
										   fl_.reception_center_tof,
										   fl_.emission_focal_law_ticks,
                                 fl_.reception_focal_law_ticks,
                                 &fl_.reception_common_delay)) < 0)
      return RLOG(result);

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetSectorialScanFocalLawNumber (   float step,
                                                   unsigned short aperture,
                                                   unsigned short startelement,
                                                   float startangle,
                                                   float endangle)
{
float pitch = pa_->GetPitch();
float lambda = specimen_->GetVelocity() / (1000. * pa_->GetFrequency());
float start_angle_rad, end_angle_rad, step_angle_rad, range_angle;
int n_fl;

   // Me aseguro de que no se den los ángulos -90º y 90º porque acabarían dándome división por 0.
   if (startangle <= -90.) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (endangle >= 90.) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

   start_angle_rad = startangle * M_PI / 180.;
   end_angle_rad = endangle * M_PI / 180.;
   step_angle_rad = step * M_PI / 180.;

   if (step == 0) // Optimum angle
   {
      range_angle = fabs(sin(end_angle_rad) - sin(start_angle_rad)); //Uso fabs porque es para un flotante   //Absoluto por si th2 < th1 (mal definido)
      n_fl = ceil(4 * aperture * pitch * range_angle / lambda) + 1;     //entero verifica criterio de Nyquist
      if (n_fl > MAX_FOCAL_LAW_NUMBER) n_fl = MAX_FOCAL_LAW_NUMBER;
   }
   else
      n_fl = Round16u((end_angle_rad - start_angle_rad) / step_angle_rad) + 1;
   return n_fl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigSectorialScanStd ( 	unsigned short emission_start_element,
														unsigned short emission_ap_size,
                                          unsigned short reception_start_element,
                                          unsigned short reception_ap_size,
                                          unsigned short inc_element_ap,
                                          float start_angle,
                                          float end_angle,
                                          float step)
{
int i, result;
unsigned short *emission_ap = NULL, *reception_ap = NULL;


try
{
	if (emission_ap_size == 0 || reception_ap_size == 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (emission_start_element < 0 || reception_start_element < 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

   if ((emission_ap = (unsigned short *)malloc(emission_ap_size*sizeof(unsigned short))) == NULL)
    	return ELOG(charEFL_NO_MEM,EFL_NO_MEM);
	if ((reception_ap = (unsigned short *)malloc(reception_ap_size*sizeof(unsigned short))) == NULL)
		return ELOG(charEFL_NO_MEM,EFL_NO_MEM);

   for (i=0; i<emission_ap_size; i++)
   {
      emission_ap[i] = emission_start_element + inc_element_ap * i;
      if (emission_ap[i] > pa_->GetElementNumber())
         return ELOG(charEFL_PAElement, EFL_PAElement);
   }

   for (i=0; i<reception_ap_size; i++)
   {
      reception_ap[i] = reception_start_element + inc_element_ap * i;
      if (reception_ap[i] > pa_->GetElementNumber())
         return ELOG(charEFL_PAElement, EFL_PAElement);
   }

   if ((result = ConfigSectorialScan ( emission_ap,
                                       reception_ap,
                                       emission_ap_size,
                                       reception_ap_size,
                                       start_angle,
                                       end_angle,
                                       step)) < 0)
      return RLOG(result);

   if ((result = SetDynamicDephFocusing()) < 0) return RLOG(result);
}
__finally
{
   if (emission_ap != NULL) free(emission_ap);
   emission_ap = NULL;
   if (reception_ap != NULL) free(reception_ap);
   reception_ap = NULL;
}
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetLinearScanFocalLawNumber (   unsigned short *emission_ap,
                                                unsigned short *reception_ap,
                                                unsigned short emission_ap_size,
                                                unsigned short reception_ap_size,
                                                unsigned short end_element,
                                                unsigned short step,
                                                unsigned short *emission_start_element,
                                                unsigned short *reception_start_element)
{
unsigned short emission_start_element_l, reception_start_element_l;
unsigned short emission_end_element, reception_end_element;
unsigned short n_fl, ap_size;
int inc_element_ap = 1;

   emission_start_element_l = MinArrayUSHORT(emission_ap,emission_ap_size);
   reception_start_element_l = MinArrayUSHORT(reception_ap,reception_ap_size);
   emission_end_element = MaxArrayUSHORT(emission_ap,emission_ap_size);
   reception_end_element = MaxArrayUSHORT(reception_ap,reception_ap_size);

   if (emission_start_element_l >= pa_->GetElementNumber())
      return ELOG(charEFL_PAElement,EFL_PAElement);

   if (emission_end_element >= pa_->GetElementNumber())
      return ELOG(charEFL_PAElement,EFL_PAElement);

   if (reception_end_element >= pa_->GetElementNumber())
      return ELOG(charEFL_PAElement,EFL_PAElement);

   if (reception_start_element_l >= pa_->GetElementNumber())
      return ELOG(charEFL_PAElement,EFL_PAElement);

   if (emission_start_element_l < reception_start_element_l)
   {
      if (emission_end_element < reception_end_element)
         ap_size = reception_end_element - emission_start_element_l + 1;
      else
         ap_size = emission_end_element - emission_start_element_l + 1;
   }
   else
   {
      if (emission_end_element < reception_end_element)
         ap_size = reception_end_element - reception_start_element_l + 1;
      else
         ap_size = emission_end_element - reception_start_element_l + 1;
   }

   n_fl = Round16u((end_element - emission_start_element_l + 1 - ap_size) / step) + 1;
   //n_fl = Round16u((end_element - (ap_size + emission_start_element_l)) / step) + 1;
   if (emission_start_element != NULL) *emission_start_element = emission_start_element_l;
   if (reception_start_element != NULL) *reception_start_element = reception_start_element_l;
   return n_fl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetLinearScanFocalLawNumber (   unsigned short emission_start_element,
                                                unsigned short emission_ap_size,
                                                unsigned short reception_start_element,
                                                unsigned short reception_ap_size,
                                                unsigned short end_element,
		      		   		   						unsigned short step)
{
unsigned short emission_end_element, reception_end_element;
unsigned short n_fl, ap_size;
unsigned short *emission_ap=NULL, *reception_ap=NULL;
int i, inc_element_ap = 1;

	if (emission_ap_size <= 0 || reception_ap_size <= 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (emission_start_element <= 0 || reception_start_element <= 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (end_element <= 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

   if ((emission_ap = (unsigned short *)malloc(emission_ap_size*sizeof(unsigned short))) == NULL)
    	return ELOG(charEFL_NO_MEM,EFL_NO_MEM);
	if ((reception_ap = (unsigned short *)malloc(reception_ap_size*sizeof(unsigned short))) == NULL)
    	return ELOG(charEFL_NO_MEM,EFL_NO_MEM);

   for (i=0; i<emission_ap_size; i++)
      emission_ap[i] = emission_start_element + inc_element_ap * i - 1;

   for (i=0; i<reception_ap_size; i++)
      reception_ap[i] = reception_start_element + inc_element_ap * i - 1;

   emission_start_element = MinArrayUSHORT(emission_ap,emission_ap_size);
   reception_start_element = MinArrayUSHORT(reception_ap,reception_ap_size);
   emission_end_element = MaxArrayUSHORT(emission_ap,emission_ap_size);
   reception_end_element = MaxArrayUSHORT(reception_ap,reception_ap_size);

   if (emission_start_element < reception_start_element)
   {
      if (emission_end_element < reception_end_element)
         ap_size = reception_end_element - emission_start_element + 1;
      else
         ap_size = emission_end_element - emission_start_element + 1;
   }
   else
   {
      if (emission_end_element < reception_end_element)
         ap_size = reception_end_element - reception_start_element + 1;
      else
         ap_size = emission_end_element - reception_start_element + 1;
   }

   n_fl = Round16u((end_element - (ap_size + emission_start_element)) / step) + 1;

   if (emission_ap != NULL) free(emission_ap);
   emission_ap = NULL;
   if (reception_ap != NULL) free(reception_ap);
   reception_ap = NULL;

   return n_fl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigLinearScan (unsigned short *emission_ap, unsigned short *reception_ap, unsigned short emission_ap_size, unsigned short reception_ap_size, unsigned short end_element, unsigned short step, float angle)
{
int i, j, result, ind, n_rfl, n_efl;
unsigned short emission_start_element, reception_start_element;
float angle_rad = angle * M_PI / 180.;
unsigned short min_e_e = numeric_limits<unsigned short>::max();
unsigned short max_e_e = numeric_limits<unsigned short>::min();
unsigned short min_e_r = numeric_limits<unsigned short>::max();
unsigned short max_e_r = numeric_limits<unsigned short>::min();
unsigned short min_e_efl = 0, max_e_efl = 0, min_e_rfl = 0, max_e_rfl = 0;

	try
   {
      if (end_element >= pa_->GetElementNumber()) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
      if ((n_rfl = GetLinearScanFocalLawNumber (emission_ap, reception_ap, emission_ap_size, reception_ap_size, end_element, step, &emission_start_element, &reception_start_element)) < 0)
         return RLOG(n_rfl);

      reception_angle_.resize(n_rfl);
      //reception_angle_yz_.resize(n_rfl);
      //reception_angle_xy_.resize(n_rfl);

      if ((result = reception_ap_->Resize(n_rfl, reception_ap_size)) < 0)
      	return RLOG(result);

      n_efl = RoundUp16u((float)n_rfl / (float)n_parallel_beamformed_lines_);
      if (n_efl == 0) n_efl = 1;//return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

      emission_angle_.resize(n_efl);
      //emission_angle_yz_.resize(n_efl);
      //emission_angle_xy_.resize(n_efl);

      if ((result = emission_ap_->Resize(n_efl, emission_ap_size)) < 0)
      	return RLOG(result);

      if ((result = Resize()) < 0) return RLOG(result);

      for (i=0; i<n_efl; i++)
      {
         emission_angle_[i] = angle_rad;

		// Emission apertures of scan
         emission_ap_->start_element_[i] = emission_ap[0] + step * i;
         if (emission_ap_->start_element_[i] < 0) emission_ap_->start_element_[i] = 0;
         for (j=0; j<emission_ap_size; j++)
         {
            ind = i * emission_ap_size + j;
            if (ind >= emission_ap_->n_delay_) return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);
            emission_ap_->ap_[ind] = emission_ap[j] + step * i;
            if (emission_ap_->ap_[ind] < 0) emission_ap_->ap_[ind] = 0;
            if (min_e_e > emission_ap_->ap_[ind])
            {
               min_e_e = emission_ap_->ap_[ind];
               min_e_efl = i;
            }
            if (max_e_e < emission_ap_->ap_[ind])
            {
               max_e_e = emission_ap_->ap_[ind];
               max_e_efl = i;
            }
         }
      }

      for (i=0; i<n_rfl; i++)
      {
         reception_angle_[i] = angle_rad;

         // Reception apertures of scan
         reception_ap_->start_element_[i] = reception_ap[0] + step * i;
         if (reception_ap_->start_element_[i] < 0) reception_ap_->start_element_[i] = 0;
			for (j=0; j<reception_ap_size; j++)
         {
            ind = i * reception_ap_size + j;
               if (ind >= reception_ap_->n_delay_) return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);
            reception_ap_->ap_[ind] = reception_ap[j] + step * i;
            if (reception_ap_->ap_[ind] < 0) reception_ap_->ap_[ind] = 0;
            if (min_e_r > reception_ap_->ap_[ind])
            {
               min_e_r = reception_ap_->ap_[ind];
               min_e_rfl = i;
            }
            if (max_e_r < reception_ap_->ap_[ind])
            {
               max_e_r = reception_ap_->ap_[ind];
               max_e_rfl = i;
            }
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

	scan_type_ = 0; // Linear Scanning
	scan_emission_start_element_ = emission_start_element;
	scan_reception_start_element_ = reception_start_element;
	scan_reception_ap_size_ = reception_ap_size;
   scan_emission_ap_size_ = emission_ap_size;
	lin_scan_angle_ = angle;
	lin_scan_end_element_ = end_element;
	lin_scan_step_ = step;
	emission_ap_->min_element_ = min_e_e;
   emission_ap_->max_element_ = max_e_e;
   reception_ap_->min_element_ = min_e_r;
	reception_ap_->max_element_ = max_e_r;

   //DDF_Aperture(reception_ap_);

   //if ((result = BackupEmissionAperture(emission_start_element)) < 0) {RLOG(result);}

   if ((result = emission_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
      return RLOG(result);

   if ((result = reception_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
      return RLOG(result);

   update_ = 1;
   if ((result = ConfigFocus (emission_focus_, reception_focus_)) < 0)
      return RLOG(result);

   if ((result = GetFocalLaw (   fl_.emission_center_tof,
										   fl_.reception_center_tof,
										   fl_.emission_focal_law_ticks,
                                 fl_.reception_focal_law_ticks,
                                 &fl_.reception_common_delay)) < 0)
      return RLOG(result);
   return n_rfl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigLinearScanStd (int emission_start_element, int emission_ap_size, int reception_start_element, int reception_ap_size, int inc_element_ap, int end_element, int step, float angle)
{
int i, result, n_rfl;
unsigned short *emission_ap = NULL, *reception_ap = NULL;

    if (angle > 90) angle = 89;
    if (angle < -90) angle = -89;

    if (emission_ap_size <= 0 || reception_ap_size <= 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (emission_start_element < 0 || reception_start_element < 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (step <= 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
	if (end_element < 0) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    if ((emission_ap = (unsigned short *)malloc(emission_ap_size*sizeof(unsigned short))) == NULL) return ELOG(charEFL_NO_MEM,EFL_NO_MEM);
	if ((reception_ap = (unsigned short *)malloc(reception_ap_size*sizeof(unsigned short))) == NULL) return ELOG(charEFL_NO_MEM,EFL_NO_MEM);

    for (i=0; i<emission_ap_size; i++) emission_ap[i] = emission_start_element + inc_element_ap * i;
    for (i=0; i<reception_ap_size; i++) reception_ap[i] = reception_start_element + inc_element_ap * i;

    if ((n_rfl = ConfigLinearScan (emission_ap, reception_ap, emission_ap_size, reception_ap_size, end_element, step, angle)) < 0)
    {
        if (emission_ap != NULL) free(emission_ap);
        emission_ap = NULL;
        if (reception_ap != NULL) free(reception_ap);
        reception_ap = NULL;
        return RLOG(n_rfl);
    }
    if (emission_ap != NULL) free(emission_ap);
    emission_ap = NULL;
    if (reception_ap != NULL) free(reception_ap);
    reception_ap = NULL;

    if ((result = SetDynamicDephFocusing()) < 0) return RLOG(result);
    return n_rfl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ScanAperturesCoordinates (void)
{
int result, n_fl = emission_ap_->n_fl_;

   if (n_fl != x_emission_focus_.size())
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (n_fl != z_emission_focus_.size())
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (n_fl != x_reception_focus_.size())
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (n_fl != z_reception_focus_.size())
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (n_fl != reception_ap_->n_fl_)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

	try
   {
      if ((result = emission_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
         return RLOG(result);

      if ((result = reception_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
         return RLOG(result);

      if ((result = ConfigFocus (emission_focus_, reception_focus_)) < 0)
    	   return RLOG(result);

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
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetFocalLaw (vector<float> &emission_center_tof, vector<float> &reception_center_tof, vector<vector<WORD> > &emission_focal_law_ticks, vector<vector<WORD> > &reception_focal_law_ticks, float *reception_common_delay)
{
int i, j, n_fl, ap_size, result, ind;
float r_tick, e_tick;
vector<float> reception_focal_law, emission_focal_law;

	try
	{
   	if (update_ == 1)
      {
      	   if ((result = SetWedgeEnable(wedge_enable_)) < 0) return RLOG(result);
			update_ = 0;
      }
		emission_focal_law.resize(GetEmissionFocalLawTotalSize());
		reception_focal_law.resize(GetReceptionFocalLawTotalSize());

		if ((result = CFL_Scan::GetFocalLaw (&emission_focal_law[0], &reception_focal_law[0], reception_common_delay)) < 0)
      	return RLOG(result);

		if ((result = CFL_Scan::GetApertureCenterTOF (emission_center_tof, reception_center_tof)) < 0)
      	return RLOG(result);

		// EMISSION: Convierte los tiempos a ticks
		n_fl = emission_ap_->n_fl_;
		emission_focal_law_ticks.resize(n_fl);

      e_tick = 1. / FL_E_FCLK;
		for (i=0; i<n_fl; i++)
		{
			ap_size = emission_ap_->ap_size_[i];
         ind = i * ap_size;
         emission_focal_law_ticks[i].resize(ap_size);
         for (j=0; j<ap_size; j++) emission_focal_law_ticks[i][j] = Round16u(emission_focal_law[ind+j] / e_tick);
      }
      
		n_fl = reception_ap_->n_fl_;
      reception_focal_law_ticks.resize(n_fl);

		r_tick = 1. / FL_R_FCLK;
		for (i=0; i<n_fl; i++)
		{
         ap_size = reception_ap_->ap_size_[i];
         ind = i * ap_size;
         reception_focal_law_ticks[i].resize(ap_size);
         for (j=0; j<ap_size; j++)
         {
            reception_focal_law_ticks[i][j] = Round16u(reception_focal_law[ind+j] / r_tick);
	        }
      }
   }
   catch (...)
   {
      NEELOG();
      throw;
   }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetEmissionFocalLaw ( 	float *emission_focal_law,
                                       vector<float> &emission_center_tof,
                                       vector<vector<WORD> > &emission_focal_law_ticks)
{
int i, j, n_fl, ap_size, result, ind;
float e_tick;

	try
   {
   	if (update_ == 1)
      {
            if ((result = SetWedgeEnable(wedge_enable_)) < 0)
         	   return RLOG(result);
         update_ = 0;
      }

	  	if ((result = CFL_Scan::GetEmissionFocalLaw (emission_focal_law)) < 0)
      	return RLOG(result);

   	if ((result = CFL_Scan::GetEmissionApertureCenterTOF (emission_center_tof)) < 0)
      	return RLOG(result);

    	// Convierte los tiempos a ticks
    	n_fl = reception_ap_->n_fl_;
		emission_focal_law_ticks.resize(n_fl);

      e_tick = 1. / FL_E_FCLK;

    	for (i=0; i<n_fl; i++)
      {
         ap_size = emission_ap_->ap_size_[i];
         ind = i * ap_size;
         emission_focal_law_ticks[i].resize(ap_size);
         for (j=0; j<ap_size; j++)
            emission_focal_law_ticks[i][j] = Round16u(emission_focal_law[ind+j] / e_tick);
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
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetEmissionFocalLaw_Ticks (	vector<float> &emission_center_tof,
															vector<vector<WORD> > &emission_focal_law_ticks)
{
int i, j, n_fl, ap_size, result, ind;
float e_tick;
vector<float> emission_focal_law;

	try
   {
   	if (update_ == 1)
      {
            if ((result = SetWedgeEnable(wedge_enable_)) < 0)
         	   return RLOG(result);
         update_ = 0;
      }

		emission_focal_law.resize(GetEmissionFocalLawTotalSize());
      if (emission_focal_law.size() > 0)
      {
         if ((result = CFL_Scan::GetEmissionFocalLaw (&emission_focal_law[0])) < 0)
      	   return RLOG(result);
      }

   	if ((result = CFL_Scan::GetEmissionApertureCenterTOF (emission_center_tof)) < 0)
      	return RLOG(result);

    	// Convierte los tiempos a ticks
		n_fl = emission_ap_->n_fl_;
		emission_focal_law_ticks.resize(n_fl);

      e_tick = 1. / FL_E_FCLK;

		for (i=0; i<n_fl; i++)
      {
         ap_size = emission_ap_->ap_size_[i];
         ind = i * ap_size;
         emission_focal_law_ticks[i].resize(ap_size);
         for (j=0; j<ap_size; j++)
            emission_focal_law_ticks[i][j] = Round16u(emission_focal_law[ind+j] / e_tick);
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
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::SetWedgeEnable(int data)
{
int result;
vector<float> surface_coefficients_ec;

   CFL_Scan::SetWedgeEnable(data);

    if ((result = CalPACoordinates_NoWedge()) < 0)
        return RLOG(result);

    if ((result = emission_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
        return RLOG(result);

    if ((result = reception_ap_->GetApertureCoordinates(&x_pa_[0], &z_pa_[0], x_pa_.size(), pa_->GetPitch())) < 0)
        return RLOG(result);

    if ((result = ConfigFocus (emission_focus_, reception_focus_)) < 0)
        return RLOG(result);
  return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**
   FL_SCAN_SetPAElementCoordinates
   ===============================
Descripcion:
	Configura las coordenadas de cada uno de los elementos del transductor PA.

Parámetros de entrada:
   vector_size:
		Tamaño de los vectores pasados como parámetros (x_element_coordinates y
      z_element_coordinates).
		El tamaño de ambos vectores debe ser igual al número de elementos del
      transductor PA.

	*x_element_coordinates (mm):
   	Vector que contiene las coordenadas X de los elementos del transductor PA.

   *z_element_coordinates (mm):
   	Vector que contiene las coordenadas Z de los elementos del transductor PA.

Devuelve: (Ver códigos de error).
*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::SetPAElementCoordinates ( int vector_size,
											         float *x_element_coordinates,
                                          float *y_element_coordinates,
                                          float *z_element_coordinates)
{
int result;

	if (vector_size <= 0)
      return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
   if (pa_->GetElementNumber() != vector_size)
      return ELOG(charEFL_PA_COORD_SIZE,EFL_PA_COORD_SIZE);

   // X coordinates
   x_pa_.resize(vector_size);
   if (x_element_coordinates == NULL) x_pa_.assign(vector_size, 0);
   else x_pa_.assign(&x_element_coordinates[0],&x_element_coordinates[vector_size]);

   // Z coordinates
   z_pa_.resize(vector_size);
   if (z_element_coordinates == NULL) z_pa_.assign(vector_size, 0);
   else z_pa_.assign(&z_element_coordinates[0],&z_element_coordinates[vector_size]);

   if ((result = emission_ap_->GetApertureCoordinates (  x_element_coordinates,
                                                         z_element_coordinates,
                                                         vector_size,
                                                         pa_->GetPitch())) < 0)
      return RLOG(result);

   if ((result = reception_ap_->GetApertureCoordinates ( x_element_coordinates,
                                                         z_element_coordinates,
                                                         vector_size,
                                                         pa_->GetPitch())) < 0)
      return RLOG(result);

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigFocusFlatSurface (float emission_focus, float reception_focus)
{
int result;

   if ((result = ConfigReceptionFocusFlatSurface (reception_focus)) < 0) 
      return RLOG(result)

   if ((result = ConfigEmissionFocusFlatSurface (emission_focus)) < 0) 
      return RLOG(result)

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigReceptionFocusFlatSurface (float reception_focus)
{
int i, result;
int n_fl;
float r_angle_sin, r_angle_cos, r_angle_tan;
float focus_NF;
float v_specimen;

   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL);
   v_specimen = specimen_->GetVelocity();

   try
   {
      if ((n_fl = CFL_Scan::GetReceptionFocalLawNumber()) < 0)
         return RLOG(n_fl);
      if (n_fl != reception_angle_.size())
         return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

      if ((result = resize_array(&x_reception_entry_point_th_, n_fl)) < 0)
			return RLOG(result);

		for (i=0; i<n_fl; i++)
	  	{
		  	r_angle_sin = sin(reception_angle_[i]);
		  	r_angle_cos = cos(reception_angle_[i]);
		  	r_angle_tan = tan(reception_angle_[i]);

            x_reception_entry_point_th_[i] = reception_ap_->x_center_[i];
		}
   	reception_focus_ = reception_focus;

	  	for (i=0; i<n_fl; i++)
	  	{
		  	r_angle_sin = sin(reception_angle_[i]);
		  	r_angle_cos = cos(reception_angle_[i]);
		  	r_angle_tan = tan(reception_angle_[i]);

            if (focus_enable_ == 1)
				{
					x_reception_focus_[i] = x_reception_entry_point_th_[i] + reception_focus_ * r_angle_sin;
               z_reception_focus_[i] = -reception_focus_ * r_angle_cos;
            }
            else
            {
         		focus_NF = 3. * reception_ap_->GetNearFieldLimit(i, pa_->GetFrequency(), specimen_->GetVelocity());
               x_reception_focus_[i] = x_reception_entry_point_th_[i] + focus_NF * r_angle_sin;
               z_reception_focus_[i] = -focus_NF * r_angle_cos;
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
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigEmissionFocusFlatSurface (float emission_focus)
{
int i, ind_e, ind_s, result;
int n_fl;
float e_angle_sin, e_angle_cos, e_angle_tan;
float x, z, focus_NF;
float v_specimen;

   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL);
   v_specimen = specimen_->GetVelocity();

   try
   {
      if ((n_fl = CFL_Scan::GetEmissionFocalLawNumber()) < 0)
         return RLOG(n_fl);
      if (n_fl != emission_angle_.size())
         return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

      if ((result = resize_array(&x_emission_entry_point_th_, n_fl)) < 0)
			return RLOG(result);

		for (i=0; i<n_fl; i++)
	  	{
         e_angle_sin = sin(emission_angle_[i]);
         e_angle_cos = cos(emission_angle_[i]);
         e_angle_tan = tan(emission_angle_[i]);

            x_emission_entry_point_th_[i] = emission_ap_->x_center_[i];
		}
		emission_focus_ = emission_focus;

	  	for (i=0; i<n_fl; i++)
	  	{
         e_angle_sin = sin(emission_angle_[i]);
         e_angle_cos = cos(emission_angle_[i]);
         e_angle_tan = tan(emission_angle_[i]);

            if (focus_enable_ == 1)
            {
               x_emission_focus_[i] = x_emission_entry_point_th_[i] + emission_focus_ * e_angle_sin;
               z_emission_focus_[i] = -emission_focus_ * e_angle_cos;
            }
            else
            {
               ind_s = i * emission_ap_->ap_size_[i];
               ind_e = ind_s + emission_ap_->ap_size_[i] - 1;
               if (ind_s >= emission_ap_->n_delay_ || ind_e >= emission_ap_->n_delay_)
                  return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);
               x = emission_ap_->x_[ind_e] - emission_ap_->x_[ind_s];
               z = emission_ap_->z_[ind_e] - emission_ap_->z_[ind_s];
               focus_NF = 3. * emission_ap_->GetNearFieldLimit(i, pa_->GetFrequency(), specimen_->GetVelocity());
               x_emission_focus_[i] = x_emission_entry_point_th_[i] + focus_NF * e_angle_sin;
               z_emission_focus_[i] = -focus_NF * e_angle_cos;
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
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigFocus (float emission_focus, float reception_focus)
{
         return RLOG(ConfigFocusFlatSurface (emission_focus, reception_focus));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::ConfigEmissionFocus(unsigned short focus_type, float emission_focus)
{
            return RLOG(ConfigFocusFlatSurface(emission_focus, reception_focus_));
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::Update (void)
{
int result;

	update_ = 1;
      if ((result = SetWedgeEnable(wedge_enable_)) < 0) return RLOG(result);
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetStartElement (   vector<unsigned short> &emission_start_element,
									         vector<unsigned short> &reception_start_element,
									         vector<unsigned short> &start_element)
{
int result;

	if ((result = CFL_Scan::GetStartElement (emission_start_element, reception_start_element)) < 0)
      return RLOG(result);

   if (reception_start_element.size() > 0)
      start_element.assign(reception_start_element.begin(), reception_start_element.end());

   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetEmissionFocalLaw (void)
{
vector<float> LFEc;
vector<vector<WORD> > lfe;
vector<WORD> vauxW;
vector<float> vauxF;
int result;

	if ((result = GetEmissionFocalLaw_Ticks (LFEc, lfe)) < 0) 
      return RLOG(result);

	if ((result = GetEmissionStartElement (fl_.emission_start_element)) < 0)
		return RLOG(result);

   return EFL_NONE;//he actualizado las leyes focales y propago la información para que se entere el interpolador
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::SetDynamicDephFocusing (void)
{
vector<float> LFEc;
vector<WORD> lfe;
vector<vector<WORD> > lfe_total;
//vector<vector<unsigned short> > lfr, lfr_rad;
//vector<vector<unsigned short> > null_samples, null_samples_rad;
int result;
//vector<vector<int> > rad_R0;// , rad_R0_rad;
//vector<vector<int> > rad_A0;// , rad_A0_rad;
//vector<vector<int> > rad_D0;// , rad_D0_rad;
//vector<float> reception_center_tof, reception_center_tof_rad;
vector<WORD> ste_dle, ste_dlr, ste;
float material_velocity, pa_frequency, pa_pitch;
float dlr_com_min;
int j, efl_size, rfl_size;
int n_fl;

   if (pa_ == NULL) return ELOG(charEFL_PA_NULL, EFL_PA_NULL);
   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL);

	material_velocity = specimen_->GetVelocity();
	pa_frequency = pa_->GetFrequency();
	pa_pitch = pa_->GetPitch();
	if ((n_fl = GetReceptionFocalLawNumber()) < 0) return RLOG(n_fl);
   if (n_fl > 0)
   {
		//lfr_rad.resize(n_fl);
		//null_samples_rad.resize(n_fl);
		//reception_center_tof_rad.resize(n_fl);
      //rad_R0_rad.resize(n_fl);
      //rad_A0_rad.resize(n_fl);
      //rad_D0_rad.resize(n_fl);

		fl_.emission_start_element.resize(n_fl);
		fl_.reception_start_element.resize(n_fl);
		fl_.start_element.resize(n_fl);
		//fl_.aperture_entrypoint_tof.resize(n_fl);
		lfe_total.resize(n_fl);

		efl_size = GetEmissionFocalLawTotalSize();
		rfl_size = GetReceptionFocalLawTotalSize();

		if (n_fl > 0)
		{
 		    // Calcula las leyes focales en emisión
			if ((result = GetEmissionFocalLaw()) < 0)	return RLOG(result);

         // Número de retardos de todas la leyes focales en recepción
         rfl_size = GetReceptionFocalLawTotalSize();
         if ((result = GetDynamicDepthFocusing_RAD ()) < 0)  //lfr,
                                                      //null_samples,
                                                      //fl_.ddf_ap_size,
                                                      //rad_R0,
                                                      //rad_A0,
                                                      //rad_D0,
                                                      //reception_center_tof)) < 0)
            return RLOG(result);

			if ((result = GetStartElement (ste_dle, ste_dlr, ste)) < 0) return RLOG(result);

			if (ste_dle.size() > 0) fl_.emission_start_element.assign(ste_dle.begin(), ste_dle.end());
			for (j=0; j<n_fl; j++)
			{
				fl_.reception_start_element[j] = ste_dlr[j];
				fl_.start_element[j] = ste[j];
				//fl_.aperture_entrypoint_tof[j] = reception_center_tof[j];
            //lfr_rad[j] = lfr[j];
            //null_samples_rad[j] = null_samples[j];
            //reception_center_tof_rad[j] = reception_center_tof[j];
            //rad_R0_rad[j] = rad_R0[j];
            //rad_A0_rad[j] = rad_A0[j];
            //rad_D0_rad[j] = rad_D0[j];
			}
			dlr_com_min = numeric_limits<float>::max();
		}
		fl_.update = 0;
   }
	return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetDynamicDepthFocusing_RAD (void)
{
int result, i, ind, ap_size, n_fl;
float r_tick;
vector<float> v_start_range;
vector<float> v_end_range;

   if (update_ == 1)
   {
         if ((result = SetWedgeEnable(wedge_enable_)) < 0)
            return RLOG(result);
      update_ = 0;
   }

   n_fl = reception_angle_.size();
   v_start_range.resize(n_fl);
   v_start_range.assign(n_fl, fl_.acq_start_distance);
   v_end_range.resize(n_fl);
   v_end_range.assign(n_fl, fl_.acq_end_distance);
	if ((result = CFL_Scan::GetDynamicDepthFocusing(	&v_start_range[0],
                                                      &v_end_range[0],
                                                      &reception_angle_[0],
                                                      n_fl)) < 0)
	   return RLOG(result);

   // Convierte los tiempos a ticks
	n_fl = reception_ap_->n_fl_;
   r_tick = 1. / FL_R_FCLK;
   for (i=0; i<n_fl; i++)
   {
      ap_size = reception_ap_->ap_size_[i];
      ind = i * ap_size;
	}

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetScanEmissionAperture (void)
{
int result;

	if (emission_ap_ != NULL && emission_ap_->ap_size_ != NULL)
		result = emission_ap_->ap_size_[0];
	else result = ELOG(charEFL_PtrAccess_NULL, EFL_PtrAccess_NULL);
	return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetEmissionAperture (	vector<int> &aperture_size,
													vector<vector<int> > &aperture)
{
int i, j, ind_ap, n_fl, ap_size, result;

	aperture_size.clear();
	aperture.clear();
	if (emission_ap_ == NULL || emission_ap_->ap_size_ == NULL)
		result = ELOG(charEFL_PtrAccess_NULL, EFL_PtrAccess_NULL);
	n_fl = emission_ap_->n_fl_;
	aperture_size.resize(n_fl);
	aperture.resize(n_fl);
	for (ind_ap=i=0; i<n_fl; i++)
	{
		ap_size = emission_ap_->ap_size_[i];
		aperture[i].resize(ap_size);
		for (j=0; j<ap_size; j++,ind_ap++)
			aperture[i][j] = emission_ap_->ap_[ind_ap];
	}
	return n_fl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::GetReceptionAperture (	vector<int> &aperture_size,
													vector<vector<int> > &aperture)
{
int i, j, ind_ap, n_fl, ap_size, result;

	if (emission_ap_ == NULL || emission_ap_->ap_size_ == NULL)
		result = ELOG(charEFL_PtrAccess_NULL, EFL_PtrAccess_NULL);
	n_fl = reception_ap_->n_fl_;
	aperture_size.resize(n_fl);
	aperture.resize(n_fl);
	for (ind_ap=i=0; i<n_fl; i++)
	{
		ap_size = reception_ap_->ap_size_[i];
		aperture[i].resize(ap_size);
		for (j=0; j<ap_size; j++,ind_ap++)
			aperture[i][j] = reception_ap_->ap_[ind_ap];
	}
	return EFL_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CFL_ScanUI::LOAD_AcqDistance (float acq_start_distance, float acq_end_distance)
{
   fl_.acq_start_distance = acq_start_distance;
   fl_.acq_end_distance = acq_end_distance;
   RELOAD_FocalLaw();
   return EFL_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CFL_ScanUI::CONFIG_Copy (CFL_ScanUI *scan)
{
int result;

   if (scan == NULL) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);

   n_parallel_beamformed_lines_ = scan->GetParallelBeamformedLines();

   switch(scan->scan_type_)
   {
   case FL_TYPE_LINEAR: 
      if ((result = ConfigLinearScanStd (	scan->scan_emission_start_element_,
														scan->scan_emission_ap_size_,
                                          scan->scan_reception_start_element_,
                                       	scan->scan_reception_ap_size_,
                                          1,
                                       	scan->lin_scan_end_element_,
                                       	scan->lin_scan_step_,
                                       	scan->lin_scan_angle_)) < 0)
      {
         RLOG(result);
      }
      break;
   case FL_TYPE_ANGULAR: 
      if ((result = ConfigSectorialScanStd (	scan->scan_emission_start_element_,
														   scan->scan_emission_ap_size_,
                                             scan->scan_reception_start_element_,
                                       	   scan->scan_reception_ap_size_,
                                             1,
                                             scan->sec_scan_start_angle_,
                                             scan->sec_scan_end_angle_,
                                             scan->sec_scan_step_)) < 0)
      {
         RLOG(result);
      }
      break;
   }

   //cfg_.pa_scan_type = vch->cfg_.pa_scan_type;
   //cfg_.pa_aperture = vch->cfg_.pa_aperture;

   //if (vch->cfg_.pa_start_element + vch->cfg_.pa_linear_step >= vch->cfg_.pa_linear_end_element)
   //{
   //   cfg_.pa_start_element = vch->cfg_.pa_start_element;
   //   cfg_.pa_linear_step = vch->cfg_.pa_linear_step;
   //   cfg_.pa_linear_end_element = cfg_.pa_start_element + cfg_.pa_linear_step;
   //}
   //else
   //{
   //   cfg_.pa_start_element = vch->cfg_.pa_start_element;
   //   cfg_.pa_linear_step = vch->cfg_.pa_linear_step;
   //   cfg_.pa_linear_end_element = vch->cfg_.pa_linear_end_element;
   //}

   //if (vch->cfg_.pa_angular_start_angle + vch->cfg_.pa_angular_step_angle >= vch->cfg_.pa_angular_end_angle)
   //{
   //   cfg_.pa_angular_start_angle = vch->cfg_.pa_angular_start_angle;
   //   cfg_.pa_angular_step = vch->cfg_.pa_angular_step;
   //   cfg_.pa_angular_end_angle = cfg_.pa_angular_start_angle + cfg_.pa_angular_step_angle;
   //}
   //else
   //{
   //   cfg_.pa_angular_start_angle = vch->cfg_.pa_angular_start_angle;
   //   cfg_.pa_angular_step = vch->cfg_.pa_angular_step;
   //   cfg_.pa_angular_end_angle = vch->cfg_.pa_angular_end_angle;
   //}

   //cfg_.pa_emission_focus = vch->cfg_.pa_emission_focus;
   //cfg_.pa_reception_focus = vch->cfg_.pa_reception_focus;


   return EFL_NONE;
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::LoadXMLConfigData (TXMLCFG_PA_FOCAL_LAW *xml)
{
   if (xml == NULL) return ELOG(charEFL_FILE,EFL_FILE);

   fl_.focus_enabled = xml->pa_focus_enabled.Value();
   scan_type_ = xml->pa_scan_type.Value();
   scan_emission_start_element_ = xml->pa_start_element.Value();
   scan_reception_start_element_ = xml->pa_start_element.Value();
   //scan_emission_ap_ = xml->pa_aperture;
   scan_reception_ap_size_ = xml->pa_aperture_size.Value();
   scan_emission_ap_size_ = xml->pa_aperture_size.Value();

   lin_scan_angle_ = xml->pa_linear_angle.Value();
	lin_scan_end_element_ = xml->pa_linear_end_element.Value();
	lin_scan_step_ = xml->pa_linear_step.Value();

	sec_scan_start_angle_ = xml->pa_angular_start_angle.Value();
	sec_scan_end_angle_ = xml->pa_angular_end_angle.Value();
	sec_scan_step_ = xml->pa_angular_step.Value();

   emission_focus_ = xml->pa_emission_focal_point.Value();
   reception_focus_ = xml->pa_reception_focal_point.Value();

   n_parallel_beamformed_lines_ = xml->pa_global_parallel_lines.Value();

	return EFL_NONE;
}
//------------------------------------------------------------------------------
/**

*/
//------------------------------------------------------------------------------
int CFL_ScanUI::RELOAD_FocalLaw (void)
{
int result;

   if (scan_type_ == 1)
   {
	   if ((result = ConfigSectorialScanStd ( scan_emission_start_element_,
    										            scan_emission_ap_size_,
                                             scan_reception_start_element_,
                                             scan_reception_ap_size_,
                                             1,
                                             sec_scan_start_angle_,
                                             sec_scan_end_angle_,
                                             sec_scan_step_)) < 0)
         return RLOG(result);
   }
   else
   {
      if ((result = ConfigLinearScanStd ( scan_emission_start_element_,
    										         scan_emission_ap_size_,
                                          scan_reception_start_element_,
                                          scan_reception_ap_size_,
                                          1,
                                          lin_scan_end_element_,
                                          lin_scan_step_,
                                          lin_scan_angle_)) < 0)
         return RLOG(result);
   }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_ScanUI::SetParallelBeamformedLines (int data)
{
   n_parallel_beamformed_lines_ = data;
   return EFL_NONE;
}
