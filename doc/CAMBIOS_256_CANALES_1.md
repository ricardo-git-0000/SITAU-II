# Resumen de Cambios Aplicados (256 Canales SITAU-II)

Se ha creado la rama Git **`feature/256-channels`** y se han aplicado y confirmado (*committed*) todos los cambios necesarios para habilitar el soporte de 256 canales en el sistema SITAU-II.

---

## 1. Archivos Modificados

### A. Librería `stlib2.dll` (Lógica Central)

#### 1. [c_pa_system.cpp](file:///c:/Users/Mario/Desktop/DEVELOPMENT/SITAU/SITAU-II/trunk/src/lib/stlib2/src/HW/c_pa_system.cpp#L95-L125)
* **Ubicación:** `trunk/src/lib/stlib2/src/HW/c_pa_system.cpp`
* **Cambio:** Se añadió el caso `case 256:` en la sentencia `switch(n_multiplexed_channels)` de la función `CPASystem::Setup`.
* **Detalle:** Configura 2 conectores multiplexados con desfase de 128 canales para direccionar correctamente la tabla de hardware.
```cpp
   case 256:
      sys_.multiplexed = true;
      sys_.n_bases = 1;
      sys_.n_active_channels = MOD_MAX_CH;
      sys_.n_multiplexed_channels = n_multiplexed_channels;
      sys_.n_total_channels = sys_.n_multiplexed_channels;
      sys_.connector.resize(2);
      for (i=0; i<sys_.connector.size(); i++)
         sys_.connector[i].start_channel = i * 128;
      break;
```

#### 2. [c_virtual_channel.h](file:///c:/Users/Mario/Desktop/DEVELOPMENT/SITAU/SITAU-II/trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.h#L38)
* **Ubicación:** `trunk/src/lib/stlib2/src/c_virtual_channel/c_virtual_channel.h`
* **Cambio:** Se actualizó la constante `VCH_MAX_LINES` de 128 a 256.
```cpp
- #define VCH_MAX_LINES                128
+ #define VCH_MAX_LINES                256
```

#### 3. [st_load.h](file:///c:/Users/Mario/Desktop/DEVELOPMENT/SITAU/SITAU-II/trunk/src/lib/stlib2/src/st_load.h#L30)
* **Ubicación:** `trunk/src/lib/stlib2/src/st_load.h`
* **Cambio:** Se actualizó la constante de número de elementos `DCFG_PA_NE` de 128 a 256.
```cpp
- #define DCFG_PA_NE      128
+ #define DCFG_PA_NE      256
```

---

### B. Librería `api_pcie.dll` (Comunicación PCIe / DMA)

#### 4. [stlib2_wrapper.cpp](file:///c:/Users/Mario/Desktop/DEVELOPMENT/SITAU/SITAU-II/trunk/src/lib/api_pcie/src/stlib2/stlib2_wrapper.cpp#L514)
* **Ubicación:** `trunk/src/lib/api_pcie/src/stlib2/stlib2_wrapper.cpp`
* **Cambio:** Se incrementó el límite de verificación de bloques por ráfaga DMA (`ST_PCIe_GetBlocksBurstNumber()`) de 256 a 512.
```cpp
- if (ST_PCIe_GetBlocksBurstNumber() > 256) return ELOG(charEPCIE_GTX_BLOCKS, EPCIE_GTX_BLOCKS);
+ if (ST_PCIe_GetBlocksBurstNumber() > 512) return ELOG(charEPCIE_GTX_BLOCKS, EPCIE_GTX_BLOCKS);
```

---

### C. Configuración de la Aplicación (XML)

#### 5. [SITAU2_PCIe_TEST.xml](file:///c:/Users/Mario/Desktop/DEVELOPMENT/SITAU/SITAU-II/trunk/src/app/pcie_vc/AppFiles/SITAU2_PCIe_TEST.xml#L57)
* **Ubicación:** `trunk/src/app/pcie_vc/AppFiles/SITAU2_PCIe_TEST.xml`
* **Cambio:** Se expandió el parámetro `<Parameter pName="pin_to_channel" ... />` a 256 elementos (0 al 255) siguiendo la secuencia patrón del hardware para 8 módulos de 32 canales cada uno.

---

## 2. Estado de Git y Confirmación

Todos estos cambios fueron confirmados localmente en la rama `feature/256-channels`.
