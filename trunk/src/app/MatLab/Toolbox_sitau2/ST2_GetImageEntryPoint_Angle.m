function [result data] = ST2_GetImageEntryPoint_Angle ()
 
nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_GetImageEntryPoint_Angle(): "' nlib '" library was not loaded']);    
else
    n_data = calllib(nlib,'ST2_GetFocalLawNumber');
    if n_data > 0
        ptr = libpointer('singlePtr', 1:n_data);    
        [result data] = calllib(nlib,'ST2_GetImageEntryPoint_Angle', ptr, n_data);  
        if (result < 0)
            errordlg(['   ST2_GetImageEntryPoint_Angle error:' int2str(result)])
        end
    end    
end
