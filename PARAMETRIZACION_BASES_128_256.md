# Parametrización de SITAU para 128 y 256 canales

## Objetivo

Hacer que el proyecto funcione en modo paralelo con las siguientes configuraciones:

| `BASES_NUMBER` | Bases FPGA | Canales por base | Canales totales |
|---:|---:|---:|---:|
| 4 | 4 | 32 | 128 |
| 8 | 8 | 32 | 256 |

El valor de configuración se obtiene de `SITAU.XML` mediante el parámetro:

```xml
<Parameter pName="BASES_NUMBER" pValue="4"/>
```

o:

```xml
<Parameter pName="BASES_NUMBER" pValue="8"/>
```

En esta etapa no se está parametrizando el modo multiplexado.

## Convenciones importantes

- `XML_DEF_N_BASES` es el valor predeterminado interno. Actualmente vale `4`.
- `BASES_NUMBER` es el valor efectivo leído desde el XML.
- `MOD_MAX_CH` es una constante existente del proyecto y vale `32`.
- En modo paralelo, los canales totales se calculan como:

```text
canales_totales = BASES_NUMBER * MOD_MAX_CH
```

- `n_fl` representa el número de leyes focales o bloques de datos de una adquisición y no el número de canales.

## Cambios realizados

### Fase 1 — Lectura, validación y almacenamiento de `BASES_NUMBER`

Fichero modificado:

```text
trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp
```

Cambios:

1. `Update()` ahora comprueba el resultado de `GetData_HardwareConfiguration()`.
2. `GetData_HardwareConfiguration()` acepta únicamente `BASES_NUMBER=4` o `BASES_NUMBER=8`.
3. El valor leído se almacena en `data_.sitau_hardware.bases_number`.
4. El vector `data_.sitau_hardware.base` se redimensiona con el número de bases seleccionado.
5. Un valor diferente de 4 u 8 genera `EHW_BASE_Number`.
6. `ResetData()` inicializa `bases_number` con `XML_DEF_N_BASES` antes de leer el XML.

Motivo:

Antes se aplicaba una corrección mínima para valores menores que el valor predeterminado, pero no se validaba explícitamente el conjunto de configuraciones soportadas ni se propagaba el error al flujo principal.

### Fase 2 — Valores predeterminados del frontend y del mapa de canales

Fichero modificado:

```text
trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp
```

Cuando el XML no contiene un bloque `FRONTEND`, `Update()` crea ahora un frontend coherente con `BASES_NUMBER`:

- `n_total_channels = bases_number * MOD_MAX_CH`.
- `n_channels` toma el número total calculado.
- `n_multiplexed_channels` se fija a cero para indicar modo paralelo.
- `id_base` se dimensiona con `bases_number`.
- `pin_to_channel` se dimensiona con el número total de canales.
- `pin_disabled` se dimensiona con el número total de canales.
- El mapa de canales se inicializa secuencialmente.

También se actualizaron los valores iniciales de `ResetData()` para que no partan de 256 canales ni de multiplexado:

- `n_multiplexed_channels = 0`.
- `n_channels = XML_DEF_N_BASES * MOD_MAX_CH`.
- El conector usa el mismo cálculo para `n_total_channels` y sus vectores.

Motivo:

El parser limpia la lista de frontends. Cuando el XML no define uno, se crea una estructura nueva. El código anterior fijaba cuatro bases y varios tamaños a 256, independientemente de `BASES_NUMBER`.

### Fase 3 — Inicialización de módulos en `CPASystem`

Fichero modificado:

```text
trunk/src/lib/stlib2/src/HW/c_pa_system.cpp
```

En `CPASystem::Setup()` se inicializan los campos que antes estaban comentados:

```cpp
sys_.n_modules_base = 1;
sys_.n_modules = sys_.n_bases * sys_.n_modules_base;
sys_.id_mod_first = sys_.id_base_first;
sys_.id_mod_last = sys_.id_mod_first + sys_.n_modules - 1;
```

Interpretación actual:

- Cada base FPGA representa un módulo de 32 canales.
- Para 4 bases se obtienen 4 módulos.
- Para 8 bases se obtienen 8 módulos.
- En multiplexado, el camino existente deja `n_bases=1`; por tanto se conserva un módulo lógico.

Motivo:

`n_modules`, `id_mod_first` e `id_mod_last` se consultaban posteriormente desde el sistema de canales virtuales, pero sus asignaciones estaban comentadas. Eso podía producir valores no inicializados.

### Fase 4 — Corrección de mensajes UCI extendidos

Ficheros modificados:

```text
trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp
trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_fl_pa.cpp
trunk/src/lib/stlib2/src/HW/UCI/c_log_protocol.cpp
trunk/src/lib/stlib2/src/HW/UCI/c_log_protocol.h
```

El protocolo ya dispone de mensajes extendidos con capacidad para transportar un segundo bloque de hasta cuatro bases. Para una configuración de 8 bases, el primer mensaje transporta las bases `0..3` y el mensaje extendido transporta las bases `4..7`.

Se corrigieron los recorridos para que:

- El segundo bloque solo recorra las bases restantes.
- Las bases físicas `4..7` se almacenen en las posiciones internas `0..3` del mensaje extendido.
- No se escriba en índices `4..7` de arrays cuya capacidad es cuatro.
- El cálculo de `t0_max` del mensaje extendido use únicamente las bases incluidas en ese mensaje.
- TFM backward también envíe un segundo mensaje para las bases que superen las cuatro primeras.
- El logger almacene los mensajes extendidos en una estructura del mismo tipo, evitando copiar más bytes de los que admite el destino.

No se ha cambiado todavía el tamaño binario de `MAX_N_BASES_MSG` ni de las estructuras UCI. Ese cambio requiere validar previamente la compatibilidad exacta con el firmware, porque modificar el tamaño de una estructura altera el tamaño del mensaje enviado por TCP.

### Fase 5 — TFM backward y logger UCI

Se completó el uso del mensaje extendido para TFM backward y se corrigió el almacenamiento de mensajes extendidos en el logger.

En TFM backward:

- El mensaje principal transporta las primeras cuatro bases.
- `TMSG_BeamformerExtended` transporta las bases restantes.
- Las bases restantes se almacenan en las posiciones internas `0..3` del mensaje extendido.

En el logger se añadió una estructura específica para `TMSG_BeamformerExtended`. Antes se copiaba ese mensaje en una estructura `TMSG_Beamformer`, lo que podía sobrescribir memoria fuera del objeto destino.

### Fase 6 — Análisis de TFM forward

No se ha aplicado un cambio de código en esta fase por una limitación de compatibilidad que requiere confirmación del firmware.

El mensaje `TMSG_TFM_FMC_Forward` no tiene actualmente una variante extendida. Además:

- La memoria interna `TFM_FMC_FORWARD_MEMORY_SIZE` se reserva usando `MAX_N_BASES=8`.
- El mensaje UCI usa `TFM_FMC_FORWARD_MEMORY_SIZE_MSG`, calculado usando `MAX_N_BASES_MSG=4`.
- `TFM_SET_FMC_Forward()` copia al mensaje únicamente el tamaño correspondiente a cuatro bases.

Por tanto, ampliar directamente `MAX_N_BASES_MSG` o la estructura `TMSG_TFM_FMC_Forward` modificaría el tamaño de la trama TCP y podría hacer incompatible la DLL con el firmware actual.

Opciones pendientes, que deben validarse con el firmware:

1. Confirmar que el firmware de 256 canales espera una trama TFM FMC de ocho bases y ampliar ambos lados de forma coordinada.
2. Crear un comando/mensaje extendido específico para TFM FMC, si el firmware lo admite.
3. Mantener TFM FMC limitado a cuatro bases y documentar esa limitación mientras se parametrizan PA, adquisición general y TFM PWI.

### Fase 7 — Auditoría de la capa PCIe

La capa `api_pcie` mantiene actualmente el número de módulos como una constante de compilación:

```text
trunk/src/lib/api_pcie/src/pcie_tad.h
```

```cpp
#define N_MODULES 8
```

Los usos relevantes se encuentran en:

```text
trunk/src/lib/api_pcie/src/c_image_handle.cpp
trunk/src/lib/api_pcie/src/pcie.cpp
trunk/src/lib/api_pcie/src/pcie.h
```

La constante interviene en:

- El tamaño de datos por ley focal en `CImageHandle`.
- El array de comprobación de canales.
- El bucle que recorre los módulos recibidos por PCIe.
- El cálculo del offset de canal dentro de cada módulo.

Resultado de la auditoría:

- La DLL PCIe no recibe actualmente `BASES_NUMBER` desde `CXML_SITAU_HW`.
- `stlib2` y `api_pcie` no comparten una interfaz dinámica para el número de bases.
- `N_CHANNELS_MODULE=32` es coherente con la arquitectura y no debe cambiarse.
- Sustituir directamente `N_MODULES=8` por una variable global no sería suficiente: habría que garantizar que los tamaños de los bloques DMA, el image manager y las comprobaciones de canales usan exactamente el mismo valor.
- Cambiar la estructura o el tamaño de los bloques DMA sin confirmar el formato producido por el firmware podría romper la adquisición PCIe.

No se ha aplicado todavía ningún cambio de código en `api_pcie`. La siguiente implementación deberá introducir una interfaz explícita para pasar el número de canales/bases a la capa PCIe y mantener un valor predeterminado compatible de 256 canales.

## Estado actual

Completado:

- Lectura y validación de `BASES_NUMBER`.
- Almacenamiento explícito del número de bases.
- Cálculo de canales paralelos para 128/256.
- Dimensionamiento del frontend fallback.
- Inicialización de módulos y sus identificadores.
- Comentarios explicativos en las líneas modificadas.
- Corrección del direccionamiento de bases en mensajes UCI extendidos.
- TFM backward utilizando el mensaje UCI extendido.
- Corrección del almacenamiento de mensajes extendidos en el logger.
- Inicialización TCP dinámica de la sonda y del límite de emisión TFM según el número real de canales.
- Elementos de sonda automáticos cuando el XML omite probe_elements_number.
- Gráficas de Noise adaptadas a 128 y 256 canales.
- Compilación de stlib2 Release x64 y 12 pruebas de carga de configuración sin hardware superadas (16/10/2026).

Pendiente:

1. Confirmar con el firmware el formato de TFM FMC forward para ocho bases.
2. Diseñar la interfaz para propagar `BASES_NUMBER` desde `stlib2` hasta `api_pcie`.
3. Revisar las estructuras y tamaños fijos del protocolo PCIe.
4. Revisar buffers, tamaños de transferencia y memoria asociados al número de canales.
5. Completar la revisión de MATLAB y de los mapas de cableado de las configuraciones de prueba. Noise y los XML Noise/RF ya incluyen los ajustes descritos abajo.
6. Desplegar la nueva DLL de stlib2 en MATLAB. El usuario confirmó la compilación de ambos proyectos antes del último cambio; el último cambio solo requiere actualizar stlib2.
7. Completar las pruebas de adquisición RF y las pruebas con hardware en 128 y 256 canales. Las pruebas automáticas del último cambio validan carga y dimensiones, no adquisición.
8. Ampliar la regresión existente de carga para cubrir adquisición y protocolo físico; el script temporal actual se encuentra en tmp_probe_test/test_probe.ps1.

## Fase TCP: inicialización dinámica de sonda y TFM

### `trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp`

- `cfg_.pa_probe` se crea con `VCH_MAX_LINES` únicamente como capacidad inicial necesaria para construir el objeto.
- Después de `HardwareSetup()`, se obtiene `GET_ChannelsNumber()` y se reajusta la sonda a ese número. Así, `BASES_NUMBER=4` parte de 128 elementos y `BASES_NUMBER=8` de 256.
- El valor explícito de `probe_elements_number` del XML sigue teniendo prioridad durante `LoadXMLConfigData()`; `LOAD_PAConfig()` lo limita además al máximo de canales disponibles.
- El límite TFM por defecto se inicializa con `n_channels - 1`.

### `trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_load.cpp`

- `LOAD_DefaultConfig()` ya no fuerza el final de emisión TFM a `127`.
- Utiliza `n_ch - 1`, evitando que la configuración de 256 canales herede el límite de una configuración fija de 128.
- No se han cambiado estructuras de protocolo ni registros enviados al firmware.

## Nota sobre la compilación

En las fases iniciales no se compiló el proyecto. Posteriormente, el usuario confirmó que compiló ambos proyectos y que Noise funcionaba sin hardware con 256 y 128 canales. El 16/10/2026, el asistente compiló el último cambio de stlib2 en Release x64 con MSVC v143 de Community 2022 y ejecutó las pruebas descritas a continuación. La DLL de pruebas no se ha copiado automáticamente a MATLAB.

## Archivos locales no relacionados

Antes de iniciar este trabajo ya existían cambios locales en ejemplos MATLAB, logs, documentación PDF y el ZIP de DLL. Se conservan esos cambios. Las modificaciones posteriores del asistente en Demo_GE_Noise.m y en los XML Noise/RF se detallan a continuación; la selección emulates_sitau2=true ya estaba hecha por el usuario antes de ajustar las gráficas.

## Actualización 16/10/2026 — MATLAB y elementos de sonda automáticos

### Gráficas de ruido para ambas configuraciones

Archivo: `trunk/src/app/MatLab/Examples/Demo_GE_Noise.m`, líneas 99–107.

- Se sustituyeron los ejes fijos `1:256` por `1:n_channels` en las barras de pico y RMS.
- Los límites horizontales y las líneas de referencia utilizan también `n_channels`.
- Esto corrige el error «X must be same length as Y» al recibir 128 valores.
- Cada línea modificada incluye un comentario. No se modificó la adquisición.
- Prueba realizada en MATLAB R2024b con matrices sintéticas de 128 y 256 canales: ambas gráficas se generaron correctamente. El usuario confirmó después que Noise funcionaba con 128 canales.

### Número de elementos opcional en el XML

Archivo: `trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_config.cpp`, función `CVirtualChannel::LoadXMLConfigData`, antes de `LOAD_PAConfig()`.

1. Se consulta `probe_elements_number.Update()` para distinguir un parámetro presente de uno omitido. Antes, al omitirlo, permanecía el valor predeterminado del parser de 32 elementos.
2. Si se omite, se obtiene el número efectivo mediante `GET_ChannelsNumber()`. En las configuraciones paralelas tratadas aquí equivale a `BASES_NUMBER * MOD_MAX_CH`, con 32 canales por base.
3. Si el resultado es cero o negativo, se devuelve `EVCH_DataInconsistency`.
4. Se escribe el valor resuelto con `Value(...)`, sin marcar el parámetro como explícito. `LOAD_PAConfig()` y la comprobación de longitud de `pin_to_channel` reciben así el mismo número.
5. Si el XML incluye explícitamente el parámetro, se conserva el comportamiento anterior, incluido el límite al máximo de canales aplicado por `LOAD_PAConfig()`.

Se eligió consultar los canales efectivos del equipo para reutilizar la configuración ya validada y evitar duplicar el cálculo de bases en el cargador de la sonda. El cambio no amplía el alcance de validación al modo multiplexado.

Archivos de ejemplo actualizados:

- `trunk/src/app/MatLab/Examples/SITAU2_GE_NOISE.XML`.
- `trunk/src/app/MatLab/Examples/SITAU2_GE_RF.XML`.

En ambos se sustituyó la línea `probe_elements_number` por un comentario explicando su cálculo automático. Para seleccionar el número de elementos de estas demos ahora basta con cambiar `BASES_NUMBER` en el `SITAU.XML` de la carpeta usada por `OpenSys` y volver a abrir el sistema. `pixels_x` sigue describiendo la imagen y no necesita coincidir con los canales.

### Compilación y entrega

- Compilación realizada por el asistente: `stlib2`, `Release | x64`, MSVC v143, Visual Studio Community 2022.
- Resultado: MSBuild terminó con código 0.
- DLL generada: `tmp_probe_test/bin/stlib2_x64.dll`.
- Objetos intermedios: `tmp_probe_test/obj/`.
- Se desactivaron las copias de postcompilación y se usaron directorios separados para no reemplazar las DLL usadas por MATLAB.
- Para usar el cambio, cerrar MATLAB, conservar una copia de la DLL anterior y sustituir la `stlib2_x64.dll` que carga MATLAB por la nueva. No cambió la interfaz exportada y este ajuste no requiere recompilar `api_pcie`.
- Alternativamente, el usuario puede recompilar `stlib2` en `Release | x64` desde su solución y desplegar esa DLL.

### Regresión del último cambio

Script: `tmp_probe_test/test_probe.ps1`. Ejecuta la DLL compilada mediante P/Invoke desde Windows PowerShell, con `ST_SetNoHardware(1)` y copias temporales de la carpeta BIN. No cambia el `SITAU.XML` usado por MATLAB.

Se verificaron 12 cargas: 2 configuraciones de bases × 2 XML de demo × 3 estados consecutivos de elementos.

| Bases | XML | Secuencia solicitada | Elementos obtenidos |
|---|---|---|---|
| 4 | Noise | Omitido → 128 explícitos → omitido | 128 → 128 → 128 |
| 4 | RF | Omitido → 128 explícitos → omitido | 128 → 128 → 128 |
| 8 | Noise | Omitido → 128 explícitos → omitido | 256 → 128 → 256 |
| 8 | RF | Omitido → 128 explícitos → omitido | 256 → 128 → 256 |

Todas estas cargas devolvieron éxito y los getters coincidieron con los valores esperados. La ejecución final conservó los mapas originales de las demos. Estas pruebas verifican apertura, carga y número de canales/elementos; no ejecutan adquisición ni validan el transporte TCP o el cableado físico.

### Límites y observaciones

- `pin_to_channel` sigue siendo independiente: representa el cableado, no solo el número de canales. Los mapas de las demos contienen 128 entradas. Con 256 elementos, la comprobación existente registra `ES2_RANGO` y no aplica ese mapa; que la carga complete no valida un mapa físico de 256 canales.
- El mapa secuencial predeterminado no corrige valores erróneos de un vector cuya longitud sí coincide. El cableado debe verificarse antes de las pruebas con hardware.
- Una prueba exploratoria con 64 elementos explícitos devolvió `-1501`. El parser mantiene `tfm_emission_end_element=127` como valor predeterminado y la carga configura TFM también. No se modificó ese comportamiento en esta tarea; sondas menores pueden requerir ajustar además los parámetros TFM.
- Las pruebas del último cambio son de simulación. Continúan pendientes las verificaciones con hardware y las limitaciones de TFM FMC forward/PCIe descritas en las fases anteriores.

## Actualización 16/10/2026 — Comparación de DLL y ampliación del mapa

### Diferencia de tamaño confirmada

Se inspeccionaron los binarios con `dumpbin`, sin compilar:

| DLL | Bytes | Configuración identificada |
|---|---:|---|
| `trunk/src/lib/api_pcie/src/stlib2/stlib2_x64.dll` | 4.591.616 | Debug: referencia a PDB en `bin/x64/Debug`, dependencias `MSVCP140D.dll`, `VCRUNTIME140D.dll` y `ucrtbased.dll` |
| `tmp_probe_test/bin/stlib2_x64.dll` | 3.347.968 | Release: dependencias `MSVCP140.dll` y `VCRUNTIME140.dll` |

Ambas indican linker 14.44 y tienen los mismos 321 nombres de funciones exportadas. La copia de Examples tiene el mismo tamaño que la DLL Release. La comparación explica la reducción por las diferentes configuraciones de compilación; no se eliminaron funciones exportadas. Debug tiene la optimización desactivada en el proyecto y bibliotecas de depuración. El proyecto también diferencia `_NO_HW` en Debug de `n_NO_HW` en Release; para las pruebas debe configurarse explícitamente `emulates_sitau2` según corresponda. No debe atribuirse la reducción a las seis líneas del cambio de elementos automáticos.

### Ampliación automática de pin_to_channel

Archivo: `trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_config.cpp`, dentro de `LoadXMLConfigData()` y antes de validar/asignar el mapa de la sonda.

- Si la sonda efectiva tiene 256 elementos y el XML aporta 128 entradas, se comprueba que estas forman una permutación de los canales 0 a 127 (sin duplicados ni valores fuera de rango).
- Si el patrón es válido, se conserva la primera mitad y se construye la segunda con `map[i + 128] = map[i] + 128`.
- El desplazamiento es **128**, no 127: sumar 127 duplicaría el canal 127 y dejaría sin asignar el canal 255.
- Para 128 elementos se conserva el mapa original. Un mapa explícito de 256 entradas también se conserva.
- Las demás longitudes y los patrones inválidos siguen la comprobación de dimensiones y el registro de error existentes. No se ha implementado todavía un warning visible en MATLAB ni una validación general de todos los mapas completos.
- Los dos XML Noise/RF mantienen el patrón original de 128 entradas y ahora incluyen un comentario sobre su ampliación. El archivo XML no se reescribe durante la carga; solo se resuelve el mapa en memoria.

Se eligió ampliar únicamente el caso conocido de 128 a 256, evitando interpretar cualquier mapa incompleto como un patrón repetible. Esta regla incorpora el cableado repetido indicado por el usuario; todavía requiere validación física.

### Verificaciones y compilación pendiente

- Se leyeron los patrones reales de Noise y RF y se verificó que ambos tienen 128 entradas únicas, con rango 0..127.
- Se comprobó mediante PowerShell que su ampliación propuesta contiene 256 entradas únicas, con rango 0..255, preservando el orden de ambas mitades y el desplazamiento de 128.
- `git diff --check` sobre el C++ modificado terminó sin errores.
- Estas comprobaciones verifican los datos y la regla matemática, **no ejecutan el C++ nuevo**. Las 12 pruebas de DLL documentadas antes corresponden al cambio anterior de elementos automáticos.
- Siguiendo la nueva instrucción del proyecto, el asistente **no ha compilado esta ampliación**. El usuario debe recompilar **stlib2 en Release | x64 con v143** y sustituir la DLL que carga MATLAB. No es necesario recompilar api_pcie por este cambio.
- La DLL de `tmp_probe_test/bin` sigue siendo la del cambio anterior: **todavía no contiene la ampliación de pin_to_channel**.
- Después de compilar, probar Noise y RF sin hardware con 4 y 8 bases y comprobar el mapa efectivo; después validar el orden de canales con hardware.

### Próxima iteración: errores y warnings

Pendiente presentar en MATLAB un aviso claro cuando la longitud del mapa no coincida con el número efectivo de elementos, indicando longitud recibida, longitud esperada y decisión tomada. Diferenciar el patrón válido de 128 ampliado a 256 (caso admitido) de los mapas realmente inconsistentes. Revisar también duplicados y valores fuera de rango en mapas completos, y acordar si se rechaza la configuración o se conserva un mapa anterior; no ocultar el fallo mediante un mapa secuencial silencioso.

### Referencia XML comentada de 256 canales

Se añadió en los XML SITAU2_GE_NOISE.XML y SITAU2_GE_RF.XML el parámetro pin_to_channel completo de 256 entradas dentro de un comentario XML, junto al mapa activo de 128. La segunda mitad reproduce la primera desplazada en +128. Se verificaron el parseo XML, la conservación de un único mapa activo sin cambios y las 256 entradas únicas con el patrón esperado. Este añadido es documental y no requiere compilación; sigue pendiente compilar el cambio C++ de ampliación indicado arriba.

## Actualización 19/10/2026 — Descripciones de errores y validación del mapa

### Mensajes y catálogo

- Se revisaron `doc/SITAU-II SDK r4.pdf` y `doc/Config File Format r8.pdf`. El SDK documenta `ST2_GetError`, pero no contiene una tabla completa de códigos: las correspondencias se obtuvieron de las constantes y descripciones del código fuente.
- Se documentaron 268 descripciones (248 números distintos) en `doc/CODIGOS_ERROR.md`. Se completaron los traductores de errores, incluidas las familias de XML, encoders, bases, hardware y PCIe, y se corrigieron dos descripciones de acceso que devolvían por error el texto de líneas inválidas.
- `ST2_GetError` consulta los catálogos SITAU y PCIe. Los números -101 a -119 tienen dos significados heredados y muestran ambas posibilidades, identificadas como TCP y PCIe. El -3003 también tiene una descripción combinada. No se renumeraron errores existentes.
- El nuevo ayudante MATLAB `ST2_ErrorText.m` consulta el código recibido y produce `codigo: descripcion`. Se actualizaron los mensajes de 125 funciones del toolbox y el mensaje de adquisición de Demo_GE_RF (que utilizaba una variable incorrecta). Los códigos desconocidos se identifican expresamente y conservan su número.
- La aplicación de consola `pcie_vc` también traduce sus mensajes numéricos mediante `ST2_GetError`.

### Nuevo error -1523

- `EVCH_PinToChannelSize = -1523` indica que la longitud del mapa explícito y activo no coincide con los elementos efectivos de la sonda. Para las demos con sonda completa, son `BASES_NUMBER * 32`: 128 con 4 bases y 256 con 8.
- Se conserva la excepción admitida: un patrón válido de 128 para una sonda de 256 se amplía con +128 antes de comprobar su longitud.
- Un mapa omitido o el modo consecutivo (`pin_to_channel_assignment=1`) no se rechazan por el vector predeterminado del parser. Un mapa explícito vacío sí se comprueba.
- La carga devuelve el error en lugar de registrar el desajuste y continuar. Se libera el canal temporal rechazado. Esto no implementa una restauración transaccional de todos los ajustes globales: después de una carga fallida, corregir el XML y volver a abrir el sistema antes de adquirir.
- La comprobación utiliza los elementos efectivos para conservar la posibilidad existente de una sonda explícitamente menor que los canales disponibles. «Bloques» se interpreta aquí como bases de 32 canales, no como bloques de adquisición.
- Sigue pendiente una validación general de duplicados e índices fuera de rango en mapas completos. El nuevo código comprueba dimensiones; no certifica el cableado físico.

### Pruebas realizadas y pendientes

- `tests/error_reporting/test_error_catalog.py`: comprobación estática de las 268 descripciones, rutas de traducción, propagación del nuevo error, unicidad de -1523 y colisiones heredadas. Resultado: correcto.
- `tests/error_reporting/test_error_reporting.m`: 8 escenarios ejecutados en MATLAB R2024b con funciones simuladas, sin DLL ni hardware. Comprueba mensajes conocidos, desconocidos y fallos al consultar la descripción. Resultado: correcto.
- Estas pruebas no ejecutan el C++ modificado. No se ha compilado ningún proyecto de Visual Studio en esta actualización.
- Tras compilar, probar Noise y RF sin hardware con 4 y 8 bases. Con mapa de 128 válido deben aceptar ambas configuraciones; con 256 entradas deben aceptar 8 bases y rechazar 4 con -1523; con 127 entradas deben rechazar ambas. Probar también mapa omitido y modo consecutivo. Usar copias de los XML y reiniciar el sistema entre cargas rechazadas.

### Compilación que debe realizar el usuario

1. Compilar **stlib2, Release | x64, v143**.
2. Actualizar la DLL y biblioteca de enlace de stlib2 usadas por api_pcie si las copias de postcompilación no lo hacen, y compilar **api_pcie, Release | x64, v141**.
3. Si se utiliza la aplicación de consola, compilar también **pcie_vc, Release | x64, v141**.
4. Con MATLAB cerrado, desplegar **stlib2_x64.dll y api_pcie_x64.dll** en la carpeta que carga MATLAB, y utilizar el toolbox actualizado, incluido `ST2_ErrorText.m`.

En esta actualización se necesitan ambas DLL. Las instrucciones anteriores de reemplazar solamente stlib2 correspondían exclusivamente al cambio de elementos automáticos o de ampliación del mapa.

## Actualización 20/10/2026 — Corrección del else en c_vch_config.cpp

- El usuario detectó el error de compilación «else no válido sin el correspondiente if» en la línea 398. El cambio anterior utilizaba `if (...) return ELOG(...); else ...`, pero la macro `ELOG` ya termina en punto y coma. Su expansión dejaba una instrucción vacía entre el `if` y el `else`.
- Se encerró el retorno en llaves y se eliminó el `else` innecesario. La asignación del mapa solo se alcanza cuando la longitud es correcta; el comportamiento de -1523 no cambia. Se eligió esta corrección local para no alterar la macro compartida por el proyecto.
- Se añadió una comprobación de regresión estática al test del catálogo. No sustituye a una compilación C++.
- El usuario quiere depurar: utilizar **Debug | x64**, primero en stlib2 y después en api_pcie, conservando v143 y v141 respectivamente. Release no es necesario para corregir este error. Mantener los PDB correspondientes a las DLL para depurar.
- El asistente no ejecuta compilaciones de Visual Studio; la comprobación de compilación queda a cargo del usuario.
