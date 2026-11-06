"""Comprobaciones de fuentes; no ejecutan C++ ni certifican supervivencia de MATLAB."""
from pathlib import Path  # Localiza los fuentes sin depender del directorio actual.
import re  # Ignora comentarios al buscar llamadas de cierre.
root = Path(__file__).resolve().parents[2] / 'trunk/src/lib/stlib2/src'  # Usa la implementacion de produccion.
buffer = (root / 'c_buffer/c_buffer_hw.cpp').read_text(encoding='latin1')  # Conserva la codificacion heredada.
registers = (root / 'st_reg.cpp').read_text(encoding='latin1')  # Lee los llamantes de memoria y reset.
channels = (root / 'c_virtual_channel/c_virtual_channel.cpp').read_text(encoding='latin1')  # Lee el llamante de reconstruccion.
code = re.sub(r'//[^\n]*|/\*.*?\*/', '', buffer, flags=re.S)  # No confunde explicaciones con instrucciones.
assert not re.search(r'\b(exit|_exit|abort|ExitProcess|TerminateProcess)\s*\(', code)  # El buffer ya no termina el proceso anfitrion.
assert 'catch (const std::bad_alloc&)' in code and 'return ELOG(charEBUF_NoMemory, EBUF_NoMemory)' in code  # Convierte el fallo en el error existente.
assert 'throw std::bad_alloc();' in code  # malloc fallido no permite continuar con un puntero nulo.
for guard in ('buffer_guard', 'lock_guard', 'image_lock_guard'):  # Las reservas parciales quedan protegidas hasta el exito.
    assert f'{guard}.release();' in code and 'std::unique_ptr' in code  # Comprueba la transferencia final de recursos.
assert 'if ((result = ST_EnsureHWBuffer()) < 0) return RLOG(result);' in registers  # El arranque propaga el fallo antes de usar el buffer.
assert 'if (buffer_result < 0) return RLOG(buffer_result);' in channels  # La reconstruccion tampoco oculta el fallo.
assert 'return ST_VirtualChannelsDelete();' in channels  # La configuracion por defecto conserva el codigo.
assert 'return ST_VirtualChannelsDefault();' in registers  # El reset conserva el codigo.
constructors = sum(p.read_text(encoding='latin1').count('new CBufferHW(') for p in root.rglob('*.cpp'))  # Busca todas las reservas directas del objeto.
assert constructors == 1  # Ningun llamante evita la funcion protegida.
print('PASS: propagacion estatica de -703, construccion centralizada y ausencia de exit en CBufferHW; C++ pendiente.')  # Explicita los limites de la comprobacion.
