function [result data n_data index_trg index_aux] = ST2_PA_GetBuffer_LastImage (ptr_data, image_size)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_PA_GetBuffer_LastImage(): "' nlib '" library was not loaded']);    
else
    ptr_n_data = libpointer('ulongPtr', 1:1);
    ptr_idx_trg = libpointer('ulongPtr', 1:1);
    ptr_idx_aux = libpointer('ulongPtr', 1:1);
    [result data n_data index_trg index_aux] = calllib(nlib,'ST2_PA_GetBuffer_LastImage', ptr_data, image_size, ptr_n_data, ptr_idx_trg, ptr_idx_aux);  
    if (result < 0)
        errordlg(['   ST2_PA_GetBuffer_LastImage error:' int2str(result)])
    end
end
