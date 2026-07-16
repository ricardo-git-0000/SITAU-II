#ifndef fl_tadH
#define fl_tadH

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

#include "t_base_tad.h"

using namespace std;

#ifdef _NO_FOCALSIM
   typedef double TReal;
#else
   typedef float TReal;
#endif

#define FL_TYPE_LINEAR  0
#define FL_TYPE_ANGULAR 1

typedef struct T_VCH_FL {
	vector<vector<unsigned short> > dle; // ley focal en emisión (ticks)
	vector<vector<unsigned short> > dlr; // ley focal en recepción (ticks)
	vector<vector<float> > ap;
	vector<vector<unsigned short> > nmz;
   //vector<vector<unsigned short> > ddf_ap_size;
   int apodization_type;
   vector<unsigned short> start_element;
   vector<unsigned short> end_element;
	vector<float> ap_element; //size=NCHANNELS (número de elementos del array)
	vector<float> ap_angular; //size=número de líneas de la imagen
	float ap_apertureComp; //Factor para la compensación de aperturas grandes.
	//vector<float> aperture_entrypoint_tof;

   vector<float> gain_aperture_factor;
   vector<int> gain_aperture_factor_div2;
   vector<float> gain_angular_sensitivity;
   vector<float> gain_linear_sensitivity;

   vector<float> emission_center_tof;
   vector<float> reception_center_tof;
   vector<vector<unsigned short> > emission_focal_law_ticks;
   vector<vector<unsigned short> > reception_focal_law_ticks;
   float reception_common_delay;

   vector<unsigned short> emission_start_element;
   vector<unsigned short> reception_start_element;
   vector<unsigned short> emission_end_element;
   vector<unsigned short> reception_end_element;

   unsigned short n_fl;
   int focus_enabled;
   unsigned short scan_linear_step;
   int scan_aperture_size;
   int scan_start_element;
   int scan_end_element;
   float scan_linear_angle;
   float scan_start_angle;
   float scan_end_angle;
   float scan_sectorial_step;

	int id_pa;
	int id_wedge;
	int id_specimen;
	vector<int> id_scan;

	float acq_start_distance;      //Inicio de la adquisición desde la entrada al material (mm)
	float acq_end_distance;      //Fin de la adquisición desde la entrada al material (mm)

	int focallaw_user;
	int update;			//Actualiza parámetros (apodización...)
} T_VCH_FL;

#endif
