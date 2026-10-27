function loaded = libisloaded(~) % Simula la disponibilidad de la DLL sin cargar hardware.
global error_test_state; % Comparte el escenario elegido por la prueba.
loaded = error_test_state.loaded; % Devuelve el estado controlado por el test.
end % Finaliza el sustituto de libisloaded.
