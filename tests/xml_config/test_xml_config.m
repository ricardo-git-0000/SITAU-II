function test_xml_config() % Ejecuta el validador real sin DLL ni hardware.
test_path = fileparts(mfilename('fullpath')); repo = fileparts(fileparts(test_path)); % Localiza el repositorio desde cualquier directorio.
previous_path = path; path_cleanup = onCleanup(@() path(previous_path)); % Restaura el entorno al terminar.
toolbox = fullfile(repo, 'trunk', 'src', 'app', 'MatLab', 'Toolbox_sitau2'); % Usa los fuentes de produccion.
examples = fullfile(repo, 'trunk', 'src', 'app', 'MatLab', 'Examples'); % Comprueba los cuatro XML entregados.
addpath(toolbox); % No carga ni abre ninguna biblioteca de adquisicion.
folder = tempname; mkdir(folder); cleanup = onCleanup(@() rmdir(folder, 's')); % Aisla y elimina exclusivamente los XML temporales de esta prueba.
names = {'SITAU2_GE_NOISE.xml', 'SITAU2_GE_RF.XML', 'SITAU2_PCIe_NOISE.xml', 'SITAU2_PCIe_RF.XML'}; % Demos MC activas.
images = [1 250 3 1]; bursts = [1 150 150 10]; count = 0; % Valores esperados independientes de la implementacion del validador.
for index = 1:numel(names) % Contrasta migracion y cifras de rafaga.
    for channels = [128 256] % El mismo XML debe servir para ambos equipos.
        [valid, ~, info] = quietValidate(fullfile(examples, names{index}), channels, double(index > 2)); % Ejecuta el parser real con geometria explicita.
        assert(valid && info.channels == channels && info.focal_laws_per_image == 9); % Conserva nueve leyes PWI reales en ambos transportes.
        assert(info.images_per_burst == images(index) && info.configured_bursts == bursts(index)); % No confunde numero de rafagas e imagenes por rafaga.
        assert(isinf(info.number_of_bursts) && info.samples_per_focal_law > 0); % Las demos actuales son continuas y tienen una ventana positiva.
        actual = info; actual.probe_elements = channels; Validate_LoadedMCConfig(info, actual); % Comprueba una configuracion aplicada coherente.
        actual.focal_laws_per_image = channels; expectError(@() Validate_LoadedMCConfig(info, actual), 'SITAU:LoadedConfigMismatch'); % Detecta la caida silenciosa al FMC antiguo.
        actual = info; actual.probe_elements = channels; actual.images_per_burst = images(index) + 1; % Simula una DLL que ignora el contador de imagenes.
        expectError(@() Validate_LoadedMCConfig(info, actual), 'SITAU:LoadedConfigMismatch'); % Debe fallar antes de arrancar adquisicion.
        count = count + 1; % Cuenta escenarios reales de los XML entregados.
    end % Fin de geometria del equipo.
end % Fin de demos.
source = fileread(fullfile(examples, names{3})); fixture = fullfile(folder, 'config.xml'); % Parte de un XML PCIe valido de nueve leyes.
writeText(fixture, source); % Prepara la configuracion base sin elementos explicitos.
expectError(@() Validate_XMLConfig(fixture), 'SITAU:XMLChannels'); % La validacion offline sin elementos requiere indicar los canales.
expectError(@() quietValidate(fixture, 0, 1), 'SITAU:XMLValue'); % No toma un error del getter como numero de canales.
expectError(@() quietValidate(fixture, 256, 0), 'SITAU:XMLLink'); % Detecta discrepancia con OpenSys.
writeText(fixture, strrep(source, '<PROBE>', '<PROBE><Parameter pName="probe_elements_number" pValue="128" />')); % Simula el XML antiguo del otro ordenador.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLChannels'); % Rechaza la sonda de 128 en un equipo de 256.
[valid, ~, info] = quietValidate(fixture, 128, 1); assert(valid && info.channels == 128); % Respeta un valor explicito compatible.
writeText(fixture, strrep(source, '<PROBE>', '<PROBE><Parameter pName="probe_elements_number" pValue="256" />')); % Permite validacion offline con elementos explicitos.
evalc('[valid, ~, info] = Validate_XMLConfig(fixture);'); assert(valid && info.channels == 256); % No exige hardware cuando la geometria esta declarada.
writeText(fixture, strrep(source, 'MC_FOCAL_LAW', 'FP_FOCAL_LAW')); % Recrea la seccion que la DLL ignora.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLLegacy'); % No acepta el esquema antiguo como valido.
writeText(fixture, strrep(source, 'mc_focal_law_type', 'fp_focal_law_type')); % Recrea mezcla de nombres en una seccion nueva.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLLegacy'); % El mensaje identifica la migracion necesaria.
writeText(fixture, strrep(source, '<MC_FOCAL_LAW>', '<MC_FOCAL_LAW><Parameter pName="mc_burst_number" pValue="9" />')); % Recrea rafagas en una seccion equivocada.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLLegacy'); % No confunde esos campos con ACQUISITION.
doc = xmlread(fullfile(examples, names{3})); setValue(doc, 'acquisition_mode', '0'); setValue(doc, 'burst_number', '7'); setValue(doc, 'images_per_burst_number', '2'); % Configura una adquisicion finita reconocible.
xmlwrite(fixture, doc); [valid, ~, info] = quietValidate(fixture, 256, 1); % Ejecuta la configuracion finita.
assert(valid && info.number_of_bursts == 7 && info.images_per_burst == 2 && info.focal_laws_per_image == 9); % No duplica leyes por el transporte de ocho modulos.
setValue(doc, 'images_per_burst_number', '0'); xmlwrite(fixture, doc); % Inyecta contador cero.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % Evita dimensiones nulas.
setValue(doc, 'images_per_burst_number', '1'); setValue(doc, 'mc_pwic_step_angle', '0'); xmlwrite(fixture, doc); % Rango no trivial con paso cero.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % No divide por cero ni oculta el rango incorrecto.
setValue(doc, 'mc_pwic_start_angle', '0'); setValue(doc, 'mc_pwic_end_angle', '0'); xmlwrite(fixture, doc); % Una sola ley legitima.
[valid, ~, info] = quietValidate(fixture, 256, 1); assert(valid && info.focal_laws_per_image == 1); % Acepta PWI de un angulo.
setValue(doc, 'mc_focal_law_type', '0'); xmlwrite(fixture, doc); % Configura FMC real.
[valid, ~, info] = quietValidate(fixture, 128, 1); assert(valid && info.focal_laws_per_image == 128); % Ya no accede a n_channels sin definir.
setValue(doc, 'mc_focal_law_type', '2'); xmlwrite(fixture, doc); % Configura leyes desde fichero.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLUnsupported'); % No declara validado un modo sin implementar.
writeText(fixture, strrep(source, '<MC_FOCAL_LAW>', '<MC_FOCAL_LAW><Parameter pName="mc_pwic_angles" pValue="[-7,0,4]" />')); % La lista debe prevalecer sobre el rango existente.
[valid, ~, info] = quietValidate(fixture, 256, 1); assert(valid && info.focal_laws_per_image == 3); % Coincide con la precedencia del parser C++.
writeText(fixture, strrep(source, '<MC_FOCAL_LAW>', '<MC_FOCAL_LAW><Parameter pName="mc_focal_law_type" pValue="0" />')); % Parametro repetido con valores distintos.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLDuplicate'); % No elige arbitrariamente uno.
doc = xmlread(fullfile(examples, names{3})); setValue(doc, 'prf_frequency', 'disp(123)'); xmlwrite(fixture, doc); % Intenta introducir una expresion en un numero.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % El lector no evalua texto como codigo.
setValue(doc, 'prf_frequency', '5e1'); xmlwrite(fixture, doc); expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % Rechaza exponentes que el lector C++ altera.
setValue(doc, 'prf_frequency', '50'); setValue(doc, 'burst_number', '150.0'); xmlwrite(fixture, doc); % Un entero con punto se interpretaria de otra forma en C++.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % Exige literales enteros en los contadores.
defaults = regexprep(source, '<Parameter pName="(burst_number|images_per_burst_number)"[^>]*/>', ''); % Omite ambos contadores para contrastar los valores predeterminados de la DLL.
writeText(fixture, defaults); [valid, ~, info] = quietValidate(fixture, 256, 1); assert(valid && info.configured_bursts == 1 && info.images_per_burst == 1); % Coincide con ResetData.
writeText(fixture, strrep(defaults, '<MC_FOCAL_LAW>', '<MC_FOCAL_LAW><Parameter pName="burst_number" pValue="7" />')); % Un nombre nuevo en la seccion antigua tambien es incorrecto.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLSchema'); % No convierte el contador mal ubicado en el valor predeterminado.
writeText(fixture, strrep(source, '<MC_DSP>', '<MC_DSP><Parameter pName="mc_pwic_angles" pValue="[-7,0,4]" />')); % La lista de angulos no pertenece a DSP.
expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLSchema'); % Rechaza una lista que la DLL ignoraria.
for angles = {'[-7 0 4]', '[-7;0;4]', '-7,0,4', '[1e1,2]', '[]', '[0,,4]'} % Formatos que el lector C++ no interpreta como la lista solicitada.
    writeText(fixture, strrep(source, '<MC_FOCAL_LAW>', ['<MC_FOCAL_LAW><Parameter pName="mc_pwic_angles" pValue="' angles{1} '" />'])); % Introduce cada lista incorrecta.
    expectError(@() quietValidate(fixture, 256, 1), 'SITAU:XMLValue'); % No acepta una interpretacion distinta entre MATLAB y DLL.
end % Fin de formatos de lista.
[valid, warnings, info] = quietValidate(fullfile(examples, names{2}), 256, 0); % GE RF supera la referencia de ancho de banda con su PRF existente.
assert(valid && warnings && ~isempty(info.warning_messages)); % La estimacion avisa; no modifica PRF ni certifica rendimiento sostenido.
fprintf('PASS: %d configuraciones GE/PCIe 128/256 y rechazos de esquema, elementos, rafagas y DLL incoherente.\n', count); % Publica el resultado sin adquirir datos.
end % Fin de pruebas funcionales MATLAB.
function [valid, warnings, info] = quietValidate(file, channels, link) % Silencia el informe de rendimiento para que el resultado sea legible.
evalc('[valid, warnings, info] = Validate_XMLConfig(file, channels, link);'); % Ejecuta el validador de produccion.
end % Fin de envoltorio de salida.
function expectError(action, identifier) % Exige un rechazo con causa concreta.
try, action(); catch failure, assert(strcmp(failure.identifier, identifier), '%s: %s', failure.identifier, failure.message); return; end % No confunde un error inesperado con un test correcto.
error('SITAU:TestFailure', 'Se esperaba el error %s.', identifier); % Falla si se acepto la configuracion invalida.
end % Fin de comprobacion de errores.
function writeText(file, text) % Escribe solo fixtures temporales propiedad del test.
handle = fopen(file, 'w'); assert(handle >= 0); cleanup = onCleanup(@() fclose(handle)); % Cierra el archivo incluso si falla la escritura.
fprintf(handle, '%s', text); % No interpreta el XML como formato de fprintf.
end % Fin de escritura de fixture.
function setValue(doc, name, value) % Modifica un parametro existente en el DOM de prueba.
parameters = doc.getElementsByTagName('Parameter'); % Obtiene los parametros de la fixture.
for index = 0:parameters.getLength-1 % Localiza el nombre solicitado.
    parameter = parameters.item(index); % Obtiene el nodo modificable.
    if strcmp(char(parameter.getAttribute('pName')), name), parameter.setAttribute('pValue', value); return; end % Cambia unicamente su valor.
end % Fin de busqueda del parametro.
error('SITAU:TestFailure', 'No existe el parametro de prueba %s.', name); % No oculta una fixture distinta de la prevista.
end % Fin de modificacion de fixture.
