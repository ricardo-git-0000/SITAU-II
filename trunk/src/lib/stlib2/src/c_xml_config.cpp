// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <time.h>
#include <stdio.h>

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "c_xml_config.h"
#include "c_log_file.h"
#include "xml_err.h"
#include "fileformat.h"
#include "tad.h"
#include "cxml.h"
#include "st_reg.h"
#include "t_vch.h"
#include <ctime>

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// _____________________________________________________________________________
// ------ CXML_CONFIG class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML_CONFIG::CXML_CONFIG()
	   : CXML()
{
   ResetData();
//int i;
//
//   if (cfg_.vch.size() != 1) cfg_.vch.resize(1);
//   for (i=0; i<cfg_.vch.size(); i++)
//      ResetData(&cfg_.vch[i]);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML_CONFIG::CXML_CONFIG(AnsiString file_name)
	   : CXML()
{
   ResetData();
#if !defined(_MSC_VER)
   if (FileExists(file_name))
#else
   if (FS_FileReadAccess((char*)file_name.c_str()))
#endif
   {
      CXML::LoadFile(file_name);
      LoadData();
   }
   else { ELOG(charEXML_FILE, EXML_FILE); }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML_CONFIG::CXML_CONFIG(vector<char> &file_buffer)
    : CXML()
{
    ResetData();
    CXML::LoadBuffer(file_buffer);
    LoadData();
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML_CONFIG::CXML_CONFIG(TXMLCFG_Config *cfg)
	   : CXML()
{
   Copy(cfg);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CXML_CONFIG::ResetData(void)
{
int i;

    //cfg_.acq.data_link_mode.Reset(0);
    cfg_.acq.acquisition_mode.Reset(ACQUISITION_MODE_FIXED);
    cfg_.acq.delay_sync_time.Reset(0);
    cfg_.acq.trigger_source.Reset(0);
    cfg_.acq.pause_after_burst.Reset(0);
    cfg_.acq.n_bursts.Reset(1);
    cfg_.acq.n_images_per_burst.Reset(1);

    cfg_.fus.central_frequency.Reset(0);
    cfg_.fus.rapport_focal_aperture.Reset(0);
    cfg_.fus.apodization_coefficient.Reset(0);
    cfg_.fus.lateral_position_x.Reset(0);
    cfg_.fus.pixel_size_x.Reset(0);
    cfg_.fus.pixels_x.Reset(0);
    cfg_.fus.depth_z.Reset(0);
    cfg_.fus.pixels_z.Reset(0);
    cfg_.fus.pixel_size_z.Reset(0);
    cfg_.fus.single_plane.Reset(0);
    cfg_.fus.samples_wavelength.Reset(0);
    cfg_.fus.interpolation_coefficients.Reset(0);

   if (cfg_.vch.size() != 1) cfg_.vch.resize(1);
   for (i=0; i<cfg_.vch.size(); i++)
      XMLCFG_ResetData(&cfg_.vch[i]);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Copy(TXMLCFG_Config* cfg)
{
int i;
TXMLCFG_VirtualChannel vch_struct;

    if (cfg == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    cfg_.acq.acquisition_mode.Value(cfg->acq.acquisition_mode.Value());
    //cfg_.acq.data_link_mode.Value(cfg->acq.data_link_mode.Value());
    //cfg_.acq.n_images_burst.Value(cfg->acq.n_images_burst.Value());
    //cfg_.acq.n_bursts.Value(cfg->acq.n_bursts.Value());
    cfg_.acq.delay_sync_time.Value(cfg->acq.delay_sync_time.Value());
    cfg_.acq.trigger_source.Value(cfg->acq.trigger_source.Value());
    cfg_.acq.pause_after_burst.Value(cfg->acq.pause_after_burst.Value());
    cfg_.acq.n_bursts.Value(cfg->acq.n_bursts.Value());
    cfg_.acq.n_images_per_burst.Value(cfg->acq.n_images_per_burst.Value());

    cfg_.fus.central_frequency.Value(cfg->fus.central_frequency.Value());
    cfg_.fus.rapport_focal_aperture.Value(cfg->fus.rapport_focal_aperture.Value());
    cfg_.fus.apodization_coefficient.Value(cfg->fus.apodization_coefficient.Value());
    cfg_.fus.lateral_position_x.Value(cfg->fus.lateral_position_x.Value());
    cfg_.fus.pixel_size_x.Value(cfg->fus.pixel_size_x.Value());
    cfg_.fus.pixels_x.Value(cfg->fus.pixels_x.Value());
    cfg_.fus.depth_z.Value(cfg->fus.depth_z.Value());
    cfg_.fus.pixels_z.Value(cfg->fus.pixels_z.Value());
    cfg_.fus.pixel_size_z.Value(cfg->fus.pixel_size_z.Value());
    cfg_.fus.single_plane.Value(cfg->fus.single_plane.Value());
    cfg_.fus.samples_wavelength.Value(cfg->fus.samples_wavelength.Value());
    cfg_.fus.interpolation_coefficients.Value(cfg->fus.interpolation_coefficients.Value());

    if (cfg->vch.size() == cfg_.vch.size())
    {
        for (i = 0; i < cfg->vch.size(); i++)
        {
            // .............................................................................
            cfg_.vch[i].ut_parameters.gain.Value(cfg->vch[i].ut_parameters.gain.Value());
            cfg_.vch[i].ut_parameters.tgc_enabled.Value(cfg->vch[i].ut_parameters.tgc_enabled.Value());
            cfg_.vch[i].ut_parameters.tgc_type.Value(cfg->vch[i].ut_parameters.tgc_type.Value());
            cfg_.vch[i].ut_parameters.tgc_attenuation.Value(cfg->vch[i].ut_parameters.tgc_attenuation.Value());
            cfg_.vch[i].ut_parameters.tgc_points_db.Value(cfg->vch[i].ut_parameters.tgc_points_db.Value());
            cfg_.vch[i].ut_parameters.tgc_points_mm.Value(cfg->vch[i].ut_parameters.tgc_points_mm.Value());
            cfg_.vch[i].ut_parameters.prf_frequency.Value(cfg->vch[i].ut_parameters.prf_frequency.Value());
            cfg_.vch[i].ut_parameters.prf_burst_frequency.Value(cfg->vch[i].ut_parameters.prf_burst_frequency.Value());
            cfg_.vch[i].ut_parameters.material_velocity.Value(cfg->vch[i].ut_parameters.material_velocity.Value());
            cfg_.vch[i].ut_parameters.acq_start_distance.Value(cfg->vch[i].ut_parameters.acq_start_distance.Value());
            cfg_.vch[i].ut_parameters.acq_end_distance.Value(cfg->vch[i].ut_parameters.acq_end_distance.Value());
            cfg_.vch[i].ut_parameters.acq_time_mode.Value(cfg->vch[i].ut_parameters.acq_time_mode.Value());
            cfg_.vch[i].ut_parameters.acq_start_samples.Value(cfg->vch[i].ut_parameters.acq_start_samples.Value());
            cfg_.vch[i].ut_parameters.acq_n_samples.Value(cfg->vch[i].ut_parameters.acq_n_samples.Value());
            cfg_.vch[i].ut_parameters.sampling_freq_decimation.Value(cfg->vch[i].ut_parameters.sampling_freq_decimation.Value());
            cfg_.vch[i].ut_parameters.acquisition_type.Value(cfg->vch[i].ut_parameters.acquisition_type.Value());
            cfg_.vch[i].ut_parameters.signal_mode.Value(cfg->vch[i].ut_parameters.signal_mode.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].pulser.pulser_enabled.Value(cfg->vch[i].pulser.pulser_enabled.Value());
            cfg_.vch[i].pulser.pulses_number.Value(cfg->vch[i].pulser.pulses_number.Value());
            cfg_.vch[i].pulser.pulse_width.Value(cfg->vch[i].pulser.pulse_width.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].probe.probe_elements_number.Value(cfg->vch[i].probe.probe_elements_number.Value());
            cfg_.vch[i].probe.probe_pitch.Value(cfg->vch[i].probe.probe_pitch.Value());
            cfg_.vch[i].probe.probe_frequency.Value(cfg->vch[i].probe.probe_frequency.Value());
            cfg_.vch[i].probe.high_voltage_maximum.Value(cfg->vch[i].probe.high_voltage_maximum.Value());
            cfg_.vch[i].probe.pin_to_channel_assignment.Value(cfg->vch[i].probe.pin_to_channel_assignment.Value());
            cfg_.vch[i].probe.pin_to_channel.Value(cfg->vch[i].probe.pin_to_channel.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].dsp.fir_enabled.Value(cfg->vch[i].dsp.fir_enabled.Value());
            cfg_.vch[i].dsp.fir_low_frequency.Value(cfg->vch[i].dsp.fir_low_frequency.Value());
            cfg_.vch[i].dsp.fir_high_frequency.Value(cfg->vch[i].dsp.fir_high_frequency.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].fp_focal_law.fp_focal_law_type.Value(cfg->vch[i].fp_focal_law.fp_focal_law_type.Value());
            cfg_.vch[i].fp_focal_law.fp_pwic_start_angle.Value(cfg->vch[i].fp_focal_law.fp_pwic_start_angle.Value());
            cfg_.vch[i].fp_focal_law.fp_pwic_end_angle.Value(cfg->vch[i].fp_focal_law.fp_pwic_end_angle.Value());
            cfg_.vch[i].fp_focal_law.fp_pwic_step_angle.Value(cfg->vch[i].fp_focal_law.fp_pwic_step_angle.Value());
            //cfg_.vch[i].fp_focal_law.fp_burst_number.Value(cfg->vch[i].fp_focal_law.fp_burst_number.Value());
            //cfg_.vch[i].fp_focal_law.fp_burst_images_number.Value(cfg->vch[i].fp_focal_law.fp_burst_images_number.Value());
            cfg_.vch[i].fp_focal_law.fp_pwic_angles.Value(cfg->vch[i].fp_focal_law.fp_pwic_angles.Value());
            cfg_.vch[i].fp_focal_law.fp_focal_law_file.Value(cfg->vch[i].fp_focal_law.fp_focal_law_file.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].fp_dsp.mc_processing_type.Value(cfg->vch[i].fp_dsp.mc_processing_type.Value());
            cfg_.vch[i].fp_dsp.mc_average_signals_number.Value(cfg->vch[i].fp_dsp.mc_average_signals_number.Value());
            //cfg_.vch[i].fp_dsp.mc_fir_enabled.Value(cfg->vch[i].fp_dsp.mc_fir_enabled.Value());
            //cfg_.vch[i].fp_dsp.mc_fir_low_frequency.Value(cfg->vch[i].fp_dsp.mc_fir_low_frequency.Value());
            //cfg_.vch[i].fp_dsp.mc_fir_high_frequency.Value(cfg->vch[i].fp_dsp.mc_fir_high_frequency.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].pa_focal_law.pa_aperture_size.Value(cfg->vch[i].pa_focal_law.pa_aperture_size.Value());
            cfg_.vch[i].pa_focal_law.pa_scan_type.Value(cfg->vch[i].pa_focal_law.pa_scan_type.Value());
            cfg_.vch[i].pa_focal_law.pa_start_element.Value(cfg->vch[i].pa_focal_law.pa_start_element.Value());
            cfg_.vch[i].pa_focal_law.pa_linear_step.Value(cfg->vch[i].pa_focal_law.pa_linear_step.Value());
            cfg_.vch[i].pa_focal_law.pa_linear_end_element.Value(cfg->vch[i].pa_focal_law.pa_linear_end_element.Value());
            cfg_.vch[i].pa_focal_law.pa_linear_angle.Value(cfg->vch[i].pa_focal_law.pa_linear_angle.Value());
            cfg_.vch[i].pa_focal_law.pa_angular_step.Value(cfg->vch[i].pa_focal_law.pa_angular_step.Value());
            cfg_.vch[i].pa_focal_law.pa_angular_start_angle.Value(cfg->vch[i].pa_focal_law.pa_angular_start_angle.Value());
            cfg_.vch[i].pa_focal_law.pa_angular_end_angle.Value(cfg->vch[i].pa_focal_law.pa_angular_end_angle.Value());
            cfg_.vch[i].pa_focal_law.pa_focus_enabled.Value(cfg->vch[i].pa_focal_law.pa_focus_enabled.Value());
            cfg_.vch[i].pa_focal_law.pa_reception_auto_focus.Value(cfg->vch[i].pa_focal_law.pa_reception_auto_focus.Value());
            cfg_.vch[i].pa_focal_law.pa_emission_focal_point.Value(cfg->vch[i].pa_focal_law.pa_emission_focal_point.Value());
            cfg_.vch[i].pa_focal_law.pa_reception_focal_point.Value(cfg->vch[i].pa_focal_law.pa_reception_focal_point.Value());
            cfg_.vch[i].pa_focal_law.pa_increased_acquired_samples.Value(cfg->vch[i].pa_focal_law.pa_increased_acquired_samples.Value());
            cfg_.vch[i].pa_focal_law.pa_global_parallel_lines.Value(cfg->vch[i].pa_focal_law.pa_global_parallel_lines.Value());
            // .............................................................................
            // .............................................................................
            //cfg_.vch[i].pa_dsp.pa_fir_enabled.Value(cfg->vch[i].pa_dsp.pa_fir_enabled.Value());
            //cfg_.vch[i].pa_dsp.pa_fir_low_frequency.Value(cfg->vch[i].pa_dsp.pa_fir_low_frequency.Value());
            //cfg_.vch[i].pa_dsp.pa_fir_high_frequency.Value(cfg->vch[i].pa_dsp.pa_fir_high_frequency.Value());
            cfg_.vch[i].pa_dsp.pa_processing_type.Value(cfg->vch[i].pa_dsp.pa_processing_type.Value());
            cfg_.vch[i].pa_dsp.pa_average_signals_number.Value(cfg->vch[i].pa_dsp.pa_average_signals_number.Value());
            cfg_.vch[i].pa_dsp.pa_emi_signals_number.Value(cfg->vch[i].pa_dsp.pa_emi_signals_number.Value());
            // .............................................................................
            // .............................................................................
            cfg_.vch[i].tfm_focal_law.tfm_global_parallel_lines.Value(cfg->vch[i].tfm_focal_law.tfm_global_parallel_lines.Value());
            cfg_.vch[i].tfm_focal_law.tfm_emission_start_element.Value(cfg->vch[i].tfm_focal_law.tfm_emission_start_element.Value());
            cfg_.vch[i].tfm_focal_law.tfm_emission_end_element.Value(cfg->vch[i].tfm_focal_law.tfm_emission_end_element.Value());
            cfg_.vch[i].tfm_focal_law.tfm_emission_step_element.Value(cfg->vch[i].tfm_focal_law.tfm_emission_step_element.Value());
            cfg_.vch[i].tfm_focal_law.tfm_receiver_angle.Value(cfg->vch[i].tfm_focal_law.tfm_receiver_angle.Value());
            cfg_.vch[i].tfm_focal_law.tfm_receiver_start_distance.Value(cfg->vch[i].tfm_focal_law.tfm_receiver_start_distance.Value());
            cfg_.vch[i].tfm_focal_law.tfm_receiver_end_distance.Value(cfg->vch[i].tfm_focal_law.tfm_receiver_end_distance.Value());
            cfg_.vch[i].tfm_focal_law.tfm_receiver_lines_number.Value(cfg->vch[i].tfm_focal_law.tfm_receiver_lines_number.Value());
        }
    }
    else
    {
        if (cfg_.vch.size() > 0) cfg_.vch.clear();
        for (i = 0; i < cfg->vch.size(); i++)
        {
            //vch_struct.afe = cfg->vch[i].afe;
            //vch_struct.reg_pulser = cfg->vch[i].reg_pulser;
            vch_struct.ut_parameters = cfg->vch[i].ut_parameters;
            vch_struct.dsp = cfg->vch[i].dsp;
            vch_struct.pulser = cfg->vch[i].pulser;
            vch_struct.probe = cfg->vch[i].probe;
            vch_struct.fp_focal_law = cfg->vch[i].fp_focal_law;
            vch_struct.fp_dsp = cfg->vch[i].fp_dsp;
            vch_struct.pa_focal_law = cfg->vch[i].pa_focal_law;
            vch_struct.pa_dsp = cfg->vch[i].pa_dsp;
            vch_struct.tfm_focal_law = cfg->vch[i].tfm_focal_law;
            cfg_.vch.push_back(vch_struct);
        }
    }   
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_ACQUISITION(pugi::xml_node node_root, TXMLCFG_ACQUISITION * data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Data link Mode
    //node = node_root.append_child(XMLCFG_eParameter);
    //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ACQ_DataLinkMode;
    //node.append_attribute(XMLCFG_pValue) = cfg_.acq.data_link_mode.Value();

    // Acquisition Mode
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ACQ_AcquisitionMode;
    node.append_attribute(XMLCFG_pValue) = cfg_.acq.acquisition_mode.Value();

    // Delay Sync Time
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DelaySyncTime_2;
    node.append_attribute(XMLCFG_pValue) = cfg_.acq.delay_sync_time.Value();

    // Trigger Source
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TriggerSource;
    node.append_attribute(XMLCFG_pValue) = cfg_.acq.trigger_source.Value();

    // Pause After Burst
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PauseAfterBurst;
    node.append_attribute(XMLCFG_pValue) = UCIP.user_pause_after_burst;

    // Burst Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_BurstNumber;
    node.append_attribute(XMLCFG_pValue) = UCIP.n_bursts;

    // Images Per Burst Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ImagesPerBurstNumber;
    node.append_attribute(XMLCFG_pValue) = UCIP.n_images_burst;

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_FUS(pugi::xml_node node_root, TXMLCFG_FUS * data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Central Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_CentralFrequency;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.central_frequency.Value();

    // Rapport Focal Aperture
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_RapportFocalAperture;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.rapport_focal_aperture.Value();

    // Apodization Coefficient
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_ApodizationCoefficient;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.apodization_coefficient.Value();

    // Lateral Position X
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_LateralPositionX;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.lateral_position_x.Value();

    // Pixel Size X
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_PixelSizeX;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.pixel_size_x.Value();

    // Pixels X
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_PixelsX;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.pixels_x.Value();

    // Depth Z
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_DepthZ;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.depth_z.Value();

    // Pixel Size Z
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_PixelSizeZ;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.pixel_size_z.Value();

    // Pixels Z
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_PixelsZ;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.pixels_z.Value();

    // Single Plane
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_SinglePlane;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.single_plane.Value();

    // Samples Wavelength
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_SamplesWavelength;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.samples_wavelength.Value();

    // Interpolation Coefficients
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_FUS_InterpolationCoefficients;
    node.append_attribute(XMLCFG_pValue) = cfg_.fus.interpolation_coefficients.Value();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_UT_PARAMETERS(pugi::xml_node node_root, TXMLCFG_UT_PARAMETERS *data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Gain
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_Gain_2;
    node.append_attribute(XMLCFG_pValue) = data->gain.Value();

    // TGC Enabled
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TGC_Enabled_2;
    node.append_attribute(XMLCFG_pValue) = data->tgc_enabled.Value();

    // TGC Type
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TGC_Type_2;
    node.append_attribute(XMLCFG_pValue) = data->tgc_type.Value();

    // TGC Slope
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TGC_Slope;
    node.append_attribute(XMLCFG_pValue) = data->tgc_attenuation.Value();

    // TGC Points (mm)
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TGC_Points_mm;
    node.append_attribute(XMLCFG_pValue) = XML_Array2Attribute(&data->tgc_points_mm).c_str();

    // TGC Points (dB)
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TGC_Points_dB;
    node.append_attribute(XMLCFG_pValue) = XML_Array2Attribute(&data->tgc_points_db).c_str();

    // PRF Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PRF_Frequency_2;
    node.append_attribute(XMLCFG_pValue) = data->prf_frequency.Value();

    // PRF Burst Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PRF_Burst_Frequency;
    node.append_attribute(XMLCFG_pValue) = data->prf_burst_frequency.Value();

    // Sampling Frequency Decimation
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_SamplingDecimation_2;
    node.append_attribute(XMLCFG_pValue) = data->sampling_freq_decimation.Value();

    // Material Velocity
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MaterialVelocity_2;
    node.append_attribute(XMLCFG_pValue) = data->material_velocity.Value();

    // Acquisition  Start Distance
    //if (data->acq_start_distance.Update())
    {
        node = node_root.append_child(XMLCFG_eParameter);
        node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcqStartDistance_2;
        node.append_attribute(XMLCFG_pValue) = data->acq_start_distance.Value();
    }

    // Acquisition  End Distance
    //if (data->acq_end_distance.Update())
    {
        node = node_root.append_child(XMLCFG_eParameter);
        node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcqEndDistance_2;
        node.append_attribute(XMLCFG_pValue) = data->acq_end_distance.Value();
    }
    // Acquisition Time Mode
    //if (data->acq_time_mode.Update())
    {
        node = node_root.append_child(XMLCFG_eParameter);
        node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcqTimeMode;
        node.append_attribute(XMLCFG_pValue) = data->acq_time_mode.Value();
    }
    // Acquisition Start Samples
    //if (data->acq_start_samples.Update())
    {
        node = node_root.append_child(XMLCFG_eParameter);
        node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcqStartSamples;
        node.append_attribute(XMLCFG_pValue) = data->acq_start_samples.Value();
    }

    // Acquisition Samples Number
    //if (data->acq_n_samples.Update())
    {
        node = node_root.append_child(XMLCFG_eParameter);
        node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcqSamplesNumber;
        node.append_attribute(XMLCFG_pValue) = data->acq_n_samples.Value();
    }

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_AcquisitionType;
    node.append_attribute(XMLCFG_pValue) = data->acquisition_type.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_SignalMode;
    node.append_attribute(XMLCFG_pValue) = data->signal_mode.Value();
    

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_PULSER(pugi::xml_node node_root, TXMLCFG_PULSER* data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Pulser Enabled
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PulserEnabled_2;
    node.append_attribute(XMLCFG_pValue) = data->pulser_enabled.Value();

    // Pulses Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PulsesNumber_2;
    node.append_attribute(XMLCFG_pValue) = data->pulses_number.Value();

    // Pulse Width
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PulseWidth_2;
    node.append_attribute(XMLCFG_pValue) = data->pulse_width.Value();

    // Pulse Amplitude
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PulseAmplitude_2;
    node.append_attribute(XMLCFG_pValue) = data->pulse_amplitude.Value();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_PROBE(pugi::xml_node node_root, TXMLCFG_PROBE *data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Probe Element Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ProbeElementsNumber_2;
    node.append_attribute(XMLCFG_pValue) = data->probe_elements_number.Value();

    // Probe Pitch
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ProbePitch_2;
    node.append_attribute(XMLCFG_pValue) = data->probe_pitch.Value();

    // Probe Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_ProbeFrequency_2;
    node.append_attribute(XMLCFG_pValue) = data->probe_frequency.Value();

    // High Voltage Maximum
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_HighVoltageMaximum_2;
    node.append_attribute(XMLCFG_pValue) = data->high_voltage_maximum.Value();

    // Pin To Channel Assignmet
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PinToChannelAssignment;
    node.append_attribute(XMLCFG_pValue) = data->pin_to_channel_assignment.Value();

    // Pin To Channel
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PinToChannel;
    node.append_attribute(XMLCFG_pValue) = XML_Array2Attribute(&data->pin_to_channel).c_str();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_MC_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_FP_FOCAL_LAW* data)
{
    pugi::xml_node node;
    string str;

    if (data == NULL) return EXML_NONE;

    // (Full Parallel) Focal Law: Type
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_FocalLawType;
    node.append_attribute(XMLCFG_pValue) = data->fp_focal_law_type.Value();

    // (Full Parallel) Focal Law: Angles
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_PWICAngles;
    node.append_attribute(XMLCFG_pValue) = XML_Array2Attribute(&data->fp_pwic_angles).c_str();

    // (Full Parallel) Focal Law: PWIC Start Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_PWICStartAngle;
    node.append_attribute(XMLCFG_pValue) = data->fp_pwic_start_angle.Value();

    // (Full Parallel) Focal Law: PWIC End Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_PWICEndAngle;
    node.append_attribute(XMLCFG_pValue) = data->fp_pwic_end_angle.Value();

    // (Full Parallel) Focal Law: PWIC Step Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_PWICStepAngle;
    node.append_attribute(XMLCFG_pValue) = data->fp_pwic_step_angle.Value();

    // (Full Parallel) Focal Law: File
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_FocalLawFile;
    node.append_attribute(XMLCFG_pValue) = data->fp_focal_law_file.Value().c_str();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_PA_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_PA_FOCAL_LAW*data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Scan Type
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_ScanType_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_scan_type.Value();

    // PA Aperture Size
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_Aperture_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_aperture_size.Value();

    // Start Element
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_StartElement_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_start_element.Value();

    // Linear End Element
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_LinearEndElement_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_linear_end_element.Value();

    // Linear step
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_LinearStep_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_linear_step.Value();

    // Linear Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_LinearAngle_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_linear_angle.Value();

    // Angular Start Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_AngularStartAngle_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_angular_start_angle.Value();

    // Angular End Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_AngularEndAngle_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_angular_end_angle.Value();

    // Angular Step Angle
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_AngularStep_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_angular_step.Value();

    // Focus Enabled
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_FocusEnabled;
    node.append_attribute(XMLCFG_pValue) = data->pa_focus_enabled.Value();

    // Reception Auto-Focus (DDF)
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_ReceptionAutoFocus;
    node.append_attribute(XMLCFG_pValue) = data->pa_reception_auto_focus.Value();

    // Emission Focal Point
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_EmissionFocus_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_emission_focal_point.Value();

    // Reception Focal Point
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_ReceptionFocus_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_reception_focal_point.Value();

    // Beamformer Parallel Lines Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_ParallelLines_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_global_parallel_lines.Value();

    // Increased Acquired Samples Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_PA_IncreasedAcquiredSamples_2;
    node.append_attribute(XMLCFG_pValue) = data->pa_increased_acquired_samples.Value();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_PA_DSP(pugi::xml_node node_root, TXMLCFG_PA_DSP *data)
{
   pugi::xml_node node;

   if (data == NULL) return EXML_NONE;

   //// (DSP) FIR: Enabled
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIREnabled;
   //node.append_attribute(XMLCFG_pValue) = data->pa_fir_enabled.Value();

   //// (DSP) FIR: Low Frequency
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIRLowFrequency;
   //node.append_attribute(XMLCFG_pValue) = data->pa_fir_low_frequency.Value();

   //// (DSP) FIR: High Frequency
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIRHighFrequency;
   //node.append_attribute(XMLCFG_pValue) = data->pa_fir_high_frequency.Value();

   //// (DSP) Processing Type
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_ProcessingType;
   //node.append_attribute(XMLCFG_pValue) = data->pa_processing_type.Value();

   //// (DSP) Average Signals Number
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_AVRSignalsNumber;
   //node.append_attribute(XMLCFG_pValue) = data->pa_average_signals_number.Value();

   //// (DSP) EMI Signals Number
   //node = node_root.append_child(XMLCFG_eParameter);
   //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_EMISignalsNumber;
   //node.append_attribute(XMLCFG_pValue) = data->pa_emi_signals_number.Value();

   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_MC_DSP(pugi::xml_node node_root, TXMLCFG_MC_DSP* data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    //// (MC_DSP) FIR: Enabled
    //node = node_root.append_child(XMLCFG_eParameter);
    //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_DSP_FIREnabled;
    //node.append_attribute(XMLCFG_pValue) = data->mc_fir_enabled.Value();

    //// (MC_DSP) FIR: Low Frequency
    //node = node_root.append_child(XMLCFG_eParameter);
    //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_DSP_FIRLowFrequency;
    //node.append_attribute(XMLCFG_pValue) = data->mc_fir_low_frequency.Value();

    //// (MC_DSP) FIR: High Frequency
    //node = node_root.append_child(XMLCFG_eParameter);
    //node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_DSP_FIRHighFrequency;
    //node.append_attribute(XMLCFG_pValue) = data->mc_fir_high_frequency.Value();

    // (MC_DSP) Processing Type
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_DSP_ProcessingType;
    node.append_attribute(XMLCFG_pValue) = data->mc_processing_type.Value();

    // (MC_DSP) Average Signals Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_MC_AverageSignalsNumber;
    node.append_attribute(XMLCFG_pValue) = data->mc_average_signals_number.Value();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_DSP(pugi::xml_node node_root, TXMLCFG_DSP* data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // (MC_DSP) FIR: Enabled
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIREnabled;
    node.append_attribute(XMLCFG_pValue) = data->fir_enabled.Value();

    // (MC_DSP) FIR: Low Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIRLowFrequency;
    node.append_attribute(XMLCFG_pValue) = data->fir_low_frequency.Value();

    // (MC_DSP) FIR: High Frequency
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_DSP_FIRHighFrequency;
    node.append_attribute(XMLCFG_pValue) = data->fir_high_frequency.Value();

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_TFM_FOCAL_LAW(pugi::xml_node node_root, TXMLCFG_TFM_FOCAL_LAW* data)
{
    pugi::xml_node node;

    if (data == NULL) return EXML_NONE;

    // Beamformer Parallel Lines Number
    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_ParallelLines;
    node.append_attribute(XMLCFG_pValue) = data->tfm_global_parallel_lines.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_EmissionStartElement;
    node.append_attribute(XMLCFG_pValue) = data->tfm_emission_start_element.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_EmissionEndElement;
    node.append_attribute(XMLCFG_pValue) = data->tfm_emission_end_element.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_EmissionStepElement;
    node.append_attribute(XMLCFG_pValue) = data->tfm_emission_step_element.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_ReceiverLinesNumber;
    node.append_attribute(XMLCFG_pValue) = data->tfm_receiver_lines_number.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_ReceiverAngle;
    node.append_attribute(XMLCFG_pValue) = data->tfm_receiver_angle.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_ReceiverStartDistance;
    node.append_attribute(XMLCFG_pValue) = data->tfm_receiver_start_distance.Value();

    node = node_root.append_child(XMLCFG_eParameter);
    node.append_attribute(XMLCFG_pName) = XMLCFG_pName_TFM_ReceiverEndDistance;
    node.append_attribute(XMLCFG_pValue) = data->tfm_receiver_end_distance.Value();
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::Create_Version (pugi::xml_node node_root)
{
pugi::xml_node node, node_version;

   node_version = node_root.append_child(XMLCFG_eVersion);

   // XML File Version
   node = node_version.append_child(XMLCFG_eParameter);
   node.append_attribute(XMLCFG_pName) = XMLCFG_pNameVersionXML;
   node.append_attribute(XMLCFG_pValue) = XMLCFG_VERSION;
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**
         
*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::CreateXMLFile(AnsiString file_name)
{
pugi::xml_node node, node_axis, node_root, node_vch, node_base, node_acq;
AnsiString version;
char str[250];
time_t t;
int i;

   // SITAU
   node_root = CXML::doc_.append_child(XMLCFG_eSTFPLIB2);

   // Date - Time
   time(&t);
   strftime(str, sizeof(str), "%a %b %d %T %Y", localtime(&t));
   node = node_root.append_child(XMLCFG_eDateTime);
   node.append_attribute(XMLCFG_pName) = XMLCFG_pNameDateTime;
   node.append_attribute(XMLCFG_pValue) = str;

   // Version
   Create_Version(node_root);

   // fUS
   Create_FUS(node_root.append_child(XMLCFG_eFUS), &cfg_.fus);

   // Acquisition Config
   Create_ACQUISITION(node_root.append_child(XMLCFG_eACQUISITION), &cfg_.acq);

   // Virtual Channels
   node_vch = node_root.append_child(XMLCFG_eVirtualChannel);

   for (i=0; i<cfg_.vch.size(); i++)
   {
      // UT Parameters
      //Create_UTParameters_v1(node_vch.append_child(XMLCFG_eUTParameters_1), &cfg_.vch[i].ut_parameters);
      Create_UT_PARAMETERS(node_vch.append_child(XMLCFG_eUT_PARAMETERS), &cfg_.vch[i].ut_parameters);

      // Pulser
      Create_PULSER(node_vch.append_child(XMLCFG_ePULSER), &cfg_.vch[i].pulser);

      // Probe
      Create_PROBE(node_vch.append_child(XMLCFG_ePROBE), &cfg_.vch[i].probe);

      // FP Focal Law
      Create_MC_FOCAL_LAW(node_vch.append_child(XMLCFG_eMC_FOCAL_LAW), &cfg_.vch[i].fp_focal_law);

      // FP DSP
      Create_MC_DSP(node_vch.append_child(XMLCFG_eMC_DSP), &cfg_.vch[i].fp_dsp);

      // PA Focal Law
      Create_PA_FOCAL_LAW(node_vch.append_child(XMLCFG_ePA_FOCAL_LAW), &cfg_.vch[i].pa_focal_law);

      // PA DSP
      Create_PA_DSP(node_vch.append_child(XMLCFG_ePA_DSP), &cfg_.vch[i].pa_dsp);

      Create_TFM_FOCAL_LAW(node_vch.append_child(XMLCFG_eTFM_FOCAL_LAW), &cfg_.vch[i].tfm_focal_law);

      // DSP
      Create_DSP(node_vch.append_child(XMLCFG_eDSP), &cfg_.vch[i].dsp);


      // TGC
      //Create_TGC_v1(node_vch.append_child(XMLCFG_eTGC), &cfg_.vch[i].tgc);

      // PULSER
      //Create_PULSER(node_vch.append_child(XMLCFG_ePULSER), &cfg_.vch[i].pulser);

      // AFE
      //Create_AFE(node_vch.append_child(XMLCFG_eAFE), &cfg_.vch[i].afe);

      // TGC
      //Create_AFE(node_vch.append_child(XMLCFG_eTGC), &cfg_.vch[i].tgc);

      //for (b=0; b<cfg_.vch[i].base.size(); b++)
      //{
      //   node_base = node_vch.append_child(XMLCFG_eBASERegisters);

      //   //// PULSER
      //   //Create_BASE_PULSER(node_base.append_child(XMLCFG_ePULSER), &cfg_.vch[i].base[b].pulser);

      //   //// AFE
      //   //Create_BASE_AFE(node_base.append_child(XMLCFG_eAFE), &cfg_.vch[i].base[b].afe);
      //}
   }
   
   //Create_Hardware(node, eX_Axis);
   //Create_Hardware(node, eY_Axis);
   //Create_Hardware(node, eZ_Axis);

   //// Axis Configuration
   //node_axis = node_root.append_child(eAxis_Configuration);
   //node = node_axis.append_child(eX_Axis);
   //Create_AxisConfiguration_Button(node, eButtonLeft);
   //Create_AxisConfiguration_Button(node, eButtonRight);

   //node = node_axis.append_child(eY_Axis);
   //Create_AxisConfiguration_Button(node, eButtonUp);
   //Create_AxisConfiguration_Button(node, eButtonDown);

   //node = node_axis.append_child(eZ_Axis);
   //Create_AxisConfiguration_Button(node, eButtonUp);
   //Create_AxisConfiguration_Button(node, eButtonDown);

   //// Scan Configuration
   //node = node_root.append_child(eScan);
   //Create_Scan(node);

   CXML::doc_.save_file(file_name.c_str());
   //CXML::LoadFile(file_name);
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_String ( pugi::xml_attribute *attribute,
//                                          char *name,
//                                          AnsiString *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tString) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
//         {
//            *parameter = AnsiString(next_attribute.value());
//            result = true;
//         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_Boolean (pugi::xml_attribute *attribute, char *name, CXMLParameter<bool>* parameter)
//{
//bool data, result = false;
//pugi::xml_attribute next_attribute;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tBoolean) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
//         {
//            strcmp(next_attribute.value(), XML_tfalse) == 0 ? data = false : data = true;
//            parameter->Set(data);
//            result = true;
//         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_ArrayUInt16 (pugi::xml_attribute *attribute, char *name, vector<unsigned short> &parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//int i, start_element;
//AnsiString str, str_int;
//bool fin;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tArrayUInt16) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         if (strcmp(next_attribute.name(), XMLCFG_pStartArrayElement) == 0)
//         {
//#if defined(_MSC_VER)
//            start_element = std::stoi(AnsiString(next_attribute.value()));
//#else
//            start_element = StrToInt(AnsiString(next_attribute.value()));
//#endif
//            next_attribute = next_attribute.next_attribute();
//            if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
//            {
//               if (parameter.size() < start_element) parameter.resize(start_element);
//               str = AnsiString(next_attribute.value());
//#if defined(_MSC_VER)
//               for (i=1, fin=false; !fin && i<=str.length(); i++)
//#else
//               for (i=1, fin=false; !fin && i<=str.Length(); i++)
//#endif
//               {
//                  if (str[i] == ']')
//                  {
//#if defined(_MSC_VER)
//                     parameter.push_back((unsigned short)std::stoi(str_int));
//#else
//                     parameter.push_back((unsigned short)StrToInt(str_int));
//#endif
//                     fin = true;
//                  }
//                  else if (str[i] == ',')
//                  {
//#if defined(_MSC_VER)
//                     parameter.push_back((unsigned short)std::stoi(str_int));
//#else
//                     parameter.push_back((unsigned short)StrToInt(str_int));
//#endif
//                     str_int = "";
//                  }
//                  else if (str[i] >= '0' && str[i] <= '9')
//                  {
//#if defined(_MSC_VER)
//                     //str_int.insert(str_int.length(), i, str[i]);
//                     str_int.insert(str_int.length(), 1, str[i]);
//#else
//                     str_int.Insert(str[i], str_int.Length()+1);
//#endif
//                  }
//               }
//               result = true;
//            }
//         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_Memory (pugi::xml_attribute *attribute, char *name, CXMLParameter<vector<unsigned char> > *memory)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//int i, start_element;
//AnsiString str, str_int;
//bool fin;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tArrayUInt16) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         if (strcmp(next_attribute.name(), XMLCFG_pStartArrayElement) == 0)
//         {
//#if defined(_MSC_VER)
//            start_element = std::stoi(AnsiString(next_attribute.value()));
//#else
//            start_element = StrToInt(AnsiString(next_attribute.value()));
//#endif
//            next_attribute = next_attribute.next_attribute();
//            if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
//            {
//               if (memory->Size() < start_element) memory->Resize(start_element);
//               str = AnsiString(next_attribute.value());
//#if defined(_MSC_VER)
//               for (i=1, fin=false; !fin && i<=str.length(); i++)
//#else
//               for (i=1, fin=false; !fin && i<=str.Length(); i++)
//#endif
//               {
//                  if (str[i] == ']')
//                  {
//#if defined(_MSC_VER)
//                     memory->PushBack_UInt8((unsigned char)std::stoi(str_int));
//#else
//                     memory->PushBack_UInt8((unsigned char)StrToInt(str_int));
//#endif
//                     fin = true;
//                  }
//                  else if (str[i] == ',')
//                  {
//#if defined(_MSC_VER)
//                     memory->PushBack_UInt8((unsigned char)std::stoi(str_int));
//#else
//                     memory->PushBack_UInt8((unsigned char)StrToInt(str_int));
//#endif
//                     str_int = "";
//                  }
//                  else if (str[i] >= '0' && str[i] <= '9')
//                  {
//#if defined(_MSC_VER)
//                     //str_int.insert(str_int.length(), i, str[i]);
//                     str_int.insert(str_int.length(), 1, str[i]);
//#else
//                     str_int.Insert(str[i], str_int.Length()+1);
//#endif
//                  }
//               }
//               result = true;
//            }
//         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<float> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//float data;
//
//    if (strcmp(attribute->value(), name) == 0)
//    {
//        next_attribute = attribute->next_attribute();
//        if (strcmp(next_attribute.value(), XML_tFloat) == 0)
//        {
//            next_attribute = next_attribute.next_attribute();
//            data = std::stof(AnsiString(next_attribute.value()));
//            parameter->Set(data);
//            result = true;
//        }
//    }
//    return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<int> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//int data;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tInt32) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         //if (update || !parameter->Update())
//         {
//#if defined(_MSC_VER)
//            data = std::stoi(AnsiString(next_attribute.value()));
//#else
//            data = StrToInt(AnsiString(next_attribute.value()));
//#endif
//            parameter->Set(data);
//            result = true;
//         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_UInt32 (pugi::xml_attribute *attribute, char *name, CXMLParameter<unsigned long> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//unsigned long data;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tUInt32) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         //if (update || !parameter->Update())
//         {
//#if defined(_MSC_VER)
//            data = std::stoul(AnsiString(next_attribute.value()));
//#else
//            data = StrToInt(AnsiString(next_attribute.value()));
//#endif
//            parameter->Set(data);
//            result = true;
//         }
////         if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
////         {
////#if defined(_MSC_VER)
////            *parameter = std::stoul(AnsiString(next_attribute.value()));
////#else
////            *parameter = StrToInt(AnsiString(next_attribute.value()));
////#endif
////            result = true;
////         }
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<unsigned long> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//unsigned long data;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      //if (update || !parameter->Update())
//      {
//#if defined(_MSC_VER)
//         data = std::stoul(AnsiString(next_attribute.value()));
//#else
//         data = StrToInt(AnsiString(next_attribute.value()));
//#endif
//         parameter->Set(data);
//         result = true;
//      }
////      if (strcmp(next_attribute.name(), XMLCFG_pValue) == 0)
////      {
////#if defined(_MSC_VER)
////         *parameter = std::stoul(AnsiString(next_attribute.value()));
////#else
////         *parameter = StrToInt(AnsiString(next_attribute.value()));
////#endif
////         result = true;
////      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<unsigned short> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//unsigned long data;
//
//    if (strcmp(attribute->value(), name) == 0)
//    {
//        next_attribute = attribute->next_attribute();
//        data = std::stoul(AnsiString(next_attribute.value()));
//        parameter->Set((unsigned short)data);
//        result = true;
//    }
//    return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_Register8 (pugi::xml_attribute *attribute, char *name, CXMLParameter<unsigned char> *parameter)
//{
//bool result = false;
//pugi::xml_attribute next_attribute;
//unsigned long data;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      //if (update || !parameter->Update())
//      {
//#if defined(_MSC_VER)
//         data = std::stoul(AnsiString(next_attribute.value()));
//#else
//         data = StrToInt(AnsiString(next_attribute.value()));
//#endif
//         parameter->Set((unsigned char)data);
//         result = true;
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//bool CXML_CONFIG::GetAttribute_UInt16 (pugi::xml_attribute *attribute, char *name, CXMLParameter<unsigned short> *parameter)
//{
//bool result = false;
//unsigned long data;
//pugi::xml_attribute next_attribute;
//
//   if (strcmp(attribute->value(), name) == 0)
//   {
//      next_attribute = attribute->next_attribute();
//      if (strcmp(next_attribute.value(), XML_tUInt32) == 0)
//      {
//         next_attribute = next_attribute.next_attribute();
//         data = std::stoul(AnsiString(next_attribute.value()));
//         parameter->Set((unsigned short)data);
//         result = true;
//      }
//   }
//   return result;
//}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_FUS(pugi::xml_node node_data, TXMLCFG_FUS* data)
{
    pugi::xml_node node, node_tgc;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eFUS) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_CentralFrequency, &data->central_frequency)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_RapportFocalAperture, &data->rapport_focal_aperture)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_ApodizationCoefficient, &data->apodization_coefficient)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_LateralPositionX, &data->lateral_position_x)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_PixelSizeX, &data->pixel_size_x)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_PixelsX, &data->pixels_x)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_DepthZ, &data->depth_z)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_PixelsZ, &data->pixels_z)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_PixelSizeZ, &data->pixel_size_z)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_SinglePlane, &data->single_plane)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_SamplesWavelength, &data->samples_wavelength)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FUS_InterpolationCoefficients, &data->interpolation_coefficients)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_ACQUISITION(pugi::xml_node node_data, TXMLCFG_ACQUISITION* data)
{
    pugi::xml_node node, node_tgc;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eACQUISITION) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                //if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ACQ_DataLinkMode, &data->data_link_mode)) {}
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ACQ_AcquisitionMode, &data->acquisition_mode)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ACQ_BurstsNumber, &data->n_bursts)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ACQ_ImagesBurstNumber, &data->n_images_burst)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_DelaySyncTime_2, &data->delay_sync_time)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TriggerSource, &data->trigger_source)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PauseAfterBurst, &data->pause_after_burst)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_BurstNumber, &data->n_bursts)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ImagesPerBurstNumber, &data->n_images_per_burst)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_UT_PARAMETERS(pugi::xml_node node_data, TXMLCFG_UT_PARAMETERS* data)
{
    pugi::xml_node node, node_tgc;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eUT_PARAMETERS) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_Gain_2, &data->gain)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Type_2, &data->tgc_type)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Enabled_2, &data->tgc_enabled)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Slope, &data->tgc_attenuation)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Points_mm, &data->tgc_points_mm)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Points_dB, &data->tgc_points_db)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PRF_Frequency_2, &data->prf_frequency)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PRF_Burst_Frequency, &data->prf_burst_frequency)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_SamplingDecimation_2, &data->sampling_freq_decimation)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MaterialVelocity_2, &data->material_velocity)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcqStartDistance_2, &data->acq_start_distance)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcqEndDistance_2, &data->acq_end_distance)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcqTimeMode, &data->acq_time_mode)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcqStartSamples, &data->acq_start_samples)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcqSamplesNumber, &data->acq_n_samples)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_AcquisitionType, &data->acquisition_type)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_SignalMode, &data->signal_mode)) {}
            }
        }
    }
    if (data->acquisition_type.Value() != ACQUISITION_TYPE_MC && data->acquisition_type.Value() != ACQUISITION_TYPE_PA && data->acquisition_type.Value() != ACQUISITION_TYPE_TFM) data->acquisition_type.Set(0);
#if !defined(_PA)
    if (data->acquisition_type.Value() == ACQUISITION_TYPE_PA) data->acquisition_type.Set(ACQUISITION_TYPE_MC);
#endif
#if !defined(_TFM)
    if (data->acquisition_type.Value() == ACQUISITION_TYPE_TFM) data->acquisition_type.Set(ACQUISITION_TYPE_MC);
#endif

    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_PULSER(pugi::xml_node node_data, TXMLCFG_PULSER* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_ePULSER) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PulserEnabled_2, &data->pulser_enabled)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PulsesNumber_2, &data->pulses_number)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PulseWidth_2, &data->pulse_width)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PulseAmplitude_2, &data->pulse_amplitude)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_PROBE(pugi::xml_node node_data, TXMLCFG_PROBE* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_ePROBE) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ProbeElementsNumber_2, &data->probe_elements_number)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ProbePitch_2, &data->probe_pitch)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_ProbeFrequency_2, &data->probe_frequency)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_HighVoltageMaximum_2, &data->high_voltage_maximum)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PinToChannel, &data->pin_to_channel)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PinToChannelAssignment, &data->pin_to_channel_assignment)) {}                
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_MC_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_FP_FOCAL_LAW *data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eMC_FOCAL_LAW) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_FocalLawType, &data->fp_focal_law_type)) {}
                else if (!data->fp_pwic_angles.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_PWICStartAngle, &data->fp_pwic_start_angle)) {}
                else if (!data->fp_pwic_angles.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_PWICEndAngle, &data->fp_pwic_end_angle)) {}
                else if (!data->fp_pwic_angles.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_PWICStepAngle, &data->fp_pwic_step_angle)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_PWICAngles, &data->fp_pwic_angles)) 
                {
                    if (data->fp_pwic_angles.Value().size() > 1)
                    {
                        data->fp_pwic_start_angle.Set(data->fp_pwic_angles.Value().front());
                        data->fp_pwic_end_angle.Set(data->fp_pwic_angles.Value().back());
                        data->fp_pwic_step_angle.Set(data->fp_pwic_angles.Value()[1] - data->fp_pwic_angles.Value()[0]);
                    }
                    else if (data->fp_pwic_angles.Value().size() == 1)
                    {
                        data->fp_pwic_start_angle.Set(data->fp_pwic_angles.Value().front());
                        data->fp_pwic_end_angle.Set(data->fp_pwic_start_angle.Value());
                        data->fp_pwic_step_angle.Set(0);
                    }
                    else
                    {
                        data->fp_pwic_start_angle.Set(0);
                        data->fp_pwic_end_angle.Set(0);
                        data->fp_pwic_step_angle.Set(0);
                    }
                }
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FP_BurstNumber, &data->fp_burst_number)) {}                
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_FP_BurstImagesNumber, &data->fp_burst_images_number)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_FocalLawFile, &data->fp_focal_law_file)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_DSP(pugi::xml_node node_data, TXMLCFG_DSP* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eDSP) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_DSP_FIREnabled, &data->fir_enabled)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_DSP_FIRLowFrequency, &data->fir_low_frequency)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_DSP_FIRHighFrequency, &data->fir_high_frequency)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_MC_DSP(pugi::xml_node node_data, TXMLCFG_MC_DSP* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eMC_DSP) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_DSP_ProcessingType, &data->mc_processing_type)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_AverageSignalsNumber, &data->mc_average_signals_number)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_DSP_FIREnabled, &data->mc_fir_enabled)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_DSP_FIRLowFrequency, &data->mc_fir_low_frequency)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_MC_DSP_FIRHighFrequency, &data->mc_fir_high_frequency)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_PA_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_PA_FOCAL_LAW* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_ePA_FOCAL_LAW) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_Aperture_2, &data->pa_aperture_size)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_ScanType_2, &data->pa_scan_type)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_StartElement_2, &data->pa_start_element)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_LinearStep_2, &data->pa_linear_step)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_LinearEndElement_2, &data->pa_linear_end_element)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_LinearAngle_2, &data->pa_linear_angle)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_AngularStartAngle_2, &data->pa_angular_start_angle)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_AngularEndAngle_2, &data->pa_angular_end_angle)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_AngularStep_2, &data->pa_angular_step)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_FocusEnabled, &data->pa_focus_enabled)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_ReceptionAutoFocus, &data->pa_reception_auto_focus)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_EmissionFocus_2, &data->pa_emission_focal_point)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_ReceptionFocus_2, &data->pa_reception_focal_point)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_ParallelLines_2, &data->pa_global_parallel_lines))
                {
                    if (data->pa_global_parallel_lines.Value() <= 0) data->pa_global_parallel_lines.Set(1);
                }
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_IncreasedAcquiredSamples_2, &data->pa_increased_acquired_samples)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_PA_DSP(pugi::xml_node node_data, TXMLCFG_PA_DSP *data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_ePA_DSP) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_ProcessingType, &data->pa_processing_type)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_AVRSignalsNumber, &data->pa_average_signals_number)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_EMISignalsNumber, &data->pa_emi_signals_number)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_FIREnabled, &data->pa_fir_enabled)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_FIRLowFrequency, &data->pa_fir_low_frequency)) {}
                //else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_PA_DSP_FIRHighFrequency, &data->pa_fir_high_frequency)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_TFM_FOCAL_LAW(pugi::xml_node node_data, TXMLCFG_TFM_FOCAL_LAW* data)
{
    pugi::xml_node node;

    if (node_data == NULL) return EXML_NONE;
    if (data == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    if (strcmp(node_data.name(), XMLCFG_eTFM_FOCAL_LAW) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_ParallelLines, &data->tfm_global_parallel_lines)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_EmissionStartElement, &data->tfm_emission_start_element)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_EmissionEndElement, &data->tfm_emission_end_element)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_EmissionStepElement, &data->tfm_emission_step_element)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_ReceiverLinesNumber, &data->tfm_receiver_lines_number)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_ReceiverAngle, &data->tfm_receiver_angle)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_ReceiverStartDistance, &data->tfm_receiver_start_distance)) {}
                else if (GetAttribute(&node.first_attribute(), XMLCFG_pName_TFM_ReceiverEndDistance, &data->tfm_receiver_end_distance)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::GetData_TGC (pugi::xml_node node_data, TXMLCFG_VirtualChannel *vch)
{
pugi::xml_node node, node_tgc;
TXMLCFG_UT_PARAMETERS* ut = NULL;

    if (vch == NULL) return ELOG(charEXML_Parameter, EXML_Parameter);
    ut = &vch->ut_parameters;

    if (strcmp(node_data.name(), XMLCFG_eTGC) == 0)
    {
        LogWriteLine("Loading Data: '%s'", node_data.name());
        for (node = node_data.first_child(); node; node = node.next_sibling())
        {
            if (strcmp(node.name(), XMLCFG_eParameter) == 0)
            {
                //if (GetAttribute_Memory(&node.first_attribute(), XMLCFG_pName_TGC_Curve, &ut->curve)) {}
                if (!ut->tgc_type.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Type_1, &ut->tgc_type)) {}
                else if (!ut->tgc_attenuation.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Attenuation, &ut->tgc_attenuation)) {}
                else if (!ut->tgc_enabled.Update() && GetAttribute(&node.first_attribute(), XMLCFG_pName_TGC_Enabled_1, &ut->tgc_enabled)) {}
            }
        }
    }
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::LoadData(void)
{
pugi::xml_node node, node_vch;
TXMLCFG_VirtualChannel vch_struct;

   cfg_.vch.clear();
   logFile.open(LOG_XML_FILE);

   GetData_FUS(CXML::doc_.child(XMLCFG_eSTFPLIB2).child(XMLCFG_eFUS), &cfg_.fus);
   GetData_ACQUISITION(CXML::doc_.child(XMLCFG_eSTFPLIB2).child(XMLCFG_eACQUISITION), &cfg_.acq);
   for (node_vch = CXML::doc_.child(XMLCFG_eSTFPLIB2).child(XMLCFG_eVirtualChannel); node_vch != NULL; node_vch = node_vch.next_sibling())
    {
        if (strcmp(node_vch.name(), XMLCFG_eVirtualChannel) == 0)
        {         
            XMLCFG_ResetData(&vch_struct);
            GetData_UT_PARAMETERS(node_vch.child(XMLCFG_eUT_PARAMETERS), &vch_struct.ut_parameters);
            GetData_PULSER(node_vch.child(XMLCFG_ePULSER), &vch_struct.pulser);
            GetData_PROBE(node_vch.child(XMLCFG_ePROBE), &vch_struct.probe);
            GetData_DSP(node_vch.child(XMLCFG_eDSP), &vch_struct.dsp);
            GetData_MC_FOCAL_LAW(node_vch.child(XMLCFG_eMC_FOCAL_LAW), &vch_struct.fp_focal_law);
            GetData_MC_DSP(node_vch.child(XMLCFG_eMC_DSP), &vch_struct.fp_dsp);
            GetData_PA_FOCAL_LAW(node_vch.child(XMLCFG_ePA_FOCAL_LAW), &vch_struct.pa_focal_law);
            GetData_PA_DSP(node_vch.child(XMLCFG_ePA_DSP), &vch_struct.pa_dsp);
            GetData_TFM_FOCAL_LAW(node_vch.child(XMLCFG_eTFM_FOCAL_LAW), &vch_struct.tfm_focal_law);
         //GetData_AFE(node_vch.child(XMLCFG_eAFE), &vch_struct.afe);
         //GetData_REG_PULSER(node_vch.child(XMLCFG_ePULSER), &vch_struct.reg_pulser);
            //GetData_PAFocalLaw(node_vch.child(XMLCFG_ePAFocalLaw), &vch_struct);
         //GetData_DSP(node_vch.child(XMLCFG_eDSP), &vch_struct);
            //GetData_UTParameters(node_vch.child(XMLCFG_eUTParameters_1), &vch_struct);
         //vch_struct.pulser.pulser_enabled.Set(vch_struct.reg_pulser.control.pul_enable.Value());
         //vch_struct.pulser.pulses_number.Set(vch_struct.reg_pulser.control.n_pulses.Value());
            GetData_TGC(node_vch.child(XMLCFG_eTGC), &vch_struct);
            cfg_.vch.push_back(vch_struct);
        }
    }
   logFile.close();
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
TXMLCFG_VirtualChannel *CXML_CONFIG::GetVCHConfig(int id_vch)
{
TXMLCFG_VirtualChannel *result = NULL;

   if (id_vch < cfg_.vch.size()) result = &cfg_.vch[id_vch];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML_CONFIG::SaveToFile(AnsiString file)
{
   if (FS_FileExists((char *)file.c_str())) remove((char *)file.c_str());
   CXML::doc_.save_file(file.c_str());
   return EXML_NONE;
}









// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void XMLCFG_ResetData(TXMLCFG_VirtualChannel *xml_vch)
{
    vector<float> v_float;
    vector<unsigned short> v_ushort;
    float data_float;

    if (xml_vch != NULL) 
    {
// .............................................................................
        xml_vch->ut_parameters.gain.Reset(20);
        xml_vch->ut_parameters.tgc_enabled.Reset(0);
        xml_vch->ut_parameters.tgc_type.Reset(0);
        xml_vch->ut_parameters.tgc_attenuation.Reset(0.1);
        v_float.clear();
        v_float.push_back(0);
        v_float.push_back(1);
        xml_vch->ut_parameters.tgc_points_db.Reset(v_float);
        xml_vch->ut_parameters.tgc_points_mm.Reset(v_float);
        xml_vch->ut_parameters.prf_frequency.Reset(1000);
        xml_vch->ut_parameters.prf_burst_frequency.Reset(1000);
        xml_vch->ut_parameters.material_velocity.Reset(3000);
        xml_vch->ut_parameters.acq_start_distance.Reset(0);
        xml_vch->ut_parameters.acq_end_distance.Reset(10);
        xml_vch->ut_parameters.acq_time_mode.Reset(0);
        xml_vch->ut_parameters.acq_start_samples.Reset(0);
        xml_vch->ut_parameters.acq_n_samples.Reset(xml_vch->ut_parameters.acq_end_distance.Value() / xml_vch->ut_parameters.material_velocity.Value() * FCLK);
        xml_vch->ut_parameters.sampling_freq_decimation.Reset(1);
        xml_vch->ut_parameters.acquisition_type.Reset(ACQUISITION_TYPE_MC);
        xml_vch->ut_parameters.signal_mode.Reset(0);
        //xml_vch->ut_parameters.delay_sync_time.Reset(0);
// .............................................................................
// .............................................................................
        xml_vch->pulser.pulser_enabled.Reset(1);
        xml_vch->pulser.pulses_number.Reset(1);
        xml_vch->pulser.pulse_width.Reset(50);
// .............................................................................
// .............................................................................
        xml_vch->probe.probe_elements_number.Reset(32);
        xml_vch->probe.probe_pitch.Reset(0.5);
        xml_vch->probe.probe_frequency.Reset(5);
        xml_vch->probe.high_voltage_maximum.Reset(50);
        xml_vch->probe.pin_to_channel_assignment.Reset(0);
        v_ushort.clear();
        for (int i = 0; i < VCH_MAX_LINES; i++) v_ushort.push_back(i);
        xml_vch->probe.pin_to_channel.Reset(v_ushort);        
// .............................................................................
// .............................................................................
        //xml_vch->fp_focal_law.fp_acquisition_mode.Reset(0);
        xml_vch->fp_focal_law.fp_focal_law_type.Reset(0);
        xml_vch->fp_focal_law.fp_pwic_start_angle.Reset(-20);
        xml_vch->fp_focal_law.fp_pwic_end_angle.Reset(20);
        xml_vch->fp_focal_law.fp_pwic_step_angle.Reset(0.5);
        //xml_vch->fp_focal_law.fp_burst_number.Reset(100);
        //xml_vch->fp_focal_law.fp_burst_images_number.Reset(1);
        v_float.clear();
        for (data_float = xml_vch->fp_focal_law.fp_pwic_start_angle.Value(); data_float < xml_vch->fp_focal_law.fp_pwic_end_angle.Value(); data_float += xml_vch->fp_focal_law.fp_pwic_step_angle.Value())
            v_float.push_back(data_float);
        xml_vch->fp_focal_law.fp_pwic_angles.Reset(v_float);
        xml_vch->fp_focal_law.fp_focal_law_file.Reset("");
// .............................................................................
// .............................................................................
        xml_vch->dsp.fir_enabled.Reset(0);
        xml_vch->dsp.fir_low_frequency.Reset(0);
        xml_vch->dsp.fir_high_frequency.Reset(30);
        // .............................................................................
        // .............................................................................
        xml_vch->fp_dsp.mc_average_signals_number.Reset(4);
        //xml_vch->fp_dsp.mc_fir_enabled.Reset(0);
        xml_vch->fp_dsp.mc_processing_type.Reset(0);
        //xml_vch->fp_dsp.mc_fir_low_frequency.Reset(0);
        //xml_vch->fp_dsp.mc_fir_high_frequency.Reset(30);
// .............................................................................
// .............................................................................
        xml_vch->pa_focal_law.pa_aperture_size.Reset(8);
        xml_vch->pa_focal_law.pa_scan_type.Reset(0);
        xml_vch->pa_focal_law.pa_start_element.Reset(0);
        xml_vch->pa_focal_law.pa_linear_step.Reset(1);
        xml_vch->pa_focal_law.pa_linear_end_element.Reset(32);
        xml_vch->pa_focal_law.pa_linear_angle.Reset(0);
        xml_vch->pa_focal_law.pa_angular_step.Reset(1);
        xml_vch->pa_focal_law.pa_angular_start_angle.Reset(-45);
        xml_vch->pa_focal_law.pa_angular_end_angle.Reset(45);
        xml_vch->pa_focal_law.pa_focus_enabled.Reset(0);
        xml_vch->pa_focal_law.pa_reception_auto_focus.Reset(0);
        xml_vch->pa_focal_law.pa_emission_focal_point.Reset(10);
        xml_vch->pa_focal_law.pa_reception_focal_point.Reset(10);
        xml_vch->pa_focal_law.pa_increased_acquired_samples.Reset(200);
        xml_vch->pa_focal_law.pa_global_parallel_lines.Reset(1);
// .............................................................................
// .............................................................................
        //xml_vch->pa_dsp.pa_fir_enabled.Reset(0);
        //xml_vch->pa_dsp.pa_fir_low_frequency.Reset(0);
        //xml_vch->pa_dsp.pa_fir_high_frequency.Reset(30);
        xml_vch->pa_dsp.pa_processing_type.Reset(0);
        xml_vch->pa_dsp.pa_average_signals_number.Reset(2);
        xml_vch->pa_dsp.pa_emi_signals_number.Reset(3);
// .............................................................................
// .............................................................................
        xml_vch->tfm_focal_law.tfm_global_parallel_lines.Reset(4);
        xml_vch->tfm_focal_law.tfm_emission_start_element.Reset(0);
        xml_vch->tfm_focal_law.tfm_emission_end_element.Reset(127);
        xml_vch->tfm_focal_law.tfm_emission_step_element.Reset(1);
        xml_vch->tfm_focal_law.tfm_receiver_lines_number.Reset(32);
        xml_vch->tfm_focal_law.tfm_receiver_angle.Reset(0);
        xml_vch->tfm_focal_law.tfm_receiver_start_distance.Reset(0);
        xml_vch->tfm_focal_law.tfm_receiver_end_distance.Reset(40);

      //xml_vch->reg_pulser.main_delay.Reset(0);
      //xml_vch->reg_pulser.delay_mem_ini_end.delay_mem_ini.Reset(0);
      //xml_vch->reg_pulser.delay_mem_ini_end.delay_mem_end.Reset(0);
      //xml_vch->reg_pulser.delay_mem_ptr.Reset(0);
      //xml_vch->reg_pulser.control.pul_enable.Reset(0);
      //xml_vch->reg_pulser.control.gnd0_hiz1.Reset(0);
      //xml_vch->reg_pulser.control.pulser_load.Reset(0);
      //xml_vch->reg_pulser.control.pulser_auto_load.Reset(0);
      //xml_vch->reg_pulser.control.pulser_mode.Reset(0);
      //xml_vch->reg_pulser.control.pulser_status.Reset(0);
      //xml_vch->reg_pulser.control.n_pulses.Reset(0);
      //xml_vch->reg_pulser.control.stop.Reset(0);
      //xml_vch->reg_pulser.control.external_trigger.Reset(0);
      //xml_vch->reg_pulser.control.not_busy.Reset(0);
      //xml_vch->reg_pulser.control.software_trigger.Reset(0);
      //xml_vch->reg_pulser.control.pul_enable.Reset(1);
      //xml_vch->reg_pulser.control.n_pulses.Reset(1);

      //xml_vch->afe.delay_offset_.Reset(17);

      //xml_vch->afe.bussar.external_trigger.Reset(0);
      //xml_vch->afe.bussar.decimation_ratio.Reset(0);
      //xml_vch->afe.bussar.n_samples.Reset(100);
      //xml_vch->afe.bussar.water_delay.Reset(0);

      //xml_vch->afe.spi.reg51.LPF_PROGRAMMABILITY.Reset(0);
      //xml_vch->afe.spi.reg51.PGA_INTEGRATOR_DISABLE.Reset(0);
      //xml_vch->afe.spi.reg51.PGA_CLAMP_LEVEL.Reset(0);
      //xml_vch->afe.spi.reg51.PGA_GAIN_CONTROL.Reset(0);

      //xml_vch->afe.spi.reg52.ACTIVE_TERMINATION.Reset(0);
      //xml_vch->afe.spi.reg52.ACT_TER_ENA_CTRL.Reset(0);
      //xml_vch->afe.spi.reg52.PRESET_ACTIVE_TERMINATIONS.Reset(0);
      //xml_vch->afe.spi.reg52.ACTIVE_TERMINATION_ENABLE.Reset(0);
      //xml_vch->afe.spi.reg52.LNA_INPUT_CLAMP_SETTING.Reset(0);
      //xml_vch->afe.spi.reg52.LNA_INTEGRATOR_DISABLE.Reset(0);
      //xml_vch->afe.spi.reg52.LNA_GAIN.Reset(0);
      //xml_vch->afe.spi.reg52.LNA_INDIVIDUAL_CH_CNTL.Reset(0);

      //xml_vch->afe.spi.reg59.HPF_LNA.Reset(0);
      //xml_vch->afe.spi.reg59.DIG_TGC_ATT_GAIN.Reset(0);
      //xml_vch->afe.spi.reg59.DIG_TGC_ATT.Reset(0);
      //xml_vch->afe.spi.reg59.CW_SUM_AMP_PDN.Reset(0);
      //xml_vch->afe.spi.reg59.PGA_TEST_MODE.Reset(0);

      //xml_vch->afe.spi.reg61.V2I_CLAMP.Reset(0);
      //xml_vch->afe.spi.reg61.LPF_5MHz.Reset(0);
      //xml_vch->afe.spi.reg61.PGA_CLAMP_minus6dBFS.Reset(0);
     

      //xml_vch->tgc.enabled.Reset(0);
      //xml_vch->tgc.type.Reset(0);
      //xml_vch->tgc.attenuation.Reset(0);
      //if (xml_vch->tgc.curve.Size() > 0) xml_vch->tgc.curve.Clear();
   }
}



