from pathlib import Path  # Extrae el gestor real sin las dependencias Windows del driver.
import re  # Elimina exclusivamente directivas include de la implementacion.
root = Path(__file__).resolve().parents[2]  # Localiza el repositorio.
source = root / 'trunk/src/lib/api_pcie/src'  # Selecciona los fuentes de produccion.
output = Path(__file__).resolve().parent / 'generated'  # Mantiene los adaptadores fuera de los fuentes versionados.
output.mkdir(exist_ok=True)  # Crea solo la carpeta de resultados de prueba.
types = (source / 'pcie_tad.h').read_bytes().decode('latin1')  # Usa las cabeceras binarias reales de produccion.
types = types[types.index('typedef union'):types.index('enum TThreadPriority')]  # Omite los alias System::IO y Windows ajenos al formato.
header = (source / 'c_image_handle.h').read_bytes().decode('latin1')  # Usa la declaracion real de la clase.
header = header[header.index('class CImageHandle\n') if 'class CImageHandle\n' in header else header.index('class CImageHandle\r\n'):]  # Comienza en la clase, no en el comentario del encabezado.
header = header[:header.index('};') + 2]  # Conserva todos los campos y metodos privados/publicos.
implementation = (source / 'c_image_handle.cpp').read_bytes().decode('latin1')  # No reimplementa la logica del FIFO ni los punteros.
implementation = re.sub(r'^#include[^\r\n]*', '', implementation, flags=re.M)  # El harness proporciona solo las dependencias externas simuladas.
(output / 'types.inc').write_text(types, encoding='utf-8')  # Publica el formato real de las cabeceras para el test nativo Windows.
(output / 'manager.inc').write_text(header + '\n' + implementation, encoding='utf-8')  # Compila la clase actual, no una copia mantenida a mano.
pcie_source = (source / 'pcie.cpp').read_bytes().decode('latin1')  # Extrae tambien el verificador de secuencia de produccion.
checker = pcie_source[pcie_source.index('int PCIe_CheckPCIeHeader_fast(void *ptr_block)'):]  # Selecciona su firma exacta.
checker = checker[:checker.index('// -----------------------------------------------------------------------------')]  # Termina antes de la funcion siguiente.
(output / 'checker.inc').write_text(checker, encoding='utf-8')  # Evita sustituir por exito la conversion del contador de bloques.
print('Preparados gestor y verificador reales; no se ha compilado ni ejecutado C++.')  # Explicita el alcance de esta preparacion.
