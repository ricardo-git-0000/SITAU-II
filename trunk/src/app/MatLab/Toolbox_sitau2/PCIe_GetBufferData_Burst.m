function [result data] = PCIe_GetBufferData_Burst (ptr_data, image_size, timeout)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error PCIe_GetBufferData_Burst(): "' nlib '" library was not loaded']);    
else
    [result data] = calllib(nlib,'PCIe_GetBufferData_Burst', ptr_data, image_size, timeout);  
    if (result < 0)
        errordlg(['   PCIe_GetBufferData_Burst error:' int2str(result)])
    end
end
