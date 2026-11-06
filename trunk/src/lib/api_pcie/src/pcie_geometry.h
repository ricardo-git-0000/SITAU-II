#ifndef pcie_geometryH // Evita definiciones repetidas de la geometria de adquisicion.
#define pcie_geometryH // Mantiene esta configuracion independiente del driver y de Windows.
#include <cstdint> // Calcula productos en 64 bits antes de convertirlos a tamanos del driver.
#include "pcie_error_code.h" // Reutiliza el catalogo publico para propagar fallos descriptivos.
struct T_PCIeGeometry // Describe una adquisicion paralela del canal virtual cero.
{ // Agrupa valores que deben permanecer coherentes durante toda la adquisicion.
    int channel_count; // Canales efectivos obtenidos de stlib2.
    int module_count; // Modulos fisicos; cada uno transmite una trama de 32 canales.
    int samples_per_channel; // Muestras de 16 bits por canal y ley focal.
    int focal_laws_per_image; // Leyes que forman una imagen de usuario.
    int focal_laws_per_block; // Leyes reales completas en un bloque DMA.
    int transport_units_per_law; // Un grupo de cuatro modulos en 128 y dos en 256.
    int transport_units_per_block; // Contador enviado a la placa, distinto de las leyes reales.
    int beats_per_module; // Beats de 32 bytes por trama; la cabecera transmite este valor menos uno.
    std::uint64_t shorts_per_focal_law; // Distancia entre leyes en muestras de 16 bits.
    std::uint64_t block_size_bytes; // Cabecera de 32 bytes mas todas las leyes del bloque.
}; // No representa memoria DMA reservada ni autoriza por si sola el arranque.
inline int PCIe_BuildGeometry(int channel_count, int module_count, int probe_elements, int map_size, int samples_per_channel, int focal_laws_per_image, int focal_laws_per_block, T_PCIeGeometry &geometry) // Valida sin acceder al hardware ni modificar globals.
{ // Los limites corresponden al formato actual de SITAU en modo paralelo.
    geometry = T_PCIeGeometry(); // Un fallo no deja reutilizable una configuracion anterior.
    if ((channel_count != 128 && channel_count != 256) || module_count != channel_count / 32 || probe_elements != channel_count || map_size != channel_count) return EPCIE_Geometry; // Rechaza multiplexado, sondas parciales y mapas de longitud incompatible.
    if (samples_per_channel <= 0 || focal_laws_per_image <= 0 || focal_laws_per_block <= 0) return EPCIE_Geometry; // Evita productos y divisores sin dimensiones validas.
    const int transport_units_per_law = module_count / 4; // La placa intermedia siempre cuenta grupos de cuatro modulos.
    if (samples_per_channel > 32768 || focal_laws_per_block > 1024 / transport_units_per_law || focal_laws_per_image > 65535) return EPCIE_GeometrySize; // Limita a 1024 unidades sin duplicar las leyes por imagen.
    const std::uint64_t shorts_per_focal_law = static_cast<std::uint64_t>(samples_per_channel) * channel_count; // Convierte antes del producto para evitar overflow de int.
    const std::uint64_t block_size_bytes = 32ULL + shorts_per_focal_law * 2ULL * focal_laws_per_block; // Una trama por modulo, sin relleno hasta ocho.
    const std::uint64_t max_block_size_bytes = (1ULL << 23) - 32ULL; // Coincide con GTX_SIZE_BLOCK de stlib2/src/st_reg.h, cabecera incluida.
    if (block_size_bytes > max_block_size_bytes) return EPCIE_GeometrySize; // Este limite tambien mantiene la lectura dentro de DWORD y size_t de 32 bits.
    geometry.channel_count = channel_count; // Publica solo valores de una configuracion ya validada.
    geometry.module_count = module_count; // Conserva el numero fisico contrastado con los canales.
    geometry.samples_per_channel = samples_per_channel; // Fija las muestras para la adquisicion.
    geometry.focal_laws_per_image = focal_laws_per_image; // Fija las dimensiones de la imagen de usuario.
    geometry.focal_laws_per_block = focal_laws_per_block; // Conserva leyes reales para punteros, imagenes y DMA.
    geometry.transport_units_per_law = transport_units_per_law; // Mantiene factor uno para cuatro modulos.
    geometry.transport_units_per_block = focal_laws_per_block * transport_units_per_law; // Producto acotado por el campo de diez bits.
    geometry.beats_per_module = samples_per_channel * 2; // 32 canales por 2 bytes divididos por 32 bytes por beat.
    geometry.shorts_per_focal_law = shorts_per_focal_law; // Entrega el salto que consumira el gestor en la fase 2.
    geometry.block_size_bytes = block_size_bytes; // Entrega el tamano que consumira DMA en la fase 2.
    return EPCIE_NONE; // Geometria valida, independientemente del soporte de arranque instalado.
} // La funcion es pura para poder probarla sin DLL, driver ni MATLAB.
inline int PCIe_DecodeBlockLaws(unsigned int encoded_units, int units_per_law, unsigned int &real_laws) // Decodifica solo el contador nominal; los indices y totales siguen siendo reales.
{ // No modifica la cabecera original recibida del hardware.
    real_laws = 0; // No conserva resultados anteriores cuando falla la validacion.
    if (encoded_units > 1023 || (units_per_law != 1 && units_per_law != 2)) return EPCIE_BlockLayout; // Rechaza formato o geometria sin inicializar.
    const unsigned int units = encoded_units + 1; // El campo guarda unidades menos uno.
    if (units % units_per_law) return EPCIE_BlockLayout; // No acepta media ley real en equipos de ocho modulos.
    real_laws = units / units_per_law; // Agrupa dos unidades consecutivas para 256; identidad para 128.
    return EPCIE_NONE; // El llamante puede comprobar la capacidad real configurada.
} // Fin de conversion comun al gestor, diagnostico y verificador.
#endif // Fin de la geometria PCIe compartida.
