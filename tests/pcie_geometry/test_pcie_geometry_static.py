from pathlib import Path  # Resuelve los fuentes desde cualquier directorio de ejecucion.
import re  # Inspecciona el orden de validacion y los limites del contrato sin compilar C++.
root = Path(__file__).resolve().parents[2]  # Localiza el repositorio.
api = root / 'trunk/src/lib/api_pcie/src'  # Localiza la capa PCIe.
def read(path):  # Lee sin alterar la codificacion historica de los fuentes.
    return path.read_bytes().decode('latin1')  # Los identificadores inspeccionados son ASCII.
header = read(api / 'pcie_geometry.h')  # Inspecciona la funcion que utilizara el binario.
wrapper = read(api / 'stlib2/stlib2_wrapper.cpp').split('int ST2_Start(int data_sorted)', 1)[1].split('int ST2_SetTriggerSource', 1)[0]  # Aisla el arranque activo.
active = re.sub(r'//[^\r\n]*', '', wrapper)  # No confunde llamadas historicas comentadas con ejecucion.
assert active.index('PCIe_ResolveGeometry') < active.index('PCIe_Open()') < active.index('PCIe_SetGeometryBuffers')  # Impide reservar DMA antes de validar las dimensiones.
assert active.index('if (gb_pcie_enabled)') < active.index('PCIe_ResolveGeometry')  # Mantiene TCP fuera de la validacion PCIe.
assert 'EPCIE_GeometryPending' not in active and 'PCIe_InitGeometryManager(dma_blocks_number, geometry)' in active  # Las cuatro tramas ya usan reserva y gestor parametrizados.
assert 'PCIe_SetGeometryBuffers(geometry, images_ddr_number, n_images)' in active and 'PCIe_InitGeometryManager(dma_blocks_number, geometry)' in active  # Comprueba que el arranque reutiliza los valores validados.
resolver = read(api / 'pcie.cpp').split('int PCIe_ResolveGeometry', 1)[1].split('int PCIe_Open', 1)[0]  # Aisla el adaptador de stlib2.
assert resolver.index('geometry = T_PCIeGeometry()') < resolver.index('ST_GetChannelsNumber()')  # Invalida la salida aunque falle el primer getter.
for getter in ['ST_GetChannelsNumber()', 'ST_GetModulesNumber()', 'ST_GetProbeElementNumber()', 'ST_GetProbePinToChannelSize(0)', 'ST_MC_GetAScanDataNumber(0)', 'ST_GetFocalLawNumber(0)', 'ST_PCIe_GetFocalLawsBlockNumber()']:  # Enumera las fuentes de la configuracion comun.
    assert resolver.count(getter) == 1, getter  # Evita releer la misma dimension durante la resolucion.
for path in [api / 'stlib2/stlib2.h', root / 'trunk/src/lib/stlib2/src/stlib2.h']:  # Comprueba ambas cabeceras usadas al compilar las DLL.
    assert 'int _TYPE ST_GetModulesNumber(void);' in read(path)  # Evita divergencias entre exportacion e importacion.
assert 'return gb_virtual_channel[0]->GET_MODULENumber()' in read(root / 'trunk/src/lib/stlib2/src/st_get.cpp')  # Obtiene modulos fisicos, no canales totales divididos por 32.
assert header.index('samples_per_channel > 32768') < header.index('static_cast<std::uint64_t>')  # Rechaza dimensiones extremas antes del producto.
assert 'module_count != channel_count / 32' in header and 'probe_elements != channel_count' in header  # Rechaza topologia y sonda parcial fuera de alcance.
assert '(1ULL << 23) - 32ULL' in header  # Mantiene el limite de bloque de GTX.
registers = read(root / 'trunk/src/lib/stlib2/src/st_reg.h')  # Comprueba que el limite duplicado coincide con stlib2.
assert re.search(r'#define\s+GTX_SIZE_BLOCK\s+\(\(1<<23\) - 32\)', registers)  # Obliga a revisar el contrato si cambia la capacidad hardware.
assert 'std::uint64_t' in header and '32ULL + shorts_per_focal_law * 2ULL * focal_laws_per_block' in header  # Verifica las unidades y la anchura de los productos.
print('PASS: integracion estatica de geometria PCIe; no ejecuta C++, DMA ni hardware.')  # Explicita el alcance de esta prueba.

fuse_source = read(api / 'fuse/fuse.cpp')  # Revisa la ruta real de lectura y reserva.
assert 'transport_modules_ * frame_len * 32ULL' in fuse_source and 'read_size != dma_buffer_size' in fuse_source  # La misma topologia determina reserva y lectura.
assert 'if (n_bytes != hw_img_size)' in fuse_source and 'return EPCIE_BlockLayout' in fuse_source  # No entrega colas de memoria sin datos nuevos.
assert 'transport_modules_ != EMULATOR_FRAMES_PER_PACKAGE' in fuse_source  # Mantiene explicita la limitacion del emulador.
manager = read(api / 'c_image_handle.cpp')  # Inspecciona el consumidor real del bloque.
assert 'PCIe_LawOffset(geometry_' in manager and 'frame_length * N_CHANNELS_MODULE * N_MODULES' not in manager  # Comprueba capacidad antes de publicar punteros dinamicos.
assert 'current_block_read_ = next_block_read_index++' in manager and 'calc_used_fifo_blocks() > 1' in manager  # Incluye imagenes que necesitan varios bloques.
assert 'current_block_read_ - current_block_fred_ + 1' in manager  # La ocupacion no depende de confundir indices iguales con FIFO vacia.
wrapper_full = read(api / 'stlib2/stlib2_wrapper.cpp')  # Comprueba la carga transaccional del mapa.
assert 'copied != channels' in wrapper_full and 'gb_pin_to_channel.swap(channel_map)' in wrapper_full  # Publica solo mapas completos y validados.
assert 'PCIe_ValidateMap' in wrapper_full and 'PCIe_RefreshChannelMap()' in active  # Valida tambien si cambia la configuracion antes del arranque.
utils = read(api / 'pcie_utils.cpp')  # Inspecciona la ordenacion usada por C_PCIBuffer.
assert 'PCIe_SortLaw' in utils and 'i_fl_data = size_fl - 1' not in utils  # No oculta indices erroneos mediante recorte.
gtx = read(root / 'trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp')  # Revisa la division anterior al arranque.
assert 'if (!law_bytes' in gtx and 'gtx_nfl_blocks > 1 && burst_laws % gtx_nfl_blocks' in gtx  # Protege capacidad cero y divisor del ajuste de rafaga.
print('PASS: integracion estatica DMA, FIFO, mapa, ordenacion y GTX; C++/hardware pendientes.')  # Delimita la evidencia obtenida.
