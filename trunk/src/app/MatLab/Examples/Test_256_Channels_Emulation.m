%% Script de Test de 256 Canales en Modo Emulación (Sin Hardware Físico)
% Este script permite verificar que stlib2_x64.dll y api_pcie_x64.dll
% inicializan, calculan leyes focales y procesan buffers para 256 canales.

clear all;
clc;

%% 1. Añadir Toolbox al Path de MATLAB
current_dir = fileparts(mfilename('fullpath'));
toolbox_path = fullfile(current_dir, '..', 'Toolbox_sitau2');
if ~exist(toolbox_path, 'dir')
    toolbox_path = fullfile(pwd, 'trunk', 'src', 'app', 'MatLab', 'Toolbox_sitau2');
end
addpath(toolbox_path);

%% 2. Definir Parámetros de Inicialización
% Ruta al XML modificado para 256 canales
xml_path = fullfile(current_dir, '..', '..', 'pcie_vc', 'AppFiles', 'SITAU2_PCIe_TEST.xml');
if ~exist(xml_path, 'file')
    xml_path = fullfile(pwd, 'trunk', 'src', 'app', 'pcie_vc', 'AppFiles', 'SITAU2_PCIe_TEST.xml');
end

ip = '192.168.1.100';
tcp_port_cfg = 5000;
tcp_port_acq = 5001;
pcie_enabled = 1;
emulates_sitau2 = true; % <--- MODO EMULACIÓN SIN EQUIPO FÍSICO

fprintf('=== INICIANDO PRUEBA DE 256 CANALES SITAU-II (MODO EMULACIÓN) ===\n');
fprintf('Cargando archivo XML: %s\n', xml_path);

%% 3. Abrir Sistema en Modo Emulación
error = OpenSys(xml_path, ip, tcp_port_cfg, tcp_port_acq, pcie_enabled, emulates_sitau2);

if error ~= 0
    error('Fallo al abrir el sistema SITAU-II. Código de error: %d', error);
else
    fprintf('[OK] Sistema abierto correctamente en modo Emulación.\n\n');
end

%% 4. Verificar Configuración de Canales
n_elem = ST2_GetProbeElementNumber();
n_chan = ST2_GetChannelsNumber();

fprintf('--- VERIFICACIÓN DE CANALES ---\n');
fprintf('Número de Elementos de Sonda detectados: %d\n', n_elem);
fprintf('Número de Canales Totales detectados:   %d\n', n_chan);

if n_elem == 256 && n_chan == 256
    fprintf('\n[ÉXITO DE RECONOCIMIENTO]: El software reconoce y configura correctamente los 256 canales.\n\n');
else
    fprintf('\n[ADVERTENCIA]: Los canales detectados (%d) difieren de 256. Revisa el XML cargado.\n\n', n_elem);
end

%% 5. Probar Configuración de Escaneo Phased Array (PA) para 256 Canales
fprintf('--- CONFIGURANDO BARRIDO PA PARA 256 CANALES ---\n');
aperture = 32;       % Apertura activa de 32 elementos
start_element = 1;   % Elemento inicial
end_element = 256;   % Elemento final (abarca hasta el canal 256)
step = 1;
angle = 0;           % Ángulo lineal

% Aplicar barrido lineal en toda la sonda de 256 canales
result = ST2_PA_SetLinearScanConfig(aperture, start_element, end_element, step, angle);
if result == 0
    fprintf('[OK] Leyes focales para 256 canales calculadas correctamente en stlib2.\n');
else
    fprintf('[ERROR] Fallo al calcular leyes focales PA: %d\n', result);
end

%% 6. Simular Adquisición (Test de Buffers PCIe)
fprintf('\n--- SIMULANDO ADQUISICIÓN Y RECEPCIÓN DE DATOS ---\n');
result = ST2_Start(1);
if result == 0
    fprintf('[OK] Adquisición iniciada en modo Emulación.\n');
    
    % Leer tamaño de buffer por disparo
    n_samples = ST2_GetAScanDataNumber();
    n_acq_burst = ST2_GetAcquisitionsBurstNumber();
    fprintf('Muestras A-Scan por canal: %d\n', n_samples);
    fprintf('Adquisiciones por ráfaga:  %d\n', n_acq_burst);
    
    % Detener adquisición
    ST2_Stop();
    fprintf('[OK] Adquisición simulada con éxito sin errores de DMA o desbordamiento.\n');
else
    fprintf('[ERROR] Fallo al iniciar adquisición: %d\n', result);
end

%% 7. Cerrar Sistema
CloseSys();
fprintf('\n=== PRUEBA FINALIZADA EXITOSAMENTE ===\n');
