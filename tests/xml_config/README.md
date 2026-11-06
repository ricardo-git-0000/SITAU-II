# Validación de los XML MC de MATLAB

Desde la raíz del repositorio, en la consola de MATLAB:

```matlab
addpath('tests/xml_config');
test_xml_config;
```

No compila ni carga DLL, no abre conexiones ni ejecuta las demos. Usa el validador de producción y XML temporales que elimina al finalizar. Restaura el path de MATLAB al salir. Ejecutarlo en una sesión sin las DLL de adquisición cargadas para comprobar también el rechazo de una validación sin geometría.

La prueba cubre los cuatro XML Noise/RF de GE y PCIe con 128 y 256 canales; omisión y presencia explícita de `probe_elements_number`; leyes FMC/PWI, listas de ángulos y paso cero; ráfagas finitas/continuas y valores por defecto; esquema antiguo, ubicación incorrecta y parámetros duplicados; formato numérico compatible con el lector C++; avisos de ancho de banda y comparación con una instantánea simulada de los getters de la DLL.

El último caso comprueba la lógica de `Validate_LoadedMCConfig`, no la ejecución de los getters reales. Estos se contrastan al ejecutar las demos: después de `FUS_LoadXMLFileConfig`, se comparan canales, elementos, leyes reales, imágenes por ráfaga, ráfagas configuradas y modo de adquisición antes de `ST2_Start`.

Para validar un XML sin abrir el equipo:

```matlab
addpath('trunk/src/app/MatLab/Toolbox_sitau2');
[valid, withWarnings, performance] = Validate_XMLConfig( ...
    'trunk/src/app/MatLab/Examples/SITAU2_PCIe_NOISE.xml', 256, 1);
```

El segundo argumento es el número de canales, no de bases. El tercero es el enlace esperado (`0`: GE; `1`: PCIe). En las demos, los canales proceden de `ST2_GetChannelsNumber()` tras `OpenSys`, que ya ha leído `BASES_NUMBER` de `SITAU.XML`.

Alcance: MC con sonda completa de 128/256 elementos, un canal virtual, FMC o PWI, decimación uno y modo de tiempo cero. El modo de leyes desde fichero se rechaza explícitamente hasta disponer de su validador. El informe temporal/de ancho de banda es una estimación; no certifica DMA, pausas ni rendimiento sostenido. Las pruebas físicas de Noise/RF y de la adaptación PCIe 4+4 siguen siendo necesarias.
