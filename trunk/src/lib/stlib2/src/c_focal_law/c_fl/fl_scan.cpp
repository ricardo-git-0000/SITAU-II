#if defined(_MSC_VER)
   #define _USE_MATH_DEFINES
   #define WIN32_LEAN_AND_MEAN
   #define NOMINMAX
#endif

#define FERMAT_OPT
#define _NO_MIN_RFL_TO_DLRG
// -----------------------------------------------------------------------------
/**
 @file fl_scan.cpp

 @brief Este archivo contiene la implementación de las funciones de la clase \c CFL_Scan.<br>

 Esta clase tienen la funcionalidad necesaria para definir los barridos.

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

#include <algorithm>
#include <limits>
#include <dos.h>
#include <math.h>
#include <time.h>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_scan.h"
#include "fl_error_code.h"
#include "fl_calc.h"
#include "c_log_file.h"
#include "fl.h"
#include "utils.h"
#include "st_reg.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

float FL_E_FCLK = 250.;
float FL_R_FCLK = 250.;
int gb_down_sampling_factor = 1;
int gb_reduction_factor = 0;

using namespace std;

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CFL_Scan::CFL_Scan (CFL_PA *pa, CFL_Specimen *specimen, int id)
{
    if (pa == NULL || specimen == NULL)
        throw ELOG(charEFL_PARAMETER,EFL_PARAMETER)

    if (id < 0) throw ELOG(charEFL_PARAMETER,EFL_PARAMETER)

    pa_ = pa;
    x_pa_.resize(pa_->GetElementNumber());
    z_pa_.resize(pa_->GetElementNumber());
    wedge_enable_ = 0;

    specimen_ = specimen;
    focus_enable_ = 1;
    update_ = 1;
    sampling_frequency_ = FCLK;

    emission_ap_ = new CFL_Aperture();
    reception_ap_ = new CFL_Aperture();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CFL_Scan::~CFL_Scan()
{
    if (emission_ap_ != NULL) delete emission_ap_;
    emission_ap_ = NULL;
    if (reception_ap_ != NULL) delete reception_ap_;
    reception_ap_ = NULL;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::CheckEmissionFocalLawSize (int n_focallaw, int *eap_size, int *e_ndelay)
{
int i, size;

    if (n_focallaw != GetEmissionFocalLawNumber())
        return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    for (size=i=0; i<n_focallaw; i++)
    {
        if (eap_size[i] != GetEmissionFocalLawSize(i))
            return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
        size += eap_size[i];
    }
    if (size != GetEmissionFocalLawTotalSize())
        return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    if (e_ndelay != NULL) *e_ndelay = size;
    return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::CheckReceptionFocalLawSize (int n_focallaw, int *rap_size, int *r_ndelay)
{
int i, size;

    if (n_focallaw != GetReceptionFocalLawNumber())
        return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    for (size=i=0; i<n_focallaw; i++)
    {
        if (rap_size[i] != GetReceptionFocalLawSize(i))
            return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
        size += rap_size[i];
    }
    if (size != GetReceptionFocalLawTotalSize())
        return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

    if (r_ndelay != NULL) *r_ndelay = size;
    return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::Fermat_NoWedge_Emission (float *emission_focallaw)
{
float a, b, max_efl, e_tick;
int i, fl, n_fl, ap_size, max_element_ap, pa_element, ind, ind_fl;
int n_ifaz_point;

   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL); 

   n_fl = emission_ap_->n_fl_;
   pa_element = pa_->GetElementNumber();

   e_tick = 1. / FL_E_FCLK;

	emission_center_tof_.resize(n_fl);

	for (fl=0; fl< n_fl; fl++)
   {
   	try
      {
      	if ((ap_size = emission_ap_->ap_size_[fl]) == 0)
         	return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

         max_element_ap = emission_ap_->max_element_;
         if (max_element_ap > pa_element) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

// --> FERMAT ALGORITM START

	// TIME OF FLY
         // Time of fly from surface interface to focus
         n_ifaz_point = emission_ap_->ap_size_[fl];
         ind_fl = fl * n_ifaz_point;
         for (i=0; i<n_ifaz_point; i++)
         {
         	ind = ind_fl + i;
            if (ind >= emission_ap_->n_delay_ || ind >= reception_ap_->n_delay_)
            	return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);

            a = emission_ap_->x_[ind] - x_emission_focus_[fl];
            b = -z_emission_focus_[fl];
            emission_focallaw[ind] = (1000. / specimen_->GetVelocity() * sqrt(a * a + b * b)); //%tiempos de vuelo interfaz a foco
         }
         a = emission_ap_->x_center_[fl] - x_emission_focus_[fl];
			b = -z_emission_focus_[fl];
         emission_center_tof_[fl] = (1000. / specimen_->GetVelocity() * sqrt(a * a + b * b)); //%tiempos de vuelo interfaz a foco

// --> FERMAT ALGORITM END

			max_efl = MaxArrayFLOAT(&emission_focallaw[ind_fl],ap_size);

			for (i=0; i<ap_size; i++)
         {
         	emission_focallaw[ind_fl + i] = max_efl - emission_focallaw[ind_fl + i];
         }

			emission_center_tof_[fl] = max_efl - emission_center_tof_[fl];
		}
      catch (...)
      {
      	NEELOGIT(fl);
			throw;
      }
	}
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::Fermat_NoWedge_Reception (  float *emission_focal_law,
														float *reception_focal_law,
                                				float *reception_common_delay)
{
float a, b;
int i, j, fl, n_fl, ap_size, max_element_ap, pa_element, ind, ind_fl;
int n_ifaz_point;
float min_rfl = numeric_limits<float>::max();

   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL); 

   n_fl = reception_ap_->n_fl_;
   pa_element = pa_->GetElementNumber();

	reception_center_tof_.resize(n_fl);

	for (fl=0; fl< n_fl; fl++)
	{
   	try
      {
      	if ((ap_size = reception_ap_->ap_size_[fl]) == 0)
         	return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

			max_element_ap = reception_ap_->max_element_;
         if (max_element_ap > pa_element) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

// --> FERMAT ALGORITM START

	// TIME OF FLY
         // Time of fly from PA transducer to focus
         n_ifaz_point = reception_ap_->ap_size_[fl];
         ind_fl = fl * n_ifaz_point;
         for (i=0; i<n_ifaz_point; i++)
         {
         	ind = ind_fl + i;
            if (ind >= reception_ap_->n_delay_)
            	return ELOG(charEFL_APERTURE_SIZE,EFL_APERTURE_SIZE);

            a = reception_ap_->x_[ind] - x_reception_focus_[fl];
            b = reception_ap_->z_[ind] - z_reception_focus_[fl];
            reception_focal_law[ind] = (1000. / specimen_->GetVelocity() * sqrt(a * a + b * b)); //%tiempos de vuelo interfaz a foco
         }
         a = reception_ap_->x_center_[fl] - x_reception_focus_[fl];
         b = reception_ap_->z_center_[fl] - z_reception_focus_[fl];
         reception_center_tof_[fl] = (1000. / specimen_->GetVelocity() * sqrt(a * a + b * b)); //%tiempos de vuelo interfaz a foco
         x_reception_entry_point_[fl] = reception_ap_->x_center_[fl];
         z_reception_entry_point_[fl] = reception_ap_->z_center_[fl];

// --> FERMAT ALGORITM END

         // Reception initial time is when the wave front arrive to the material
         for (i=0; i<ap_size; i++)
         {
				reception_focal_law[ind_fl + i] -= reception_center_tof_[fl];
				if (emission_center_tof_.size() == n_fl)
				   reception_focal_law[ind_fl + i] += emission_center_tof_[fl];
				if (min_rfl > reception_focal_law[ind_fl + i]) min_rfl = reception_focal_law[ind_fl + i];
			}
         if (emission_center_tof_.size() == n_fl)
			   reception_center_tof_[fl] = emission_center_tof_[fl];
         if (min_rfl > reception_center_tof_[fl]) min_rfl = reception_center_tof_[fl];
		}
      catch (...)
      {
      	NEELOGIT(fl);
         throw;
      }
	}
   min_rfl = 0;

   // Retardo Común en Recepción
   if (reception_common_delay != NULL)
   {
   	min_rfl = floor(min_rfl * sampling_frequency_) / sampling_frequency_;
      if (min_rfl < 0)
      {
         for (i=0; i<n_fl; i++)
         {
	         ind = i * ap_size;
            for (j=0; j<ap_size; j++)
            {
					if (emission_focal_law != NULL) emission_focal_law[ind+j] -= min_rfl;
	            reception_focal_law[ind+j] -= min_rfl;
            }
            emission_center_tof_[i] -= min_rfl;
            reception_center_tof_[i] -= min_rfl;
         }
			min_rfl = 0;
      }
      else
      {
	  		for (i=0; i<n_fl; i++)
      	{
         	ind = i * ap_size;
         	for (j=0; j<ap_size; j++)
            	reception_focal_law[ind+j] -= min_rfl;
         	reception_center_tof_[i] -= min_rfl;
      	}
   	}
      (*reception_common_delay) = min_rfl;
   }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::EmissionFocusConfig (	vector<float> &x_emission_focus,
												vector<float> &y_emission_focus,
                              		vector<float> &z_emission_focus)
{
int focus_size = x_emission_focus.size();

	if (	focus_size != y_emission_focus.size() ||
   		focus_size != z_emission_focus.size() ||
         focus_size == 0)
		return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

   x_emission_focus_ = x_emission_focus;
   z_emission_focus_ = z_emission_focus;

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::ReceptionFocusConfig (	vector<float> &x_reception_focus,
													vector<float> &y_reception_focus,
                              			vector<float> &z_reception_focus)
{
int focus_size = x_reception_focus.size();

	if (	focus_size != y_reception_focus.size() ||
   		focus_size != z_reception_focus.size() ||
         focus_size == 0)
		return ELOG(charEFL_PARAMETER,EFL_PARAMETER);

   x_reception_focus_ = x_reception_focus;
   z_reception_focus_ = z_reception_focus;

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::Resize(void)
{
int n_fl_e = emission_ap_->n_fl_;
int n_fl_r = reception_ap_->n_fl_;

   x_emission_focus_.resize(n_fl_e);
   z_emission_focus_.resize(n_fl_e);
   x_reception_focus_.resize(n_fl_r);
   z_reception_focus_.resize(n_fl_r);

   x_reception_entry_point_.resize(n_fl_r);
   z_reception_entry_point_.resize(n_fl_r);

   emission_angle_.resize(n_fl_e);
   reception_angle_.resize(n_fl_r); 
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetFocalLaw (	float *emission_focal_law,
										float *reception_focal_law,
                            	float *reception_common_delay)
{
int result;

	try
   {
	    if ((result = Fermat_NoWedge_Emission (emission_focal_law)) < 0) return RLOG(result);
	    if ((result = Fermat_NoWedge_Reception (emission_focal_law, reception_focal_law, reception_common_delay)) < 0)
        return RLOG(result);
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
int CFL_Scan::GetEmissionFocalLaw (float *emission_focal_law)
{
int result, n_fl;

   try
   {
      if ((n_fl = GetEmissionFocalLawNumber()) < 0) return RLOG(n_fl);

        if ((result = Fermat_NoWedge_Emission (emission_focal_law)) < 0) return RLOG(result);
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
int CFL_Scan::GetApertureCenterTOF (	vector<float> &emission_center_tof,
													vector<float> &reception_center_tof)
{
   if (emission_center_tof_.size() <= 0 || reception_center_tof_.size() <= 0) 
      return ELOG(charEFL_DataInconsistency, EFL_DataInconsistency);

	emission_center_tof.assign(emission_center_tof_.begin(),emission_center_tof_.end());
   reception_center_tof.assign(reception_center_tof_.begin(),reception_center_tof_.end());

   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetEmissionApertureCenterTOF (vector<float> &emission_center_tof)
{
int n_fl;

   if ((n_fl = GetEmissionFocalLawNumber()) < 0) return RLOG(n_fl);

	emission_center_tof.resize(n_fl);
	emission_center_tof.assign(emission_center_tof_.begin(),emission_center_tof_.end());
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetEmissionFocalLawNumber (void)
{
int result = emission_ap_->n_fl_;

   if (result != x_emission_focus_.size())
      return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);
   if (result != z_emission_focus_.size())
      return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionFocalLawNumber (void)
{
int result = reception_ap_->n_fl_;

   if (result != x_reception_focus_.size())
		return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);
   if (result != z_reception_focus_.size())
      return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);

   if (result != x_reception_entry_point_.size())
      return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);
   if (result != z_reception_entry_point_.size())
      return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);

   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionEntryPoint_X (float *x_entry_point, int size_mem)
{
int i;

    for (i=0; i<size_mem && i<x_reception_entry_point_.size(); i++) x_entry_point[i] = x_reception_entry_point_[i];
    return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionEntryPoint_Z (float *z_entry_point, int size_mem)
{
int i;

   for (i=0; i<size_mem && i<z_reception_entry_point_.size(); i++) z_entry_point[i] = z_reception_entry_point_[i];
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionEntryPoint_Angle (float *angle, int size_mem)
{
int i;

   for (i=0; i<size_mem && i<reception_angle_.size(); i++) angle[i] = reception_angle_[i];
   return i;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetEndElement ( vector<unsigned short> &emission_end_element,
										vector<unsigned short> &reception_end_element)
{
int result;

   try
   {
   	if ((result = emission_ap_->GetEndElement(emission_end_element)) < 0)
         {return RLOG(result);}
      if ((result = reception_ap_->GetEndElement(reception_end_element)) < 0)
         {return RLOG(result);}
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
int CFL_Scan::GetStartElement (  vector<unsigned short> &emission_start_element,
                                 vector<unsigned short> &reception_start_element)
{
   try
   {
      emission_start_element.assign(&emission_ap_->start_element_[0],&emission_ap_->start_element_[emission_ap_->n_fl_]);
      reception_start_element.assign(&reception_ap_->start_element_[0],&reception_ap_->start_element_[reception_ap_->n_fl_]);
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
int CFL_Scan::GetEmissionStartElement (vector<unsigned short> &emission_start_element)
{

   try
   {
      emission_start_element.assign(&emission_ap_->start_element_[0],&emission_ap_->start_element_[emission_ap_->n_fl_]);
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
int CFL_Scan::GetEmissionApertureSize (vector<unsigned short> &aperture_size)
{
   aperture_size.assign(&emission_ap_->ap_size_[0],&emission_ap_->ap_size_[emission_ap_->n_fl_]);
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionStartElement (vector<unsigned short> &reception_start_element)
{
   try
   {
      reception_start_element.assign(&reception_ap_->start_element_[0],&reception_ap_->start_element_[reception_ap_->n_fl_]);
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
int CFL_Scan::GetReceptionApertureSize (vector<unsigned short> &aperture_size)
{
   aperture_size.assign(&reception_ap_->ap_size_[0],&reception_ap_->ap_size_[reception_ap_->n_fl_]);
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetEmissionFocalLawSize (int data)
{
	if (data >= emission_ap_->n_fl_) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
    return emission_ap_->ap_size_[data];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetReceptionFocalLawSize (int data)
{
	if (data >= reception_ap_->n_fl_) return ELOG(charEFL_PARAMETER,EFL_PARAMETER);
    return reception_ap_->ap_size_[data];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetDynamicDepthFocusing (	float *start_range, float *end_range, float *angle, int n_focal_law)
{
int result, i, ind, n_fl, ap_size;
int *mNulas = NULL;
int n_delay = reception_ap_->n_delay_;
vector<float> reception_focallaw;
float specimen_velocity;
float emission_center_tof = 0;
double xu_reception_ray, zu_reception_ray;

   if (specimen_ == NULL) return ELOG(charEFL_SPECIMEN_NULL, EFL_SPECIMEN_NULL);
   specimen_velocity = specimen_->GetVelocity();

   if ((n_fl = GetReceptionFocalLawNumber()) < 0) return RLOG(n_fl);
   if (n_focal_law != n_fl) return ELOG(charEFL_FOCAL_LAW_SIZE,EFL_FOCAL_LAW_SIZE);

   beamformer_.resize(n_fl);

   for (i=0; i<n_fl; i++)
   {
      reception_angle_[i] = angle[i];
   }

    reception_center_tof_.resize(n_fl);
    x_reception_entry_point_.resize(n_fl);
    z_reception_entry_point_.resize(n_fl);

    for (ind=i=0; i<n_fl; i++)
    {
        ap_size = reception_ap_->ap_size_[i];
        xu_reception_ray = sin(angle[i]);
        zu_reception_ray = -cos(angle[i]);
        x_reception_entry_point_[i] = reception_ap_->x_center_[i];
        z_reception_entry_point_[i] = reception_ap_->z_center_[i];

        if (emission_center_tof_.size() != n_fl) emission_center_tof = 0;
	    else emission_center_tof = emission_center_tof_[i];

        if ((result = DDF_RADParameters (   reception_ap_->ap_size_[i],
                                            NULL,
                                            emission_center_tof,
                                            start_range[i],
											&reception_ap_->x_[ind],
                                            &reception_ap_->z_[ind],
                                            reception_ap_->x_center_[i],
                                            reception_ap_->z_center_[i],
                                            reception_ap_->x_center_[i],
                                            reception_ap_->z_center_[i],
                                            x_reception_entry_point_[i],
                                            z_reception_entry_point_[i],
                                            xu_reception_ray,
                                            zu_reception_ray,
											false,
											specimen_->GetVelocity(),
											0,
                                            beamformer_[i],
                                            sampling_frequency_)) < 0)
            return RLOG(result);	
        reception_center_tof_[i] = emission_center_tof;
        ind += ap_size;
    }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetBeamformerParameters (vector<vector<TFP_VCH_BASE_BeamformerParameters> > &beamformer)
{
int i;

   beamformer.clear();
   if (beamformer_.size() > 0) beamformer.resize(beamformer_.size());
   for (i=0; i<beamformer_.size(); i++)
   {
      beamformer[i].assign(beamformer_[i].begin(), beamformer_[i].end());
   }
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CFL_Scan::GetBeamformerParameters (vector<TFP_VCH_BASE_BeamformerParameters> &beamformer, int idx_fl)
{
   beamformer.clear();
   if (idx_fl >= beamformer_.size()) return ELOG(charEFL_PARAMETER, EFL_PARAMETER);
      beamformer.assign(beamformer_[idx_fl].begin(), beamformer_[idx_fl].end());
   return EFL_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
float CFL_Scan::GetEmissionTick(void)
{ 
   return (float)1. / (float)FL_E_FCLK; 
}
