function value = getXLMparam(xml, parname, optionalParameter) % Lee valores numericos sin ejecutar expresiones del XML.
if nargin < 3, optionalParameter = false; end % Los parametros son obligatorios salvo indicacion expresa.
parameters = xml.getElementsByTagName('Parameter'); % El llamante proporciona la seccion que lee tambien la DLL.
matches = {}; % Detecta duplicados en vez de elegir silenciosamente el primero.
for index = 0:parameters.getLength-1 % Recorre los nodos DOM de la seccion.
    parameter = parameters.item(index); % Obtiene el parametro actual.
    if strcmp(char(parameter.getAttribute('pName')), char(parname)) % Compara nombres completos y sensibles a mayusculas.
        if ~parameter.getParentNode().isSameNode(xml), error('SITAU:XMLSchema', '%s debe ser hijo directo de su seccion.', char(parname)); end % La DLL no busca parametros en subsecciones.
        matches{end+1} = char(parameter.getAttribute('pValue')); % Conserva el texto antes de validar numeros.
    end % Fin de coincidencia de nombre.
end % Fin de recorrido DOM.
if isempty(matches) % Distingue ausencia de valor invalido.
    if optionalParameter, value = []; return; end % La ausencia opcional se resuelve en el validador.
    error('SITAU:XMLMissing', 'Falta el parametro %s.', char(parname)); % Identifica el dato requerido.
end % Fin de comprobacion de presencia.
if numel(matches) ~= 1, error('SITAU:XMLDuplicate', 'Parametro duplicado: %s.', char(parname)); end % Evita configuraciones ambiguas.
raw = strtrim(matches{1}); % Admite espacios exteriores.
if strcmp(char(parname), 'mc_pwic_angles') % El parser C++ exige corchetes y comas para los vectores de angulos.
    if ~(startsWith(raw, '[') && endsWith(raw, ']')), error('SITAU:XMLValue', 'Usa [angulo1,angulo2,...] en %s.', char(parname)); end % No acepta una lista que la DLL ignoraria.
    raw = strtrim(raw(2:end-1)); % Extrae el contenido del vector canonico.
end % Los demas parametros leidos por este validador son escalares.
tokens = strtrim(strsplit(raw, ',', 'CollapseDelimiters', false)); % Conserva elementos vacios para rechazarlos, sin ejecutar expresiones.
numberPattern = '^[+-]?(\d+(\.\d*)?|\.\d+)$'; % El lector C++ heredado no conserva exponentes: solo decimales literales.
integerNames = {'probe_elements_number', 'data_link_mode', 'acquisition_mode', 'burst_number', 'images_per_burst_number', 'sampling_freq_decimation', 'acquisition_type', 'acq_time_mode', 'acq_start_samples', 'acq_n_samples', 'mc_processing_type', 'mc_average_signals_number', 'mc_focal_law_type'}; % Parametros que el parser de la DLL lee como enteros.
if ismember(char(parname), integerNames), numberPattern = '^[+-]?\d+$'; end % Evita que C++ interprete 150.0 como 1500 al eliminar el punto.
if isempty(raw) || any(cellfun(@(token) isempty(regexp(token, numberPattern, 'once')), tokens)) % Rechaza expresiones, vacios y texto.
    error('SITAU:XMLValue', 'Valor numerico invalido en %s.', char(parname)); % No confunde un valor malo con uno omitido.
end % Fin de validacion del texto.
value = cellfun(@str2double, tokens); % Convierte solamente los literales ya verificados.
if any(~isfinite(value)), error('SITAU:XMLValue', 'Valor no finito en %s.', char(parname)); end % Rechaza desbordamientos numericos.
end % Fin de lectura numerica del XML.
