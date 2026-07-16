function [result] = ST2_FUS_RapportFocalAperture ()
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_FUS_RapportFocalAperture(): "' nlib '" library was not loaded']);    
else
    result = calllib(nlib,'ST2_FUS_RapportFocalAperture');  
    if (result < 0)
        errordlg(['   ST2_FUS_RapportFocalAperture error:' int2str(result)])
    end
end
