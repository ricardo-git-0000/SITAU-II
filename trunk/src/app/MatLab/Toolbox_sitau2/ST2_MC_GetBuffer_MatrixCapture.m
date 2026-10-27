function [result data] = ST2_MC_GetBuffer_MatrixCapture (ptr_data, image_size, index_trigger_line)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_MC_GetBuffer_MatrixCapture(): "' nlib '" library was not loaded']);    
else
    [result data] = calllib(nlib,'ST2_MC_GetBuffer_MatrixCapture', ptr_data, image_size, index_trigger_line);  
    if (result < 0)
        errordlg(['   ST2_MC_GetBuffer_MatrixCapture() error:' ST2_ErrorText(result, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
end
