function [result] = ST2_EnabledLoadUCIFirmware (data)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_EnabledLoadUCIFirmware(): "' nlib '" library was not loaded']);    
else
    calllib(nlib,'ST2_EnabledLoadUCIFirmware', data);  
end
