function [found, description] = calllib(~, function_name, error_code, ~) % Simula exclusivamente la consulta del catalogo.
global error_test_state; % Lee el escenario configurado por el test.
assert(strcmp(function_name, 'ST2_GetError')); % Impide realizar otra operacion de la API durante estas pruebas.
error_test_state.last_code = double(error_code); % Registra el codigo realmente consultado.
if error_test_state.fail % Simula una DLL incompatible o un fallo de consulta.
    error('SITAU:TestLookup', 'Lookup failed'); % Comprueba que se conserva el error original.
end % Finaliza la simulacion del fallo.
found = error_test_state.found; % Indica si el catalogo conoce el codigo.
description = error_test_state.description; % Entrega el texto sin consultar ninguna DLL.
end % Finaliza el sustituto de calllib.
