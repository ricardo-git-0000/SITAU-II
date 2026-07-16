function [result angles] = ST2_MC_GetPlaneWaveImaging_Angles_deg ()
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_MC_GetPlaneWaveImaging_Angles_deg(): "' nlib '" library was not loaded']);    
else   
    n_data = calllib(nlib,'ST2_MC_GetPlaneWaveImaging_AnglesNumber');
    if n_data > 0
        ptr = libpointer('singlePtr', 1:n_data);    
        [result angles] = calllib(nlib,'ST2_MC_GetPlaneWaveImaging_Angles_deg', ptr, n_data);  
        if (result < 0)
            errordlg(['   ST2_MC_GetPlaneWaveImaging_Angles_deg error:' int2str(result)])
        end
    end    
end
