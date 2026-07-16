#ifndef fl_calcH
#define fl_calcH

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "fl_tad.h"

using namespace std;

unsigned short MinArrayUSHORT (unsigned short *ptr, int size);

unsigned short MaxArrayUSHORT (unsigned short *ptr, int size);

float MaxArrayFLOAT (float *ptr, int size, int *index=NULL);

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
								//vector<int> &rad_A0,
        //                vector<int> &rad_D0,
                        //float *reception_focal_law,
                        //vector<unsigned short> &n_null_samples,
                        //float *x_focus_rfl,
                        //float *z_focus_rfl,
                        vector<TFP_VCH_BASE_BeamformerParameters> &beamformer,
                        //vector<float> &tida,
                        //vector<float> &tvue,
                        //vector<float> &tk,
                        float sampling_frequency);


#endif
