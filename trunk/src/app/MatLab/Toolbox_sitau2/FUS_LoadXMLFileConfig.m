function [error] = Load_XMLFileConfig(xml_config_file)
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

% fus.central_frequency = ST2_FUS_ApodizationCoefficient();
% fus.rapport_focal_aperture = ST2_FUS_RapportFocalAperture();
% fus.apodization_coefficient = ST2_FUS_ApodizationCoefficient();
% fus.lateral_position_x = ST2_FUS_LateralPositionX();
% fus.pixel_size_x = ST2_FUS_PixelSizeX();
% fus.pixels_x = ST2_FUS_PixelsX();
% fus.depth_z = ST2_FUS_DepthZ();
% fus.pixel_size_z = ST2_FUS_PixelSizeZ();
% fus.pixels_z = ST2_FUS_PixelsZ();
% fus.single_plane = ST2_FUS_SinglePlane();
% fus.samples_wavelength = ST2_FUS_SamplesWavelength();
% fus.interpolation_coefficients = ST2_FUS_InterpolationCoefficients();
% fus.n_samples = ST2_GetAScanDataNumber();
% fus.sampling_frequency = ST2_GetSamplingFrequency();
% fus.burst_size = ST2_MC_GetFMCMatrixSize() * ST2_GetFMCMatrixBurstNumber();


% fus.c = ST2_GetMaterialVelocity() / 1000;
% fus.Bdx = ST2_GetProbePitch();
% fus.Bnx = ST2_GetProbeElementNumber();
% fus.Bf0 = ST2_FUS_CentralFrequency();
% fus.Bfd = ST2_FUS_RapportFocalAperture();
% fus.Bapx = ST2_FUS_ApodizationCoefficient();
% fus.Ix0 = ST2_FUS_LateralPositionX();
% fus.Idx = ST2_FUS_PixelSizeX();
% fus.Inx = ST2_FUS_PixelsX();
% fus.Iz0 = ST2_FUS_DepthZ();
% fus.Inz = ST2_FUS_PixelsZ();
% fus.Idz = ST2_FUS_PixelSizeZ();
% fus.Iny = ST2_FUS_SinglePlane();
% [error,fus.Cax] = ST2_MC_GetPlaneWaveImaging_Angles_rad();
% fus.Cn = length(fus.Cax);
% fus.Rfs = ST2_GetSamplingFrequency();
% fus.Rnspw = ST2_FUS_SamplesWavelength();
% % fus.Rnt = round(fus.Inz*fus.Idz*2/fus.c*fus.Rfs*(fus.Rnspw/4)/32)*32;  % Numper of samples must be multiple of 32.
% % fus.Rnt = ST2_MC_GetAScanDataNumber();
% fus.burst_size = ST2_MC_GetFMCMatrixSize() * ST2_GetFMCMatrixBurstNumber();
% fus.RnI = ST2_GetFMCMatrixBurstNumber();
% fus.Nblocks = 1;
% fus.cint = interCoef6x16;       % interpolation coefficients
% fus.Saturation=32000;
% fus.filterSVD=0.1;
% fus.scale=0.1; 
% fus.fir = single(fir1(16,25/500*2,'high'));
% fus.Rt0 = single(ones(fus.Cn,1)*2*fus.Iz0/fus.c);    % Delay first sample in µs (float)
% fus.Rt0 = (fus.Bnx-1)*fus.Bdx*sin(fus.Cax)/fus.c.*(fus.Cax<0);% + p.Iz0/p.c;
% fus.prf = ST2_GetPRF();
% fus.n_samples = ST2_GetAScanDataNumber();
% fus.Rnt = fus.n_samples;


%% This part is tested with the function 
% [valid, withWarnings, performance] = Validate_XMLConfig(xmlFile);

% byte_rate = fus.n_samples * 128 * fus.prf * 2;
% if byte_rate > 3e9    
%     warning(['Byte Rate too high, system may pause acquisition!']);
% else
%     if (byte_rate < 1) 
%         byte_rate = byte_rate * 1000;
%         disp(['Data Rate: ' int2str(byte_rate/1e9) 'MB/s'])
%     else
%         disp(['Data Rate: ' int2str(byte_rate/1e9) 'GB/s'])
%     end
% end

end

