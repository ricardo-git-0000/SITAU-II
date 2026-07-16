function [error, fus] = Load_XMLConfig(xml_config_file)
%CONFIG_ Summary of this function goes here
%   Detailed explanation goes here

error = 0;

% CONFIGURACION INICIAL DEL SISTEMA SITAU II
% ------------------------------------------

result = ST2_LoadXMLFileConfig(xml_config_file);
if (result < 0)
    error = result;
end

% CONFIGURACION DEL SISTEMA SITAU II CON LOS NUEVOS PARAMETROS
% ------------------------------------------------------------

fus.central_frequency = ST2_FUS_ApodizationCoefficient();
fus.rapport_focal_aperture = ST2_FUS_RapportFocalAperture();
fus.apodization_coefficient = ST2_FUS_ApodizationCoefficient();
fus.lateral_position_x = ST2_FUS_LateralPositionX();
fus.pixel_size_x = ST2_FUS_PixelSizeX();
fus.pixels_x = ST2_FUS_PixelsX();
fus.depth_z = ST2_FUS_DepthZ();
fus.pixel_size_z = ST2_FUS_PixelSizeZ();
fus.pixels_z = ST2_FUS_PixelsZ();
fus.single_plane = ST2_FUS_SinglePlane();
fus.samples_wavelength = ST2_FUS_SamplesWavelength();
fus.interpolation_coefficients = ST2_FUS_InterpolationCoefficients();
fus.n_samples = ST2_MC_GetAScanDataNumber();
fus.sampling_frequency = ST2_GetSamplingFrequency();
fus.burst_size = PCIe_GetImageSize() * ST2_GetImagesBurst();

end

