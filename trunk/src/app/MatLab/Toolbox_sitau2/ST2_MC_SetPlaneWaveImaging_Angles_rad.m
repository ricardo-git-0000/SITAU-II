function [result] = ST2_MC_SetPlaneWaveImaging_Angles_rad (n_acquisitions)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_MC_SetPlaneWaveImaging_Angles_rad(): "' nlib '" library was not loaded']);    
else
    for i=1:n_angles
        angles_deg(i) = angles(i) * 180 / pi;
    end
    
    [result] = calllib(nlib,'ST2_MC_SetPlaneWaveImaging_Angles_rad', angles_deg, n_angles);  
    if (result < 0)
        errordlg(['   ST2_MC_SetPlaneWaveImaging_Angles_rad() error:' int2str(result)])
    end
end
