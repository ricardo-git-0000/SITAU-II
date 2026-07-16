function [result] = ST2_SetTriggerSource (data)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_SetTriggerSource(): "' nlib '" library was not loaded']);    
else
    [result] = calllib(nlib,'ST2_SetTriggerSource', data);  
    if (result < 0)
        errordlg(['   ST2_SetTriggerSource error:' int2str(result)])
    end
end
