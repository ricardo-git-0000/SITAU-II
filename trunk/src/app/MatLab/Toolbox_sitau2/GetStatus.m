function [data, error] = GetStatus (ptr_status, size_frame)

error = 0;

nlib = 'api_pcie';
matlab_type = computer;
if strcmp(matlab_type,'PCWIN64') == 1
    nlib = [nlib '_x64'];
else
    nlib = [nlib '_x86'];
end
result = libisloaded (nlib);

if result ~= 1
    errordlg(['   Error ST2_GetStatusFrame(): "' nlib '" library was not loaded']);    
else
    % ptr = libpointer('uint8Ptr', 1:300);
    [result, status] = calllib(nlib,'ST2_GetStatusFrame', ptr_status, size_frame);  
    if (result < 0)
        errordlg(['   ST2_GetStatusFrame error:' ST2_ErrorText(result, nlib)]) % Muestra el codigo junto a su descripcion de la DLL.
    end
end

% ---> Status Frame Version
version = swapbytes(typecast(status(4:-1:1), 'uint32'));

if version == 0x00000001
    [data, error] = GetStatus_v1(status);
else
    disp('File Version Error');
end
                        
end

