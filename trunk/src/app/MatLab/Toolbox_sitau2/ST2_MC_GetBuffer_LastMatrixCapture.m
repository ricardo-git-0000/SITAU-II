function [result data index_trigger_line] = ST2_MC_GetBuffer_LastMatrixCapture (ptr_data, image_size)
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_MC_GetBuffer_LastMatrixCapture(): "' nlib '" library was not loaded']);    
else
    ptr_index_trigger_line = libpointer('uint32Ptr', 1:1);
    data = 0;
    index_trigger_line = 0;
    [result data index_trigger_line] = calllib(nlib,'ST2_MC_GetBuffer_LastMatrixCapture', ptr_data, image_size, ptr_index_trigger_line);  
    if (result < 0)
        errordlg(['   ST2_MC_GetBuffer_LastMatrixCapture error:' int2str(result)])
    end
end
