function [error] = ST2_CloseSys ()
%
% [error] = UT_CloseSys ()
% ========================
% 
% Cierra la comunicación con el sistema y libera los recursos reservados 
% por la librería.
%
% Parametros de salida:
%
%    (error): Ver códigos de error.

nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
error = libisloaded (nlib);

if error ~= 1
    errordlg(['   Error ST2_CloseSys(): "' nlib '" library was not loaded']);    
else
    error = calllib(nlib,'ST2_CloseSys');
    if error < 0
        errordlg(['   ST2_CloseSys error: ' ST2_ErrorText(error, nlib)]); % Muestra el codigo junto a su descripcion de la DLL.
    end   
    unloadlibrary(nlib);
end

