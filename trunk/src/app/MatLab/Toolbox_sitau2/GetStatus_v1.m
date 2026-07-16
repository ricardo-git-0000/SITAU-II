function [data, error] = GetStatus_v1 (status)

error = 0;
idx = 1;

try

    % ---> Version
    size_data = 4;
    data.version = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;
    
    % ---> AFES_align_ok
    size_data = 1;
    data.AFES_align_ok = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;

    % ---> LVDS_deskew_ok
    size_data = 1;
    data.LVDS_deskew_ok = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;

    % ---> errores_lvds
    size_data = 4;
    data.errores_lvds = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;    

    % ---> errores_ddr
    size_data = 4;
    data.errores_ddr = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;

    % ---> codigo_pulser_leido
    size_data = 4;
    data.codigo_pulser_leido = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;    

    % ---> n_bases_bcc
    size_data = 1;
    data.n_bases_bcc = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      

    % ---> retardo_extra_bcc
    size_data = 1;
    data.retardo_extra_bcc = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      

    % ---> qsfp_channel_rdy_up
    size_data = 1;
    data.qsfp_channel_rdy_up = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      

    % ---> qsfp_channel_rdy_dn
    size_data = 1;
    data.qsfp_channel_rdy_dn = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      

    % ---> cpu0_flags
    size_data = 4;
    data.cpu0_flags = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> cpu0_info
    size_data = 4;
    for (i=1:5)    
        data.cpu0_info(i) = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
        idx = idx + size_data;      
    end

    % ---> cpu0_status
    size_data = 4;
    data.cpu0_status = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> cpu1_flags
    size_data = 4;
    data.cpu1_flags = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> cpu1_info
    size_data = 4;
    for (i=1:5)    
        data.cpu1_info(i) = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
        idx = idx + size_data;      
    end

    % ---> cpu1_status
    size_data = 4;
    data.cpu1_status = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> acq_status_code
    size_data = 8;
    data.acq_status_code = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint64'));
    idx = idx + size_data;      

    % ---> acq_sitau_status
    size_data = 1;
    data.acq_sitau_status = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      

    % ---> rd_address
    size_data = 4;
    data.rd_address = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> wr_address
    size_data = 4;
    data.wr_address = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> n_overflows
    size_data = 4;
    data.n_overflows = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> n_acquisitions
    size_data = 4;
    data.n_acquisitions = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> ind_acquisition
    size_data = 4;
    data.ind_acquisition = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> ind_acquisition
    size_data = 4;
    data.ind_acquisition = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;   

    % ---> trigger_source
    size_data = 1;
    data.trigger_source = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint8'));
    idx = idx + size_data;      
    
    % ---> qsfp_link_errors_up
    size_data = 4;
    data.qsfp_link_errors_up = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      

    % ---> qsfp_link_errors_dn
    size_data = 4;
    data.qsfp_link_errors_dn = swapbytes(typecast(status(idx+size_data-1:-1:idx), 'uint32'));
    idx = idx + size_data;      
    
catch ME
    error = -1
    fprintf('Error GetStatus_v1() - Buffer index:  %d \n',idx);
    disp(ME.message);               % Muestra el mensaje de error
end

end