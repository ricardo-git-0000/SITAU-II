#ifndef pcie_layoutH // Protege las operaciones puras compartidas por DMA, gestor y pruebas.
#define pcie_layoutH // No depende de Windows ni del driver.
#include "pcie_geometry.h" // Usa las mismas unidades y limites de adquisicion.
#include <cstddef> // Expresa capacidades reales de buffers y mapas.
inline int PCIe_ValidateMap(const unsigned short *map, std::size_t map_size, std::size_t channels) // Exige una permutacion completa para PCIe paralelo.
{ // Valida todo antes de escribir datos de salida.
    if (!map || (channels != 128 && channels != 256) || map_size != channels) return EPCIE_ChannelMap; // No acepta mapas cortos ni sondas parciales.
    bool seen[256] = {}; // Marca canales fisicos ya asignados sin reservar memoria dinamica.
    for (std::size_t pin = 0; pin < channels; ++pin) // Recorre exactamente la capacidad comprobada.
    { // Un canal fisico solo puede aparecer una vez.
        if (map[pin] >= channels || seen[map[pin]]) return EPCIE_ChannelMap; // Rechaza indices fuera de rango y duplicados.
        seen[map[pin]] = true; // Registra la asignacion valida.
    } // La longitud y unicidad garantizan que no faltan canales.
    return EPCIE_NONE; // El mapa se puede consumir completo.
} // Fin de validacion del mapa.
inline int PCIe_SortLaw(const short *source, std::size_t source_size, short *target, std::size_t target_size, std::size_t channels, std::size_t samples, const unsigned short *map, std::size_t map_size) // Ordena una ley sin recortar indices.
{ // Las capacidades se expresan en muestras de 16 bits.
    if (!source || !target || !samples || samples > 32768 || (channels != 128 && channels != 256)) return EPCIE_Geometry; // Acota el producto antes de calcularlo.
    const std::size_t law_size = channels * samples; // Producto acotado por el contrato.
    if (source_size < law_size || target_size < law_size) return EPCIE_BlockLayout; // No lee ni escribe una ley truncada.
    if (map && PCIe_ValidateMap(map, map_size, channels) < 0) return EPCIE_ChannelMap; // Valida antes de tocar el destino.
    for (std::size_t pin = 0; pin < channels; ++pin) // El destino agrupa todas las muestras de cada pin.
    { // El origen contiene modulos consecutivos y 32 canales intercalados por muestra.
        const std::size_t channel = map ? map[pin] : pin; // Un mapa omitido equivale al orden consecutivo.
        for (std::size_t sample = 0; sample < samples; ++sample) // Copia todas las muestras del canal.
            target[pin * samples + sample] = source[(channel / 32) * samples * 32 + sample * 32 + channel % 32]; // Traduce modulo, muestra y canal local.
    } // No sustituye entradas invalidas por el ultimo canal.
    return EPCIE_NONE; // Ley ordenada completamente.
} // Fin de ordenacion comprobable sin hardware.
inline int PCIe_LawOffset(const T_PCIeGeometry &geometry, std::size_t block_bytes, unsigned int valid_laws, unsigned int law_index, std::size_t &offset_shorts) // Comprueba cada puntero antes de publicarlo.
{ // La capacidad incluye la cabecera de 32 bytes.
    offset_shorts = 0; // Un fallo no conserva el desplazamiento anterior.
    if (!valid_laws || valid_laws > static_cast<unsigned int>(geometry.focal_laws_per_block) || law_index >= valid_laws || !geometry.shorts_per_focal_law) return EPCIE_BlockLayout; // Limita leyes normales y ultimo bloque parcial.
    const std::uint64_t end_bytes = 32ULL + (law_index + 1ULL) * geometry.shorts_per_focal_law * 2ULL; // Incluye toda la ley solicitada.
    if (end_bytes > block_bytes || block_bytes > geometry.block_size_bytes) return EPCIE_BlockLayout; // Rechaza capacidad insuficiente o ajena a esta adquisicion.
    offset_shorts = static_cast<std::size_t>(law_index * geometry.shorts_per_focal_law); // El limite GTX hace representable el resultado en 32 bits.
    return EPCIE_NONE; // Desplazamiento relativo al inicio de los datos.
} // Fin del calculo de punteros.
inline int PCIe_CheckSaw(const short *data, std::size_t data_size, const T_PCIeGeometry &geometry, unsigned int valid_laws, unsigned long long *errors, std::size_t errors_size) // Diagnostico con capacidades explicitas.
{ // El patron heredado incrementa cada canal en cuatro por muestra.
    std::size_t offset = 0; // Reutiliza los mismos limites que el gestor.
    if (!data || !errors || errors_size < static_cast<std::size_t>(geometry.channel_count) || PCIe_LawOffset(geometry, 32ULL + data_size * 2ULL, valid_laws, valid_laws ? valid_laws - 1 : 0, offset) < 0) return EPCIE_BlockLayout; // Comprueba el bloque completo antes de recorrerlo.
    for (unsigned int law = 0; law < valid_laws; ++law) // No procesa relleno del ultimo bloque.
        for (int channel = 0; channel < geometry.channel_count; ++channel) // Usa los canales efectivos, nunca ocho modulos fijos.
            for (int sample = 1; sample < geometry.samples_per_channel; ++sample) // Compara con la muestra anterior del mismo canal.
            { // La aritmetica unsigned conserva el wrap de 16 bits.
                const std::size_t index = law * geometry.shorts_per_focal_law + (channel / 32) * geometry.samples_per_channel * 32 + sample * 32 + channel % 32; // Localiza la muestra en el transporte compacto.
                if (static_cast<unsigned short>(data[index]) != static_cast<unsigned short>(static_cast<unsigned short>(data[index - 32]) + 4)) ++errors[channel]; // Acumula discontinuidades por canal.
            } // Termina el recorrido acotado.
    return EPCIE_NONE; // Los contadores contienen las discontinuidades detectadas.
} // Fin del diagnostico sin accesos al driver.
#endif // Fin de las operaciones de distribucion PCIe.
