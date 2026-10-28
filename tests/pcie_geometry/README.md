# Validación PCIe de 128/256 canales

## Estado y alcance

Implementación en fuentes preparada para MC paralelo, canal virtual 0 y sonda completa. El asistente no compila Visual Studio. Las comprobaciones estáticas se han ejecutado; los dos ejecutables C++, las DLL y las pruebas físicas quedan pendientes del usuario.

No hay que cambiar una constante de módulos ni añadir otro parámetro al XML de adquisición: `BASES_NUMBER=4/8` en el SITAU.XML que se carga determina 128/256 canales. El mapa automático o el patrón válido de 128 ampliado por stlib2 siguen disponibles. Un mapa explícito completo tiene que ser una permutación válida.

## 1. Comprobaciones sin compilación

Desde la raíz del repositorio, con Python 3:

```powershell
python tests/pcie_geometry/test_pcie_geometry_static.py
python tests/error_reporting/test_error_catalog.py
python tests/pcie_geometry/prepare_manager_test.py
```

La tercera orden genera `generated/types.inc` y `generated/manager.inc` desde los fuentes actuales. No compila ni ejecuta el gestor. No versionar `generated/` ni los ejecutables; la carpeta contiene un `.gitignore` local.

## 2. Ejecutables C++ sin DLL ni hardware

Abrir **x64 Native Tools Command Prompt for VS 2022**, situarse en la raíz del repositorio y ejecutar, después del paso anterior:

```text
cl /nologo /EHsc /W4 /std:c++14 tests\pcie_geometry\test_pcie_geometry.cpp /Fe:tests\pcie_geometry\test_pcie_geometry.exe /Fo:tests\pcie_geometry\test_pcie_geometry.obj
tests\pcie_geometry\test_pcie_geometry.exe
cl /nologo /EHsc /W4 /std:c++14 /utf-8 tests\pcie_geometry\test_pcie_stream.cpp /Fe:tests\pcie_geometry\test_pcie_stream.exe /Fo:tests\pcie_geometry\test_pcie_stream.obj
tests\pcie_geometry\test_pcie_stream.exe
```

Ambos deben terminar con `PASS` y código de salida 0. Las comprobaciones permanecen activas en Release. El segundo utiliza la clase CImageHandle y T_block_header reales extraídos del código; sustituye únicamente el transporte, logger y callbacks externos. Requiere el formato nativo Windows (`unsigned long` de 32 bits). No prueba ReadFile ni el driver. Volver a ejecutar `prepare_manager_test.py` si cambia el gestor.

Cobertura: 128/256/128, geometría inválida, mapas cortos/duplicados/fuera de rango, destino protegido por centinelas, orden de todos los canales, salto bruto entre leyes, imágenes que cruzan bloques, varias imágenes por bloque, vueltas del FIFO, último bloque parcial, liberación final, cabecera corrupta y error DMA simulado. El test de saw detecta una discontinuidad introducida y protege la capacidad del array de contadores.

## 3. Compilar y desplegar

1. Compilar **stlib2, Debug | x64, v143**.
2. Actualizar la biblioteca de importación `stlib2_x64.lib` y la DLL que consume el proyecto api_pcie. Es imprescindible por la exportación nueva `ST_GetModulesNumber` de la fase 1.
3. Compilar **api_pcie, Debug | x64, v141**. Si se usa la consola, compilar también pcie_vc.
4. Cerrar MATLAB, copiar **stlib2_x64.dll y api_pcie_x64.dll** junto con sus PDB a la ubicación que cargan las demos y volver a abrir MATLAB. No mezclar DLL nuevas y antiguas.
5. Conservar los XML utilizados y registrar commit/diff, configuración Debug/Release, fechas/hash de las DLL y versiones del driver/firmware.

No se necesita recompilar el driver de kernel para esta adaptación.

## 4. Validación progresiva en MATLAB y hardware

Trabajar con copias de los XML y ajustar la ruta binaria de la demo a la instalación real: las demos PCIe existentes usan por defecto `BIN_NOBF`, que puede no ser tu carpeta habitual. No se han modificado esas rutas ni tus XML de pruebas.

| Prueba | Configuración | Resultado esperado |
|---|---|---|
| Regresión TCP | Demo_GE_Noise y Demo_GE_RF, PCIe desactivado, 4/8 bases | Misma adquisición y dimensiones que antes |
| Simulación PCIe | Demo_PCIe_noise y Demo_PCIe_RF, `emulates_sitau2=true`, argumento PCIe de OpenSys igual a 1 | Dimensiones correctas; **no certifica DMA** |
| PCIe real | Mismas demos, `emulates_sitau2=false`, OpenSys con PCIe=1; primero equipo/configuración de 256, después 128 | Adquisición sin -171, sin mezcla o pérdida de canales |
| Varias leyes | Al menos 3 leyes por imagen; tamaños que crucen el bloque y que compartan bloque | Leyes y canales mantienen orden e índices |
| Parámetros | Repetir con 256, 1024 y 2048 muestras efectivas, finito/continuo y varias ráfagas con/sin pausa | Arranque, pausa, continuación, parada y final coherentes |
| Mapa | Consecutivo, omitido, patrón 128 válido y permutación completa | Orden consistente con el cableado; ampliación solo en stlib2 |
| Mapa inválido | Modo explícito activo; duplicado o fuera de rango manteniendo longitud, después mapa corto | -172 para contenido PCIe inválido; -1523 o -172 para longitud según dónde se detecte |
| Bloque imposible | Ley que exceda capacidad GTX o ráfaga inválida | Error descriptivo -1524 durante carga o -170 al validar; ninguna división por cero |
| Cambio de equipo | Parar/cerrar; cambiar equipo y SITAU.XML 128→256→128; reabrir, cargar, arrancar con iguales muestras | No reutiliza saltos o buffers de la configuración previa |
| Parada/recarga | Repetir abrir→cargar→arrancar→parar→cerrar y cargas rechazadas | Sin bloqueo, uso de mapa antiguo ni crecimiento persistente de memoria |

En bruto (`ST2_Start(0)`) el transporte va por módulos de 32 canales; en ordenado (`ST2_Start(1)`) agrupa muestras por pin usando el mapa. Las representaciones gráficas de las demos pueden asumir datos ordenados: no interpretar una gráfica de datos brutos como prueba de orden incorrecto. Usar una señal o patrón identificable para comprobar los límites 31/32, 63/64, 127/128 y el canal 255; ruido uniforme no demuestra el cableado. Comparar TCP/PCIe con el mismo montaje y parámetros, sin exigir igualdad exacta entre capturas analógicas independientes.

Tras un XML rechazado, corregirlo, cerrar y reabrir el sistema. No recargar mientras PCIe está adquiriendo. Si un setter cambia dimensiones después de cargar, el arranque vuelve a validar la configuración; durante adquisición no deben cambiarse esos parámetros.

## Contratos que debe confirmar el hardware

- Una trama por módulo: 4 para 128 y 8 para 256, según confirmación del usuario.
- La longitud física del bloque final se mantiene nominal, con un contador de leyes útiles menor. El código rechaza una lectura físicamente más corta con -173; confirmar este comportamiento en la captura real.
- Registrar bytes reservados/leídos y comprobar `32 + leyes_por_bloque * muestras * canales * 2`. Con 1024 muestras y 2 leyes: 524320 bytes para 128 y 1048608 para 256.
- El generador del driver conserva ocho tramas: devuelve -174 si se intenta usar con cuatro. Es distinto de la simulación MATLAB.
- El guard de 512 bloques GTX por ráfaga es el límite de software heredado; no se presenta como máximo de firmware confirmado.

La fase 4 se cierra cuando los ejecutables, la compilación de DLL y esta matriz de hardware pasan y quedan registrados sus resultados. No se considera cerrada por superar las comprobaciones Python.
