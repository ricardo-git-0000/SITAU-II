function [result] = ST2_GetPauseAfterBurst()
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_GetPauseAfterBurst(): "' nlib '" library was not loaded']);    
else
    result = calllib(nlib,'ST2_GetPauseAfterBurst');  
    if (result < 0)
        errordlg(['   ST2_GetPauseAfterBurst error:' int2str(result)])
    end
end
