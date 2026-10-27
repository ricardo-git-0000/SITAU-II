function [result] = PCIe_SaveImageData (file, data)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error PCIe_SaveImageData(): "' nlib '" library was not loaded']);    
else
    [result] = calllib(nlib,'PCIe_SaveImageData', file, data, data.length);  
    if (result < 0)
        errordlg(['   PCIe_SaveImageData() error:' ST2_ErrorText(result, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
end
