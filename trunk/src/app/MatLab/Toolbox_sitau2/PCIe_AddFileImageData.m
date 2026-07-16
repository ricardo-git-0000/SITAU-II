function [result] = PCIe_AddFileImageData (file, data)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error PCIe_AddFileImageData(): "' nlib '" library was not loaded']);    
else
    [result] = calllib(nlib,'PCIe_AddFileImageData', file, data, length(data));  
    if (result < 0)
        errordlg(['   PCIe_AddFileImageData() error:' int2str(result)])
    end
end
