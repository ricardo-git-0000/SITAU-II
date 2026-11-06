function Validate_LoadedMCConfig(expected, actual) % Comprueba que la DLL aplico las dimensiones validadas antes de adquirir.
if nargin < 2 % En las demos lee solo getters de la configuracion cargada.
    actual.channels = ST2_GetChannelsNumber(); % Canales del equipo abierto.
    actual.probe_elements = ST2_GetProbeElementNumber(); % Elementos efectivos tras la carga del XML.
    actual.focal_laws_per_image = ST2_GetFocalLawNumber(); % Leyes reales, sin multiplicar por el transporte PCIe.
    actual.images_per_burst = ST2_GetAcquisitionsBurstNumber(); % Imagenes de cada rafaga aplicada.
    actual.configured_bursts = ST2_GetBurstsNumber(); % Contador configurado tambien en modo continuo.
    actual.acquisition_mode = ST2_GetAcquisitionMode(); % Modo finito o continuo aplicado.
end % Las pruebas pueden proporcionar una instantanea sin acceder al hardware.
fields = {'channels', 'focal_laws_per_image', 'images_per_burst', 'configured_bursts', 'acquisition_mode'}; % Dimensiones cuyo significado comparten ambas capas.
for index = 1:numel(fields) % Compara cada campo con el valor interpretado del XML.
    name = fields{index}; % Selecciona el campo actual.
    if ~isscalar(actual.(name)) || actual.(name) ~= expected.(name) % Tambien detecta codigos negativos de getters.
        error('SITAU:LoadedConfigMismatch', '%s: XML=%g, DLL=%g. Comprueba el XML y las DLL cargadas.', name, expected.(name), actual.(name)); % Identifica la discrepancia antes de ST2_Start.
    end % Fin de comparacion del campo.
end % Fin de comparacion de dimensiones.
if ~isscalar(actual.probe_elements) || actual.probe_elements ~= expected.channels % Sonda completa derivada de BASES_NUMBER.
    error('SITAU:LoadedConfigMismatch', 'Elementos de sonda: esperados %g, DLL=%g.', expected.channels, actual.probe_elements); % No permite reaparecer el antiguo 128 explicito en equipo de 256.
end % Fin de comparacion de sonda.
end % Fin de comprobacion de la configuracion aplicada.
