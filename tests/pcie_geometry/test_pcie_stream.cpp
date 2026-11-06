#include "../../trunk/src/lib/api_pcie/src/pcie_layout.h" // Ejecuta ordenacion y limites de produccion.
#include <vector> // Reserva bloques sinteticos sin hardware.
#include <set> // Detecta doble liberacion de bloques.
#include <iostream> // Comunica fallos de forma automatizable.
#include <stdexcept> // Detiene el escenario ante el primer resultado incorrecto.
#include <cstdio> // Satisface el registro del gestor real.
#include <cstring> // Permite comparar el modo bruto sin ordenacion.
using namespace std; // Mantiene el entorno de la clase existente.
#include "generated/types.inc" // Utiliza T_block_header extraido del fuente real.
#define HEADER_MAGIC_NUMBER 0x45535546 // Firma del protocolo SITAU.
#define CEILING(x,y) (((x)+(y)-1)/(y)) // Macro usada por el gestor real.
#define ELOG(message,code) (code) // Sustituye solo la salida del logger.
#define RLOG(code) (code) // Conserva el codigo propagado.
#define SIZE_MSG_LOG 4096 // Espacio suficiente para diagnosticos del gestor.
#define TIMEOUT_DMA_BLOCKS_MS 2 // La fuente simulada no espera hardware.
#define GE_SOURCE_ACQ_BUFFER_DRV 0 // La pausa no tiene efecto sobre la fuente sintetica.
class fuse // Adaptador de la fuente DMA, sin reproducir la logica del gestor.
{ // Cada bloque permanece accesible hasta que el gestor lo devuelve.
public: // Expone solo las operaciones consumidas por CImageHandle.
    vector<vector<unsigned long long>> storage; // Alinea las cabeceras y mantiene estable cada reserva.
    vector<T_block_header *> ready; // Orden de entrega de los bloques generados.
    set<void *> outstanding; // Bloques entregados y todavia retenidos.
    size_t next = 0; // Cursor de la fuente simulada.
    int fault = 0; // Permite inyectar fallo DMA.
    int dma_get_sw_err(int) { return fault; } // El gestor debe propagar el fallo.
    int wait_for_dma_buffer(int, int) { return static_cast<int>(ready.size() - next); } // Informa bloques disponibles.
    int free_dma_buffer(void *block) { if (outstanding.erase(block) != 1) throw runtime_error("double free"); return 0; } // Detecta liberar memoria ajena o dos veces.
    int dma_buffer_leakage() { return static_cast<int>(outstanding.size()); } // Comprueba que el ultimo bloque se devuelve.
}; // Fin del adaptador de transporte.
fuse *gb_drv_handle = nullptr; // El gestor existente consulta tambien este puntero global.
int gb_pcie_buffer_n_blocks_buf = 4; // Provoca varias vueltas de la FIFO pequena.
char gb_log_msg[SIZE_MSG_LOG] = {}; // Recibe mensajes del gestor sin abrir archivos.
int PCIe_GetFreeBlocksNumber() { return 4; } // Evita pausas automaticas en la fuente sintetica.
void ThrGE_Pause(int) {} // La pausa fisica queda fuera de este test.
bool gb_pcie_enabled = true; // Activa la comprobacion real del protocolo.
int gb_check_pcie_header = 1; // No omite el verificador de secuencia.
int gb_pcie_n_focal_laws_block = 0; // Capacidad real de cada escenario.
int gb_pcie_transport_units_per_law = 0; // Factor de transporte de la geometria actual.
T_block_header gb_last_acquierd_header = {}; // Conserva la cabecera original entre bloques.
#include "generated/checker.inc" // Ejecuta el verificador real junto con el gestor.
void configure_header_check(const T_PCIeGeometry &g) // Reinicia el contrato al cambiar de equipo.
{ // Reproduce la inicializacion del arranque sin acceder al driver.
    gb_pcie_n_focal_laws_block = g.focal_laws_per_block; // Mantiene leyes reales.
    gb_pcie_transport_units_per_law = g.transport_units_per_law; // Usa uno para 128 y dos para 256.
    gb_last_acquierd_header = T_block_header(); // Elimina la cabecera del escenario anterior.
    gb_last_acquierd_header.header_struct.ch_id_last.bits.last_block = 1; // Marca el proximo bloque como inicio de adquisicion.
} // Fin del reinicio del verificador.
int PCIe_GetPtrBlocks(int *count, void **target, int) // Entrega un bloque real al gestor real.
{ // Registra propiedad hasta FreeOneImage_Pointers.
    *count = gb_drv_handle->next < gb_drv_handle->ready.size() ? 1 : 0; // Modela ausencia de datos sin inventar bloques.
    if (*count) { *target = gb_drv_handle->ready[gb_drv_handle->next++]; gb_drv_handle->outstanding.insert(*target); } // Entrega y marca el bloque.
    return 0; // Transporte simulado sin fallo.
} // Fin de la fuente DMA simulada.
#include "generated/manager.inc" // Ejecuta CImageHandle extraido de los fuentes de produccion.
void require(bool condition, const char *label) { if (!condition) throw runtime_error(label); } // Las comprobaciones funcionan tambien en Release.
short value(int law, int channel, int sample) { return static_cast<short>(law * 1000 + channel * 3 + sample * 4); } // Patron identificable por ley, canal y muestra.
void populate(fuse &driver, const T_PCIeGeometry &g, int images) // Fabrica el transporte compacto confirmado por el usuario.
{ // El ultimo bloque mantiene capacidad completa, con un contador de leyes utiles.
    const int total = images * g.focal_laws_per_image; // Numero total de leyes transmitidas.
    const int blocks = (total + g.focal_laws_per_block - 1) / g.focal_laws_per_block; // Incluye el ultimo bloque parcial.
    driver.storage.resize(blocks); // Mantiene estables los vectores interiores.
    for (int block = 0; block < blocks; ++block) // Construye las cabeceras reales.
    { // Dos o mas bloques permiten detectar el antiguo salto de 256.
        driver.storage[block].resize(static_cast<size_t>(g.block_size_bytes / 8)); // El bloque esta alineado a 32 bytes en longitud.
        T_block_header *header = reinterpret_cast<T_block_header *>(driver.storage[block].data()); // Usa el formato Windows real.
        const int first = block * g.focal_laws_per_block; // Ley global al inicio del bloque.
        const int valid = total - first < g.focal_laws_per_block ? total - first : g.focal_laws_per_block; // Cuenta leyes utiles.
        header->header_struct.magic_number = HEADER_MAGIC_NUMBER; // Firma correcta.
        header->header_struct.block_index = block; // Secuencia de bloques desde cero.
        header->header_struct.block_beats_word.bits.block_beats = g.beats_per_module - 1; // Campo codificado menos uno.
        header->header_struct.block_focal_laws_word.bits.block_focal_laws = g.transport_units_per_block - 1; // Dos unidades por ley real en 256; una en 128.
        header->header_struct.image_index = first / g.focal_laws_per_image; // Imagen de la primera ley del bloque.
        header->header_struct.image_focal_law_word.bits.image_focal_law_index = first % g.focal_laws_per_image; // Posicion inicial dentro de la imagen.
        header->header_struct.ch_id_last.bits.last_block = block == blocks - 1; // Marca solo el final de adquisicion.
        header->header_struct.image_focal_law_word.bits.image_focal_law_total = block == blocks - 1 ? valid : g.focal_laws_per_image; // Semantica actual del ultimo bloque.
        short *data = reinterpret_cast<short *>(header + 1); // Payload tras los 32 bytes.
        for (int law = 0; law < valid; ++law) // Solo inicializa leyes utiles.
            for (int channel = 0; channel < g.channel_count; ++channel) // Incluye limites entre todos los modulos.
                for (int sample = 0; sample < g.samples_per_channel; ++sample) // Intercala los 32 canales de cada modulo.
                    data[law * g.shorts_per_focal_law + (channel / 32) * g.samples_per_channel * 32 + sample * 32 + channel % 32] = value(first + law, channel, sample); // Genera valores verificables sin copiar la funcion de ordenacion.
        driver.ready.push_back(header); // Publica el bloque completo al adaptador.
    } // Fin de generacion de la adquisicion.
} // Fin del generador sintetico.
void scenario(int channels, int image_laws, int block_laws) // Repite la misma ruta para 128 y 256.
{ // Usa cinco imagenes para cruzar limites y dar varias vueltas al FIFO.
    T_PCIeGeometry g = {}; // Configuracion limpia en cada escenario.
    require(PCIe_BuildGeometry(channels, channels / 32, channels, channels, 4, image_laws, block_laws, g) == 0, "geometry"); // Usa el constructor real.
    fuse driver; // Nueva fuente sin estado de la adquisicion anterior.
    configure_header_check(g); // Prepara el verificador real para esta adquisicion.
    populate(driver, g, 5); // Genera datos con y sin ultimo bloque parcial.
    gb_drv_handle = &driver; // Conecta el adaptador con la clase real.
    CImageHandle manager(&driver, 4, g); // FIFO pequena para detectar sobrescrituras y vueltas.
    vector<short *> laws(image_laws); // Capacidad exacta de los punteros de una imagen.
    vector<short> sorted(static_cast<size_t>(g.shorts_per_focal_law) + 2, -123); // Centinelas para detectar escrituras fuera del destino.
    vector<unsigned short> map(channels); // Permutacion inversa detecta errores entre modulos.
    for (int pin = 0; pin < channels; ++pin) map[pin] = channels - 1 - pin; // Incluye canales extremos y limites 31/32 y 127/128.
    for (int image = 0; image < 5; ++image) // Consume y libera cada imagen por la interfaz real.
    { // Las leyes pueden compartir un bloque o repartirse entre varios.
        unsigned long index = 999; // Detecta indices de imagen no actualizados.
        require(manager.GetOneImage_Pointers(laws.data(), &index, nullptr) == 1 && index == image, "get image"); // Ejecuta el gestor de produccion.
        for (int law = 0; law < image_laws; ++law) // Contrasta todos los punteros de la imagen.
        { // Verifica tanto transporte bruto como ordenado.
            require(laws[law][0] == value(image * image_laws + law, 0, 0), "raw law stride"); // Detecta el salto fijo de 256 en equipos de 128.
            require(PCIe_SortLaw(laws[law], g.shorts_per_focal_law, sorted.data() + 1, g.shorts_per_focal_law, channels, 4, map.data(), map.size()) == 0, "sort"); // Ordena con el codigo real.
            for (int pin = 0; pin < channels; ++pin) // Verifica todos los canales, no solo los extremos.
                for (int sample = 0; sample < 4; ++sample) require(sorted[1 + pin * 4 + sample] == value(image * image_laws + law, map[pin], sample), "channel value"); // Oraculo independiente de la distribucion de memoria.
            require(sorted.front() == -123 && sorted.back() == -123, "write bounds"); // El destino conserva sus centinelas.
        } // Fin de comprobacion de la imagen.
        require(manager.FreeOneImage_Pointers(nullptr) == 1, "free image"); // Ejercita el recuento real de bloques retenidos.
    } // Fin de consumo.
    require(manager.LastImageRead() && driver.outstanding.empty(), "last block release"); // No quedan reservas retenidas tras el final.
    require(manager.FreeOneImage_Pointers(nullptr) == 0, "free underflow"); // No decrementa un contador unsigned por debajo de cero.
    require(manager.GetOneImage_Pointers(laws.data(), nullptr, nullptr) == 0, "end of stream"); // No espera bloques despues del ultimo.
} // Fin de escenario.
void rejected_blocks() // Ejercita los rechazos del gestor real antes de entregar punteros.
{ // Cada caso arranca con una fuente nueva.
    for (int channels : {128, 256}) // Repite los rechazos con factor uno y dos.
    for (int fault = 0; fault < 7; ++fault) // Incluye contador impar de unidades en 256.
    { // La configuracion esperada siempre sigue siendo valida.
        T_PCIeGeometry g = {}; // Instantanea independiente.
        require(PCIe_BuildGeometry(channels, channels / 32, channels, channels, 4, 1, 2, g) == 0, "reject geometry"); // Una imagen por ley.
        fuse driver; populate(driver, g, 1); gb_drv_handle = &driver; // Un bloque final con una ley valida.
        auto &header = driver.ready.front()->header_struct; // Modifica solo el campo bajo prueba.
        if (fault == 0) header.magic_number = 0; // Firma corrupta.
        if (fault == 1) header.block_beats_word.bits.block_beats += 1; // Muestras incompatibles.
        if (fault == 2) header.block_focal_laws_word.bits.block_focal_laws += 1; // Capacidad nominal incompatible.
        if (fault == 3) header.block_index = 2; // Perdida de bloques iniciales.
        if (fault == 4) header.image_focal_law_word.bits.image_focal_law_total = 0; // Final sin leyes utiles.
        if (fault == 5) header.image_focal_law_word.bits.image_focal_law_total = 3; // Mas leyes validas que la reserva.
        if (fault == 6) driver.fault = 1; // Lectura DMA fallida antes de tener un bloque.
        configure_header_check(g); // Reinicia la secuencia para cada fallo inyectado.
        CImageHandle manager(&driver, 4, g); // Ejecuta la misma validacion que hardware.
        short *law = nullptr; // Un rechazo no debe entregar este puntero.
        require(manager.GetOneImage_Pointers(&law, nullptr, nullptr) == EPCIE_BlockLayout && law == nullptr, "reject malformed block"); // Comprueba codigo y ausencia de salida.
        require(driver.outstanding.empty(), "rejected block release"); // Devuelve tambien las reservas rechazadas.
    } // Fin de fallos de cabecera y transporte.
} // Fin de regresion de errores del gestor.
int main() // Ejecutable nativo independiente, sin DLL ni hardware.
{ // La compilacion y ejecucion quedan a cargo del usuario.
    try // Un fallo informa el criterio que no se cumple.
    { // Repite 128 a 256 a 128 para detectar estado persistente.
        require(sizeof(T_block_header) == 32, "Windows header layout"); // Este harness esta destinado al compilador nativo Windows.
        for (int channels : {128, 256, 128}) { scenario(channels, 3, 2); scenario(channels, 1, 3); scenario(channels, 2, 2); } // Cruce, varias imagenes por bloque y frontera exacta.
        rejected_blocks(); // Comprueba que los errores se propagan sin entregar datos.
        cout << "PASS: production image manager, FIFO, raw/sorted 128/256" << endl; // Solo se imprime tras ejecutar el C++ real.
        return 0; // Exito automatizable.
    } // Fin de escenarios.
    catch (const exception &error) { cerr << "FAIL: " << error.what() << endl; return 1; } // No oculta errores del gestor ni de doble liberacion.
} // Fin del test de integracion sintetica.
