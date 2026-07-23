# Plan RIPER: topología dinámica de bases y adquisición PCIe

## 1. Propósito

Conseguir que la cantidad de bases configurada en `SITAU.XML` se propague correctamente por toda la aplicación, desde `ST_OpenSys` hasta la recepción y ordenación de datos PCIe.

El resultado no debe estar fijado a 8 bases ni a 256 canales. El mismo binario debe funcionar con cualquier número de bases soportado, sin recompilar y manteniendo compatibilidad con las configuraciones actuales.

La relación del sistema es:

```text
n_bases = BASES_NUMBER
channels_per_base = 32
n_channels = n_bases * channels_per_base
```

Casos mínimos de referencia:

| `BASES_NUMBER` | Canales esperados |
|---:|---:|
| 1 | 32 |
| 4 | 128 |
| 5 | 160 |
| 8 | 256 |

## 2. Estado RIPER

- [x] Research: recorrido actual identificado.
- [x] Innovate: diseño dinámico y estrategia de compatibilidad definidos.
- [x] Plan: este documento.
- [ ] Execute: pendiente de autorización explícita.
- [ ] Review: pendiente de implementación y pruebas.

Mientras el trabajo permanezca en `PLAN MODE`, no se modificarán fuentes ni archivos de configuración distintos de este documento.

## 3. Alcance

### Incluido

- Lectura y validación de `BASES_NUMBER`.
- Propagación del número de bases y canales.
- Creación del PA system y canal virtual predeterminado.
- Configuración de registros y leyes focales por UCI.
- Mensajes normales y extendidos para más de cuatro bases.
- Cálculo de tamaños de bloque y buffers DMA PCIe.
- Gestión de punteros a leyes focales e imágenes.
- Ordenación de muestras y `pin_to_channel`.
- Diagnóstico saw para todos los canales.
- Pruebas parametrizadas para 1, 4, 5 y 8 bases.
- Compatibilidad regresiva con 4 bases y 128 canales.

### No incluido inicialmente

- Cambios en el diseño FPGA o firmware UCI no presente en este repositorio.
- Incrementar el límite físico por encima de `MAX_N_BASES`.
- Cambiar la cantidad de 32 canales por base.
- Cambiar la API pública de `api_pcie.dll`, salvo que durante la implementación se demuestre imprescindible.

## 4. Fuente única de verdad

La topología debe proceder exclusivamente del `SITAU.XML` cargado por `ST_OpenSys`.

Reglas:

1. Si existe `BASES_NUMBER`, usar su valor.
2. Si no existe, usar `XML_DEF_N_BASES` como valor predeterminado.
3. `XML_DEF_N_BASES` no debe actuar como mínimo.
4. Validar que el valor esté entre 1 y `MAX_N_BASES`.
5. Calcular `n_channels` una sola vez a partir de `n_bases` y 32 canales por base.
6. Las capas consumidoras deben recibir la topología o consultarla mediante una única interfaz; no deben volver a inferirla usando constantes independientes.

El comportamiento actual que fuerza un mínimo de cuatro bases debe eliminarse:

```cpp
if (n_bases < XML_DEF_N_BASES) n_bases = XML_DEF_N_BASES;
```

Debe sustituirse por validación explícita. Un valor inválido debe producir un error claro durante `OpenSys`, no una corrección silenciosa.

## 5. Invariantes

Durante toda la ejecución deben cumplirse:

```text
1 <= n_bases <= MAX_N_BASES
n_channels == n_bases * 32
n_channels % 32 == 0
pin_to_channel.size() == n_channels
image_samples == focal_laws * samples_per_channel * n_channels
```

Los tamaños empleados por UCI, DMA, `CImageHandle` y `CBufferScan` deben derivarse de las mismas variables.

## 6. Hallazgos que motivan el cambio

### Configuración XML

- El parser ya lee `BASES_NUMBER`.
- El valor predeterminado actual es 4.
- Los valores inferiores a 4 se fuerzan incorrectamente a 4.
- `MAX_N_BASES` ya está definido como 8.

### UCI

- `TMSG_Config::base_reg` dispone de espacio para `MAX_N_BASES`.
- El código anuncia `msg->n_bases = sys_.base.size()`.
- Sin embargo, solo rellena hasta `MAX_N_BASES_MSG`, actualmente 4.
- Existen mensajes `TMSG_EmissionFocalLawsExtended` y `TMSG_BeamformerExtended` con `id_first_base`.
- Esos mensajes muestran que el diseño previsto para más de cuatro bases consiste en enviar grupos normales y extendidos.
- La implementación PA extendida contiene índices fuera de rango.
- La configuración TFM backward solo procesa las primeras cuatro bases.

### PCIe

- `N_CHANNELS_MODULE` vale 32 y conceptualmente representa los canales por base.
- `N_MODULES` vale 4 y fija indebidamente el número de bases.
- `FRAMES_PER_PACKAGE` vale 4 y participa en el tamaño de los buffers DMA.
- `CImageHandle` calcula el stride de una ley focal usando `32 * 4`.
- El diagnóstico saw reserva y recorre 128 canales.
- Otras capas, como `CBufferScan`, ya reciben dinámicamente `n_ch` desde `ST_GetChannelsNumber()`.

## 7. Diseño propuesto

### 7.1 Topología runtime

Usar un conjunto coherente de valores runtime:

```cpp
n_bases
channels_per_base
n_channels
```

`channels_per_base` puede permanecer como constante técnica de valor 32. `n_bases` y `n_channels` no pueden ser constantes de compilación.

Preferencia de diseño:

- Evitar nuevas variables globales si la topología ya puede obtenerse con seguridad desde `stlib2`.
- Obtener `n_channels` mediante `ST_GetChannelsNumber()` después de `ST_OpenSys`.
- Calcular `n_bases = n_channels / CHANNELS_PER_BASE` únicamente en el límite entre `stlib2` y `api_pcie`, validando antes la divisibilidad.
- Dentro de cada objeto PCIe, almacenar la topología recibida durante su inicialización para no consultar estado global durante una adquisición.

### 7.2 Compatibilidad pública

La API pública debe mantenerse siempre que sea posible:

```cpp
ST2_OpenSys(...)
ST2_Start(...)
ST2_GetChannelsNumber()
```

Las firmas internas sí pueden ampliarse para recibir `n_bases` o `n_channels`:

```cpp
PCIe_SetBuffers_mod(..., int n_bases)
PCIe_image_manager_init(..., int n_channels)
CImageHandle::image_manager_init(..., int n_channels)
fuse::calc_hw_image_size(..., int frames_per_package)
fuse::dma_start(..., int frames_per_package)
```

Las firmas finales se decidirán durante `EXECUTE` procurando pasar el dato más directo y evitar conversiones repetidas.

## 8. Plan de implementación

### Paso 1. Corregir lectura y validación del XML

Archivos principales:

- `trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.h`
- `trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp`

Acciones:

- Mantener `XML_DEF_N_BASES=4` como predeterminado.
- Validar el rango `1..MAX_N_BASES`.
- Eliminar el mínimo implícito de cuatro.
- Devolver error de configuración si el valor es cero, negativo o superior al máximo.
- Confirmar que `data_.sitau_hardware.base` se redimensiona al valor exacto.
- Confirmar que `CPASystem` recibe el valor exacto.

Verificación:

- [ ] XML sin `BASES_NUMBER` produce 4 bases.
- [ ] XML con 1 produce 1 base y 32 canales.
- [ ] XML con 4 produce 4 bases y 128 canales.
- [ ] XML con 8 produce 8 bases y 256 canales.
- [ ] XML con 0 o 9 produce error durante `OpenSys`.

### Paso 2. Propagar la topología a `api_pcie`

Archivos principales:

- `trunk/src/lib/api_pcie/src/stlib2/stlib2_wrapper.cpp`
- `trunk/src/lib/api_pcie/src/pcie.h`
- `trunk/src/lib/api_pcie/src/pcie.cpp`

Acciones:

- Después de `ST_OpenSys`, obtener `n_channels`.
- Validar que sea positivo y múltiplo de 32.
- Derivar `n_bases`.
- Redimensionar `gb_pin_to_channel` a `n_channels`.
- Pasar la topología a la inicialización de buffers y del gestor de imágenes.
- Repetir las validaciones necesarias al iniciar una adquisición si la configuración pudiera cambiar tras cargar otro XML.

Verificación:

- [ ] No se inicia PCIe con cero canales.
- [ ] No se inicia PCIe si los canales no son múltiplo de 32.
- [ ] La topología de `OpenSys` y la de `ST2_Start` coinciden.
- [ ] `pin_to_channel` contiene exactamente una entrada por canal.

### Paso 3. Completar configuración UCI para todas las bases

Archivos principales:

- `trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp`
- `trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_fl_pa.cpp`
- `trunk/src/lib/stlib2/src/HW/UCI/protocol_uci.h`
- `trunk/src/lib/stlib2/src/HW/UCI/protocol_uci.cpp`

Acciones:

- Inicializar completamente cada estructura antes de rellenarla.
- En `TMSG_Config`, recorrer hasta `MAX_N_BASES`, no hasta `MAX_N_BASES_MSG`.
- Marcar como deshabilitadas todas las posiciones no utilizadas.
- No anunciar más bases de las que están serializadas.
- Generalizar el envío de datos grandes por grupos de hasta `MAX_N_BASES_MSG`.

Patrón de agrupación:

```cpp
for (first_base = 0;
     first_base < n_bases;
     first_base += MAX_N_BASES_MSG)
{
    group_base_count = min(MAX_N_BASES_MSG,
                           n_bases - first_base);

    for (local_base = 0;
         local_base < group_base_count;
         ++local_base)
    {
        source_base = first_base + local_base;
        message.base[local_base] = data[source_base];
    }
}
```

Para mantener el protocolo existente:

- El primer grupo utilizará el mensaje normal.
- Los grupos posteriores utilizarán el mensaje extendido e `id_first_base`.
- Para el máximo actual de ocho bases habrá como máximo un grupo normal y uno extendido.

Correcciones PA:

- Leer desde `sys_.base[id_first_base + local_base]`.
- Escribir en `msg_ext->base[local_base]`.
- No recorrer `sys_.base.size()` empezando desde `id_first_base`.
- Calcular `t0_max` solo sobre las bases pertenecientes al grupo o según la semántica confirmada del receptor.
- Liberar `msg_ext` en todos los caminos.

Correcciones TFM:

- Revisar todos los caminos que solo recorren `MAX_N_BASES_MSG`.
- Añadir mensaje extendido donde sea necesario.
- Mantener el mismo esquema de índices locales y absolutos.

Verificación:

- [ ] 1 base genera un mensaje normal con una base válida.
- [ ] 4 bases generan un mensaje normal con cuatro bases.
- [ ] 5 bases generan un mensaje normal de cuatro y uno extendido de una.
- [ ] 8 bases generan un mensaje normal de cuatro y uno extendido de cuatro.
- [ ] No hay accesos fuera de rango.
- [ ] Todas las posiciones no utilizadas están inicializadas.

### Paso 4. Hacer dinámico el tamaño DMA

Archivos principales:

- `trunk/src/lib/api_pcie/src/pcie.cpp`
- `trunk/src/lib/api_pcie/src/fuse/fuse.h`
- `trunk/src/lib/api_pcie/src/fuse/fuse.cpp`
- `trunk/src/lib/api_pcie/src/fuse/fuse_types.h`

Fórmulas:

```text
bytes_per_beat = 32
bytes_per_sample = 2
channels_per_base = 32

frame_beats_per_base =
    samples_per_channel * channels_per_base * bytes_per_sample
    / bytes_per_beat

payload_bytes =
    focal_laws_per_block * n_bases *
    frame_beats_per_base * bytes_per_beat

block_bytes = header_bytes + payload_bytes
```

Acciones:

- Sustituir `FRAMES_PER_PACKAGE=4` por `frames_per_package=n_bases` en la ruta de adquisición real.
- Pasar el valor a `calc_hw_image_size`, creación de buffers y `dma_start`.
- Usar aritmética `size_t` y comprobar overflow antes de reservar.
- Asegurar que creación y lectura usan exactamente la misma fórmula.
- Mantener constantes solo los parámetros físicos: bytes por beat, bytes por muestra y canales por base.

Verificación:

- [ ] El tamaño para cuatro bases coincide con el comportamiento anterior.
- [ ] El tamaño para ocho bases es el doble que para cuatro, con iguales muestras y leyes.
- [ ] No se truncan multiplicaciones a 32 bits.
- [ ] El DMA nunca recibe un tamaño menor que el validado en la cabecera.

### Paso 5. Hacer dinámico `CImageHandle`

Archivos principales:

- `trunk/src/lib/api_pcie/src/c_image_handle.h`
- `trunk/src/lib/api_pcie/src/c_image_handle.cpp`

Acciones:

- Guardar `n_channels_` o `n_bases_` durante la inicialización.
- Calcular:

```cpp
num_shorts_per_focal_law_ = frame_length * n_channels_;
```

- Eliminar el uso de `N_MODULES`.
- Verificar que cada puntero de ley focal queda dentro del bloque DMA.
- Verificar el último puntero y el tamaño de la última ley antes de exponerlos al consumidor.

Verificación:

- [ ] El stride para 1 base es `samples * 32`.
- [ ] El stride para 4 bases es `samples * 128`.
- [ ] El stride para 8 bases es `samples * 256`.
- [ ] La última ley focal no supera el final del bloque.

### Paso 6. Ordenación dinámica de canales

Archivos principales:

- `trunk/src/lib/api_pcie/src/pcie_utils.cpp`
- `trunk/src/lib/api_pcie/src/pcie_utils.h`

El layout actual agrupa muestras por bloques de 32 canales. El índice esperado se generaliza como:

```text
base = assigned_channel / 32
channel_in_base = assigned_channel % 32

source_index =
    base * samples_per_channel * 32 +
    sample * 32 +
    channel_in_base
```

Acciones:

- Mantener `n_ascan` dinámico.
- Sustituir nombres de módulo por base cuando se toque código relacionado.
- Validar cada entrada de `pin_to_channel` contra `n_channels`.
- No corregir silenciosamente un canal inválido al último canal; devolver error de configuración.
- Validar `size_fl == n_channels * n_samples` antes de ordenar.

Verificación:

- [ ] Identidad 0..31 para una base.
- [ ] Saltos correctos entre canales 31/32, 127/128 y 223/224.
- [ ] Ordenación completa 0..255 para ocho bases.
- [ ] Mapeos personalizados no salen de rango.

### Paso 7. Diagnóstico saw dinámico

Archivos principales:

- `trunk/src/lib/api_pcie/src/pcie.cpp`
- `trunk/src/lib/api_pcie/src/pcie_tad.h`

Acciones:

- Sustituir arrays C dimensionados con `32 * 4` por contenedores dimensionados con `n_channels`.
- Recorrer `n_bases` y 32 canales por base.
- Validar que el bloque contiene suficientes datos antes del recorrido.
- Cambiar nombres `module_index` por `base_index` cuando sea seguro hacerlo.

Verificación:

- [ ] Detecta errores en canales 0 y 31.
- [ ] Detecta errores en canales 32 y 127.
- [ ] Detecta errores en canales 128 y 255.
- [ ] No accede a canales inexistentes con configuraciones menores.

### Paso 8. Hipótesis provisional del layout FPGA

No se conoce todavía si ocho bases producen:

1. un bloque con ocho frames por ley focal; o
2. dos bloques de cuatro frames que el host debe agrupar.

La implementación actual combina cuatro bases con `FRAMES_PER_PACKAGE=4`. En ausencia de otra especificación, la primera implementación generalizará el comportamiento existente:

```text
frames_per_package = n_bases
```

Esta hipótesis debe quedar encapsulada en una sola función o punto de configuración, acompañada de un comentario equivalente a:

```cpp
// TODO(PCIe topology): Confirmar con el diseño FPGA si cada bloque contiene
// n_bases frames por ley focal o varios paquetes de hasta cuatro bases.
// Por ahora se generaliza el layout existente: una base equivale a un frame.
```

No se debe dispersar este `TODO` por diferentes archivos.

Validación defensiva obligatoria:

- Comparar `block_beats` con el tamaño calculado.
- Comparar leyes focales anunciadas y esperadas.
- Comprobar el final del último frame antes de generar punteros.
- Si el layout recibido no coincide, detener la adquisición y devolver un error que indique tamaño esperado y recibido.
- Nunca intentar interpretar el segundo formato de forma implícita.

La prueba con hardware determinará si se conserva esta implementación o se introduce un agregador de dos bloques de cuatro bases.

## 9. Estrategia de pruebas

### 9.1 Pruebas unitarias o de host

Crear bloques sintéticos con una secuencia inequívoca:

```text
value(base, channel, sample, focal_law)
```

La secuencia debe permitir identificar por el valor:

- la base;
- el canal dentro de la base;
- la muestra;
- la ley focal.

Matriz mínima:

| Bases | Canales | Leyes por bloque | Objetivo |
|---:|---:|---:|---|
| 1 | 32 | 1 y varias | límite inferior |
| 4 | 128 | 1 y varias | regresión |
| 5 | 160 | 1 y varias | grupo extendido parcial |
| 8 | 256 | 1 y varias | objetivo máximo actual |

Comprobar:

- [ ] Tamaño calculado del bloque.
- [ ] Tamaño reservado por DMA.
- [ ] Posición de cada ley focal.
- [ ] Ordenación de todos los canales.
- [ ] Primer y último dato de cada base.
- [ ] Cabecera de último bloque.
- [ ] Última ley parcial.
- [ ] Liberación correcta de buffers.
- [ ] Errores limpios ante tamaños insuficientes.
- [ ] Ausencia de lecturas y escrituras fuera de rango.

### 9.2 Pruebas de serialización UCI

Interceptar o inspeccionar los mensajes antes del envío:

- [ ] `n_bases` coincide con XML.
- [ ] Las bases normales son 0..3.
- [ ] `id_first_base` del extendido es 4.
- [ ] Las bases extendidas se almacenan en índices locales 0..3.
- [ ] Con cinco bases solo se serializa una base extendida.
- [ ] Las posiciones sobrantes están a cero o deshabilitadas.

### 9.3 Compilación

Compilar al menos:

- `stlib2`.
- `api_pcie`.
- `pcie_vc`.
- Variantes que generen o consuman `BIN` y `BIN_NOBF`.

Comprobar:

- [ ] Sin errores.
- [ ] Sin nuevos warnings de conversiones de tamaño o signed/unsigned.
- [ ] Sin cambios de ABI pública no documentados.

### 9.4 Prueba integrada sin hardware

Cuando sea posible usar emulación:

- Abrir configuraciones de 1, 4, 5 y 8 bases.
- Comprobar `ST2_GetChannelsNumber()`.
- Inicializar PCIe sin iniciar lectura física.
- Verificar tamaños y memoria asignada.
- Inyectar bloques sintéticos en la ruta inmediatamente posterior al DMA.

### 9.5 Prueba con hardware

Orden recomendado:

1. Cuatro bases con configuración conocida.
2. Ocho bases, adquisición corta de una ley focal.
3. Ocho bases, varias leyes por bloque.
4. Ráfagas repetidas.
5. Adquisición continua.

Para cada prueba registrar:

- XML usado.
- `n_bases` y `n_channels` informados.
- Muestras y leyes por bloque.
- `block_beats` recibido.
- Tamaño esperado y real.
- Contadores DMA.
- Errores saw por canal.
- Primer y último valor de cada base.

## 10. Criterios de aceptación

La tarea estará terminada cuando:

- [ ] El mismo binario funciona con distintas cantidades de bases indicadas por XML.
- [ ] `ST2_GetChannelsNumber()` devuelve `BASES_NUMBER * 32`.
- [ ] No quedan constantes de cuatro bases en la ruta funcional PCIe.
- [ ] Las constantes de cuatro que permanezcan están justificadas como tamaño máximo de un mensaje, no como topología del sistema.
- [ ] UCI recibe configuración para todas las bases.
- [ ] Los buffers DMA se dimensionan según la topología runtime.
- [ ] Las leyes focales usan el stride correspondiente a todos los canales.
- [ ] La ordenación devuelve todos los canales en el orden esperado.
- [ ] Los bloques incompatibles se rechazan antes de acceder fuera de límites.
- [ ] Las pruebas de 4 bases siguen pasando.
- [ ] Las pruebas sintéticas de 8 bases y 256 canales pasan.
- [ ] La prueba hardware confirma el layout o da lugar a una revisión controlada de la función encapsulada.
- [ ] Se revisa y resuelve el `TODO(PCIe topology)` cuando exista evidencia del FPGA.

## 11. Archivos previstos

Lista inicial, sujeta a confirmación durante `EXECUTE`:

```text
trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.h
trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp
trunk/src/lib/stlib2/src/HW/MK32/t_base_tad.h
trunk/src/lib/stlib2/src/HW/UCI/protocol_uci.h
trunk/src/lib/stlib2/src/HW/UCI/protocol_uci.cpp
trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp
trunk/src/lib/stlib2/src/c_virtual_channel/c_vch_fl_pa.cpp
trunk/src/lib/api_pcie/src/stlib2/stlib2_wrapper.cpp
trunk/src/lib/api_pcie/src/pcie.h
trunk/src/lib/api_pcie/src/pcie.cpp
trunk/src/lib/api_pcie/src/pcie_tad.h
trunk/src/lib/api_pcie/src/pcie_utils.h
trunk/src/lib/api_pcie/src/pcie_utils.cpp
trunk/src/lib/api_pcie/src/c_image_handle.h
trunk/src/lib/api_pcie/src/c_image_handle.cpp
trunk/src/lib/api_pcie/src/fuse/fuse.h
trunk/src/lib/api_pcie/src/fuse/fuse.cpp
trunk/src/lib/api_pcie/src/fuse/fuse_types.h
trunk/src/app/pcie_vc/SystemFiles/BIN/SITAU.XML
trunk/src/app/pcie_vc/SystemFiles/BIN_NOBF/SITAU.XML
trunk/src/app/MatLab/Toolbox_sitau2/BIN/SITAU.XML
trunk/src/app/MatLab/Toolbox_sitau2/BIN_NOBF/SITAU.XML
```

## 12. Secuencia recomendada para `EXECUTE`

Para reducir el riesgo, implementar en commits o bloques revisables:

1. Parser XML y validaciones de topología.
2. Propagación dinámica hasta `ST2_Start`.
3. Configuración UCI para todas las bases.
4. Tamaños DMA dinámicos.
5. `CImageHandle` y strides dinámicos.
6. Ordenación y saw-check dinámicos.
7. Pruebas sintéticas y regresión.
8. Prueba hardware y resolución del layout FPGA.

Después de cada bloque:

- compilar;
- ejecutar las pruebas disponibles;
- revisar el diff;
- actualizar este checklist;
- no avanzar si falla la regresión de cuatro bases.

## 13. Riesgos conocidos

| Riesgo | Mitigación |
|---|---|
| El FPGA envía dos paquetes de cuatro bases | Encapsular el layout y validar cabecera/tamaño antes de interpretar |
| Corrupción de memoria en mensajes extendidos | Índices locales, inicialización completa y pruebas 5/8 bases |
| Desbordamiento al calcular buffers grandes | `size_t`, multiplicaciones comprobadas y límites previos a reservar |
| Regresión de 128 canales | Caso de cuatro bases obligatorio en todas las pruebas |
| XML desplegado diferente al editado | Registrar ruta efectiva de `OpenSys` y comprobar el XML copiado a `BIN` |
| Firmware UCI incompatible | Inspeccionar mensajes, probar hardware gradualmente y conservar errores explícitos |

## 14. Evidencia que debe acompañar al cierre

Al completar la implementación se adjuntará:

- Diff de los archivos modificados.
- Resultado de compilación.
- Tabla de pruebas 1/4/5/8 bases.
- Tamaños calculados y observados de los bloques.
- Volcado o resumen de mensajes UCI normal y extendido.
- Resultado de regresión con 4 bases.
- Resultado de adquisición con 8 bases.
- Decisión final sobre el layout FPGA.
- Estado del comentario `TODO(PCIe topology)`.

