function test_error_reporting_native() % Ejecutar en un proceso MATLAB separado: usa las DLL reales en modo sin hardware.
repo = fileparts(fileparts(fileparts(mfilename('fullpath')))); % Localiza el repositorio desde el test.
toolbox = fullfile(repo, 'trunk', 'src', 'app', 'MatLab', 'Toolbox_sitau2'); % Utiliza los wrappers de produccion.
examples = fullfile(repo, 'trunk', 'src', 'app', 'MatLab', 'Examples'); % Usa las mismas DLL que las demos.
previous_path = path; previous_folder = pwd; restore_path = onCleanup(@() path(previous_path)); restore_folder = onCleanup(@() cd(previous_folder)); % Restaura el entorno del llamante.
assert(~libisloaded('api_pcie_x64'), 'Ejecuta este test en una sesion nueva.'); % No altera una conexion existente.
addpath(toolbox); cd(examples); % Permite resolver stlib2 junto a api_pcie.
loadlibrary(fullfile(examples, 'api_pcie_x64.dll'), fullfile(toolbox, 'api_pcie.h')); % Carga la interfaz real, sin abrir el equipo.
fprintf('PASO 1: DLL cargada; consultando descripciones reales.\n'); % Deja un marcador antes de cruzar la interfaz C.
codes = [-1912 -703 -172 -1523 -101 -99999]; % Cubre archivo, memoria, geometria, mapa, codigo compartido y desconocido.
for index = 1:100 % Repite para detectar corrupcion al liberar buffers temporales.
    for code = codes % Consulta cada familia por la interfaz real.
        message = ST2_ErrorText(code); % Ejercita calllib con el buffer corregido.
        assert(startsWith(message, sprintf('%d:', code)) && ~contains(message, 'descripcion no disponible')); % No acepta que un fallback oculte un fallo de llamada.
    end % Fin de codigos.
end % Fin de repeticion.
fprintf('PASO 2: 600 consultas completadas; abriendo simulacion GE.\n'); % Separa traduccion de inicializacion.
result = OpenSys([fullfile(toolbox, 'BIN_256') filesep], '192.168.2.10', 55902, 50460, 0, true); % Desactiva hardware y PCIe.
assert(result >= 0, 'No se pudo abrir la simulacion: %d.', result); % No interpreta un fallo de apertura como resultado de la prueba.
missing_file = [tempname '.xml']; % No crea ni altera un XML existente.
result = calllib('api_pcie_x64', 'ST2_LoadXMLFileConfig', missing_file); % Provoca el error real sin abrir un cuadro de dialogo.
assert(result == -1912, 'Se esperaba -1912, se recibio %d.', result); % Verifica la ruta de archivo inexistente.
fprintf(2, 'Error provocado: %s\n', ST2_ErrorText(result)); % Imprime codigo y descripcion mientras la DLL sigue cargada.
fprintf('PASO 3: error tratado; cerrando y descargando DLL.\n'); % Identifica por separado un posible fallo de cierre.
CloseSys(); % Ejecuta el mismo cierre de las demos.
assert(~libisloaded('api_pcie_x64')); % Comprueba que la descarga termino.
disp('PASS: consultas nativas, XML inexistente y cierre completados; MATLAB sigue vivo.'); % Solo declara exito tras devolver el control.
end % Fin de regresion nativa sin adquisicion.
