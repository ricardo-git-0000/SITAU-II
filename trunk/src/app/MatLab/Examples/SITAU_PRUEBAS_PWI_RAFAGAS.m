%**************************************************************************
% DEMO DEL UTILIZACION DE LA TOOLBOX SITAU II
% Basada en el código de Ricardo
% 
% 20241118 A. Ibáñez (toolbox del SDK_SITAU v1.0.0.5)
% 20250122 Montse (toolbox del SDK_SITAU V. v1.0.2.0 y posteriores...)
%**************************************************************************


clear all
%clc

emular_sitau = false

% PRUEBAS
%**************************************************************************
% Hay algún problema con los índices de las imágenes que devuelve el
% sistema. Estas pruebas son un ejemplo para comprobarlo
% Al final se muestran dos variables: orden, que guarda los índices de las
% imágenes devueltas en el orden en que las envía; e indices, que tiene un
% -1 en aquellas imágenes que no se han devuelto
% (Los índices devueltos deben ir de 0 al número de imágenes solicitadas-1)

% 1. Programando el número de imágenes por ráfaga a 5 y dibujando las
% imágenes, los índices de las imágenes devueltas son superiores a 4 
dibujar = true;
inspec.imagenes_por_rafaga = 50;

% 2. Programando el número de imágenes por ráfaga a 5 y sin dibujar las
% imágenes, los índices de las imágenes devueltas son superiores a 4 
% dibujar = false;
% inspec.imagenes_por_rafaga = 5;

% 3. Programando el número de imágenes por ráfaga a 15 y dibujando las
% imágenes, hay imágenes que no se devuelven 
% dibujar = true;
% inspec.imagenes_por_rafaga = 15;

% 4. Programando el número de imágenes por ráfaga a 15 y sin dibujar las
% imágenes va bien 
% dibujar = false;
% inspec.imagenes_por_rafaga = 15;


% nomFichero = '..\medidas\aguaPWI_3_trans_mov.mat';
% nomFichero = 'kk.mat';

%% Toolboxes path

% path_toolbox_sitau = '..\sitau2_openfus_v11\SDK_v11\Matlab\Toolbox_sitau2\';
path_toolbox_sitau = '..\Toolbox_sitau2\';
addpath(path_toolbox_sitau)

% Directorios probados
% path_toolbox_sitau='C:\Users\alberto\source\repos\Kernel_MPS\sitau2_matlab\toolbox_sitau2\';
% path_toolbox_sitau='C:\PROYECTOS\FactorCoherencia\Ultrasonidos\Sitau\sitau2_matlab\';
% path_toolbox_sitau='C:\PROYECTOS\FactorCoherencia\Ultrasonidos\Sitau\sitau2_openfus-main\SDK_v6\MatLab\Toolbox_sitau2\';
% config_base = [path_toolbox_sitau 'SCANVIEW_UT.XML'];  %archivo de configuracion que se utilizara ESTE ARCHIVO ES ANTIGUO
config_base = ['SITAU2_GE.XML'];  %archivo de configuracion que se utilizara
%config_base = 'VERMONT18_PRU_SCANVIEW_UT.XML';

n_fig=10;
rep_log=true;
range_lin=[0,100];%32768];
range_log=[-50 0];


%%
%DEFINE ARRAY:
%=============
arr.nombre = 'Vermont 18MHz ';
arr.N = 128;        %Numero de elementos del array
arr.pitch = .1;     %Pitch en mm
arr.fr = 18;        %Frecuencia nominal MHz
arr.fnum = 1;       %numero f de los elementos
arr.maxVolt = 50;   %Maxima tension de excitación admisible para el array
arr.fs = 62.5;        %Frecuencia de muestreo de referencia MHz.
%arr.ts=1/arr.fm;   %Period de muestreo de referencia us (Â¿mantener?)
arr.alin = 1;        %Alineación de array:   0 elemento 0 en x=0
%                       1 centro en x=0
%                       2 elemento N-1 en x=0
arr.x = ((0:arr.N-1).'-arr.alin*(arr.N-1)/2)*arr.pitch; %Posicion X de los elementos
arr.z = zeros(arr.N,1); %Posicion z de los elementos


%%
%DEFINE INSPECCION:
%==================

inspec.nombre ='Probando version SITAU API v1.0.9.0 25/03/2025';
inspec.medio = 'Agua';
inspec.vel = 1.50;     %Velocidad del sonido (mm/us)

inspec.dist_ini=6;     %Distania mínima de la region al array (mm)
inspec.dist_fin=13;    %Distancia máxima de la region al array (mm)

% pulser
inspec.n_pulsos=1;
inspec.ancho_pulso= round(500/arr.fr);
inspec.ancho_pulso= 28;
inspec.amp_pulser=50;

%numero de imagenes:
% inspec.imagenes_por_rafaga = 10;
inspec.n_rafagas = 1;
inspec.n_imagenes = inspec.n_rafagas*inspec.imagenes_por_rafaga;
inspec.gan = 50;    %Ganancia dB
inspec.prf = 5000;  %PRF  Hz

% modalidad de acquisicion:  0: Fijo, se adquiere el numero de rafagas de imagenes especificado
%                            1: Continuo pues eso
inspec.ACQ_MOD = 'FIJO'; % EN ESTE MODO NO FUNCIONA NI CON ST2_CONTINUE NI SIN EL
inspec.ACQ_MOD_COD = 0;
% inspec.ACQ_MOD = 'CONTINUO';
% inspec.ACQ_MOD_COD = 1;

% tipo de captura: 0: MC devuelve, por cada disparo (ley focal) los datos sin procesar del todos los CANALES (no elementos)
%                  1: PA phased array devuelve la imagen CONFORMADA con el algoritmo PHASED ARRAY
%                  2: TFM devuelve la  imagen conformada con el algoritmo TFM
inspec.ACQ_TYPE = 'MC';
inspec.ACQ_TYPE_COD = 0;

% tipo de ley focal en tipo MC: 0: FMC (full matrix capture)
%                               1: PWI (plane wave image compounding)
%                               2: UDFL (user defined focal law)
inspec.FL_TYPE = 'PWI';
inspec.FL_TYPE_COD = 1;


%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% // ______________________________________________________________________
% // ------ CONEXION CON EL SITAU-II
% // ======================================================================

% ABRE EL SISTEMA
% ---------------

result = OpenSys ([path_toolbox_sitau 'BIN\'], '192.168.2.10', 55902, 50460, 0, emular_sitau);
if (result < 0)
    CloseSys();
    return;
end
disp(' ')

%%
% CARGA CONFIGURACION INICIAL DEL SISTEMA
% ---------------------------------------

%Como base uso la configuración del archivo especificado o el generico
%SCANVIEW_UT.XML incluido con la API
if exist(config_base, 'file')
    result = ST2_LoadXMLFileConfig(config_base);
else
    disp('Utilizando configuración por defecto del sistema');
    result = ST2_LoadXMLFileConfig([path_toolbox_sitau 'SCANVIEW_UT.XML']);
end
if (result < 0)
    CloseSys();
    return;
end

%%
% PROGRAMA EL SISTEMA PARA EL ARRAY Y LA INSPECCION ELEGIDOS
% ----------------------------------------------------------

% Desactivo el pulser
% -------------------
error=ST2_SetPulserEnable(false);
if error
    disp ('Atención: fallo en la función para inhabilitar el pulser');
end
if ST2_GetPulserEnable()
    %Si no pude inhabilitarlo trato de llevarlo al modo mas seguro
    ST2_SetPulseAmplitude(0);
    ST2_SetPulsesNumber(0);
    ST2_SetPulseWidth(0);
    disp('PULSER ACTIVO. No se pudo desactivar el pulser');
    CloseSys();
end

% Se programan las características del array
% ------------------------------------------
result = ST2_SetProbeConfig (arr.N, arr.pitch, arr.fr);  
if (result < 0)
    CloseSys();
    return;
end

% Se comprueba que se ha programado bien 
% --------------------------------------
if (arr.N ~= ST2_GetProbeElementNumber())   %Numero de elementos del array
    disp('Error en el número de elementos del array');
end
if ((arr.pitch - ST2_GetProbePitch()) > 1e-6)       %Pitch en mm
    disp('Error en el pitch del array');
end
if (arr.fr ~= ST2_GetProbeFrequency())   %Frecuencia nominal del array
    disp('Error en la frecuencia del array');
end

% Se programa el pulser y se comprueba que se ha programado bien
% --------------------------------------------------------------
ST2_SetHighVoltageMaximum (arr.maxVolt);
if (arr.maxVolt ~= ST2_GetHighVoltageMaximum ())
    disp('Error en la programación del voltaje máximo del pulser');
end
ST2_SetPulsesNumber(inspec.n_pulsos);      %solo un pulso
if (inspec.n_pulsos ~= ST2_GetPulsesNumber())
    disp('Error en la programación del número de pulsos del pulser');
end
ST2_SetPulseWidth (inspec.ancho_pulso);    %de medio ciclo de la frecuencia nominal (ns)
if (inspec.ancho_pulso ~= ST2_GetPulseWidth ())
    disp('Error en la programación del ancho del pulso del pulser');
end
ST2_SetPulseAmplitude(inspec.amp_pulser);   %A la maxima admisible
if (inspec.amp_pulser ~= ST2_GetPulseAmplitude())
    disp('Error en la programación de la amplitud del pulser');
end

% Se programa la inspección y se comprueba que se ha programado bien
% ------------------------------------------------------------------
ST2_SetMaterialVelocity (inspec.vel*1e3); %se programa en  m/s y yo lo tengo en mm/us
if (inspec.vel ~= ST2_GetMaterialVelocity ()*1e-3)
    disp('Error en la programación de la velocidad del material');
end

PULSO_ECO=0; % da la mitad de lo que viene en el fichero SITAU_GE.XML          
%PULSO_ECO=1; % da la mitad que con 0
ST2_SetAcqDistance (inspec.dist_ini, inspec.dist_fin, PULSO_ECO);
if (inspec.dist_ini ~= ST2_GetAcqStartDistance ())
    disp('Error en la programación del inicio de la inspección');
end
if (inspec.dist_fin ~= ST2_GetAcqEndDistance ())
    disp('Error en la programación del final de la inspección');
end
inspec.t_ini = ST2_GetAcqStartTime ();
inspec.t_fin = ST2_GetAcqEndTime ();
t_acq = ST2_GetAcqTime ();
n_samples = ST2_GetAScanDataNumber();

ST2_SetGain(inspec.gan);
if (inspec.gan ~= ST2_GetGain())
    disp(['Ganancia programada ' num2str(inspec.gan) ' Ganancia del equipo ' num2str(ST2_GetGain())]);
    inspec.gan = ST2_GetGain();
end

ST2_SetPRF(inspec.prf);
if (inspec.prf ~= ST2_GetPRF())
    disp('Error en la programación de la PRF');
end

ST2_SetAcquisitionMode(inspec.ACQ_MOD_COD);   
if (inspec.ACQ_MOD_COD ~= ST2_GetAcquisitionMode())
    disp('Error en la programación del modo de adquisición');
end

ST2_SetAcquisitionType(inspec.ACQ_TYPE_COD);
if (inspec.ACQ_TYPE_COD ~= ST2_GetAcquisitionType())
    disp('Error en la programación del tipo de adquisición');
end

% ángulos de deflexión de las emisiones en PWI
inspec.angles =-15:1:15;    
result = ST2_MC_SetPlaneWaveImaging_Angles_deg (inspec.angles, numel(inspec.angles));
if (result < 0)
    disp(['ST2_MC_SetPlaneWaveImaging_Angles_deg da error ' num2str(result)]);
end
[~, aux] = ST2_MC_GetPlaneWaveImaging_Angles_deg ();
if (aux ~= inspec.angles)
    disp('Error en la programación de los ángulos en modo PWI');
end

% por probar si era diferente pero no...
% %ST2_MC_SetPlaneWaveImaging(float start_angle, float step_angle, float end_angle);
% result = ST2_MC_SetPlaneWaveImaging(inspec.angles(1), 1, inspec.angles(end));

ST2_SetAcquisitionsNumber(inspec.n_rafagas,inspec.imagenes_por_rafaga);
if (inspec.n_rafagas ~= ST2_GetBurstsNumber())
    disp('Error en la programación del número de ráfagas');
end
if (inspec.imagenes_por_rafaga ~= ST2_GetAcquisitionsBurstNumber ())
    disp('Error en la programación del número de imágenes por ráfaga');
end
inspec.n_imagenes = ST2_GetAcquisitionsNumber();
disp(' ');
disp(['Numero de rafagas ' num2str(inspec.n_rafagas)])
disp(['Numero de imagenes por rafaga ' num2str(inspec.imagenes_por_rafaga)])
disp(['Numero de imagenes total ' num2str(inspec.n_imagenes)])
if (inspec.n_imagenes ~= (inspec.n_rafagas*inspec.imagenes_por_rafaga))
    disp(['Error en el número de imágenes total ' num2str(inspec.n_imagenes)])
end
disp(' ');


%%
% ALB. para ver si los cambios se reflejan en el archivo de
% configuracion (XML)

result = ST2_SaveXMLFileConfig('VERMONT18_PRU_SCANVIEW_UT.XML');
if (result < 0)
    CloseSys();
    return;
end

% -------------------------------------------------
% Cargando el fichero justo antes de la adquisición
% -------------------------------------------------

%% SYSTEM CONFIGURATION
% xmlFile = 'VERMONT18_PRU_SCANVIEW_UT.XML';
% 
% % [valid, withWarnings, performance] = Validate_XMLConfig(xmlFile);
% result = ST2_LoadXMLFileConfig(xmlFile);
% 
% if (result < 0)
%     CloseSys();
%     return;
% end

% -------------------------------------------

% Se reserva memoria para toda la adquisición
% -------------------------------------------
%image_size = ST2_MC_GetFMCMatrixSize(); % n_samples * n_ch * n_focal_laws
image_size = ST2_MC_GetMatrixCaptureSize();
focal_law_size = ST2_MC_GetSingleImageSize(); % n_samples * n_ch 
n_samples = ST2_GetAScanDataNumber();   %  N_muestras
n_focal_laws = ST2_GetFocalLawNumber();    %  N_disparos
n_ch = ST2_GetChannelsNumber();            %  N_receptores
inspec.n_imagenes = ST2_GetAcquisitionsNumber();
data=zeros(image_size,1, 'int16');           
ptr_data = libpointer('int16Ptr', data);

img=zeros(n_samples, n_ch * n_focal_laws, 'int16');
todas=zeros(n_samples, n_ch * n_focal_laws, inspec.n_imagenes, 'int16');
indices = ones(1, inspec.n_imagenes)*-1;
orden = [];


h_figure = figure(n_fig);
set(h_figure,'visible','on');
% if rep_log
%     tmod= [inspec.FL_tipo ' LOG '];
% else
%     tmod= [inspec.FL_tipo ' LIN '];
% end


% INICIO DE ADQUISICION
% ---------------------
error=ST2_SetPulserEnable(true);

if error
    disp ('Atención: fallo en la función para habilitar el pulser');
end

tic
result = ST2_Start(1);
if (result < 0)
    ST2_SetPulserEnable(false);
    CloseSys();
    return;
end
toc

% BUCLE DE ADQUISICION DE UNA IMAGEN COMPLETA
% ------------------------------------------- 
flag_end = false;

veces = 0;

%pause(10)

while flag_end == false
    
    index_ima = 0;
%     while ((index_ima < inspec.n_imagenes) && (~flag_end))
    while ((~flag_end))

        [result data] = ST2_MC_GetBuffer_MatrixCapture(ptr_data, image_size, index_ima);
%         [result data index_trigger_line] = ST2_MC_GetBuffer_LastMatrixCapture (ptr_data, image_size);
        %         result-image_size
        if (result > 0)
%             index_trigger_line
            index_ima
            img = reshape(data, n_samples, n_ch*n_focal_laws);
            todas(:,:,index_ima+1)=img(:,:);
            indices(index_ima+1) = index_ima;%index_ima;
            orden = [orden index_ima];
            if dibujar
                figure(h_figure)
                imagesc([1, n_ch*n_focal_laws], [1 n_samples], abs(img));
                %caxis(range_lin);
                colorbar;
                title(['Adquisición ' int2str(index_ima+1) ' - Índice de la Imagen adquirida ' int2str(index_ima+1) '/' int2str(inspec.n_imagenes)]);
                xlabel('Channel');
                ylabel('samples');
                pause(.1)
                drawnow;
            end
            veces = veces+1;
            index_ima = index_ima + 1;
        end
        flag_end = flag_end || ~ishghandle(h_figure);
    end
    
    flag_end = flag_end || ~ishghandle(h_figure);
    flag_end = flag_end || (veces == inspec.n_imagenes);
end


result = ST2_Start(1);
toc

h_figure = figure(n_fig);
set(h_figure,'visible','on');

% BUCLE DE ADQUISICION DE UNA IMAGEN COMPLETA
% ------------------------------------------- 
flag_end = false;

veces = 0;

%pause(10)

while flag_end == false
    
    index_ima = 0;
%     while ((index_ima < inspec.n_imagenes) && (~flag_end))
    while ((~flag_end))

        [result data] = ST2_MC_GetBuffer_MatrixCapture(ptr_data, image_size, index_ima);
%         [result data index_trigger_line] = ST2_MC_GetBuffer_LastMatrixCapture (ptr_data, image_size);
%                 result-image_size
        if (result > 0)
%             index_trigger_line
            index_ima
            img = reshape(data, n_samples, n_ch*n_focal_laws);
            todas(:,:,index_ima+1)=img(:,:);
            indices(index_ima+1) = index_ima;%index_ima;
            orden = [orden index_ima];
            if dibujar
                figure(h_figure)
                imagesc([1, n_ch*n_focal_laws], [1 n_samples], abs(img));
                %caxis(range_lin);
                colorbar;
                title(['Adquisición ' int2str(index_ima+1) ' - Índice de la Imagen adquirida ' int2str(index_ima+1) '/' int2str(inspec.n_imagenes)]);
                xlabel('Channel');
                ylabel('samples');
                pause(.1)
                drawnow;
            end
            veces = veces+1;
            index_ima = index_ima + 1;
        end
        flag_end = flag_end || ~ishghandle(h_figure);
    end
    
    flag_end = flag_end || ~ishghandle(h_figure);
    flag_end = flag_end || (veces == inspec.n_imagenes);
end

% FIN DE ADQUISICIÓN
% ------------------
ST2_Stop();
ST2_SetPulserEnable(false);

% CIERRE DE LA CONEXIÓN
% ---------------------
CloseSys();

disp(' ');
disp('Orden en que se devuelven las imágenes')
orden

disp('Indices de las imágenes devueltas (-1 significa que esa imagen no se ha devuelto)')
indices

disp(['Número de imágenes que faltan: ' num2str(sum(indices==-1))])

% save(nomFichero, 'inspec', 'todas', 'indices', '-mat');
