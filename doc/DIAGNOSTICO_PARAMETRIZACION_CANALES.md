# Diagnóstico y plan de parametrización de canales SITAU-II

## Alcance

Este documento es un diagnóstico de solo lectura del estado actual del repositorio. No contiene cambios de código ni pretende aplicar automáticamente ninguna de las correcciones propuestas.

El objetivo es que el número total de canales pueda ser 32, 64, 128 o 256 y que la configuración se gobierne desde `BASES_NUMBER` en `SITAU.XML`, evitando constantes independientes como `N_MODULES`, tamaños de arrays o cantidades de tramas.

## Conclusión ejecutiva

Hay dos problemas distintos:

1. El error actual `Error ST2_OpenSys(): -1912` ocurre antes de que el sistema llegue a validar la topología de canales. El log identifica que falta `BOOT.bin` en la ruta recibida por `ST2_OpenSys()`.
2. Aunque se corrija la ruta y el arranque pase, el código todavía no está parametrizado de extremo a extremo. Parte de la configuración usa `BASES_NUMBER`, otra usa valores derivados del hardware y la capa PCIe/DMA usa constantes de compilación que representan 256 canales.

La solución debe separar:

- canales lógicos configurados;
- canales multiplexados;
- módulos físicos y tramas PCIe;
- capacidad máxima compilada.

No se debe hacer simplemente `N_MODULES = BASES_NUMBER`: `BASES_NUMBER` se carga en tiempo de ejecución, mientras que `N_MODULES` se usa hoy como constante de compilación en la DLL PCIe.

## Diagnóstico de `ST2_OpenSys(): -1912`

### Evidencia directa

[LOG_STLIB2.TXT](../trunk/src/app/MatLab/Examples/LOG_STLIB2.TXT) registra:

```text
(App Error Code: -1912) Error, '..\\Toolbox_sitau2\\BIN\\BOOT.bin' doesn't exist
FILE: st_reg.cpp
LINE: 523
FUNCTION: ST_ConfigSys
```

En [st_error_code.h](../trunk/src/lib/stlib2/src/st_error_code.h), `st_error_code_start = -1900` y `ES2_FILE = st_error_code_start - 12`; por tanto, `-1912` significa **error de acceso o existencia de fichero**, no error de número de canales.

La cadena es:

```text
MATLAB OpenSys.m
  -> api_pcie_x64.dll::ST2_OpenSys()
     -> stlib2::ST_OpenSys()
        -> ST_OpenHW()
           -> ST_ConfigSys()
              -> comprueba path + BOOT.bin + firmware
```

En [st_reg.cpp](../trunk/src/lib/stlib2/src/st_reg.cpp), `ST_ConfigSys()` concatena la carpeta recibida con `FILENAME_CPU1` y `FILENAME_BASE`. El primer argumento es una **carpeta de binarios**, no un XML individual.

### Causa primaria

Los scripts [Test_256_Channels_Emulation.m](../trunk/src/app/MatLab/Examples/Test_256_Channels_Emulation.m) y [TEST_256_CANALES_MATLAB.m](../doc/TEST_256_CANALES_MATLAB.m) construyen `xml_path` apuntando a `SITAU2_PCIe_TEST.xml` y lo pasan a `OpenSys()`. Sin embargo, `OpenSys()` pasa ese argumento a `ST2_OpenSys()`, cuyo parámetro `path_bin` debe ser una carpeta que contenga los recursos de firmware y `SITAU.XML`.

En el árbol actual hay copias de `BOOT.bin`, entre otras, en:

- `trunk/src/app/pcie_vc/SystemFiles/BIN/BOOT.bin`;
- `trunk/make/pcie_vc/app/bin/BOOT.bin`.

Las carpetas `trunk/src/app/MatLab/Toolbox_sitau2/BIN` y `BIN_NOBF` contienen actualmente `SITAU.XML`, pero no `BOOT.bin`. Además, la copia de `SITAU.XML` de MATLAB no tiene `BASES_NUMBER` explícito.

### Causas secundarias

- El script de emulación usa `pcie_enabled = 1`; la emulación desactiva el driver físico cuando `ST_GetNoHardware() == 1`, pero los recursos y firmas de emulación deben seguir siendo coherentes.
- Los scripts usan IP y puertos distintos de los ejemplos históricos. En modo `NoHardware` no deberían causar `-1912`, pero sí pueden ocultar el siguiente fallo.
- MATLAB debe cargar las DLL que corresponden a la compilación actual, no otra DLL encontrada en el `PATH`.
- La solución de aplicación usa `SystemFiles/BIN`, mientras que MATLAB busca normalmente `Toolbox_sitau2/BIN`; las copias de firmware/XML deben tratarse como artefactos de despliegue coordinados.

### Comprobación previa

Antes de analizar canales hay que verificar que la carpeta usada por la prueba contiene conjuntamente:

1. `BOOT.bin`;
2. el firmware base que representa `FILENAME_BASE`;
3. `SITAU.XML`;
4. el bitstream requerido;
5. el XML realmente seleccionado por la prueba.

El log debe dejar de mostrar el mensaje de fichero inexistente antes de evaluar canales.

## Flujo actual de `BASES_NUMBER`

`BASES_NUMBER` se define en [c_xml_sitau_hw.h](../trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.h) y se lee en `CXML_SITAU_HW::GetData_HardwareConfiguration()` de [c_xml_sitau_hw.cpp](../trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp). El valor dimensiona `data_.sitau_hardware.base`.

El flujo actual es:

```text
SITAU.XML: BASES_NUMBER
  -> data_.sitau_hardware.base.size()
  -> CPASystem(n_bases, n_multiplexed_channels)
  -> n_active_channels = n_bases * MOD_MAX_CH si no hay multiplexado
  -> CVirtualChannel crea una base de 32 canales por cada base
```

En el modelo no multiplexado actual, `BASES_NUMBER` significa **número de bases de 32 canales**, no número total de canales:

| Canales totales | BASES_NUMBER | Módulos de 32 |
|---:|---:|---:|
| 32 | 1 | 1 |
| 64 | 2 | 2 |
| 128 | 4 | 4 |
| 256 | 8 | 8 |

Si el requisito exige que el XML contenga directamente 32/64/128/256, hay que cambiar el contrato y derivar `n_bases = BASES_NUMBER / 32`. Recomendación: conservar el significado actual de “número de bases” y documentar que los valores válidos son 1, 2, 4 y 8.

## Hallazgos de parametrización

### Constantes PCIe de compilación

En [pcie_tad.h](../trunk/src/lib/api_pcie/src/pcie_tad.h), `N_CHANNELS_MODULE = 32` y `N_MODULES = 8`. En [pcie.cpp](../trunk/src/lib/api_pcie/src/pcie.cpp), estos valores controlan `channel_count`, offsets de módulos y beats por canal.

La compilación actual permite 256, pero no permite cambiar a 32/64/128 sin recompilar o introducir una configuración de sesión. `N_CHANNELS_MODULE = 32` parece una propiedad física del módulo y puede mantenerse; `N_MODULES` debe convertirse en capacidad máxima o valor derivado del contexto de adquisición.

### DMA y emulación fijados a ocho tramas

`FRAMES_PER_PACKAGE` aparece en [fuse_types.h](../trunk/src/lib/api_pcie/src/fuse/fuse_types.h), [fuse.cpp](../trunk/src/lib/api_pcie/src/fuse/fuse.cpp) y [fuse_emul.c](../trunk/src/lib/api_pcie/src/xdma_driver/libxdma/fuse_emul.c), con valor 8.

Hay que confirmar si el hardware siempre transmite paquetes físicos de ocho tramas aunque se utilicen 32/64/128 canales. Si es así, el transporte puede reservar ocho y la interpretación debe limitarse a los módulos activos. Si no es así, el número debe llegar como configuración de sesión. Cambiar 8 sin comprobar el protocolo FPGA podría romper la alineación DMA.

### Sistema PA

En [c_pa_system.cpp](../trunk/src/lib/stlib2/src/HW/c_pa_system.cpp), `CPASystem::Setup()` tiene casos para 128, 256 y 512 multiplexados, pero no para 32 ni 64 multiplexados. El `default` trata el sistema como no multiplexado.

Debe existir una sola regla:

- no multiplexado: `n_total_channels = n_bases * 32`;
- multiplexado: `n_total_channels = n_multiplexed_channels`.

Con ella se deben generar conectores, `start_channel`, `pin_to_channel` y `disabled_channel`.

### Defaults impuestos de 256

En [c_xml_sitau_hw.cpp](../trunk/src/lib/stlib2/src/HW/c_xml_sitau_hw.cpp), `ResetData()` fija a 256 `n_multiplexed_channels`, `n_channels`, `n_total_channels` y los vectores de mapeo. Un XML incompleto puede arrancar como 256 aunque `BASES_NUMBER` sea 1, 2 o 4.

El default debe calcularse después de leer la configuración o el XML debe rechazarse por incompleto.

### Canales virtuales

En [c_virtual_channel.cpp](../trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.cpp), `VCH_MAX_LINES = 256` debe ser solo capacidad máxima. La sonda y los límites TFM deben usar el número activo.

En [c_xml_config.cpp](../trunk/src/lib/stlib2/src/c_xml_config.cpp) el fallback de `pin_to_channel` construye 256 elementos. Debe dimensionarse con el número activo o diferirse hasta cargar el hardware.

### XML de prueba incoherente

`SITAU2_PCIe_TEST.xml` declara `probe_elements_number = 128` pero tiene un `pin_to_channel` de 256 entradas. Además, es configuración de aplicación/virtual channel; `ST2_OpenSys()` carga `SITAU.XML` desde la carpeta binaria.

Debe distinguirse:

- `SITAU.XML`: hardware, bases, frontend y buffers;
- XML de adquisición/virtual channel: sonda, leyes, apertura y modo.

Ambos deben ser coherentes.

## Plan archivo por archivo

### Fase 0: arranque y despliegue

#### `trunk/src/app/MatLab/Toolbox_sitau2/OpenSys.m`

- Hacer explícito que `path` es una carpeta `BIN`, no un fichero XML.
- Separar la carpeta de firmware del XML de adquisición.
- Validar `BOOT.bin`, firmware base y `SITAU.XML` antes de llamar a la DLL.
- Mantener sincronizadas las firmas MATLAB, headers y DLL.

#### Scripts MATLAB de prueba

Ficheros: `Test_256_Channels_Emulation.m` y `doc/TEST_256_CANALES_MATLAB.m`.

- No pasar `xml_path` como primer argumento de `OpenSys()`.
- Seleccionar una carpeta de binarios válida.
- Parametrizar las comprobaciones y mensajes con `expected_channels`.
- Crear la misma prueba para 32, 64, 128 y 256.
- Reportar la carpeta efectiva, el XML efectivo y el número devuelto antes de configurar PA.
- No depender de `pwd` para resolver rutas.

#### Carpetas de artefactos

Ficheros/carpeta: `SystemFiles/BIN`, `SystemFiles/BIN_NOBF`, `Toolbox_sitau2/BIN`, `Toolbox_sitau2/BIN_NOBF` y `trunk/make/pcie_vc/app/bin`.

- Elegir una fuente reproducible de artefactos.
- Mantener juntos DLL, `BOOT.bin`, bitstream y `SITAU.XML`.
- No mezclar `BIN` y `BIN_NOBF` sin identificar su correspondencia.

### Fase 1: fuente única del número de canales

#### `c_xml_sitau_hw.h`

- Mantener `BASES_NUMBER` como número de bases de 32 canales.
- Validar valores 1, 2, 4 u 8.
- Revisar `XML_DEF_N_BASES` y `XML_MAX_N_MODULES_BASE`, que hoy imponen 4 en algunos defaults.
- Exponer un total derivado para que las capas superiores no vuelvan a interpretar el XML.

#### `c_xml_sitau_hw.cpp`

- Leer `BASES_NUMBER` una sola vez.
- Derivar bases, módulos y total.
- Eliminar defaults independientes de 256 en `ResetData()`.
- Dimensionar frontend, conectores, `pin_to_channel` y `pin_disabled`.
- Validar coherencia entre frontend, conectores y mapeo.
- Definir precedencia entre `BASES_NUMBER` y `n_multiplexed_channels`; nunca permitir contradicciones silenciosas.

#### `t_sitau_hw.h` y `t_pa_system_tad.h`

- Distinguir capacidad máxima de número activo.
- Inicializar `n_modules` y `n_modules_base` desde configuración.
- Mantener las invariantes:
  - no multiplexado: `n_total_channels = n_bases * 32`;
  - multiplexado: `n_total_channels = n_multiplexed_channels`.

### Fase 2: PA y canales virtuales

#### `c_pa_system.cpp/.h`

- Sustituir el `switch` cerrado por una derivación común para 32/64/128/256.
- Calcular bases, total, conectores y desplazamientos desde configuración.
- Mantener `MOD_MAX_CH = 32` como propiedad de una base.
- Dimensionar todos los vectores con `n_total_channels`.
- Rechazar combinaciones imposibles antes de crear leyes focales.

#### `c_virtual_channel.cpp/.h`

- Usar `VCH_MAX_LINES = 256` solo como límite máximo.
- Crear la sonda con el número activo.
- Sustituir límites que representen el último elemento activo por `n_channels - 1`.
- Revisar PA, TFM, FMC y PWI para que no fuercen 256.

#### `c_xml_config.cpp`

- Dimensionar el fallback de `pin_to_channel` con el hardware activo.
- No generar 256 silenciosamente cuando el hardware aún no se ha cargado.
- Validar que `probe_elements_number`, mapeo y leyes focales son compatibles.

#### `c_vch_load.cpp`, `c_vch_config.cpp` y `c_vch_fl_pa.cpp`

- `MOD_MAX_CH` debe controlar el interior de cada base.
- `n_bases` debe controlar el número de bases.
- El total debe controlar la API global.
- Revisar defaults de apertura, elemento final y listas de elementos.
- Probar PA, FMC, TFM y PWI por separado.

### Fase 3: PCIe y DMA

#### `pcie_tad.h` y `pcie.h`

- Mantener `N_CHANNELS_MODULE = 32` si es propiedad física.
- Retirar `N_MODULES` como número activo global.
- Si se conserva como capacidad máxima, renombrarlo conceptualmente y no usarlo para tamaños lógicos.
- Introducir una configuración de sesión derivada de hardware.

#### `pcie.cpp`

- Recibir módulos/canales activos desde la configuración cargada.
- Dimensionar `channel_count`, offsets y beats con el valor activo.
- Revisar el array `channel_count[N_CHANNELS_MODULE*N_MODULES]` y el bucle de módulos.
- Validar que no se lean módulos no configurados.
- Probar orden de canales y `pin_to_channel` para los cuatro casos.

#### `c_image_handle.cpp`

- Derivar `num_shorts_per_focal_law_` del número activo o de la geometría real del paquete.
- Revisar offsets de frame, focal law, adquisición y buffer circular.

#### `fuse_types.h`, `fuse.cpp` y `fuse_api.cpp`

- Unificar la fuente de `FRAMES_PER_PACKAGE`.
- Confirmar el formato físico: ocho frames fijos o frames activos.
- Derivar el tamaño de imagen, cabeceras y bucle de extracción del mismo contrato.
- Actualizar mensajes que aún hablan de un máximo GTX de 256 si el límite operativo es 512.

#### `xdma_driver/libxdma/fuse_emul.c`

- Mantener la emulación idéntica al transporte real.
- Compartir la definición conceptual de frames y tamaño de imagen.
- Añadir pruebas de longitud de buffer para 32/64/128/256.

#### `stlib2_wrapper.cpp`

- Después de `ST_OpenSys()`, tomar `ST_GetChannelsNumber()` como configuración de sesión.
- Propagarlo a PCIe antes de habilitar recepción.
- Validar que `gb_pin_to_channel` coincide con el total activo.
- No usar `N_MODULES` para el tamaño de esa tabla.

### Fase 4: XML y ejemplos

#### XML hardware

Ficheros: `SystemFiles/BIN/SITAU.XML`, `BIN_NOBF/SITAU.XML`, `Toolbox_sitau2/BIN/SITAU.XML` y `BIN_NOBF/SITAU.XML`.

- Crear perfiles válidos para `BASES_NUMBER = 1, 2, 4, 8`.
- Propagar la misma variante a todos los artefactos.
- No dejar `BASES_NUMBER = 4` como único XML cuando se prueban 256.
- Asegurar que MATLAB no carga una copia distinta de la aplicación.

#### `SITAU2_PCIe_TEST.xml`

- Hacer coincidir `probe_elements_number` con el número de entradas de `pin_to_channel`.
- Crear perfiles 32/64/128/256 o generar el mapeo desde el parámetro.
- No confundir capacidad máxima 256 con canales activos 128.

#### Ejemplos MATLAB

Revisar `SITAU_PRUEBAS_PA_M.m`, `SITAU_PRUEBAS_PA_RAFAGAS.m`, `SITAU_PRUEBAS_PWI_M.m`, `SITAU_PRUEBAS_PWI_RAFAGAS.m`, `Demo_GE_Noise.m`, `Demo_PCIe_noise.m` y `Demo_GE_MC_PA.m`.

- Sustituir arrays y gráficos fijados a 128 por `ST2_GetChannelsNumber()`.
- No confundir apertura activa 32 con total de canales 32.
- Revisar indexación MATLAB 1-based frente a C/C++ 0-based.
- Usar el último elemento válido de la sesión.

### Fase 5: límites que no son canales

#### `trunk/src/lib/stlib2/src/HW/MK32/t_base_tad.h`

`MAX_PWI_FL = 128` parece ser un máximo de leyes/ángulos PWI, no de canales. No debe cambiarse automáticamente por soportar 256 canales; solo si se requiere superar 128 ángulos.

#### `st_range.cpp`, `st_load.h` y buffers generales

Revisar `DCFG_PA_NE`, `TRM_MAX_N_LINES` y límites de procesamiento como capacidades máximas. Deben permanecer independientes del número activo salvo que el protocolo imponga lo contrario.

## Matriz esperada

| Caso | Bases | Canales activos | Módulos PCIe lógicos | Elementos/mapeo | Frames DMA |
|---|---:|---:|---:|---:|---:|
| 32 | 1 | 32 | derivado | 32 | según protocolo |
| 64 | 2 | 64 | derivado | 64 | según protocolo |
| 128 | 4 | 128 | derivado | 128 | según protocolo |
| 256 | 8 | 256 | derivado | 256 | según protocolo |

La columna de frames no debe fijarse a `canales / 32` hasta confirmar que cada frame representa exactamente un módulo lógico y que el hardware no empaqueta siempre ocho.

## Orden recomendado

1. Corregir la ruta de `ST2_OpenSys()` y demostrar que desaparece `-1912`.
2. Formalizar `BASES_NUMBER` y validar valores.
3. Hacer que `CXML_SITAU_HW` produzca un único total coherente.
4. Eliminar defaults de 256 y casos PA dispersos.
5. Propagar el número activo a virtual channels, buffers y PCIe.
6. Resolver el contrato físico de frames DMA.
7. Adaptar XML y MATLAB para una matriz 32/64/128/256.
8. Compilar ambas soluciones y comprobar qué DLL carga MATLAB.

## Criterios de aceptación

Para cada caso:

- `ST2_OpenSys()` devuelve cero en emulación con una carpeta válida.
- El log no contiene errores de `BOOT.bin`, XML ni inconsistencias.
- `ST2_GetChannelsNumber()` devuelve 32, 64, 128 o 256.
- Elementos de sonda y `pin_to_channel` coinciden.
- PA usa el último elemento válido.
- TFM/FMC/PWI no acceden fuera de rango.
- Frame, imagen y buffer tienen el tamaño esperado.
- La emulación no desborda ni pierde módulos.
- El orden físico de canales coincide con emulación cuando exista hardware.

## Estado de este análisis

Este informe se ha añadido como documentación. No se ha modificado ningún fichero de código. Las modificaciones locales, artefactos compilados y documentos previos que ya existían en el repositorio se han preservado.

