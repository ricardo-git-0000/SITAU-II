function [result] = ST2_SetAcqDistance (start_distance, end_distance, acquisition_time_mode)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_SetAcqDistance(): "' nlib '" library was not loaded']);    
else
    [result] = calllib(nlib,'ST2_SetAcqDistance', start_distance, end_distance, acquisition_time_mode);  
    if (result < 0)
        errordlg(['   ST2_SetAcqDistance error:' int2str(result)])
    end
end
