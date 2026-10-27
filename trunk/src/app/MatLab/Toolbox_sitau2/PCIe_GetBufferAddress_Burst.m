function [result address idx_burst timestamp n_img_buffer] = PCIe_GetBufferAddress_Burst (timeout)
 
% this 3 pointers can be defined inside becuse return only 1 adress 
ptr_address_data = libpointer('uint64Ptr', 1:1);
ptr_idx_burst = libpointer('uint32Ptr', 1:1);
ptr_timestamp = libpointer('uint32Ptr', 1:1);
ptr_n_img_buffer = libpointer('uint32Ptr', 1:1);

nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error PCIe_GetBufferAddress_Burst(): "' nlib '" library was not loaded']);    
else
    [result address idx_burst timestamp n_img_buffer] = calllib(nlib,'PCIe_GetBufferAddress_Burst', ptr_address_data,  ptr_idx_burst, ptr_timestamp, ptr_n_img_buffer, timeout);  
    if (result < 0)
        errordlg(['   PCIe_GetBufferAddress_Burst error:' ST2_ErrorText(result, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
end
