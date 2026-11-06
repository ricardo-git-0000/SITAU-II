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

## Plan de parametrización PCIe — 128 y 256 canales

Estado: fases 1–3 implementadas en fuentes. Fase 4: pruebas sintéticas y procedimiento de integración preparados; compilación, ejecución C++ y validación física pendientes del usuario. Esta sección agrupa el diseño, los criterios de aceptación y las limitaciones; las futuras iteraciones actualizarán el estado de cada fase, sin registrar cada ejecución como un diario.

### Objetivo y alcance

Seleccionar 4 u 8 bases mediante `BASES_NUMBER` en `SITAU.XML`, sin recompilar ni editar constantes al cambiar de equipo. Reutilizar la configuración resuelta por stlib2 (`ST_GetChannelsNumber()`), sin introducir otro número de módulos en el XML de adquisición. Primera entrega: modo paralelo, canal virtual 0 y sonda completa de 128/256 elementos. Sondas parciales, multiplexado y otros modos requieren un contrato explícito y no se consideran automáticamente soportados.

Las pruebas Noise/RF realizadas con `pcie_enabled=0` no validan la ruta PCIe. El usuario ha confirmado la aparición del error de longitud del mapa en MATLAB; eso tampoco verifica DMA ni el orden físico de canales PCIe.

### Resultado del estudio inicial (antes de la implementación)

La parametrización está incompleta en varias capas, no solamente en `N_MODULES`:

| Área y referencia de código | Estado observado | Trabajo necesario |
|---|---|---|
| `api_pcie/src/pcie_tad.h:52` | `N_MODULES=8`; 32 canales por módulo | Eliminar su uso como dimensión efectiva. Mantener separadas las constantes físicas y los valores de configuración. |
| `api_pcie/src/c_image_handle.cpp:86,249` | Cada ley focal ocupa `frame_length * 32 * 8` muestras; ese valor determina los punteros dentro del bloque | Pasar al gestor la geometría validada del transporte; comprobar desplazamientos y límites antes de entregar punteros. |
| `api_pcie/src/pcie.cpp:328` (`PCIe_SetBuffers_mod`) | Calcula beats por módulo, pero no incluye el número de canales en la condición que decide recalcular buffers | Recalcular al cambiar la geometría y los demás parámetros que afectan a la adquisición, incluso con las mismas muestras y leyes. |
| `api_pcie/src/fuse/fuse.cpp:1339–1355,1531` | `FRAMES_PER_PACKAGE=8` interviene en tamaño DMA, cálculo de separación del emulador y tamaño de lectura en `dma_start` | Unificar el tamaño usado al reservar y al leer; cambiar solo la reserva dejaría la lectura incorrecta. |
| `api_pcie/src/fuse/fuse.cpp:1409` | El comprobador del patrón del emulador recorre ocho tramas | Mantenerlo coherente con el formato que realmente genera el emulador. |
| `api_pcie/src/xdma_driver/libxdma/fuse_emul.c:227,254` | El emulador del driver también calcula ocho tramas y programa registros de longitud | Verificar si es contrato fijo del emulador/FPGA. No modificar el driver ni su protocolo sin evidencia de necesidad. |
| `api_pcie/src/pcie.cpp:790` | El diagnóstico `PCIe_CheckPCIeBlockSaw` reserva y recorre 8 × 32 canales | Adaptar contador y recorrido; su interfaz no recibe la capacidad del bloque ni del array de errores, por lo que necesita una vía interna con límites conocidos. No se ha localizado una llamada activa en los fuentes de api_pcie revisados. |
| `api_pcie/src/pcie_thread_acq.cpp:176,193` y `pcie.cpp:870,936` | El buffer de usuario y funciones de lectura ya usan `ST_GetChannelsNumber()` | Conservar esta parametrización y hacerla coherente con el tamaño del transporte. |
| `api_pcie/src/pcie_utils.cpp:343` | Ordena por grupos de 32, pero recorta índices inválidos al último canal o muestra | Validar longitudes e índices y devolver error; evitar que el recorte oculte un mapa/configuración incorrectos. |
| `api_pcie/src/stlib2/stlib2_wrapper.cpp:74,223` | Reserva el mapa con el número de canales y solo comprueba retorno negativo del getter | Comprobar también cuántas entradas se copiaron: el getter puede devolver menos sin error. Evitar entradas residuales al recargar configuraciones. |
| `stlib2/src/c_virtual_channel/c_virtual_channel.cpp:397–405` | Las leyes por bloque GTX ya se calculan con canales dinámicos | Verificar consistencia con PCIe y proteger divisores/productos: una ley que no cabe puede producir cero leyes por bloque. |

Flujo relevante: `ST2_OpenSys` abre recursos → `ST2_LoadXMLFileConfig` carga parámetros/mapa → `ST2_Start` obtiene muestras y leyes, prepara buffers y gestor de imágenes → `fuse::dma_start` calcula el tamaño de lectura → `GetOneImage_Pointers` separa leyes → `C_PCIBuffer::Add` copia u ordena → MATLAB obtiene la imagen. La configuración PCIe debe resolverse después de disponer de la configuración válida, y permanecer estable durante la adquisición.

### Contrato de transporte confirmado

El usuario confirma que el sistema de 128 canales envía cuatro tramas y el de 256 envía ocho, una por módulo. Se adopta este contrato para la adquisición real: no hay ocho posiciones fijas con relleno en el equipo de 128. La confirmación procede del usuario, no de una captura realizada por el asistente. El encabezado `T_block_header` no aporta un contador explícito de bases. El cálculo actual trata `block_beats + 1` como beats por módulo y `block_focal_laws + 1` como leyes por bloque; sus unidades, alineación y orden se comprobarán en las pruebas de integración. No se debe confundir el número de tramas con el de enlaces físicos.

- Canales de transporte = canales efectivos; módulos y tramas por ley = canales / 32: cuatro para 128 y ocho para 256.
- La reserva DMA, el tamaño de lectura y el salto entre leyes deben utilizar el mismo número de módulos resuelto desde stlib2. No se necesita una opción adicional de relleno en el XML.
- Esta confirmación describe la adquisición real. El formato del generador de pruebas del driver/FPGA se revisará por separado antes de sustituir sus constantes de ocho tramas; no se presupone que dicho emulador sea configurable.

Para transporte compacto y muestras de 16 bits, con `S` muestras/canal, `C` canales y `L` leyes por bloque: muestras por ley = `S*C`; bytes por ley = `S*C*2`; bytes por bloque = `32 + L*S*C*2`. Los 32 bytes iniciales corresponden al encabezado actual. Los beats por módulo siguen siendo `S*32*2/32 = 2*S`; el ancho del bus de 256 bits es 32 bytes y no es el número de canales. Ejemplo con S=1024 y L=2: 524320 bytes para 128 canales y 1048608 para 256, sujeto a confirmar alineación/relleno del protocolo.

### Diseño recomendado

Resolver una única geometría por adquisición, con nombres como `channel_count`, `module_count`, `transport_channel_count`, `samples_per_channel`, `focal_laws_per_block` y `block_size_bytes`. Validar primero y pasar esos valores a las clases internas, en vez de consultar globals en cada bucle o releer el XML desde el driver. Conservar las firmas públicas ST2 y los prototipos MATLAB cuando sea posible; revisar los wrappers históricos `fuse_api.cpp` y `api_pcie_old.h` antes de alterar firmas internas que puedan estar expuestas.

Validar 128/256 en el alcance inicial, multiplicidad de 32, valores positivos, capacidad del bloque y productos sin desbordamiento antes de reservar o leer. Usar aritmética de tamaño apropiada, respetando los límites DWORD de las lecturas Windows. Validar el mapa por número efectivo de entradas y rango, y su unicidad cuando deba ser una permutación. La ampliación 128→256 ya resuelta por stlib2 no se duplica en api_pcie. Si se pide una sonda parcial, rechazar explícitamente el caso no soportado o diseñar su proyección; no rellenar silenciosamente el resto con ceros.

Mantener separados: bases del equipo, grupos de canales, leyes focales, bloques GTX, buffers DMA y enlaces físicos. También hay una discrepancia heredada entre el límite `>512` en `ST2_Start` y el mensaje de `EPCIE_GTX_BLOCKS` que dice 256; se resolverá contra el límite real, sin cambiarlo por asociación con los canales. Los errores nuevos deberán tener números no ambiguos y descripción, aprovechando el catálogo existente.

### Fases propuestas y criterios de cierre

| Fase | Cambios previstos | Prueba y criterio de aceptación | Estado |
|---|---|---|---|
| 1. Contrato y geometría | Aplicar el contrato confirmado de una trama por módulo; definir unidades y resolver/validar la geometría desde stlib2 antes del arranque. Definir rechazo de modos fuera de alcance. | Cálculos y límites con casos válidos e inválidos; contrastar bytes por bloque y orden de tramas durante integración. | Implementada en fuentes; pruebas estáticas correctas; compilación y pruebas C++ pendientes |
| 2. DMA y gestor de imágenes | Parametrizar conjuntamente `pcie.cpp`, `c_image_handle.*` y la ruta necesaria de `fuse.*`; actualizar cachés, reinicialización y comprobaciones de capacidad. Mantener el protocolo del driver salvo necesidad demostrada. | Bloques sintéticos con varias leyes, imágenes repartidas entre bloques, último bloque parcial y vuelta del FIFO; ningún puntero fuera del bloque. Regresión de 256 y cambio 128→256→128 tras parada/reapertura. | Implementada en fuentes; pruebas C++ y hardware pendientes |
| 3. Mapa, ordenación y diagnósticos | Validar el número copiado por el getter, límites en `GetFocalLawSorted`, mapas y diagnóstico saw; revisar patrón y tamaño del emulador sin confundirlo con simulación MATLAB. | Datos identificables por canal/muestra/ley: orden correcto en límites 31/32, 63/64, 127/128 y 255; modo ordenado y bruto; mapas omitido, consecutivo, permutado, duplicado, corto y fuera de rango. | Implementada en fuentes; pruebas C++ y hardware pendientes |
| 4. Integración y validación física | Añadir pruebas PCIe reproducibles, errores y ejemplos para seleccionar transporte/configuración; comprobar arranque, parada y recarga; consolidar documentación. | Noise/RF TCP siguen funcionando; pruebas PCIe reales 4/8 bases con adquisición finita/continua, ráfagas, pausas, varios tamaños de muestra y reinicios, sin fugas ni pérdida/mezcla de canales. Comparación TCP/PCIe con señal o patrón controlado. | Preparación completada; cierre pendiente de compilación y pruebas físicas |

Cada iteración debe entregar un conjunto coherente que preserve el funcionamiento de 256 canales; no desplegar aislado el cambio de salto entre leyes con una lectura DMA que todavía presuponga otro formato. Las comprobaciones del mapa mínimas necesarias para consumir datos válidos se adelantarán a la fase 2 si son precondición de esa ruta; la fase 3 completa su cobertura.

### Estrategia de pruebas y entregas

Las pruebas sintéticas deben ejercitar los cálculos y recorridos C++ de producción mediante un adaptador o fuente DMA simulada; una réplica matemática en Python sirve como comprobación auxiliar, pero no certifica el binario. La simulación existente de `TThread_Acq::Acquire` evita `GetOneImage_Pointers` cuando el driver está desactivado y genera datos artificiales en `C_PCIBuffer::Add`: por eso no detecta por sí sola el error del salto fijo de 256. Se necesitan al menos dos leyes para detectar dicho salto.

La matriz mínima combina 4/8 bases, ordenación activada/desactivada, mapa válido/inválido, una y varias leyes, límites de bloque y ciclo abrir→cargar→arrancar→parar→cerrar. Añadir cambios de configuración con igual número de muestras para detectar cachés obsoletas, y casos que excedan capacidad o dejen cero leyes por bloque. La validación física queda separada de la simulación y debe registrar versión de firmware/driver y configuración usada.

Las modificaciones se concentran en api_pcie, con una consulta adicional en stlib2 para verificar módulos físicos. No se propone recompilar ni reinstalar el driver de kernel en las primeras fases sin necesidad confirmada. El usuario realizará las compilaciones Visual Studio (stlib2 v143 cuando cambie; api_pcie y pcie_vc v141; inicialmente Debug x64), desplegará DLL/PDB coincidentes y ejecutará las pruebas físicas. El asistente revisará fuentes, preparará pruebas y actualizará este informe por fase. No se ha compilado esta implementación.

### Fase 1 — Configuración común y validación previa al arranque

Este apartado describe la primera entrega. Las limitaciones temporales y tareas pendientes que menciona se actualizan en el apartado de fases 2–4 al final.

Se añade `pcie_geometry.h`, con `T_PCIeGeometry` y la función pura `PCIe_BuildGeometry`. Reúne canales, módulos, muestras, leyes por imagen/bloque, beats por módulo, salto entre leyes y bytes por bloque. Usa productos de 64 bits con límites previos; la salida queda a cero si hay error. Se elige un objeto local por arranque frente a una caché global para evitar dimensiones residuales al cambiar de equipo.

`PCIe_ResolveGeometry`, en `pcie.cpp`, consulta una vez cada dimensión de stlib2, después de parar la adquisición y antes de abrir nuevos recursos DMA en `ST2_Start`. El arranque reutiliza las muestras y leyes ya validadas. La ruta TCP no invoca esta nueva validación. El cambio no añade ningún parámetro al XML ni altera la interfaz pública ST2 o los prototipos MATLAB.

Se añade la exportación `ST_GetModulesNumber()` a stlib2 y a su cabecera de importación en api_pcie. Devuelve los módulos físicos del canal virtual 0: es necesaria porque `ST_GetChannelsNumber()` devuelve canales totales y un sistema multiplexado también puede declarar 128/256 canales sobre un único módulo. Se exige `module_count == channel_count / 32`, sonda completa, longitud completa del mapa y dimensiones positivas. La validación detallada del contenido del mapa y del número efectivamente copiado permanece en fase 3.

Límites: 65536 beats por módulo (campo de 16 bits menos uno), 1024 leyes por bloque (campo de 10 bits menos uno), y tamaño total máximo `GTX_SIZE_BLOCK = (1<<23)-32` bytes, coherente con el cálculo existente de stlib2 que descuenta la cabecera de ese máximo. No se confunden los 256 bits del bus con canales. Las pruebas estáticas verifican que la copia del límite GTX coincide con `st_reg.h`. No se corrige aún el riesgo previo de división por cero durante la carga de una configuración que no deje ninguna ley por bloque en stlib2; esta protección de arranque no sustituye aquella revisión pendiente.

Nuevos códigos sin colisiones: `-169` para geometría/topología incompatible, `-170` para capacidad del formato/bloque y `-171` para el arranque PCIe de 128 todavía pendiente de adaptar. Se incorporan al traductor y a `doc/CODIGOS_ERROR.md`.

**Limitación deliberada de esta fase:** el cálculo reconoce cuatro tramas, pero no se permite arrancar PCIe de 128 con los consumidores DMA actuales de ocho. Devuelve `-171` antes de abrir DMA. Se mantiene el arranque de 256 para configuraciones válidas del alcance indicado; debe verificarse tras compilar. La fase 2 conectará la geometría a reserva, lectura y punteros, y retirará este bloqueo únicamente cuando esas capas sean coherentes. El soporte TCP de 128 no se bloquea. Los emuladores y el driver no se modifican en esta fase.

Pruebas entregadas en `tests/pcie_geometry/`: comprobación estática de integración, orden de validación, exportaciones y límites (ejecutada correctamente); prueba C++ que llama a la función real, con casos 128/256, transición de configuraciones, multiplexado, sonda parcial, mapa corto, dimensiones cero/negativas, límites de cabecera/GTX y extremos de int (pendiente de compilar y ejecutar). La regresión estática de errores también pasa: 271 descripciones y sin nuevas colisiones. Ninguna comprobación realizada aquí ejecuta las DLL nuevas ni el hardware.

Para usar esta fase, el usuario debe compilar **primero stlib2 y después api_pcie**, en Debug x64, actualizar la biblioteca de enlace que consume api_pcie y desplegar ambas DLL/PDB. La nueva api_pcie requiere la nueva exportación de stlib2: no mezclarla con una stlib2 anterior. Para la prueba C++ independiente y las comprobaciones estáticas, consultar `tests/pcie_geometry/README.md`. No se han sustituido DLL, creado commits ni subido cambios como parte de esta fase.


### Fases 2–4 — Transporte compacto, validación y pruebas de integración

**Estado de esta entrega:** implementación de fases 2 y 3 completada en fuentes; fase 4 preparada para validación. No se han compilado las DLL ni ejecutado las pruebas C++ o el hardware. Las comprobaciones Python son estáticas y no certifican adquisición real. El procedimiento detallado y los comandos están en `tests/pcie_geometry/README.md`.

#### Fase 2: una configuración para reserva, lectura y gestor

- `ST2_Start` pasa la misma `T_PCIeGeometry` a `PCIe_SetGeometryBuffers` y `PCIe_InitGeometryManager`. Los adaptadores históricos conservan sus firmas y contrastan los argumentos con stlib2. No se modifica la interfaz ST2 ni los prototipos MATLAB.
- `fuse` conserva el número de módulos de cada adquisición; tanto la reserva como `dma_start` calculan cuatro tramas para 128 y ocho para 256. Comprueba que la longitud de lectura coincide con la capacidad reservada y rechaza transferencias incompletas. Se retira el bloqueo temporal `-171` del arranque real; el código queda reservado por compatibilidad.
- Se elimina la caché parcial: cada arranque parado recalcula todos los contadores y reconstruye buffers/FIFO. Esta elección prioriza coherencia al cambiar 128→256→128 o solo el número de imágenes frente al ahorro de reservas. Se revisan capacidad de DMA, productos de 32/64 bits, fallos de memoria y propagación de errores de inicialización del buffer de usuario.
- El gestor conserva la geometría, calcula el salto con canales efectivos y comprueba cabecera, secuencia, canal virtual cero y límites antes de entregar cada puntero. Se corrigen ocupación del FIFO, avance cuando una imagen necesita varios bloques y decremento de la última imagen. El vector de punteros se reinicializa también al reducir el número de leyes.
- El último bloque puede contener menos leyes útiles que su capacidad nominal. **El contrato implementado conserva la longitud física completa del bloque**, como la ruta anterior, y usa el contador de leyes válidas para excluir relleno. Una lectura físicamente más corta se rechaza con `-173`; confirmar este punto con una captura o registro de hardware antes de dar por cerrada la integración.

#### Fase 3: mapa, diagnóstico y carga segura

- El mapa PCIe se valida al cargar la configuración y de nuevo antes del arranque: tamaño declarado, número realmente copiado, rango y unicidad. Se publica mediante una copia temporal solo después de validar; se mantiene la ampliación 128→256 de stlib2 sin duplicarla. Esta validación adicional se aplica a PCIe y exige sonda completa. La recarga con adquisición PCIe activa se rechaza: parar antes de cargar otro XML.
- `GetFocalLawSorted` utiliza la función pura `PCIe_SortLaw`, con capacidades comprobadas. Se elimina el recorte silencioso al último canal/muestra. Los fallos de copia, ordenación o liberación se propagan al hilo de adquisición.
- El diagnóstico saw usa canales dinámicos y un recorrido con capacidades explícitas (`PCIe_CheckSaw`). El adaptador histórico `void*` se conserva, pero su llamante debe garantizar la capacidad real de los punteros porque esa firma no permite descubrirla.
- Se mantiene el driver/kernel y su emulador fijo de ocho tramas. Se rechaza arrancar o comprobar ese emulador con transporte de cuatro tramas (`-174`). La simulación MATLAB sin hardware es una ruta distinta y no prueba DMA. Se corrige también la posición de escritura de leyes simuladas para no sobrescribir siempre la primera.
- El cálculo GTX de stlib2 usa productos de 64 bits y evita divisores cero. Con PCIe, una ley que no cabe, una ráfaga inválida o contadores no representables producen `-1524`; se limita el campo de leyes por bloque a 1024. TCP no se rechaza por dimensiones GTX que no utiliza. Se conserva el límite de software de 512 bloques por ráfaga y se corrige su mensaje; el máximo físico del firmware continúa pendiente de contrastar.
- Alcance explícito: adquisición MC paralela, canal virtual 0, sonda completa, 4/8 módulos. PA, TFM, multiplexado y sondas parciales no se presentan como soportados por esta adaptación.

Nuevos códigos: `-172` mapa PCIe inválido, `-173` bloque/longitud incompatible, `-174` emulador incompatible y `-1524` geometría GTX inválida durante carga. El catálogo alcanza 275 descripciones y 255 códigos distintos, conservando las 20 colisiones heredadas.

#### Fase 4: pruebas entregadas y criterio de cierre

- `test_pcie_geometry.cpp` ejecuta las funciones de producción de geometría, mapa, ordenación, límites de punteros y saw, incluyendo longitudes cortas, duplicados, índices fuera de rango, límites numéricos y destinos con centinelas.
- `prepare_manager_test.py` extrae la declaración y la implementación actuales de `CImageHandle`; solo sustituye las dependencias externas. `test_pcie_stream.cpp` las ejecuta con una fuente DMA simulada y cabeceras de producción. Cubre 128/256, varias leyes por imagen, imágenes compartiendo bloques, cruces de bloque, FIFO circular, último bloque parcial, liberación final y cabeceras/estado DMA inválidos. Estos ejecutables quedan pendientes de compilar y ejecutar por el usuario.
- Se ejecutan las comprobaciones estáticas de geometría/integración y catálogo de errores. La preparación del adaptador no equivale a una ejecución del gestor.
- La validación física debe ejecutar Noise/RF con PCIe real, 4/8 bases, modos bruto/ordenado, adquisición finita/continua, ráfagas con y sin pausa, varias muestras/leyes y ciclos abrir→cargar→arrancar→parar→cerrar. Registrar firmware, driver, DLL y XML utilizados; verificar orden físico mediante una señal identificable y comparar con TCP. Tras un error de carga, cerrar y reabrir antes de reintentar.

Para compilar: **stlib2 (v143) → actualizar su `.lib`/DLL usada por api_pcie → api_pcie (v141)**, ambos Debug x64; también pcie_vc si se usa la consola. Desplegar las dos DLL y PDB coincidentes con MATLAB cerrado. No se han recompilado ni reemplazado binarios, modificado el driver, creado commits ni hecho push en esta entrega.


### Adaptación PCIe de 256 a la placa de transporte de cuatro módulos (2026-10-30)

La placa intermedia cuenta grupos de cuatro módulos. Para ocho módulos se envían dos unidades de transporte por cada ley real, sin duplicar las emisiones ni las leyes por imagen. Esta adaptación sustituye el contrato previo del contador nominal de cabecera para 256; 128 mantiene factor uno y TCP conserva su configuración.

- En `CVirtualChannel::FP_UpdateAcquisitionSize()` el contador interno sigue expresado en leyes reales; el límite del campo es 1024 leyes para 128 y 512 para 256. Se conserva el ajuste de capacidad en bytes, ráfagas y pausas. `Set_Registers()` multiplica únicamente `msg->gtx_n_focal_law` por dos cuando PCIe está activo y hay ocho bases. Los getters continúan devolviendo cantidades reales.
- `T_PCIeGeometry` distingue leyes reales y unidades de transporte. `PCIe_DecodeBlockLaws()` interpreta `(block_focal_laws + 1) / factor` y rechaza unidades impares en 256. `GetOneImage_Pointers()`, `PCIe_CheckPCIeHeader_fast()` y el diagnóstico saw usan esa conversión. La cabecera DMA original no se reescribe. El avance de imagen se calcula con leyes reales.
- Reserva, lectura DMA y saltos de punteros conservan `32 + leyes_reales * canales * muestras * 2` bytes. Dos grupos de cuatro módulos equivalen a una ley de ocho: no se duplica otra vez el tamaño ni el número de bloques. No cambia `block_beats`.
- Contrato de cabecera implementado: solo `block_focal_laws` cuenta unidades de cuatro módulos (menos uno). `image_focal_law_index` y `image_focal_law_total` siguen en leyes reales; cuando `last_block=1`, este último conserva el número de leyes **reales** válidas del bloque. No se divide ni duplica esos campos. Es la interpretación de mantener la cabecera indicada por el usuario; contrastar especialmente el último bloque en hardware. Si el firmware expresa alguno en unidades de transporte, esta implementación lo rechazará y habrá que adaptar explícitamente ese campo.
- Orden físico indicado: una ley real de 256 llega como `[M5 M6 M7 M8][M4 M3 M2 M1]`, sin cabecera entre mitades; 128 conserva `[M4 M3 M2 M1]`. No se modifica `pin_to_channel_assignment` ni la ordenación actual: la correspondencia física de canales de 256 queda pendiente de revisión. La agrupación de leyes no certifica ese mapa.
- Se presupone una cabecera de 32 bytes por bloque, leyes completas y longitud nominal también en el último bloque (relleno excluido por su contador). No se cambia firmware, driver ni su generador de emulación; este último no certifica el nuevo contador 4+4 de 256.

Pruebas ampliadas: límites 512/513 en 256 y 1024 en 128, barrido de las 1024 codificaciones con ambos factores, rechazos de medias leyes y geometría sin inicializar. El flujo sintético usa cabeceras con el contador adaptado y ejecuta el gestor y el verificador de secuencia extraídos de producción; conserva escenarios 128→256→128, cruces de bloque, varias imágenes por bloque y último bloque parcial. Los tests Python son estáticos; los ejecutables C++, la compilación de DLL y la validación física quedan a cargo del usuario. No se recompilan ni sustituyen DLL en esta entrega.

Validación requerida: primero regresión PCIe 128 con Noise/RF; después 256 con varias leyes identificables y adquisición finita que deje un último bloque parcial. Verificar que MATLAB conserva el número real de leyes por imagen y recibe los datos de las dos mitades, además de pausas y cambios 128→256→128. Para desplegar, compilar primero stlib2 y después api_pcie en la misma configuración x64 y actualizar ambas DLL; para otro ordenador usar Release. Registrar el resultado físico antes de dar esta adaptación por validada.

Comprobaciones de esta entrega: PASS en integración estática PCIe/4+4, PASS en catálogo de errores (275 descripciones), preparación del gestor/verificador sintéticos completada y `git diff --check` sin errores. Estos resultados no equivalen a ejecutar los tests C++ ni el equipo.


### Demos MATLAB PCIe para 128 y 256

Demo_PCIe_noise.m y Demo_PCIe_RF.m cargan BIN_256 con hardware real. BASES_NUMBER en su SITAU.XML determina los canales (actualmente 8). Noise usa ejes dinamicos y calcula una rafaga mediante muestras, canales, leyes reales e imagenes por rafaga, sin multiplicar por el numero de rafagas; se elimina una reserva auxiliar sin uso. Los XML PCIe de Noise y RF omiten probe_elements_number para derivarlo del equipo. RF carga el SITAU2_PCIe_RF.XML ya existente, consulta las leyes reales y limita los indices de visualizacion a los datos disponibles. Se conservan los mapas y los restantes parametros de adquisicion. Para 128 usar BASES_NUMBER=4 en BIN_256/SITAU.XML; para 256 usar 8. Pasan las comprobaciones estaticas de rutas, XML y limites; MATLAB/hardware no ejecutados. No requiere recompilar DLL.

### Coherencia de XML y validación MATLAB entre GE y PCIe (2026-11-06)

**Problema y alcance.** La adaptación anterior permitía omitir `probe_elements_number` en stlib2, pero el validador MATLAB seguía exigiéndolo. Además, los XML PCIe de las demos Noise/RF y el validador usaban `FP_FOCAL_LAW`/`fp_*`, mientras el cargador C++ actual lee `MC_FOCAL_LAW`/`mc_*` para ambos transportes. Una sección FP podía ignorarse y dejar el tipo FMC predeterminado en vez del PWI solicitado. No era una diferencia necesaria entre GE y PCIe. Este cambio corrige las cuatro demos activas y sus XML, junto con el validador común; los ejemplos históricos y copias de otras pruebas no se migran automáticamente.

**Geometría única.** `Validate_XMLConfig` acepta el número de canales del sistema abierto como segundo argumento y el enlace esperado como tercero. Las cuatro demos pasan `ST2_GetChannelsNumber()` tras `OpenSys`: así validan 128/256 según `BASES_NUMBER`, sin deducirlo del nombre de la carpeta BIN. Si se omite `probe_elements_number`, se usa esa geometría; si se declara, debe coincidir para estas demos de sonda completa. La llamada antigua sin segundo argumento sigue funcionando con elementos explícitos o una biblioteca abierta; sin ninguna geometría disponible se explica qué argumento falta. La derivación en stlib2 ya existía y no se modifica.

**Un único esquema MC.** Los XML PCIe pasan a `MC_FOCAL_LAW` y parámetros `mc_*`; GE Noise elimina un campo FP residual. El validador rechaza nombres FP, parámetros duplicados y ubicaciones incorrectas de los campos comprobados, con mensajes que indican el esquema esperado. Se mantiene la prioridad de la lista `mc_pwic_angles` frente al rango y se corrige el antiguo nombre `fp_pwix_angles`. FMC usa los elementos efectivos; PWI cuenta leyes reales. En los cuatro XML actuales hay nueve ángulos de -12 a 12 grados, con paso 3, tanto en 128 como en 256: el transporte PCIe 4+4 no duplica este número.

**Ráfagas realmente aplicadas.** `GetData_ACQUISITION` lee `burst_number` e `images_per_burst_number` en `ACQUISITION`. Los antiguos `fp_burst_*`/`mc_burst_*` dentro de las leyes no se leían. Se migran los valores declarados, dando prioridad a los que ya existían en la sección correcta:

| XML de la demo | `burst_number` | `images_per_burst_number` | Decisión |
|---|---:|---:|---|
| GE Noise | 1 | 1 | Se hacen explícitos los valores predeterminados anteriores. |
| GE RF | 150 | 250 | Se trasladan los valores de los campos MC antiguos. Ahora sí se aplican. |
| PCIe Noise | 150 | 3 | Se trasladan los valores de los campos FP antiguos. Ahora sí se aplican. |
| PCIe RF | 10 | 1 | Se conservan los valores de ACQUISITION y se eliminan los FP contradictorios de 150/3. |

Las cuatro configuraciones están en modo continuo (`acquisition_mode=1`): el contador de ráfagas configurado no impone un final, pero las imágenes por ráfaga sí determinan el tamaño entregado. En modo finito (`0`) se utiliza el contador configurado. Esto puede cambiar memoria, latencia y cadencia respecto a las configuraciones antiguas ignoradas, especialmente las 250 imágenes de GE RF; no es solo una sustitución de nombres.

**Validador y comprobación posterior.** Se corrigen `valid`, que antes nunca llegaba a `true`, y la variable no definida de la rama FMC. Se comprueban escalares, contadores positivos, coherencia con el enlace de `OpenSys`, ventanas de muestras/distancias y rangos PWI. `getXLMparam` deja de evaluar expresiones; exige literales compatibles con el lector C++: enteros sin punto en los contadores y listas de ángulos con corchetes y comas, sin exponentes. Se rechazan formatos que MATLAB interpretaría de otra manera que la DLL. Las estimaciones respetan la prioridad de distancia final sobre número de muestras y el ajuste a muestras pares. El alcance validado es MC de sonda completa, un canal virtual, FMC/PWI, decimación uno y modo de tiempo cero; leyes de fichero y otros modos no se declaran soportados.

GE también activa la validación previa. Tras `FUS_LoadXMLFileConfig`, las cuatro demos llaman a `Validate_LoadedMCConfig`, que contrasta mediante getters los canales, elementos, leyes reales, imágenes por ráfaga, ráfagas configuradas y modo. Si no coinciden, cierra el sistema y da un error antes de iniciar la adquisición. Esta comprobación detecta dimensiones ignoradas o una biblioteca incoherente, pero no valida por sí sola ángulos, mapas ni todos los parámetros del hardware.

El exceso de ancho de banda estimado se informa como **warning**, conservando el PRF solicitado: las demos GE tenían esa validación desactivada y pueden requerir pausas. No se garantiza adquisición sostenida sin pausas. La incompatibilidad entre PRF y ventana de adquisición sigue siendo un error. Los cálculos de capacidad conservan las referencias existentes; no son una medición del enlace.

**Validación y despliegue.** Ejecutado `tests/xml_config/test_xml_config.m` con MATLAB R2024b: PASS para ocho combinaciones de los cuatro XML con 128/256, más pruebas de geometría omitida/incorrecta, FMC/PWI, ráfagas finitas/continuas, valores por defecto, formatos antiguos o mal ubicados, duplicados, formatos numéricos y discrepancias simuladas de DLL. No se han cargado bibliotecas ni ejecutado adquisición en estas pruebas. Instrucciones reproducibles en `tests/xml_config/README.md`.

Estos cambios afectan a MATLAB/XML y no requieren recompilar DLL: desplegar las cuatro demos, sus cuatro XML y los tres helpers `Validate_XMLConfig.m`, `getXLMparam.m` y `Validate_LoadedMCConfig.m`. Usar las DLL actuales con la derivación de elementos ya incorporada. Se elige alinear los consumidores con el esquema existente, sin añadir alias FP ni renombrar estructuras internas C++ por estética. No se modifican el transporte 4+4, los mapas ni los problemas de memoria compartida aplazados. Quedan pendientes las pruebas físicas de esta configuración actualizada en 128 y 256.

### Evitar terminar MATLAB por fallo del buffer hardware (2026-11-06)

Se identifica un `exit(1)` en el constructor `CBufferHW`: si falla su `malloc`, termina el proceso anfitrión de la DLL, incluido MATLAB. Un código negativo devuelto por la API no tiene ese efecto. Este hallazgo no confirma que todos los cierres observados tengan esa causa; los accesos inválidos a memoria u otras excepciones nativas pueden cerrar MATLAB por rutas distintas.

Se sustituye esa terminación por `std::bad_alloc`, capturada dentro de la nueva función interna `ST_EnsureHWBuffer`, que devuelve el código existente `EBUF_NoMemory` (-703). Ambas rutas de creación, `ST_MemoryResize` y `ST_VirtualChannelsDelete`, usan esta función y propagan el resultado antes de utilizar el buffer. `ST_VirtualChannelsDefault` y `ST_ResetScan` dejan de ocultar ese error con un éxito incondicional. El puntero global solo se publica si el constructor termina correctamente. Guardas RAII liberan el buffer y los locks si falla una reserva posterior durante la construcción.

No se elige simplemente borrar `exit` ni cambiarlo por `return` dentro del constructor: eso permitiría usar un objeto incompleto con un buffer nulo. La conversión de excepción queda dentro de stlib2; no se pretende capturar y continuar tras corrupción de memoria ni certificar el resto de reservas de la biblioteca.

Validación ejecutada: PASS en `tests/error_reporting/test_host_survival_static.py` (construcción centralizada, propagación y ausencia de terminación explícita en el buffer), PASS en el catálogo de errores y comprobación de espacios de Git. Son comprobaciones de fuentes; no se ha compilado ni ejecutado C++ ni reproducido el fallo de MATLAB. Quedan pendientes la compilación y una prueba con fallo de reserva inducido, además de una captura de la excepción nativa si persisten cierres por otros errores.

Esta corrección, a diferencia de la anterior de XML/MATLAB, **sí requiere recompilar stlib2** y sustituir `stlib2_x64.dll` con MATLAB cerrado. No cambia la API pública ni requiere recompilar api_pcie por sí sola. Si persiste el cierre, depurar MATLAB como proceso anfitrión y detenerse en la excepción nativa para identificar la ruta real antes de atribuirla a la UCI.
