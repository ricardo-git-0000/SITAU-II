from pathlib import Path  # Localiza los catalogos de produccion.
import re  # Comprueba las definiciones y sus traducciones sin compilar.
root = Path(__file__).resolve().parents[2]  # Resuelve el repositorio independientemente del directorio actual.
source_root = root / 'trunk/src/lib/stlib2/src'  # Agrupa los catalogos de la DLL principal.
def read(path):  # Preserva la codificacion historica de los archivos C++.
    return path.read_bytes().decode('latin1')  # Permite auditar incluso comentarios antiguos no UTF-8.
files = list(source_root.rglob('*error_code.cpp')) + [root / 'trunk/src/lib/api_pcie/src/pcie_error_code.cpp']  # Incluye tambien los errores del wrapper.
entry_count = 0  # Cuenta las descripciones verificadas.
for source_path in files:  # Revisa todas las familias con traductor propio.
    header_path = source_path.with_suffix('.h')  # Asocia cada traductor a sus definiciones.
    if not header_path.exists():  # Omite archivos sin un catalogo homonimo.
        continue  # No supone una correspondencia inexistente.
    source_text = read(source_path)  # Lee el codigo real modificado.
    header_text = read(header_path)  # Lee las descripciones publicadas en el proyecto.
    descriptions = re.findall(r'^#define\s+char(\w+)\s+', header_text, re.M)  # Enumera los codigos que tienen significado documentado.
    for name in descriptions:  # Verifica la cobertura individual de cada error.
        if name == 'EHW_File':  # Este simbolo comparte -3003 con EHW_XML_Format.
            assert 'EHW_File / EHW_XML_Format' in source_text  # Exige una descripcion que reconozca la ambiguedad.
        else:  # Los demas codigos deben tener un caso propio.
            assert re.search(r'case\s+' + name + r'\s*:', source_text), (source_path, name)  # Detecta errores definidos pero sin traducir.
            if name != 'EHW_XML_Format':  # La descripcion combinada del alias es intencionada.
                case_body = re.search(r'case\s+' + name + r'\s*:([\s\S]*?)break;', source_text).group(1)  # Aisla el mensaje de ese codigo.
                assert 'char' + name in case_body, (source_path, name, 'wrong description')  # Detecta textos copiados de otro error.
        entry_count += 1  # Registra la descripcion comprobada.
    cases = re.findall(r'case\s+(\w+)\s*:', source_text)  # Obtiene las etiquetas del switch.
    assert len(cases) == len(set(cases)), source_path  # Impide duplicar una etiqueta al completar el catalogo.
main_text = read(source_root / 'st_error_code.cpp')  # Comprueba que las familias son alcanzables desde ST_GetError.
for translator in ['ST_GetBASEErrorCode', 'ST_GetSITAUErrorCode', 'FL_GetErrorCode', 'ST_GetVCHErrorCode', 'ST_GetTCPErrorCode', 'GetCommonErrorCode', 'ST_GetBufferErrorCode']:  # Enumera las rutas necesarias.
    assert re.search(r'if \(' + translator + r'\(error_code, str\)', main_text), translator  # Detecta traductores no conectados.
for header in ['TOOLS/XML/xml_err.h', 'st_encoder_error_code.h', 'HW/c_sitau_hw_err.h']:  # Comprueba las familias auxiliares sin traductor propio.
    for name in re.findall(r'^#define\s+char(\w+)\s+', read(source_root / header), re.M):  # Enumera sus descripciones.
        assert re.search(r'case\s+' + name + r'\s*:', main_text), name  # Exige que ST_GetError traduzca cada una.
        entry_count += 1  # Incluye esos codigos en el total comprobado.
wrapper_text = read(root / 'trunk/src/lib/api_pcie/src/stlib2/stlib2_wrapper.cpp')  # Revisa el punto de entrada usado por MATLAB.
assert 'PCIe_GetError(error_code, pcie_description)' in wrapper_text and 'sitau_found == 1 && pcie_found == 1' in wrapper_text  # Exige cobertura de errores propios de PCIe.
vch_text = read(source_root / 'c_virtual_channel/c_vch_config.cpp')  # Comprueba la propagacion del nuevo fallo de mapa.
assert 'return ELOG(charEVCH_PinToChannelSize, EVCH_PinToChannelSize)' in vch_text  # Rechaza la configuracion en vez de continuar silenciosamente.
vch_without_comments = re.sub(r'//[^\r\n]*', '', vch_text)  # Aisla las instrucciones para revisar el bloque afectado por la macro.
assert re.search(r'if\s*\(xml->probe\.pin_to_channel\.Value\(\)\.size\(\) != cfg_\.pa_probe->GetElementNumber\(\)\)\s*\{\s*return ELOG\(charEVCH_PinToChannelSize, EVCH_PinToChannelSize\);\s*\}\s*xml->probe\.pin_to_channel\.Assign', vch_without_comments)  # Evita que el punto y coma de ELOG deje un else sin if.
assert '!= cfg_.pa_probe->GetElementNumber()' in vch_text  # Usa los elementos efectivos tras aplicar los limites del equipo.
assert 'pin_to_channel.Update() && !xml->probe.pin_to_channel_assignment.Value()' in vch_text  # Respeta mapas omitidos y el modo consecutivo.
vch_header = read(source_root / 'c_virtual_channel/vch_error_code.h')  # Verifica la identidad del nuevo codigo.
assert re.search(r'EVCH_PinToChannelSize\s+\(vch_error_code_start - 23\)', vch_header)  # Reserva -1523 dentro de la familia de canales virtuales.
for header in source_root.rglob('*.h'):  # Detecta una colision literal accidental con el nuevo codigo.
    if header.name != 'vch_error_code.h':  # La definicion intencionada ya se ha comprobado.
        assert not re.search(r'#define\s+\w+\s+-1523\b', read(header)), header  # No permite reutilizar -1523 en otro encabezado.
for path in (root / 'trunk/src/app/MatLab/Toolbox_sitau2').glob('*.m'):  # Revisa todos los mensajes existentes de la toolbox.
    for line in read(path).splitlines():  # Inspecciona cada presentacion de resultados.
        if re.search(r'(errordlg|disp|fprintf|sprintf)\(', line):  # Limita la comprobacion a salida de texto.
            assert not re.search(r'(int2str|num2str)\((result|error)\)', line), (path, line)  # Evita volver a mostrar solo el numero.
print(f'PASS: {entry_count} descripciones cubiertas, rutas de traduccion y propagacion de -1523 verificadas estaticamente')  # Distingue estas comprobaciones de pruebas binarias.
# Comprueba las colisiones numericas entre familias sin ejecutar codigo C++.
values = {}  # Agrupa los simbolos por su numero publico.
messages = {}  # Conserva los textos para verificar la longitud del mensaje combinado.
header_paths = [p.with_suffix('.h') for p in files if p.with_suffix('.h').exists()] + [source_root / p for p in ['TOOLS/XML/xml_err.h', 'st_encoder_error_code.h', 'HW/c_sitau_hw_err.h']]  # Incluye todas las familias alcanzables.
for header in header_paths:  # Evalua las expresiones simples de las constantes.
    header_text = read(header)  # Lee el catalogo original.
    definitions = dict(re.findall(r'^#define[ \t]+(\w+)[ \t]+([^\r\n]+)', header_text, re.M))  # Evita consumir la linea siguiente en las guardas vacias.
    for name, description in re.findall(r'^#define\s+char(\w+)\s+"((?:\\.|[^"\\])*)"', header_text, re.M):  # Revisa los codigos con descripcion.
        expression = definitions[name].split('//')[0].strip()  # Descarta comentarios de la constante.
        for constant, definition in definitions.items():  # Sustituye el comienzo numerico de cada familia.
            literal = definition.split('//')[0].strip()  # Aisla el valor de la constante base.
            if re.fullmatch(r'-?\d+', literal):  # Solo acepta constantes enteras simples.
                expression = re.sub(r'\b' + constant + r'\b', literal, expression)  # Resuelve la expresion sin ejecutar macros.
        assert re.fullmatch(r'[\d\s()+\-]+', expression), expression  # Impide evaluar expresiones que no sean aritmetica elemental.
        number = eval(expression, {'__builtins__': {}})  # Calcula el numero publico a partir de sus constantes.
        values.setdefault(number, []).append(name)  # Detecta simbolos que comparten codigo.
        messages.setdefault(number, []).append(description)  # Conserva ambas causas de los errores compartidos.
shared_codes = {number for number, names in values.items() if len(names) > 1}  # Obtiene las ambiguedades reales del proyecto.
assert shared_codes == {-3003, *range(-119, -100)}, shared_codes  # Permite solo los solapamientos heredados documentados.
assert values[-1523] == ['EVCH_PinToChannelSize']  # El error nuevo debe ser inequivoco.
for number in range(-119, -100):  # Revisa los mensajes combinados TCP/PCIe nuevos.
    combined = f"(Error Code {number}) 'Codigo compartido. TCP: {messages[number][0]} / PCIe: {messages[number][1]}'"  # Mide las dos descripciones y su prefijo.
    assert len(combined.encode('latin1')) < 255, (number, len(combined))  # Mantiene estos mensajes dentro de los buffers historicos de 255 bytes.
print('PASS: -1523 unico; 20 codigos compartidos documentados; mensajes combinados menores de 255 bytes')  # Resume las verificaciones de compatibilidad.
