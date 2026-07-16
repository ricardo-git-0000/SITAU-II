#ifndef fl_scanH
#define fl_scanH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif
#include <string>

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_pa.h"

#include "fl_specimen.h"
#include "fl_aperture.h"
#include "fl_tad.h"

using namespace std;

class CFL_Scan
{
public:

	CFL_Scan (CFL_PA *pa, CFL_Specimen *specimen, int id);

   ~CFL_Scan();

   int GetDynamicDepthFocusing (	float *start_range,
                                 float *end_range,
                                 float *angle,
                                 int n_focal_law);

   int EmissionFocusConfig (	vector<float> &x_emission_focus,
                     			vector<float> &y_emission_focus,
                     			vector<float> &z_emission_focus);

   int ReceptionFocusConfig ( vector<float> &x_reception_focus,
                     			vector<float> &y_reception_focus,
                     			vector<float> &z_reception_focus);

	int GetEmissionFocalLawTotalSize (void) {return emission_ap_->n_delay_;}

	int GetReceptionFocalLawTotalSize (void) {return reception_ap_->n_delay_;}

	int GetEmissionFocalLawSize (int data);

	int GetReceptionFocalLawSize (int data);

   int CheckEmissionFocalLawSize (	int n_focallaw,
			   								int *eap_size,
         			                  int *e_ndelay);

   int CheckReceptionFocalLawSize (	int n_focallaw,
			                           int *rap_size,
         			                  int *r_ndelay);

	int GetFocalLaw (	float *emission_focal_law,
    						float *reception_focal_law,
                     float *reception_common_delay);

	int GetEmissionFocalLaw (float *emission_focal_law);

   int GetApertureCenterTOF (	vector<float> &emission_center_tof,
   									vector<float> &reception_center_tof);

	int GetEmissionApertureCenterTOF (vector<float> &emission_center_tof);

   int GetReceptionEntryPoint_X (float *x_entry_point, int size_mem);
   int GetReceptionEntryPoint_Z (float *z_entry_point, int size_mem);
   int GetReceptionEntryPoint_Angle (float *angle, int size_mem);


   void GetReceptionEntryPointAngle (vector<float> &angle) {angle = reception_angle_;}


   void SetWedgeEnable(int data) {wedge_enable_ = data; }

	int GetEmissionFocalLawNumber (void);

	int GetReceptionFocalLawNumber (void);

   int GetStartElement (   vector<unsigned short> &emission_start_element,
                           vector<unsigned short> &reception_start_element);

   int GetEndElement (	vector<unsigned short> &emission_end_element,
   							vector<unsigned short> &reception_end_element);

   int GetEmissionStartElement (vector<unsigned short> &emission_start_element);

   int GetEmissionApertureSize (vector<unsigned short> &aperture_size);

   int GetReceptionStartElement (vector<unsigned short> &reception_start_element);

   int GetReceptionApertureSize (vector<unsigned short> &aperture_size);

   int GetBeamformerParameters (vector<vector<TFP_VCH_BASE_BeamformerParameters> > &beamformer);

   int GetBeamformerParameters (vector<TFP_VCH_BASE_BeamformerParameters> &beamformer, int idx_fl);

   T_VCH_FL fl_;

// PHASED ARRAY TRANSDUCER

   // X coordinates of PA transducer elements
   // (mm) Coordenadas X de los elementos del transductor
   vector<float> x_pa_;

   // Z coordinates of PA transducer elements
   // (mm) Coordenadas Z de los elementos del transductor
   vector<float> z_pa_;

// SCAN

   // (mm) Coordenada X del punto de entrada al material del haz principal, de
   // cada una de las leyes focales en recepción.
   vector<float> x_reception_entry_point_;

   // (mm) Coordenada Z del punto de entrada al material del haz principal, de
   // cada una de las leyes focales en recepción.
   vector<float> z_reception_entry_point_;

   // (º) Angulo de entrada al material del haz principal, de
   // cada una de las leyes focales en emisión.
   vector<float> emission_angle_;

   // (º) Angulo de entrada al material del haz principal, de
   // cada una de las leyes focales en recepción.
   vector<float> reception_angle_;

   CFL_Aperture *emission_ap_;
   CFL_Aperture *reception_ap_;

   // Puntero al componente correspondiente al transductor "phased array"
   CFL_PA *pa_;

   // Puntero al componente correspondiente al material en el que realiza
   // la inspeccion
   CFL_Specimen *specimen_;

   float GetEmissionTick(void);

protected:

   vector<vector<TFP_VCH_BASE_BeamformerParameters> > beamformer_;
   
	int Resize(void);

   float sampling_frequency_;

   // (mm) Coordenada X del foco de cada una de las leyes focales en emisión
   vector<float> x_emission_focus_;

   // (mm) Coordenada Z del foco de cada una de las leyes focales en emisión
   vector<float> z_emission_focus_;

   // (mm) Coordenada X del foco de cada una de las leyes focales en recepción
   vector<float> x_reception_focus_;

   // (mm) Coordenada Z del foco de cada una de las leyes focales en recepción
   vector<float> z_reception_focus_;

// PHASED ARRAY TRANSDUCER

   int wedge_enable_;

   int focus_enable_;

   int update_;

   // (μs) Time of fly of the emission apertures center.
	vector<float> emission_center_tof_;
   // (μs) Time of fly of the reception apertures center.
   vector<float> reception_center_tof_;

private:

	int Fermat_NoWedge_Emission (float *emission_focallaw);

	int Fermat_NoWedge_Reception (float *emission_focal_law, float *reception_focal_law, float *reception_common_delay);

};

extern float FL_E_FCLK;
extern float FL_R_FCLK;

#endif

