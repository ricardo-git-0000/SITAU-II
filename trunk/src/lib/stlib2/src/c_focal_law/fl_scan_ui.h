#ifndef fl_scan_uiH
#define fl_scan_uiH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#if defined(_MSC_VER)
   #include <vector>
#else
   #include <vector.h>
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "fl_scan.h"
#include "fl_pa_ui.h"

#include "fl_specimen_ui.h"
#include "c_xml_config.h"

using namespace std;

#define MAX_FOCAL_LAW_NUMBER  1024

class CFL_ScanUI : public CFL_Scan
{
public:   

	CFL_ScanUI (CFL_PAUI *pa, CFL_SpecimenUI *specimen, int id=0);

   ~CFL_ScanUI();

	void SetSamplingFreq (float data) {sampling_frequency_ = data;}

	int ConfigSectorialScan (	unsigned short *emission_ap,
                              unsigned short *reception_ap,
                              unsigned short emission_ap_size,
                              unsigned short reception_ap_size,
                              float start_angle,
                              float end_angle,
                              float step_angle);

	int ConfigLinearScan (	unsigned short *emission_ap,
   								unsigned short *reception_ap,
                           unsigned short emission_ap_size,
                           unsigned short reception_ap_size,
                           unsigned short end_element,
                           unsigned short step_element,
                           float angle);

   int GetLinearScanFocalLawNumber (   unsigned short emission_start_element,
                                       unsigned short emission_ap_size,
                                       unsigned short reception_start_element,
                                       unsigned short reception_ap_size,
                                       unsigned short end_element,
                                       unsigned short step);

   int GetLinearScanFocalLawNumber (   unsigned short *emission_ap,
                                       unsigned short *reception_ap,
                                       unsigned short emission_ap_size,
                                       unsigned short reception_ap_size,
                                       unsigned short end_element,
                                       unsigned short step,
                                       unsigned short *emission_start_element=NULL,
                                       unsigned short *reception_start_element=NULL);

   int GetSectorialScanFocalLawNumber (   float step,
                                          unsigned short aperture,
                                          unsigned short startelement,
                                          float startangle,
                                          float endangle);

   int ConfigLinearScanStd (	int emission_start_element,
                                int emission_ap_size,
                                int reception_start_element,
                                int reception_ap_size,
                                int inc_element_ap,
                                int end_element,
                                int emission_step,
                                float angle);

	int ConfigSectorialScanStd (	unsigned short emission_start_element,
    										unsigned short emission_ap_size,
                                 unsigned short reception_start_element,
                                 unsigned short reception_ap_size,
                                 unsigned short inc_element_ap,
                                 float start_angle,
                                 float end_angle,
                                 float step);

	int GetScanType (void) {return scan_type_;}	// 1 --> Sectorial, 0 --> Lineal

   float GetLinearScanAngle (void) {return lin_scan_angle_;}

   int GetLinearScanEndElement (void) {return lin_scan_end_element_;}

   int GetLinearScanStep (void) {return lin_scan_step_;}

   float GetAngularScanStartAngle (void) {return sec_scan_start_angle_;}

   float GetAngularScanEndAngle (void) {return sec_scan_end_angle_;}

   float GetAngularScanStepAngle (void) {return sec_scan_step_;}

   int GetScanEmissionStartElement (void) {return scan_emission_start_element_;}

   int GetScanReceptionStartElement (void) {return scan_reception_start_element_;}

   int GetScanEmissionAperture (void);

   int GetScanReceptionAperture (void) {return scan_reception_ap_size_;}

   int ConfigFocus (float emission_focus, float reception_focus);
   int ConfigEmissionFocus(unsigned short focus_type, float emission_focus);

	int GetFocalLaw (	vector<float> &emission_center_tof,
                     vector<float> &reception_center_tof,
							vector<vector<unsigned short> > &emission_focal_law_tick,
                     vector<vector<unsigned short> > &reception_focal_law_tick,
							float *reception_common_delay);

	int GetEmissionFocalLaw ( 	float *emission_focal_law,
										vector<float> &emission_center_tof,
										vector<vector<unsigned short> > &emission_focal_law_ticks);

	int GetEmissionFocalLaw_Ticks (	vector<float> &emission_center_tof,
												vector<vector<unsigned short> > &emission_focal_law_ticks);

   int SetWedgeEnable(int data);

   int GetWedgeEnable (void) {return wedge_enable_;}

   int GetFocusEnable (void) {return focus_enable_;}

   float GetEmissionFocus (void) {return emission_focus_;}

   float GetReceptionFocus (void) {return reception_focus_;}

	int Update (void);

   int GetStartElement (   vector<unsigned short> &emission_start_element,
									         vector<unsigned short> &reception_start_element,
									         vector<unsigned short> &start_element);

   int SetDynamicDephFocusing (void);

   int GetEmissionFocalLaw (void);

   int GetDynamicDepthFocusing_RAD(void);

	int GetEmissionAperture (	vector<int> &aperture_size,
										vector<vector<int> > &aperture);

	int GetReceptionAperture (	vector<int> &aperture_size,
										vector<vector<int> > &aperture);

   int SetPAElementCoordinates ( int vector_size,
                                 float *x_element_coordinates,
                                 float *y_element_coordinates,
                                 float *z_element_coordinates);

   int LOAD_AcqDistance (float acq_start_distance, float acq_end_distance);

   int CONFIG_Copy (CFL_ScanUI *scan);

   int LoadXMLConfigData (TXMLCFG_PA_FOCAL_LAW*xml);

   int RELOAD_FocalLaw (void);

   int SetParallelBeamformedLines (int data);

   int GetParallelBeamformedLines (void) {return n_parallel_beamformed_lines_;}

private:

	CFL_PAUI *pa_;
   CFL_SpecimenUI *specimen_;

   int n_parallel_beamformed_lines_;

	float emission_focus_;
	float reception_focus_;

   float *x_emission_entry_point_th_;
   float* x_reception_entry_point_th_;

	int scan_type_;

	unsigned short scan_emission_start_element_;

    unsigned short scan_reception_start_element_;

	unsigned short scan_reception_ap_size_;

	unsigned short scan_emission_ap_size_;

   // Linear Scan

	float lin_scan_angle_;

	unsigned short lin_scan_end_element_;

	unsigned short lin_scan_step_;

    // Sectorial Scan

	float sec_scan_start_angle_;

	float sec_scan_end_angle_;

	float sec_scan_step_;

// PHASED ARRAY TRANSDUCER

	int CalPACoordinates_NoWedge (void);

	int ScanAperturesCoordinates (void);

// SPECIMEN

   int ConfigFocusFlatSurface (float emission_focus, float reception_focus);

   int ConfigEmissionFocusFlatSurface (float reception_focus);

   int ConfigReceptionFocusFlatSurface (float emission_focus);
};

#endif
