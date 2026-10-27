function [result] = ST_GetHighVoltageMaximum_AI ()
 
%Lee la tensíon máxima a que se ha limitado el pulser
% Modificado 20240626 A. Ibáñez

nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_GetHighVoltageMaximum(): "' nlib '" library was not loaded']);    
else
    result = calllib(nlib,'ST2_GetHighVoltageMaximum');  
    if (result < 0)
        errordlg(['   ST2_GetHighVoltageMaximum error:' ST2_ErrorText(result, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
end
