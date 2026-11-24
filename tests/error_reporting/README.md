# Pruebas de presentación de errores

Desde la raíz del repositorio, en MATLAB:

```matlab
addpath('tests/error_reporting');
test_error_reporting;
```

La prueba usa mocks y verifica texto, códigos, fallback y que el buffer de salida tenga al menos 4096 caracteres sin nulos iniciales. No carga DLL.

Para comprobar la conversión real de `calllib`, ejecutar en una sesión MATLAB nueva y separada, con las dos DLL x64 actualizadas en `trunk/src/app/MatLab/Examples`:

```matlab
addpath('tests/error_reporting');
test_error_reporting_native;
```

Esta prueba carga las DLL reales y hace 600 consultas del catálogo, abre GE en modo sin hardware usando BIN_256, provoca el error -1912 con un archivo inexistente, imprime su descripción y ejecuta CloseSys. No inicia adquisición. Deja marcadores de cada fase para identificar el punto de un posible cierre. Usa calllib directamente para provocar el error sin mostrar diálogos. La prueba solo pasa si MATLAB recupera el control después de descargar la biblioteca.

Resultado comprobado el 24/11/2026: ambas pruebas PASS; el proceso MATLAB terminó con código cero. Las DLL utilizadas son las Release del 06/11/2026. No certifica el fallo de reserva -703 ni errores físicos UCI/PCIe.

La regresión evita volver a reservar la cadena como `repmat(char(0),1,4096)`: un buffer MATLAB largo lleno de nulos puede convertirse en una cadena C vacía. Se utiliza `blanks(4096)` para mantener la capacidad visible a la interfaz C. Los mocks anteriores solo verificaban el texto devuelto y no detectaban esta diferencia.
