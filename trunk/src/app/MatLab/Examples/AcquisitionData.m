
function [result] = AcquisitionData ()

%% ACQUISITION TYPES
ACQUISITION_TYPE_MC = 0;
ACQUISITION_TYPE_PA = 1;
ACQUISITION_TYPE_TFM = 2;

%% ACQUISITION MODES
ACQUISITION_MODE_FIXED = 0;
ACQUISITION_MODE_CONTINOUS = 1;

STATUS_FRAME_SIZE = 300;

%% Get parameters from system

pause_after_burst = ST2_GetPauseAfterBurst();
if (pause_after_burst < 0)
    ST2_CloseSys();
    return;
end

acquisition_type = ST2_GetAcquisitionType();
if (acquisition_type < 0)
    ST2_CloseSys();
    return;
end

acquisition_mode = ST2_GetAcquisitionMode();
if (acquisition_mode < 0)
    ST2_CloseSys();
    return;
end

n_samples = ST2_GetAScanDataNumber();
switch(acquisition_type)
    case ACQUISITION_TYPE_PA
        n_ascan = ST2_GetFocalLawNumber();
        ptr_data = libpointer('int16Ptr', 1:n_samples * n_ascan);
    case ACQUISITION_TYPE_MC
        mc_size = ST2_MC_GetMatrixCaptureSize();
        n_ascan = ST2_GetChannelsNumber();
        n_focal_laws = ST2_GetFocalLawNumber();
        n_ch = ST2_GetChannelsNumber();
        ptr_data = libpointer('int16Ptr', 1:mc_size);
end
n_acquisitions = ST2_GetAcquisitionsNumber();
ptr_status = libpointer('uint8Ptr', 1:STATUS_FRAME_SIZE);

[result x] = ST2_GetImageEntryPoint_X();
if (result < 0)
    ST2_CloseSys();
    return;
end
[result z] = ST2_GetImageEntryPoint_Z();
if (result < 0)
    ST2_CloseSys();
    return;
end
[result angle] = ST2_GetImageEntryPoint_Angle();
if (result < 0)
    ST2_CloseSys();
    return;
end
start_mm = ST2_GetAcqStartDistance();
if (result < 0)
    ST2_CloseSys();
    return;
end
end_mm = ST2_GetAcqEndDistance();
if (result < 0)
    ST2_CloseSys();
    return;
end

%% Start Adquisition

result = ST2_Start(1);
if (result < 0)
    ST2_CloseSys();
    return;
end

%% Prepare figure
if ~exist('h_figure', 'var') || ~isvalid(h_figure)
    h_figure = figure('visible','on');
    hButtonStop = uicontrol('Style', 'pushbutton', 'String', 'STOP',...
        'Position', [20 20 100 40], 'Callback', 'setappdata(gcbf, ''stopFlag'', 1)');
    set(h_figure, 'CloseRequestFcn', @closeFigCallback);
    imagesc(rand(5)); 
    colormap default;
end
setappdata(h_figure, 'stopFlag', 0);

%% Acquisition loop

flag_end = false;
counter = 0;
i_acq = 0;
try
    while (flag_end == false)
        
        if exist('h_figure', 'var') || isvalid(h_figure)
            
            switch (acquisition_type)

                case ACQUISITION_TYPE_PA

                    [result data n_data index_trg index_aux] = ST2_PA_GetBuffer_LastImage(ptr_data, n_samples * n_ascan);
                    if (result < 0)
                        flag_end = true;
                    else    
                        if (result > 0)
                            Paint_PAImage(data, start_mm, end_mm, x, z, angle, n_samples, n_ascan, h_figure);
                            title(['Acquiring... >>> Acquisition: ' int2str(counter+1) ' - ' int2str(index_trg+1) '/' int2str(n_acquisitions)]);
                        end
                        xlabel('Focal Laws');
                        ylabel('samples'); 
                        % if pause_after_burst == 1
                        %     ST2_Continue();
                        % end
                        counter = counter + 1;
                    end
                    drawnow;      

                case ACQUISITION_TYPE_MC

                    paint_all_focal_laws = true;

                    if acquisition_mode == ACQUISITION_MODE_CONTINOUS

                        [result data i_acq] = ST2_MC_GetBuffer_LastMatrixCapture (ptr_data, mc_size);
                        if (result < 0)
                            flag_end = true;
                        else
                            if result > 0
                                if paint_all_focal_laws
                                    img = reshape(data, n_samples, n_ch * n_focal_laws);
                                    imagesc(img);
                                    colorbar;
                                    title(['Acquiring... >>> Aquisition: ' int2str(counter+1) ' - ' int2str(i_acq+1) '/' int2str(n_acquisitions) ]);
                                    xlabel('Channel')
                                    ylabel('samples')   
                                    drawnow;
                                    i_acq = i_acq + 1;
                                else           
                                    for i_fl=1:n_focal_laws
                                        offset = fl_size * (i_fl - 1) + 1;
                                        if offset + fl_size <= image_size + 1            
                                            img = reshape(data(offset:offset+fl_size-1), n_samples, n_ch);
                                            imagesc(img);
                                            colorbar;
                                            title(['Acquiring... >>> Acquisition: ' int2str(i_acq+1) '/' int2str(n_acquisitions) ' - Focal Law ' int2str(i_fl) '/' int2str(n_focal_laws) ]);
                                            xlabel('Channel')
                                            ylabel('samples')   
                                            drawnow;
                                        end
                                        if isdeleted(h_figure) == true
                                            flag_abort = true;
                                            break;
                                        end
                                    end
                                end
                                counter = counter + 1;                            
                            end                  
                            drawnow;
                        end 
                    else

                        [result data] = ST2_MC_GetBuffer_MatrixCapture (ptr_data, mc_size, i_acq);
                        if (result < 0)
                            flag_end = true;
                        else
                            if result > 0
                                if paint_all_focal_laws
                                    img = reshape(data, n_samples, n_ch * n_focal_laws);
                                    imagesc(img);
                                    colorbar;
                                    title(['Acquiring... >>> Aquisition: ' int2str(i_acq+1) '/' int2str(n_acquisitions) ]);
                                    xlabel('Channel')
                                    ylabel('samples')   
                                    drawnow;
                                else           
                                    for i_fl=1:n_focal_laws
                                        offset = fl_size * (i_fl - 1) + 1;
                                        if offset + fl_size <= image_size + 1            
                                            img = reshape(data(offset:offset+fl_size-1), n_samples, n_ch);
                                            imagesc(img);
                                            colorbar;
                                            title(['Acquiring... >>> Acquisition: ' int2str(i_acq+1) '/' int2str(n_acquisitions) ' - Focal Law ' int2str(i_fl) '/' int2str(n_focal_laws) ]);
                                            xlabel('Channel')
                                            ylabel('samples')   
                                            drawnow;
                                        end
                                        if isdeleted(h_figure) == true
                                            flag_abort = true;
                                            break;
                                        end
                                    end
                                end
                                i_acq = i_acq + 1;    
                                if i_acq >= n_acquisitions
                                    flag_end = true;
                                end
                            end                  
                            drawnow;
                        end 
                    end
            end       
        end
        
        % Check if STOP button was pressed
        if getappdata(h_figure, 'stopFlag') == 1
            flag_end = true;
        end

        % Check if the window was clossed
        if getappdata(h_figure, 'isClosed') == 1
            flag_end = true;
        end

        % [status, error] = GetStatus(ptr_status, STATUS_FRAME_SIZE);
        % if (error < 0)
        %     flag_end = true;
        % end

    end
    
catch ME
    flag_end = true;
    fprintf('Error inside acquisition loop line %d \n',ME.stack(end).line);
    disp(ME.message);               % Muestra el mensaje de error
end

%% Stop acquisition and close connection

ST2_Stop();

delete(h_figure);  % Cerrar la figura

%% Auxiliar function when figure is closed

function closeFigCallback(src, ~)
setappdata(src, 'isClosed', 1);  % Cambiar la variable de control
flag_end = true;

end

end