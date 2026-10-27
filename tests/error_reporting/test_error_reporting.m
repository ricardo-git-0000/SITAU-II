function test_error_reporting() % Verifica la presentacion de errores sin compilar ni cargar DLL.
test_path = fileparts(mfilename('fullpath')); % Localiza los mocks exclusivos de esta prueba.
repo_path = fileparts(fileparts(test_path)); % Localiza la raiz del proyecto.
previous_path = path; % Conserva las rutas de la sesion.
path_cleanup = onCleanup(@() path(previous_path)); % Restaura las rutas al terminar incluso si falla una asercion.
addpath(fullfile(repo_path, 'trunk', 'src', 'app', 'MatLab', 'Toolbox_sitau2')); % Carga el formateador real.
addpath(fullfile(test_path, 'mocks'), '-begin'); % Sustituye solo las llamadas externas de la consulta.
global error_test_state; % Comparte las respuestas con los sustitutos de la API.
error_test_state = struct('loaded', true, 'fail', false, 'found', 1, 'description', '', 'last_code', 0); % Inicializa una consulta reconocida.
error_codes = [-1906, -3005, -1523, -167]; % Cubre ejemplos de sistema, hardware, mapa y PCIe.
for error_code = error_codes % Comprueba que el mensaje conserva cada codigo exacto.
    error_test_state.description = [sprintf('(Error Code %d) ''Descripcion de prueba''', error_code), char(0), 'relleno']; % Incluye terminador y datos que no deben mostrarse.
    actual_message = ST2_ErrorText(error_code, 'api_pcie_x64'); % Ejecuta el codigo de produccion con una respuesta controlada.
    assert(strcmp(actual_message, sprintf('%d: Descripcion de prueba', error_code))); % Verifica formato, comillas y recorte del buffer.
    assert(error_test_state.last_code == error_code); % Verifica que no se consulta otro error anterior.
end % Finaliza los casos de codigos reconocidos.
error_test_state.description = 'Descripcion sin prefijo'; % Simula un formato alternativo sin numero duplicado.
assert(strcmp(ST2_ErrorText(-1906), '-1906: Descripcion sin prefijo')); % Verifica la arquitectura predeterminada y el texto simple.
error_test_state.found = 0; % Simula un codigo ausente del catalogo.
assert(strcmp(ST2_ErrorText(-99999), '-99999: codigo de error no documentado')); % No atribuye una descripcion ajena al codigo desconocido.
error_test_state.loaded = false; % Simula una DLL que no esta cargada.
assert(contains(ST2_ErrorText(-1523), '-1523: descripcion no disponible; biblioteca no cargada')); % Conserva el codigo aun sin biblioteca.
error_test_state.loaded = true; % Restablece la disponibilidad de la biblioteca simulada.
error_test_state.fail = true; % Simula un fallo al llamar a ST2_GetError.
assert(contains(ST2_ErrorText(-1523), '-1523: descripcion no disponible; fallo al consultar ST2_GetError')); % El fallo secundario no oculta el original.
clear global error_test_state; % Elimina el estado temporal del test.
disp('PASS: 8 escenarios de mensajes MATLAB, sin DLL ni hardware'); % Resume las verificaciones de presentacion.
end % Finaliza la regresion.
