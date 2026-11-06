function [valid, withWarnings, performance] = Validate_XMLConfig(xmlFileWithPath, system_channels, expected_link) % Valida el formato MC comun a GE y PCIe.
valid = false; withWarnings = false; performance = struct(); % Solo publica exito al completar todas las comprobaciones.
warningMessages = strings(0); % Acumula avisos de rendimiento.
if nargin < 1 || isempty(xmlFileWithPath), error('SITAU:XMLMissing', 'Indica el XML de adquisicion.'); end % No confunde el XML de adquisicion con SITAU.XML.
if nargin < 2, system_channels = []; end % Permite validacion offline cuando el XML declara los elementos.
if nargin < 3, expected_link = []; end % Las demos indican el mismo transporte utilizado en OpenSys.
ERR_FORMAT = "ERROR :: Invalid XML format :: "; % Conserva el contexto de errores heredado.
ERR_VALUES = "ERROR :: Invalid XML values :: "; % Conserva el contexto de errores de rango.
WRN_GENERAL = "WARNING :: "; % Conserva el prefijo de avisos de rendimiento.
OPTIONAL = true; % Distingue parametros opcionales de los obligatorios.
PCIE_BANDWIDTH_HARD_LIMIT = 3.2; PCIE_BANDWIDTH_WARNING_LIMIT = 2.5; % Conserva los limites de estimacion PCIe existentes.
GBE_BANDWIDTH_HARD_LIMIT = 0.07; GBE_BANDWIDTH_WARNING_LIMIT = 0.05; % Conserva los limites de estimacion GE existentes.
ADC_BASE_FREQ = 62.5; LINE_ACQ_OVH = 1; % Frecuencia MHz y margen temporal us del modelo existente.
doc = xmlread(xmlFileWithPath); % Lee configuracion sin cargar DLL ni acceder al equipo.
root = doc.getDocumentElement(); % Comprueba la raiz usada por el parser C++.
if ~strcmp(char(root.getNodeName()), 'STLIB2'), error('SITAU:XMLSchema', 'Se esperaba STLIB2.'); end % Rechaza documentos de otro tipo.
acquisition = section(root, 'ACQUISITION'); % Seccion global leida por GetData_ACQUISITION.
vch = section(root, 'VirtualChannel'); % Estas demos trabajan con un unico canal virtual.
ut = section(vch, 'UT_PARAMETERS'); probe = section(vch, 'PROBE'); % Limita cada busqueda a su seccion.
if vch.getElementsByTagName('FP_FOCAL_LAW').getLength > 0 % Detecta el formato antiguo que la DLL actual ignora.
    error('SITAU:XMLLegacy', 'Sustituye FP_FOCAL_LAW/fp_* por MC_FOCAL_LAW/mc_* y mueve las rafagas a ACQUISITION.'); % No acepta un XML distinto del que lee la DLL.
end % Fin de rechazo de seccion antigua.
laws = section(vch, 'MC_FOCAL_LAW'); dsp = section(vch, 'MC_DSP'); % Comparte los nombres canonicos del parser.
parameters = root.getElementsByTagName('Parameter'); % Revisa tambien parametros antiguos dentro de secciones nuevas.
for index = 0:parameters.getLength-1 % Detecta nombres heredados que podrian pasar inadvertidos.
    name = char(parameters.item(index).getAttribute('pName')); % Lee el identificador del parametro.
    if startsWith(name, 'fp_') || ismember(name, {'mc_burst_number', 'mc_burst_images_number'}) % No permite mezclar dos esquemas.
        error('SITAU:XMLLegacy', 'Parametro antiguo %s: usa MC y burst_number/images_per_burst_number en ACQUISITION.', name); % Explica como migrarlo.
    end % Fin de comprobacion del nombre.
    parent_name = char(parameters.item(index).getParentNode().getNodeName()); % Contrasta la ubicacion con el cargador C++.
    if ismember(name, {'burst_number', 'images_per_burst_number'}) && ~strcmp(parent_name, 'ACQUISITION'), error('SITAU:XMLSchema', '%s debe estar en ACQUISITION.', name); end % No sustituye silenciosamente una rafaga mal ubicada por el valor uno.
    if startsWith(name, 'mc_') && ~strcmp(parent_name, 'MC_FOCAL_LAW') && ~strcmp(parent_name, 'MC_DSP'), error('SITAU:XMLSchema', '%s debe estar en su seccion MC.', name); end % Impide parametros MC activos fuera de las secciones leidas.
    if ismember(name, {'mc_focal_law_type', 'mc_pwic_angles', 'mc_pwic_start_angle', 'mc_pwic_end_angle', 'mc_pwic_step_angle', 'mc_focal_law_file'}) && ~strcmp(parent_name, 'MC_FOCAL_LAW'), error('SITAU:XMLSchema', '%s debe estar en MC_FOCAL_LAW.', name); end % No ignora una lista de angulos ubicada en DSP.
    if ismember(name, {'mc_processing_type', 'mc_average_signals_number'}) && ~strcmp(parent_name, 'MC_DSP'), error('SITAU:XMLSchema', '%s debe estar en MC_DSP.', name); end % No desactiva el promedio por una ubicacion incorrecta.
    if strcmp(name, 'probe_elements_number') && ~strcmp(parent_name, 'PROBE'), error('SITAU:XMLSchema', 'probe_elements_number debe estar en PROBE.'); end % No trata un valor mal ubicado como omitido.
end % Fin de revision del esquema.
probe_elements_number = getXLMparam(probe, 'probe_elements_number', OPTIONAL); % La omision es valida igual que en stlib2.
if isempty(system_channels) && isempty(probe_elements_number) % Resuelve la geometria solo si resulta necesaria.
    if ~(libisloaded('api_pcie_x64') || libisloaded('api_pcie_x86')) % No invoca getters sin una biblioteca cargada.
        error('SITAU:XMLChannels', 'Sin probe_elements_number, pasa los canales del sistema abierto (BASES_NUMBER * 32) como segundo argumento.'); % Permite pruebas offline explicitas.
    end % Fin de comprobacion de la biblioteca.
    system_channels = ST2_GetChannelsNumber(); % Obtiene la geometria que ya cargo OpenSys desde SITAU.XML.
end % Fin de resolucion automatica.
if ~isempty(system_channels), positiveInteger(system_channels, 'canales del sistema'); end % No acepta errores negativos del getter.
if isempty(probe_elements_number), probe_elements_number = system_channels; end % Misma regla de sonda completa que stlib2.
positiveInteger(probe_elements_number, 'probe_elements_number'); % Comprueba el valor efectivo, sea explicito o calculado.
if ~ismember(probe_elements_number, [128 256]), error('SITAU:XMLChannels', 'Estas demos MC requieren 128 o 256 elementos.'); end % Acota el alcance a los dos equipos soportados.
if ~isempty(system_channels) && probe_elements_number ~= system_channels % Detecta el antiguo XML de 128 en equipo de 256.
    error('SITAU:XMLChannels', 'probe_elements_number=%d no coincide con los %d canales. Omite el parametro para usar BASES_NUMBER.', probe_elements_number, system_channels); % Evita llegar al error de mapa posterior.
end % Fin de coherencia con el sistema abierto.
data_link_mode = scalarParam(acquisition, 'data_link_mode'); % Se usa para estimar el enlace; OpenSys selecciona el transporte real.
if ~ismember(data_link_mode, [0 1]), error('SITAU:XMLValue', 'data_link_mode debe ser 0 o 1.'); end % Rechaza transportes desconocidos.
if ~isempty(expected_link) && (~isscalar(expected_link) || data_link_mode ~= expected_link), error('SITAU:XMLLink', 'data_link_mode no coincide con OpenSys.'); end % No calcula GE para una conexion PCIe o viceversa.
acquisition_mode = scalarParam(acquisition, 'acquisition_mode'); % Mantiene 0 finito y 1 continuo.
if ~ismember(acquisition_mode, [0 1]), error('SITAU:XMLValue', 'acquisition_mode debe ser 0 o 1.'); end % Evita inferir modos desconocidos.
burst_number = scalarParam(acquisition, 'burst_number', 1); % Mismo valor predeterminado que CXML_CONFIG::ResetData.
images_per_burst = scalarParam(acquisition, 'images_per_burst_number', 1); % Nunca toma este dato de MC_FOCAL_LAW.
positiveInteger(burst_number, 'burst_number'); positiveInteger(images_per_burst, 'images_per_burst_number'); % Evita tamanos cero o fraccionarios.
total_bursts = burst_number; if acquisition_mode == 1, total_bursts = inf; end % El modo continuo mantiene la longitud de cada rafaga.
prf_frequency = scalarParam(ut, 'prf_frequency'); % Frecuencia de disparos configurada.
if prf_frequency <= 0, error('SITAU:XMLValue', 'prf_frequency debe ser positiva.'); end % Evita divisiones por cero.
sampling_freq_decimation = scalarParam(ut, 'sampling_freq_decimation', 1); % Mantiene el parametro del modelo de rendimiento.
if sampling_freq_decimation ~= 1, error('SITAU:XMLUnsupported', 'El validador MC actual solo certifica sampling_freq_decimation=1.'); end % El getter actual de stlib2 devuelve factor uno.
if scalarParam(ut, 'acquisition_type', 0) ~= 0, error('SITAU:XMLUnsupported', 'Este validador es para adquisicion MC.'); end % No presenta PA/TFM como certificados.
if scalarParam(ut, 'acq_time_mode', 0) ~= 0, error('SITAU:XMLUnsupported', 'La estimacion por distancia requiere acq_time_mode=0.'); end % Mantiene explicito el supuesto de pulso eco.
acq_start_samples = getXLMparam(ut, 'acq_start_samples', OPTIONAL); acq_n_samples = getXLMparam(ut, 'acq_n_samples', OPTIONAL); % Lee la alternativa por muestras.
acq_start_distance = getXLMparam(ut, 'acq_start_distance', OPTIONAL); acq_end_distance = getXLMparam(ut, 'acq_end_distance', OPTIONAL); % Lee la alternativa por distancias.
if ~isempty(acq_end_distance) % La DLL da prioridad a acq_end_distance cuando aparece.
    if isempty(acq_start_distance), acq_start_distance = 0; end % Igual que el valor predeterminado del parser.
    material_velocity = scalarParam(ut, 'material_velocity') / 1000; % Convierte m/s a mm/us.
    if ~isscalar(acq_start_distance) || ~isscalar(acq_end_distance) || acq_start_distance < 0 || acq_end_distance <= acq_start_distance || material_velocity <= 0 % Valida antes de calcular tiempos.
        error('SITAU:XMLValue', 'Distancias o velocidad de adquisicion invalidas.'); % Identifica la configuracion incorrecta.
    end % Fin de comprobacion de distancias.
    acq_start_samples = ceil(double(single(2 * acq_start_distance / material_velocity * ADC_BASE_FREQ)) - 0.5); % Reproduce Round32 con empate hacia abajo para valores positivos.
    acq_n_samples = ceil(double(single(2 * (acq_end_distance - acq_start_distance) / material_velocity * ADC_BASE_FREQ)) - 0.5); % Estima la ventana de muestras que configura stlib2.
else % Usa muestras explicitas si no hay distancia final, como el cargador.
    if isempty(acq_start_samples), acq_start_samples = 0; end % Inicio predeterminado del parser.
    if isempty(acq_n_samples), error('SITAU:XMLMissing', 'Falta acq_n_samples o acq_end_distance.'); end % No utiliza una ventana implicita inesperada.
end % Fin de seleccion de la ventana.
positiveInteger(acq_n_samples, 'acq_n_samples'); % Exige al menos una muestra antes del ajuste par.
if ~isscalar(acq_start_samples) || acq_start_samples < 0 || fix(acq_start_samples) ~= acq_start_samples, error('SITAU:XMLValue', 'acq_start_samples debe ser entero no negativo.'); end % Valida el desplazamiento.
acq_n_samples = 2 * ceil(acq_n_samples / 2); % La DLL ajusta a un numero par de muestras.
processing_type = scalarParam(dsp, 'mc_processing_type', 0); % Conserva el significado MC del procesamiento.
if ~ismember(processing_type, [0 1]), error('SITAU:XMLUnsupported', 'mc_processing_type no soportado por el validador.'); end % No supone un promedio para otro procesamiento.
average_signals_number = 1; % Sin promedio se realiza un disparo por ley.
if processing_type == 1 % Solo consulta el exponente si el promedio esta activo.
    average_exponent = scalarParam(dsp, 'mc_average_signals_number'); % Exponente log2 usado por la configuracion.
    if average_exponent < 0 || average_exponent > 15 || fix(average_exponent) ~= average_exponent, error('SITAU:XMLValue', 'Exponente de promedio invalido.'); end % Acota el calculo.
    average_signals_number = 2 ^ average_exponent; % Calcula el coste temporal del promedio.
end % Fin del promedio.
mc_focal_law_type = scalarParam(laws, 'mc_focal_law_type'); % No permite caer silenciosamente al FMC predeterminado.
if mc_focal_law_type == 0 % FMC de sonda completa.
    number_of_focal_laws = probe_elements_number; % Un emisor por ley para el alcance de estas demos.
elseif mc_focal_law_type == 1 % PWI con lista explicita o rango.
    angles = getXLMparam(laws, 'mc_pwic_angles', OPTIONAL); % Usa el nombre correcto, sin el antiguo error pwix.
    if isempty(angles) % El rango solo se usa cuando no hay lista, como en la DLL.
        start_angle = scalarParam(laws, 'mc_pwic_start_angle'); end_angle = scalarParam(laws, 'mc_pwic_end_angle'); step_angle = scalarParam(laws, 'mc_pwic_step_angle'); % Exige un rango completo.
        if step_angle == 0 && start_angle == end_angle % Una sola ley PWI explicita.
            angles = start_angle; % Evita dividir por un paso cero.
        elseif step_angle <= 0 || end_angle < start_angle + step_angle % Rechaza rangos que el codigo heredado ampliaria o recorreria mal.
            error('SITAU:XMLValue', 'Rango PWI invalido: usa paso positivo o una lista mc_pwic_angles.'); % No altera los angulos solicitados.
        else % Simula el recorrido float de LOAD_PWICFocalLaw sin reservar un vector enorme.
            angles = []; angle = single(start_angle); limit = single(end_angle); step = single(step_angle); % Misma precision que el cargador C++.
            while angle <= limit % Incluye el extremo solo si se alcanza, como la DLL.
                if numel(angles) >= 65535 || angle + step <= angle, error('SITAU:XMLValue', 'Rango PWI no representable.'); end % Acota iteraciones y evita un paso sin avance.
                angles(end+1) = double(angle); angle = angle + step; % Acumula leyes reales, sin factor PCIe 4+4.
            end % Fin del recorrido del rango.
        end % Fin de resolucion de angulos.
    end % Fin de seleccion lista/rango.
    number_of_focal_laws = numel(angles); % Una ley real por angulo.
elseif mc_focal_law_type == 2 % Las leyes de fichero requieren interpretar ese archivo adicional.
    error('SITAU:XMLUnsupported', 'La validacion de mc_focal_law_type=2 requiere el fichero de leyes y no esta implementada.'); % No inventa un numero de leyes.
else % Tipo desconocido.
    error('SITAU:XMLValue', 'mc_focal_law_type debe ser 0, 1 o 2.'); % Rechazo explicito.
end % Fin de seleccion de las leyes.
if number_of_focal_laws < 1 || number_of_focal_laws > 65535, error('SITAU:XMLValue', 'Numero de leyes fuera de rango.'); end % Comprueba el resultado sin duplicarlo para PCIe.

% Common variables
sampling_time = 1 / (ADC_BASE_FREQ / sampling_freq_decimation); %us
prf_time = 1 / prf_frequency * 1e6; %us

% Check PRF
focal_law_acq_time = (acq_start_samples + acq_n_samples - 1) * sampling_time + LINE_ACQ_OVH; %us

if focal_law_acq_time > prf_time
    error('SITAU:XMLValue', 'PRF demasiado alta para la ventana de adquisicion: reduce PRF o numero de muestras.'); % La incompatibilidad temporal sigue siendo un error, no un aviso de ancho de banda.
end

% Calculate performance data
focal_law_acq_time = prf_time * average_signals_number; %us
image_acq_time = focal_law_acq_time * number_of_focal_laws; %us
burst_acq_time = image_acq_time * images_per_burst; % Usa las rafagas de ACQUISITION, en unidades reales.
focal_law_PRF = 1 / focal_law_acq_time * 1e6; %Hz
image_PRF = 1 / image_acq_time * 1e6; %Hz
burst_PRF = image_PRF / images_per_burst; % Usa las rafagas de ACQUISITION, en unidades reales.
acq_total_time = burst_acq_time * total_bursts; % Usa las rafagas de ACQUISITION, en unidades reales.

% Check data bandwidth and memory size
focal_law_size_MB =  probe_elements_number * acq_n_samples * 2 / (1024^2);
image_size_MB = focal_law_size_MB * number_of_focal_laws;
burst_size_MB = image_size_MB * images_per_burst; % Usa las rafagas de ACQUISITION, en unidades reales.
total_data_GB = burst_size_MB * total_bursts / 1024; % Usa las rafagas de ACQUISITION, en unidades reales.
required_bandwidth_GBPS = burst_size_MB / 1024 / (burst_acq_time / 1e6);


if data_link_mode == 0 %gbE    

    data_bandwidth_hard_limit = GBE_BANDWIDTH_HARD_LIMIT;
    data_bandwidth_soft_limit = GBE_BANDWIDTH_WARNING_LIMIT;

elseif data_link_mode == 1 %PCIe

    data_bandwidth_hard_limit = PCIE_BANDWIDTH_HARD_LIMIT;
    data_bandwidth_soft_limit = PCIE_BANDWIDTH_WARNING_LIMIT;

else
    error(ERR_VALUES + " Unknown value in data_link_mode");
end

required_bandwidth_perc = required_bandwidth_GBPS / data_bandwidth_hard_limit * 100;

if required_bandwidth_GBPS > data_bandwidth_hard_limit
    
    message = sprintf("Estimated bandwidth (%.3f GiB/s) exceeds the reference link capacity (%.3f GiB/s). Reduce PRF or acquisition depth for sustained operation.", ... % El modelo no incluye pausas ni certifica rendimiento real.
    required_bandwidth_GBPS, data_bandwidth_hard_limit); % Muestra ambas estimaciones con precision suficiente para GE.

    warning('SITAU:XMLBandwidth', '%s', message); % El rendimiento sostenido es una estimacion; el equipo puede pausar la adquisicion.
    warningMessages = [warningMessages, WRN_GENERAL + " " + message]; % Conserva el aviso en el resultado estructurado.
    withWarnings = true; % No confunde exceso estimado de ancho de banda con formato XML invalido.

elseif required_bandwidth_GBPS > data_bandwidth_soft_limit
    
    message = sprintf("Required bandwidth is above %d%% of the link bandwidth (%.1f GBPS).\n Acquisition will probably be paused if the data buffer is not read fast enough by the user", ...
    round(required_bandwidth_perc), data_bandwidth_hard_limit);

    warning(message);
    warningMessages = [warningMessages, WRN_GENERAL + " " + message];
    withWarnings = true;

end

% Get theoretical values for this configuration

performance.samples_per_focal_law = acq_n_samples;
performance.focal_laws_per_image = number_of_focal_laws;
performance.images_per_burst = images_per_burst; % Usa las rafagas de ACQUISITION, en unidades reales.
performance.number_of_bursts = total_bursts; % Usa las rafagas de ACQUISITION, en unidades reales.
performance.averaging = average_signals_number;
performance.focal_law_acq_time_ms = focal_law_acq_time / 1e3;
performance.image_acq_time_ms = image_acq_time / 1e3;
performance.burst_acq_time_ms = burst_acq_time / 1e3;
performance.focal_law_PRF_Hz = focal_law_PRF;
performance.image_PRF_Hz = image_PRF;
performance.burst_PRF_Hz = burst_PRF;
performance.acq_total_time_sec = acq_total_time / 1e6;
performance.image_size_MB = image_size_MB;
performance.burst_size_MB = burst_size_MB;
performance.acq_total_size_GB = total_data_GB;
performance.required_bandwidth_GBPS = required_bandwidth_GBPS;
performance.required_bandwidth_PERC = required_bandwidth_perc;
performance.warning_messages = warningMessages;

% Show results

% Mostrar resultados de manera legible
fprintf('\n----------------------------------------------\n');
fprintf('Configuration Summary:                        \n');
fprintf('----------------------------------------------\n');
fprintf('Number of Averaged Signals       : %d\n', performance.averaging);
fprintf('Number of Samples per Focal Law  : %d\n', performance.samples_per_focal_law);
fprintf('Number of Focal Laws per Image   : %d\n', performance.focal_laws_per_image);
fprintf('Number of Images per Burst       : %d\n', performance.images_per_burst);
fprintf('Number of Bursts                 : %d\n', performance.number_of_bursts);
fprintf('----------------------------------------------\n');
fprintf('Acquisition Time:\n');
fprintf('----------------------------------------------\n');
fprintf('Focal Law Acquisition Time       : %.2f ms\n', performance.focal_law_acq_time_ms);
fprintf('Image Acquisition Time           : %.2f ms\n', performance.image_acq_time_ms);
fprintf('Burst Acquisition Time           : %.2f ms\n', performance.burst_acq_time_ms);
fprintf('Total Acquisition Time           : %.2f sec\n', performance.acq_total_time_sec);
fprintf('----------------------------------------------\n');
fprintf('Repetition Frequency:\n');
fprintf('----------------------------------------------\n');
fprintf('Focal Law Repetition Frequency   : %.2f Hz\n', performance.focal_law_PRF_Hz);
fprintf('Image Repetition Frequency       : %.2f Hz\n', performance.image_PRF_Hz);
fprintf('Burst Repetition Frequency       : %.2f Hz\n', performance.burst_PRF_Hz);
fprintf('----------------------------------------------\n');
fprintf('Data Size:\n');
fprintf('----------------------------------------------\n');
fprintf('Image Size                       : %.2f MB\n', performance.image_size_MB);
fprintf('Burst Size                       : %.2f MB\n', performance.burst_size_MB);
fprintf('Total Data Size                  : %.2f GB\n', performance.acq_total_size_GB);
fprintf('----------------------------------------------\n');
fprintf('Data Bandwidth:\n');
fprintf('----------------------------------------------\n');
fprintf('Required Bandwidth               : %.2f GBPS\n', performance.required_bandwidth_GBPS);
fprintf('Required Link Capacity           : %.2f%%\n', performance.required_bandwidth_PERC);
fprintf('----------------------------------------------\n');


performance.channels = probe_elements_number; % Publica la geometria efectiva para contrastarla despues de cargar.
performance.configured_bursts = burst_number; % Distingue el contador configurado del infinito de modo continuo.
performance.acquisition_mode = acquisition_mode; % Permite comprobar que la DLL aplico el modo.
performance.data_link_mode = data_link_mode; % Registra el enlace validado.
valid = true; % Solo devuelve true despues de superar todas las comprobaciones.
end % Fin del validador MC comun.
function node = section(parent, name) % Busca una unica seccion hija como el parser de la DLL.
children = parent.getChildNodes(); matches = {}; % No acepta una seccion anidada en un lugar distinto.
for index = 0:children.getLength-1 % Recorre solamente hijos directos.
    child = children.item(index); % Obtiene el nodo DOM.
    if strcmp(char(child.getNodeName()), name), matches{end+1} = child; end % Recoge secciones con nombre exacto.
end % Fin del recorrido de secciones.
if numel(matches) ~= 1, error('SITAU:XMLSchema', 'Debe existir una sola seccion %s en %s.', name, char(parent.getNodeName())); end % Rechaza ausencias y duplicados.
node = matches{1}; % Entrega la seccion inequívoca.
end % Fin de seleccion de seccion.
function value = scalarParam(node, name, defaultValue) % Lee escalares numericos con opcion de valor predeterminado.
value = getXLMparam(node, name, nargin == 3); % Solo permite ausencia si se declara un valor por defecto.
if isempty(value), value = defaultValue; end % Aplica un valor compartido con la DLL.
if ~isscalar(value), error('SITAU:XMLValue', '%s debe ser escalar.', name); end % Evita vectores en parametros escalares.
end % Fin de lectura escalar.
function positiveInteger(value, name) % Comprueba dimensiones y contadores antes de multiplicar.
if ~isnumeric(value) || ~isscalar(value) || ~isfinite(value) || value <= 0 || value ~= fix(value), error('SITAU:XMLValue', '%s debe ser un entero positivo.', name); end % Rechaza errores del getter y valores invalidos.
end % Fin de comprobacion de enteros.
