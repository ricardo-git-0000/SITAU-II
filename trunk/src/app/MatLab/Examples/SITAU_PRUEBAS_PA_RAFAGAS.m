%**************************************************************************
% DEMO DEL UTILIZACION DE LA TOOLBOX SITAU II
% Basada en el código de Ricardo
% 
% 20241118 A. Ibáñez (toolbox del SDK_SITAU v1.0.0.5)
% 20250122 Montse (toolbox del SDK_SITAU V. v1.0.2.0)
%**************************************************************************


clear all
%clc

emular_sitau = false

%%
% CONFIGURACIÓN PARA BARRIDO AUTOMÁTICO
%======================================
motores = false;
recorrido = 5;
pasoMm = 1;
if motores
    % Se conectan los motores
    [result, NBSDevice] = ConectarMotor();
    if (result == -1)
        return
    end
    % Se programa la velocidad
    EnviarVelocidad(NBSDevice, 500);
    % Se calcula el número de movimientos que va a hacer el motor
    nMovimientos = recorrido/pasoMm;
    indiceFich = 0;
%     nomFichero = '..\Medidas\Cerebro\11_07_2025\CerebroPAL';
    nomFichero = 'kk';
else
%     nomFichero = '..\Medidas\Cerebro\11_07_2025\CerebroPAL.mat';
    nomFichero = 'kk.mat';
end


%%
% CONFIGURACIÓN DE LA ADQUISICIÓN
%======================================
dibujar = false;
inspec.imagenes_por_rafaga = 150;
inspec.n_rafagas = 1;

%% Toolboxes path

path_sitau = '..\sitau2_openfus_v15\SDK_v15\Matlab\';
path_sitau = '.\';
% en la v15 SITAU UCI HW Firmware Version:   160 (45/98/f)
% path_sitau = '..\sitau2_openfus_v14\SDK_v14\Matlab\';
% en la v14 SITAU UCI HW Firmware Version:   128 (45/c0/0)
%path_sitau = '..\sitau2_openfus_v11\SDK_v11\Matlab\';

path_toolbox_sitau = [path_sitau 'Toolbox_sitau2\'];
path_toolbox_sitau = '..\Toolbox_sitau2\';
addpath(path_toolbox_sitau)

% Se carga la librería Stlib2 para poder hacer phased array
% CargarStlib2

% Directorios probados
% path_toolbox_sitau='C:\Users\alberto\source\repos\Kernel_MPS\sitau2_matlab\toolbox_sitau2\';
% path_toolbox_sitau='C:\PROYECTOS\FactorCoherencia\Ultrasonidos\Sitau\sitau2_matlab\';
% path_toolbox_sitau='C:\PROYECTOS\FactorCoherencia\Ultrasonidos\Sitau\sitau2_openfus-main\SDK_v6\MatLab\Toolbox_sitau2\';
% config_base = [path_toolbox_sitau 'SCANVIEW_UT.XML'];  %archivo de configuracion que se utilizara ESTE ARCHIVO ES ANTIGUO
config_base = [path_sitau 'SITAU2_GE.XML'];  %archivo de configuracion que se utilizara 
%config_base = ['SITAU2_GE.XML'];  %archivo de configuracion que se utilizara
%config_base = 'VERMONT18_PRU_SCANVIEW_UT.XML';

n_fig=10;
rep_log=true;
range_lin=[0,32768];
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

inspec.nombre ='Probando version SITAU API v1.0.13.0 23/04/2025';
inspec.medio = 'Agua';

inspec.dist_ini=0;     %Distania mínima de la region al array (mm)
inspec.dist_ini=6;     %Distania mínima de la region al array (mm)
inspec.dist_fin=19;    %Distancia máxima de la region al array (mm)
% inspec.dist_ini=6;     %Distania mínima de la region al array (mm)
% inspec.dist_fin=12;    %Distancia máxima de la region al array (mm)
inspec.vel = 1.54;     %Velocidad del sonido (mm/us)
inspec.gan = 30;    %Ganancia dB
inspec.prf = 500;  %PRF  Hz

% filtro hw
inspec.filtro = 0;
inspec.fmin = 10*1e3;
inspec.fmax = 30*1e3;

% pulser
inspec.n_pulsos=3;
inspec.ancho_pulso= round(500/arr.fr);
inspec.ancho_pulso= 28;
inspec.amp_pulser=50;

%numero de imagenes:
% inspec.imagenes_por_rafaga = 2;
% inspec.imagenes_por_rafaga = 10; % da igual...
% inspec.n_rafagas = 1;
% inspec.n_rafagas = 12; % da igual...
inspec.n_imagenes = inspec.n_rafagas*inspec.imagenes_por_rafaga;

% modalidad de acquisicion:  0: Fijo, se adquiere el numero de rafagas de imagenes especificado
%                            1: Continuo pues eso
% inspec.ACQ_MOD = 'CONTINUO';
% inspec.ACQ_MOD_COD = 1;
inspec.ACQ_MOD = 'FIJO'; % EN ESTE MODO NO FUNCIONA NI CON ST2_CONTINUE NI SIN EL
inspec.ACQ_MOD_COD = 0;

% tipo de captura: 0: MC devuelve, por cada disparo (ley focal) los datos sin procesar del todos los CANALES (no elementos)
%                  1: PA phased array devuelve la imagen CONFORMADA con el algoritmo PHASED ARRAY
%                  2: TFM devuelve la  imagen conformada con el algoritmo TFM
inspec.ACQ_TYPE = 'PA';
inspec.ACQ_TYPE_COD = 1;

% 0-LINEAL 1-SECTORIAL
inspec.TIPO_PA = 'LINEAL';
inspec.TIPO_PA_COD = 0;
% inspec.TIPO_PA = 'SECTORIAL';
% inspec.TIPO_PA_COD = 1;

% if (inspec.TIPO_PA_COD == 0)
%     inspec.prf = 3000;  %PRF Hz 
% else    
%     inspec.prf = 1000;  %PRF Hz 
% end

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

ST2_SetGain(20);
% ST2_SetFocalLawType_AI(2,'miley.txt');


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

% Se programa el filtro y se comprueba que se ha programado bien
% ------------------------------------------------------------------
ST2_SetHWRFFilter(inspec.fmin, inspec.fmax);
if (inspec.fmin ~=  ST2_GetHWRFFilterLow())
    disp('Error en la programación de la frecuencia mínima del filtro digital');
end
if (inspec.fmax ~=  ST2_GetHWRFFilterHigh())
    disp('Error en la programación de la frecuencia máxima del filtro digital');
end
ST2_SetHWRFFilterEnable(inspec.filtro);

if (inspec.filtro ~=  ST2_GetHWRFFilterEnable())
    disp('Error en la programación del filtro digital');
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

%%   PHASED ARRAY

% inspec.foco_emi = 1000;
inspec.foco_emi = 10;
error=ST2_PA_SetEmissionFocalPoint(inspec.foco_emi);
if error
    disp ('Error en ST2_PA_SetEmissionFocalPoint');
end
if (inspec.foco_emi ~= ST2_PA_GetEmissionFocalPoint())
    disp('Error en la programación del foco en emisión');
end

if inspec.TIPO_PA_COD == 0
%     e_ini=32;
%     e_fin=64;
    e_ini=0;
    e_fin=127;
    sub_ap=8;
    sub_ap=16;
%     sub_ap=32;
%     paso=8;
    paso=1;
    angulo=0;
    error=ST2_PA_SetLinearScanConfig(paso, sub_ap, e_ini, e_fin, angulo);
    if error
        disp ('Error en ST2_PA_SetLinearScanConfig');
    end
    titulo = 'Imagen phased array lineal';
    xl = 'lines';
    yl = 'samples';
else
    %sub_ap=8;
    sub_ap=128;
    e_ini=0;
    paso=1;
    s_ang_i=-25;
    s_ang_f=25;
%     s_ang_i=0;
%     s_ang_f=0;
    error=ST2_PA_SetAngularScanConfig (paso, sub_ap, e_ini, s_ang_i, s_ang_f);
    if error
        disp ('Error en ST_SetAngularScanConfig');
    end
    titulo = 'Imagen phased array sectorial';
    xl = 'mm';
    yl = 'mm';
    [r,a]=meshgrid(linspace(inspec.dist_ini, inspec.dist_fin, ST2_GetAScanDataNumber()),(s_ang_i:paso:s_ang_f)*pi/180-pi/2);
    [x,y]=pol2cart(a,r);
end

if (inspec.TIPO_PA_COD ~= ST2_PA_GetScanType())
    disp('Error en la programación del tipo de adquisición');
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
inspec.n_images = ST2_GetAcquisitionsNumber();
disp(' ');
disp(['Numero de rafagas ' num2str(inspec.n_rafagas)])
disp(['Numero de imagenes por rafaga ' num2str(inspec.imagenes_por_rafaga)])
disp(['Numero de imagenes total ' num2str(inspec.n_images)])
if (inspec.n_images ~= (inspec.n_rafagas*inspec.imagenes_por_rafaga))
    disp(['Error en el número de imágenes total ' num2str(inspec.n_images)])
end
disp(' ');


%%
% ALB. para ver si los cambios se reflejan en el archivo de
% configuracion (XML)

result = ST2_SaveXMLFileConfig('.\VERMONT18_PRU_SCANVIEW_UT.XML');
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
n_samples = ST2_GetAScanDataNumber();   %  N_muestras
n_focal_laws = ST2_GetFocalLawNumber();    %  N_disparos
image_size = n_samples * n_focal_laws;
n_ch = ST2_GetChannelsNumber();            %  N_receptores
inspec.n_images = ST2_GetAcquisitionsNumber();
data=zeros(image_size,1, 'int16');           
ptr_data = libpointer('int16Ptr', data);

img=zeros(n_samples, n_focal_laws, 'int16');
imagenes=zeros(n_samples, n_focal_laws, inspec.n_imagenes, 'int16');
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

% BUCLE DE ADQUISICION DE UNA IMAGEN
% ---------------------------------- 
flag_end = false;
fallos=0;   % para salir con orden si el equipo se atasca (pero poco)
veces = 0;
error = 0;
while flag_end == false
    index_ima = 0;
    index_aux = 0;
    while ((veces < inspec.n_imagenes) && (~flag_end))
%int _API_PCIe_DLL ST2_PA_GetBuffer_Image(short int* data, unsigned long size_data, unsigned long* n_data, unsigned long index_trg, unsigned long index_aux);
        [result data n_data] = ST2_PA_GetBuffer_Image(ptr_data, image_size, index_ima, index_aux);
%        [result data n_data index_ima index_aux] = ST2_PA_GetBuffer_LastImage(ptr_data, image_size);
%         [result data index_ima] = ST2_MC_GetBuffer_LastMatrixCapture(ptr_data, image_size);
        %         result-image_size
        if (result > 0)
            
            if (max(data) == 0)
                error = error + 1
                n_data
            end
%             index_ima
            img = reshape(data, n_samples, n_focal_laws);
            imagenes(:,:,index_ima+1)=img(:,:);
            indices(index_ima+1) = index_ima;
            orden = [orden index_ima];
            if dibujar
                figure(h_figure)
                if (inspec.TIPO_PA_COD == 0)
                    imagesc(abs(img)); % para ver mejor los artefactos
                    %            imagesc(abs(hilbert(double(img))));
                else
                    pcolor(x,y,abs(hilbert(double(img'))))
                    shading interp
                end
                %         caxis(range_lin);
                colorbar;
                title(titulo);
                xlabel(xl);
                ylabel(yl);
                pause(.1)
                drawnow;
            end
            index_ima = index_ima+1;
            veces = veces+1
        end
        flag_end = flag_end || ~ishghandle(h_figure);
    end
    
    %imagesc(squeeze(imagenes(:,:,18)))
    
    if motores
        nomFicheroAux = [nomFichero num2str(indiceFich) '.mat'];
        disp(['Voy a grabar el fichero ' nomFicheroAux])
        if  (inspec.n_imagenes == 1)
            imagenes=img;
        end
        save(nomFicheroAux, 'inspec', 'arr', 'imagenes', 'indices', '-mat');
        disp('Grabé el fichero')
        disp(' ');
        if (indiceFich < nMovimientos)
            % mover el motor (movimiento relativo)
            EnviarPosicionX(NBSDevice, pasoMm);
            % Espero a que se pare el motor
            while EstaEnMovimiento(NBSDevice)
                pause(.1)
            end
            indiceFich = indiceFich + 1;
            flag_end = flag_end || ~ishghandle(h_figure);
            veces = 0;
        else
            flag_end = true;
        end
    else
        flag_end = flag_end || ~ishghandle(h_figure);
        flag_end = flag_end || (veces == inspec.n_imagenes);
    end   
end

if ~motores
    disp('Voy a grabar el fichero')
    if  (inspec.n_imagenes == 1)
        imagenes=img;
    end
    save(nomFichero, 'inspec', 'arr', 'imagenes', 'indices', '-mat');
else
    disp('Voy a detener el equipo')
end

% FIN DE ADQUISICIÓN
% ------------------
ST2_Stop();
ST2_SetPulserEnable(false);

% CIERRE DE LA CONEXIÓN
% ---------------------
CloseSys();
disp('Cerrada la conexión con el Sitau')

% DESCONEXION DE LOS MOTORES
if motores
    % Se envía el motor a la posición inicial
    EnviarPosicionX(NBSDevice, -recorrido);
    % Espero a que se pare el motor
    while EstaEnMovimiento(NBSDevice)
        pause(.1)
    end
    % Se desconectan los motores
    DesconectarMotor(NBSDevice);
    disp('Motores desconectados')
end


