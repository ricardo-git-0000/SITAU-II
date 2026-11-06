#include "../../trunk/src/lib/api_pcie/src/pcie_layout.h" // Prueba la funcion de produccion sin DLL, MATLAB ni driver.
#include <vector> // Reserva mapas y datos sinteticos acotados.
#include <iostream> // Informa del resultado al ejecutar el binario de pruebas.
int main() // Ejecutable independiente que debe compilar el usuario.
{ // No utiliza assert para que las comprobaciones sigan activas en Release.
    T_PCIeGeometry geometry = T_PCIeGeometry(); // Comparte la salida para detectar configuraciones residuales tras un fallo.
    int failures = 0; // Cuenta diferencias entre el contrato y la funcion real.
    const auto check = [&failures](bool valid) { if (!valid) ++failures; }; // Registra cada comprobacion sin abortar el resto.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 3, 2, geometry) == EPCIE_NONE); // Acepta cuatro tramas y una imagen que cruza bloques.
    check(geometry.module_count == 4 && geometry.beats_per_module == 2048 && geometry.shorts_per_focal_law == 131072 && geometry.block_size_bytes == 524320); // Contrasta tamanos independientes calculados para 128.
    check(PCIe_BuildGeometry(256, 8, 256, 256, 1024, 1, 2, geometry) == EPCIE_NONE); // Permite varias imagenes dentro de un bloque.
    check(geometry.module_count == 8 && geometry.beats_per_module == 2048 && geometry.shorts_per_focal_law == 262144 && geometry.block_size_bytes == 1048608); // Contrasta tamanos de 256 sin alterar los beats por modulo.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 3, 2, geometry) == EPCIE_NONE && geometry.channel_count == 128); // Comprueba la secuencia 128 a 256 a 128 sin cache.
    check(PCIe_BuildGeometry(128, 1, 128, 128, 1024, 3, 2, geometry) == EPCIE_Geometry); // Rechaza canales multiplexados sobre un unico modulo.
    check(geometry.channel_count == 0 && geometry.block_size_bytes == 0); // No deja la geometria valida anterior tras un rechazo.
    check(PCIe_BuildGeometry(256, 4, 256, 256, 1024, 3, 2, geometry) == EPCIE_Geometry); // Detecta discrepancia entre modulos fisicos y canales.
    check(PCIe_BuildGeometry(64, 2, 64, 64, 1024, 3, 2, geometry) == EPCIE_Geometry); // Rechaza equipos fuera del alcance inicial.
    check(PCIe_BuildGeometry(256, 8, 128, 128, 1024, 3, 2, geometry) == EPCIE_Geometry); // No interpreta una sonda parcial como transporte completo.
    check(PCIe_BuildGeometry(128, 4, 128, 127, 1024, 3, 2, geometry) == EPCIE_Geometry); // Rechaza mapas resueltos incompletos.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 0, 3, 2, geometry) == EPCIE_Geometry); // Rechaza cero muestras antes de calcular tamanos.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 0, 2, geometry) == EPCIE_Geometry); // Rechaza imagenes sin leyes.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 3, 0, geometry) == EPCIE_Geometry); // Rechaza bloques con cero leyes.
    check(PCIe_BuildGeometry(128, 4, 128, 128, -1, 3, 2, geometry) == EPCIE_Geometry); // Rechaza dimensiones negativas.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 32769, 3, 1, geometry) == EPCIE_GeometrySize); // Detecta desbordamiento del campo de beats menos uno.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1, 3, 1025, geometry) == EPCIE_GeometrySize); // Detecta desbordamiento del campo de leyes menos uno.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 32768, 3, 1, geometry) == EPCIE_GeometrySize); // Una cabecera representable no garantiza que el bloque quepa en GTX.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 3, 31, geometry) == EPCIE_NONE); // Acepta el ultimo numero entero de leyes que cabe para este caso.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 1024, 3, 32, geometry) == EPCIE_GeometrySize); // Rechaza el siguiente bloque que supera la capacidad.
    check(PCIe_BuildGeometry(256, 8, 256, 256, 2147483647, 3, 2147483647, geometry) == EPCIE_GeometrySize); // Rechaza extremos de int antes de multiplicarlos.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 4, 65536, 2, geometry) == EPCIE_GeometrySize); // El contador de leyes de imagen tiene 16 bits.
    check(PCIe_BuildGeometry(256, 8, 256, 256, 4, 3, 512, geometry) == 0 && geometry.transport_units_per_block == 1024 && geometry.focal_laws_per_image == 3); // El limite de transporte no duplica la imagen.
    check(PCIe_BuildGeometry(256, 8, 256, 256, 4, 3, 513, geometry) == EPCIE_GeometrySize); // Rechaza 1026 unidades antes de truncar el campo.
    check(PCIe_BuildGeometry(128, 4, 128, 128, 4, 3, 1024, geometry) == 0 && geometry.transport_units_per_block == 1024); // Conserva el limite original de cuatro modulos.
    for (int factor : {1, 2}) // Prueba todas las combinaciones representables de la cabecera.
        for (unsigned int encoded = 0; encoded < 1024; ++encoded) // Incluye los extremos y unidades impares.
        { // Comprueba el decodificador de produccion, no una copia.
            unsigned int laws = 999; // Detecta resultados residuales tras un rechazo.
            const int result = PCIe_DecodeBlockLaws(encoded, factor, laws); // Convierte unidades menos uno a leyes reales.
            check((encoded + 1) % factor ? (result == EPCIE_BlockLayout && laws == 0) : (result == 0 && laws * factor == encoded + 1)); // Solo admite leyes completas.
        } // Fin de barrido del campo de diez bits.
    unsigned int invalid_laws = 99; // Verifica entradas que no proceden de un campo binario valido.
    check(PCIe_DecodeBlockLaws(1024, 2, invalid_laws) == EPCIE_BlockLayout && invalid_laws == 0); // No acepta valores de once bits.
    check(PCIe_DecodeBlockLaws(3, 0, invalid_laws) == EPCIE_BlockLayout && invalid_laws == 0); // No divide con geometria sin inicializar.
    for (int channels : {128, 256}) // Comprueba las dos topologias con datos y mapas reales.
    { // Los buffers tienen una ley de cuatro muestras por canal.
        check(PCIe_BuildGeometry(channels, channels / 32, channels, channels, 4, 3, 2, geometry) == 0); // Reutiliza geometria valida.
        std::vector<unsigned short> map(channels); // Mapa consecutivo inicial.
        for (int channel = 0; channel < channels; ++channel) map[channel] = channel; // Incluye todos los canales una vez.
        check(PCIe_ValidateMap(map.data(), map.size(), channels) == 0); // Acepta consecutivo.
        check(PCIe_ValidateMap(map.data(), map.size() - 1, channels) == EPCIE_ChannelMap); // Detecta copia incompleta.
        map.back() = 0; // Introduce un duplicado sin salir de rango.
        check(PCIe_ValidateMap(map.data(), map.size(), channels) == EPCIE_ChannelMap); // Rechaza duplicados completos.
        map.back() = channels; // Introduce un indice fuera de rango.
        check(PCIe_ValidateMap(map.data(), map.size(), channels) == EPCIE_ChannelMap); // No lo recorta al ultimo canal.
        std::vector<short> source(channels * 4, 7), target(channels * 4, -123); // Destino centinela para comprobar rechazo antes de escribir.
        check(PCIe_SortLaw(source.data(), source.size(), target.data(), target.size(), channels, 4, map.data(), map.size()) == EPCIE_ChannelMap && target.front() == -123 && target.back() == -123); // Un mapa invalido no deja una imagen parcialmente ordenada.
        check(PCIe_SortLaw(source.data(), source.size() - 1, target.data(), target.size(), channels, 4, nullptr, 0) == EPCIE_BlockLayout); // Rechaza origen corto.
        check(PCIe_SortLaw(source.data(), source.size(), target.data(), target.size() - 1, channels, 4, nullptr, 0) == EPCIE_BlockLayout); // Rechaza destino corto.
        check(PCIe_SortLaw(source.data(), source.size(), target.data(), target.size(), channels, 4, nullptr, 0) == 0 && target.back() == 7); // Mapa omitido usa orden consecutivo.
        std::size_t offset = 999; // Detecta desplazamientos residuales tras error.
        check(PCIe_LawOffset(geometry, geometry.block_size_bytes, 2, 1, offset) == 0 && offset == geometry.shorts_per_focal_law); // Segunda ley de un bloque normal.
        check(PCIe_LawOffset(geometry, geometry.block_size_bytes, 1, 1, offset) == EPCIE_BlockLayout && offset == 0); // No entrega relleno de un bloque final parcial.
        check(PCIe_LawOffset(geometry, geometry.block_size_bytes - 2, 2, 1, offset) == EPCIE_BlockLayout); // Detecta un bloque truncado por una muestra.
        check(PCIe_LawOffset(geometry, geometry.block_size_bytes, 0, 0, offset) == EPCIE_BlockLayout); // Rechaza ultimo bloque sin leyes utiles.
        std::vector<short> saw(static_cast<std::size_t>(geometry.shorts_per_focal_law) * 2); // Dos leyes detectan saltos de modulo y ley.
        std::vector<unsigned long long> errors(channels); // Capacidad exacta de contadores.
        for (int law = 0; law < 2; ++law) // Construye el patron incremental esperado.
            for (int channel = 0; channel < channels; ++channel) // Incluye todos los modulos.
                for (int sample = 0; sample < 4; ++sample) saw[law * geometry.shorts_per_focal_law + channel / 32 * 128 + sample * 32 + channel % 32] = static_cast<short>(channel + sample * 4); // Incremento de cuatro con formato compacto.
        check(PCIe_CheckSaw(saw.data(), saw.size(), geometry, 2, errors.data(), errors.size()) == 0); // Diagnostico real sobre dos leyes.
        for (auto error : errors) check(error == 0); // No inventa discontinuidades en un patron correcto.
        saw[32] += 1; // Corrompe una muestra del primer canal.
        check(PCIe_CheckSaw(saw.data(), saw.size(), geometry, 2, errors.data(), errors.size()) == 0 && errors[0] > 0); // Detecta el defecto en su canal.
        check(PCIe_CheckSaw(saw.data(), saw.size(), geometry, 2, errors.data(), errors.size() - 1) == EPCIE_BlockLayout); // No escribe fuera de los contadores.
    } // Fin de pruebas de mapas, capacidades y diagnostico.
    std::cout << (failures == 0 ? "PASS" : "FAIL") << ": PCIe geometry; failures=" << failures << std::endl; // Resume las pruebas del C++ real.
    return failures == 0 ? 0 : 1; // Permite integrar el ejecutable en una futura automatizacion.
} // No realiza adquisicion ni modifica configuraciones del equipo.
