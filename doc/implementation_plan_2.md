# Diagnóstico Completo: SITAU-II para 256 Canales

## Veredicto General

> [!CAUTION]
> **Los cambios aplicados hasta ahora son necesarios pero NO suficientes para que el sistema funcione con 256 canales.** Se han identificado 7 problemas críticos adicionales que causarían crashes, corrupción de datos o pérdida silenciosa de la mitad de los canales.

El código tiene **dos capas de problemas**: los que ya corregimos (la capa de configuración del sistema) y los que quedan pendientes (la capa de transporte de datos y el driver PCIe).

---

## Arquitectura del Software (Cómo Encaja Todo)

```
┌─────────────────────────────────────────────────────────────┐
│  APLICACIÓN (pcie_vc.exe / MATLAB)                         │
│  Lee XML → Llama ST2_OpenSys → Configura canales → Adquiere│
└───────────────────┬─────────────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────────────┐
│  stlib2.dll  (Lógica Central)                               │
│  ┌────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │ CPASystem      │  │ CVirtualChannel │  │ CXML_CONFIG  │ │
│  │ Topología HW   │  │ Leyes Focales   │  │ Carga XML    │ │
│  │ ✅ Corregido   │  │ ⚠️ Parcial      │  │ ✅ Corregido │ │
│  └────────────────┘  └─────────────────┘  └──────────────┘ │
└───────────────────┬─────────────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────────────┐
│  api_pcie.dll  (Comunicación PCIe + DMA)                    │
│  ┌────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │ stlib2_wrapper │  │ c_image_handle  │  │ fuse/pcie    │ │
│  │ Mapeo canales  │  │ Tamaño buffer   │  │ Driver DMA   │ │
│  │ ✅ Dinámico    │  │ ❌ N_MODULES=4  │  │ ❌ 4 frames  │ │
│  └────────────────┘  └─────────────────┘  └──────────────┘ │
└───────────────────┬─────────────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────────────┐
│  HARDWARE SITAU-II (FPGA + Frontend Multiplexado 256 ch)    │
└─────────────────────────────────────────────────────────────┘
```

---

## Estado Actual: Cambios Ya Aplicados ✅

| Fichero | Cambio | Estado |
|---|---|---|
| `c_pa_system.cpp` | Añadido `case 256:` con 2 conectores | ✅ OK |
| `c_virtual_channel.h` | `VCH_MAX_LINES` 128 → 256 | ✅ OK |
| `st_load.h` | `DCFG_PA_NE` 128 → 256 | ✅ OK |
| `stlib2_wrapper.cpp` | Límite DMA 256 → 512 bloques | ✅ OK |
| `c_xml_config.cpp` | Fallback `pin_to_channel` 128 → VCH_MAX_LINES | ✅ OK |
| `SITAU2_PCIe_TEST.xml` | `pin_to_channel` ampliado a 256 elementos | ✅ OK |

---

## Problemas Críticos Pendientes ❌

### CRÍTICO 1 — `pcie_tad.h` línea 52
```cpp
// ACTUAL (INCORRECTO):
#define N_MODULES   4    // 4 × 32 = 128 canales

// CORRECTO:
#define N_MODULES   8    // 8 × 32 = 256 canales
```
**Impacto**: Esta constante afecta a 3 ficheros de forma en cadena (ver problemas 2 y 3).

---

### CRÍTICO 2 — `pcie.cpp` línea 796 (Stack Overflow)
```cpp
// ACTUAL (INCORRECTO - array de tamaño fijo en la pila):
unsigned short channel_count[N_CHANNELS_MODULE * N_MODULES]; // = channel_count[128]

// Línea 818-819: el bucle escribe índices 0..255 → DESBORDAMIENTO DE PILA
channel_count[channel_module_index + channel_offset] = ...;
```
**Impacto**: **Crash inmediato en la primera adquisición.** Escritura fuera de los límites del array en la pila → corrupción de memoria o violación de acceso.

---

### CRÍTICO 3 — `c_image_handle.cpp` línea 86 (Tamaño de buffer a la mitad)
```cpp
// ACTUAL (INCORRECTO):
num_shorts_per_focal_law_ = frame_length * N_CHANNELS_MODULE * N_MODULES;
// Con N_MODULES=4: calcula espacio para 128 canales

// CORRECTO: Con N_MODULES=8 quedará bien automáticamente.
```
**Impacto**: Los punteros de imagen calculan la mitad del tamaño real → las lecturas de los canales 129-256 leen datos del disparo siguiente → imagen corrupta.

---

### CRÍTICO 4 — `fuse/fuse_types.h` línea 29 y `fuse/fuse.cpp` línea 1339
```cpp
// ACTUAL (INCORRECTO):
#define FRAMES_PER_PACKAGE  4  // 1 frame por módulo × 4 módulos = 128 canales

// CORRECTO:
#define FRAMES_PER_PACKAGE  8  // 1 frame por módulo × 8 módulos = 256 canales
```
**Impacto**: El driver DMA reserva la **mitad** del buffer necesario. Los 128 canales superiores nunca se transfieren. El bucle de extracción `for (frm = 0; frm < FRAMES_PER_PACKAGE; frm++)` en `fuse.cpp:1409` solo lee 4 de las 8 tramas.

---

### CRÍTICO 5 — `xdma_driver/libxdma/fuse_emul.c` línea 227
```cpp
// ACTUAL (INCORRECTO):
#define FRAMES_PER_PACKAGE  4

// CORRECTO:
#define FRAMES_PER_PACKAGE  8
```
**Impacto**: La emulación del driver (usada en pruebas sin hardware) también tendrá el buffer a la mitad → el script de MATLAB de test tampoco funcionará bien.

---

### IMPORTANTE 6 — `c_virtual_channel.cpp` líneas 267, 298-299 (Defaults erróneos)
```cpp
// Línea 267 - Sonda PA inicializada con 128 elementos por defecto:
cfg_.pa_probe = new CFL_PAUI(128, 0.3, 0, 10, 0);
// Corrección: debería ser VCH_MAX_LINES o leerlo del XML

// Líneas 298-299 - TFM hardcoded a 127 como elemento final:
cfg_.tfm.emi_end_element = 127;
// Corrección: debería ser VCH_MAX_LINES - 1
```
**Impacto**: Al arrancar con la configuración por defecto (sin XML), el sistema se configura a 128 canales aunque la sonda tenga 256.

---

### IMPORTANTE 7 — `t_base_tad.h` línea 115 y arrays `anglesMemory[MAX_PWI_FL]`
```cpp
// ACTUAL:
#define MAX_PWI_FL  128   // Límite de ángulos para modo onda plana (PWI) en TFM

// Arrays estáticos que dependen de esto:
// protocol_uci.h:215 → unsigned short anglesMemory[MAX_PWI_FL];
// t_vch.h:64         → u16 anglesMemory[MAX_PWI_FL];
```
**Impacto**: En modo TFM/PWI con más de 128 ángulos, los ángulos adicionales son silenciosamente ignorados. Este límite NO está relacionado con el número de canales sino con el número de ángulos de inspección — puede ser aceptable dependiendo de tu configuración de uso.

---

### PENDIENTE 8 — `c_xml_sitau_hw.cpp` líneas 775-794 (Defaults del hardware)
```cpp
// ResetData() inicializa siempre con 128 canales:
data_.sitau_hardware.frontend[0].n_multiplexed_channels = 128;
data_.sitau_hardware.frontend[0].n_channels = 128;
data_.sitau_hardware.frontend[0].connector[0].pin_to_channel.resize(128);
data_.sitau_hardware.frontend[0].connector[0].pin_disabled.resize(128);
```
**Impacto**: Si el XML del hardware no especifica explícitamente 256 canales, el sistema arrancará con la configuración por defecto de 128.

---

## Plan de Acción Priorizado

### Fase 1 — Bloqueantes (CRASHES / CORRUPCIÓN DE DATOS)
Estos deben corregirse ANTES de cualquier prueba:

1. **`pcie_tad.h:52`** → `N_MODULES 4` → `8`
2. **`fuse/fuse_types.h:29`** → `FRAMES_PER_PACKAGE 4` → `8`
3. **`xdma_driver/libxdma/fuse_emul.c:227`** → `FRAMES_PER_PACKAGE 4` → `8`

> [!NOTE]
> Los cambios 2 y 3 de `c_image_handle.cpp` y `pcie.cpp` se corrigen **automáticamente** al actualizar `N_MODULES` en `pcie_tad.h` (son expresiones con `N_MODULES`, no valores literales).

### Fase 2 — Importante (COMPORTAMIENTO INCORRECTO)
4. **`c_virtual_channel.cpp:267`** → `new CFL_PAUI(128, ...)` → `new CFL_PAUI(VCH_MAX_LINES, ...)`
5. **`c_virtual_channel.cpp:298-299`** → `emi_end_element = 127` → `VCH_MAX_LINES - 1`
6. **`c_xml_sitau_hw.cpp:775-794`** → Cambiar valores por defecto de 128 a 256

### Fase 3 — Condicional (DEPENDE DEL USO)
7. **`t_base_tad.h:115`** → `MAX_PWI_FL 128` → Solo necesario si usáis más de 128 ángulos PWI en TFM

---

## Respuesta Directa a Tu Pregunta

**¿Funcionará para 256 canales?**

Con los cambios actuales: **No.** El sistema arrancaría, reconocería los 256 canales en la capa de configuración, pero en cuanto se intentase una adquisición real:
1. La primera adquisición causaría un **crash por desbordamiento de pila** (`channel_count[128]` escrito hasta el índice 255) — `pcie.cpp:796`.
2. Incluso si no crashease, el driver DMA solo transferiría los datos de los primeros **128 canales** porque `FRAMES_PER_PACKAGE = 4`.

Con los cambios de Fase 1 adicionales: **Probablemente sí funcione** para los modos PA y MC estándar. Los cambios de Fase 2 mejoran la robustez de la inicialización por defecto.
