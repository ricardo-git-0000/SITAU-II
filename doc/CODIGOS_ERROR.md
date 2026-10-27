# Catálogo de errores SITAU / PCIe

Actualizado el 19/10/2026 a partir de los encabezados del código fuente. El manual `SITAU-II SDK r4.pdf`, página impresa 23, documenta `ST2_GetError`: devuelve 1 si reconoce el código y 0 si no lo reconoce. Los significados numéricos de esta tabla proceden de las constantes y descripciones del proyecto; no se atribuyen a una tabla inexistente del manual.

La salida de MATLAB utiliza `codigo: descripcion`. Las descripciones existentes se conservan en su idioma original. Un código desconocido se identifica explícitamente, sin asignarle el texto del último error.

## Nuevo error del mapa

`EVCH_PinToChannelSize = -1523`: el mapa explícito y activo, después de la ampliación admitida de 128 a 256, no coincide con el número efectivo de elementos de sonda. Para sonda completa el número efectivo es `BASES_NUMBER * 32`. No debe confundirse con el número de leyes focales o de bloques de adquisición. Un mapa omitido utiliza el mapa automático y `pin_to_channel_assignment=1` selecciona el modo consecutivo.

La carga devuelve el error antes de aplicar la nueva configuración del canal. No se añade una restauración transaccional de todos los ajustes globales: después de corregir un XML rechazado, volver a abrir el sistema antes de adquirir. Un patrón de 128 inválido no se amplía y, en una configuración de 256, se rechaza por este error de tamaño. Esta tarea no añade una validación general de duplicados/índices para mapas completos.

## Compatibilidad y ambigüedad heredada

`EHW_File` y `EHW_XML_Format` comparten **-3003**. Se conservan sus números para mantener compatibilidad y el traductor devuelve una descripción combinada; el código numérico por sí solo no permite distinguir ambas causas. Los errores auxiliares del sistema operativo o de terceros que no estén en este catálogo se muestran como no documentados.

Los códigos **-101 a -119** también se solapan entre TCP y PCIe. `ST2_GetError` muestra ambas descripciones precedidas por «Codigo compartido», sin elegir una causa arbitrariamente. El número por sí solo no identifica la capa que falló. Se han auditado 268 descripciones correspondientes a 248 números distintos.

## Códigos y descripciones

| Código | Símbolo | Descripción definida en el proyecto |
|---:|---|---|
| -1 | `EXML_FILE` | Error de archivo, acceso al archivo incorrecto |
| -2 | `EXML_Parameter` | Error de rango, la variable pasada como parámetro está fuera de rango |
| -3 | `EXML_NoMemory` | Error en la reerva de memoria |
| -51 | `ECOM_PathFile` | Error accessing path or file |
| -52 | `ECOM_Parameter` | Error: Parámetro fuera de rango. |
| -53 | `ECOM_NoMemory` | Error en la reserva de memoria dinámica |
| -54 | `ECOM_MathFunction` | Error en el cálculo de una función matemática. |
| -55 | `ECOM_RecursiveAcquire` | Error, Recursive Acquire at critical section. |
| -56 | `ECOM_RecursiveRelease` | Error, Recursive Release at critical section. |
| -57 | `ECOM_Acquire` | Error acquiring critical section . |
| -58 | `ECOM_DeleteAcquire` | Error deleting component whitout acquiring critical section . |
| -59 | `ECOM_IdThreadNULL` | Error deleting component whitout acquiring critical section . |
| -60 | `ECOM_DataInconsistency` | Error, Existe una inconsistencia en los datos. |
| -61 | `ECOM_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -101 | `ETCP_FILE` | Error de archivo, acceso al archivo incorrecto |
| -101 | `EPCIE_DRV_NULL` | PCIe driver handle null |
| -102 | `ETCP_RANGO` | Error de rango, la variable pasada como parámetro está fuera de rango |
| -102 | `EPCIE_BUFFER_LEAKAGE` | There are some bufferes used by user (leakege buffers) |
| -103 | `ETCP_IdMOD` | Error de número de módulo, el módulo al que se desea acceder esta fuera de rango |
| -103 | `EPCIE_NOINIT` | Error: Se ha llamado a ST_ConfigSys sin una llamada previa a ST_OpenSys |
| -104 | `ETCP_IdBASE` | Error de número de base, la base a la que se desea acceder esta fuera de rango |
| -104 | `EPCIE_NOCONNECT` | Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys |
| -105 | `ETCP_CH` | Error de número de canal, el canal al que se desea acceder esta fuera de rango |
| -105 | `EPCIE_PARAMETER` | Error: Rango de datos no válido |
| -106 | `ETCP_NO_MEM` | Error en la reserva de memoria |
| -106 | `EPCIE_TIMEOUT` | Se ha salido de un bucle por timeout |
| -107 | `ETCP_WMPAR` | Error de desboramiento de la memoria de parámetros |
| -107 | `EPCIE_CH` | Error número de canales, el numero de canales es distinto al configurado por la función ST_OpenSys() |
| -108 | `ETCP_INVALID_ACCESS` | Error Se ha intentado acceder a una posición no válida del vector |
| -108 | `EPCIE_NO_MEMORY` | Error en la reserva de memoria dinámica |
| -109 | `ETCP_NO_INI` | Error Se ha intentado acceder las estructuras de los módulos antes de su inicialización MOD_RegIni() |
| -109 | `EPCIE_BUFFER_NULL` | Error buffer is null |
| -110 | `ETCP_N_FL` | Error en el número de lineas (leyes focales) |
| -110 | `EPCIE_BUFFER_ADQ_OVF_1` | ERROR Se ha llenado el buffer de adquisición |
| -111 | `ETCP_MEM` | Error en la reserva de memoria dinámica |
| -111 | `EPCIE_BUFFER_ADQ_OVF_2` | Error desbordamiento de buffer (durante la adquisicion), el numero de datos leídos es superior al tamaño del buffer temporal de adquisición |
| -112 | `ETCP_CONNECT` | Error connecting with the SITAU equipment |
| -112 | `EPCIE_FILE` | Error accessing a file |
| -113 | `ETCP_CONNECT_TIMEOUT` | Timeout en la conexión TCP |
| -113 | `EPCIE_ZERO_DIVISION` | Se va a intentar dividir por cero. Se aborta la operación |
| -114 | `ETCP_SOCKET` | Error socket |
| -114 | `EPCIE_EmptyBuffer` | Intento de lectura de Buffer vacío |
| -115 | `ETCP_CSIC` | Error en la lectura de la palabra de control de la FPGA |
| -115 | `EPCIE_NOCONECT` | Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys() |
| -116 | `ETCP_MODULE_Number` | Error en el número de módulos |
| -116 | `EPCIE_SCAN_ENC` | Error de Disparo: El sistema se encuentra en un estado de barrido por encoder ST_RunScanENC() |
| -117 | `ETCP_BASE_Number` | Error en el número de bases |
| -117 | `EPCIE_SCAN` | Error de Programación: Intento de programación mientras el sistema encuentra en un estado de barrido configurado con ST_RunScanSW() o ST_RunScanENC() |
| -118 | `ETCP_Checksum` | Error de Checksum |
| -118 | `EPCIE_INVALID_ACCESS` | Error Se ha intentado acceder a una posición no válida del vector |
| -119 | `ETCP_WSAStartup` | Error WSAStartup (winsock is not started) |
| -119 | `EPCIE_INVALID_LINES` | Error No se han calculado correctamente las líneas del barrido |
| -120 | `EPCIE_FL_FILE` | Error en el formato del archivo de leyes focales |
| -121 | `EPCIE_NMU_LOW` | Error en el número de muestras, no puede ser inferior a 31 después del diezmado |
| -122 | `EPCIE_NMU_HIGH` | Error en el número de muestras, no puede ser superior a 20478 después del diezmado |
| -123 | `EPCIE_EEPROM_CFG` | Error de formato en la información del sistema leida de la memoria EEPROM |
| -124 | `EPCIE_FILE_CFG` | Error accediendo al archivo de configuración del sistema 'SITAU.SC' |
| -125 | `EPCIE_AD_TEST` | Error en los conversores AD de un módulo con el patrón de test |
| -126 | `EPCIE_NOT_PA_SYSTEM` | Intento de obtener datos de un sistema phased array en un sistema de tipo multicanal |
| -127 | `EPCIE_SCAN_LINES_NUMBER` | Error, el número de lineas total de la adquisición excede el número máximo |
| -128 | `EPCIE_USER_MODE` | Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode() |
| -129 | `EPCIE_DataInconsistency` | Error, inconsistencia entre los datos |
| -130 | `EPCIE_InterfaceDetect` | Error en la detección d ela interfaz |
| -131 | `EPCIE_NoDataAcquired` | Error en la adquisición de datos |
| -132 | `EPCIE_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -133 | `EPCIE_MathFunction` | Error en el cálculo de una función matemática. |
| -134 | `EPCIE_FunctionNotAvailable` | Error function not available. |
| -135 | `EPCIE_NoBuffer` | Error, el buffer interno ha sido creado |
| -136 | `EPCIE_IdThreadNULL` | Error Thread handle is NULL. |
| -137 | `EPCIE_ThreadResume` | Error starting thread. |
| -138 | `EPCIE_IdVirtualChannel` | Error en el identificador del barrido 'scan' o canal virtual |
| -139 | `EPCIE_IdGate` | Error en el identificador de la puerta |
| -140 | `EPCIE_IdEncoder` | Error en el identificador del encoder |
| -141 | `EPCIE_Acquiring` | Error, intento de disparo mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan() |
| -142 | `EPCIE_RUN_NO_DATA` | Error intento de disparo con cero muestras programado |
| -143 | `EPCIE_BUFFER_RD_WORD_INI` | Error trama incorrecta en el buffer del canal, la trama leida del buffer no tiene la palabra de control inicial |
| -144 | `EPCIE_OVERFLOW` | Error desbordamiento de la memoria |
| -145 | `EPCIE_BUFFER_ADD_OVERFLOW` | Error buffer overflow |
| -146 | `EPCIE_BUFFER_ADD_OVF_2` | Error desbordamiento de buffer TIPO 2, el numero de datos leidos es superior al tamaño del buffer de los canales |
| -147 | `EPCIE_BUFFER_ADD_OVF_3` | Error desbordamiento de buffer TIPO 3, el numero de datos leidos es superior al tamaño del buffer de los canales |
| -148 | `EPCIE_BUFFER_RD_WR1` | Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura |
| -149 | `EPCIE_BUFFER_RD_WR2` | Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura |
| -150 | `EPCIE_SYSTEM_NULL` | PA Sytem structure doesn't exist (Null Pointer) |
| -151 | `EPCIE_MODULE_Number` | Error en el número de módulos |
| -152 | `EPCIE_BASE_Number` | Error en el número de bases |
| -153 | `EPCIE_IdConnector` | Error en el identificador del conector |
| -154 | `EPCIE_TriggerSource` | Error, la fuente de disparo no existe. |
| -155 | `EPCIE_File` | Error accessing file |
| -156 | `EPCIE_IMAGE_HANDLE_NULL` | Image handle null |
| -157 | `EPCIE_ACQUIRE` | Error al intentar desbloquear el buffer |
| -158 | `EPCIE_RecursiveRelease` | Error, Recursive Release at critical section. |
| -159 | `EPCIE_NO_THREAD_ACQ` | Error, Acquisition Thread is not running. |
| -160 | `EPCIE_IMAGE_SIZE` | Image Size Error. |
| -161 | `EPCIE_THREAD_ACQ` | Error, Acquisition Thread is running. |
| -162 | `EPCIE_MUTEX_ACQUIRE` | Error, No Mutex Acquired |
| -163 | `EPCIE_NO_THREAD_DMA` | Error, DMA Thread is not running. |
| -164 | `EPCIE_EVENT` | Event function return error. |
| -165 | `EPCIE_SET_EVENT` | SetEvent() function return error. |
| -166 | `EPCIE_NO_THREAD_GE` | Error, SITAU GE Thread is not running. |
| -167 | `EPCIE_DISABLED` | Error, SITAU PCIe interface disabled. |
| -168 | `EPCIE_GTX_BLOCKS` | Error, el número de bloques gtx es superior a 256. |
| -601 | `CFGHW_NO_BOARD` | Error: la tarjeta no existe |
| -602 | `CFGHW_BUFFER_OVERFLOW` | Error: Buffer overflow |
| -603 | `CFGHW_NO_MEM` | Error en la reserva de memoria dinámica |
| -604 | `CFGHW_EEPROM` | Error en la lectura de la EEPROM, dato corrupto |
| -605 | `CFGHW_FILE` | Error al abrir el archivo de configuración .SC' |
| -701 | `EBUF_Acquire` | Error al intentar desbloquear el buffer |
| -702 | `EBUF_Parameter` | Error: Rango de datos no válido |
| -703 | `EBUF_NoMemory` | Error en la reserva de memoria dinámica |
| -704 | `EBUF_DataInconsistency` | Error, inconsistencia entre los datos |
| -705 | `EBUF_IdScan` | Error en el identificador del barrido 'scan' |
| -706 | `EBUF_Overflow` | Error, desbordamiento del buffer al intentar escribir datos |
| -707 | `EBUF_Underflow` | Error, desbordamiento del buffer al intenter leer datos |
| -708 | `EBUF_NoHeader` | Error, no se encuentra la palabra de inicio de trama en el buffer |
| -709 | `EBUF_Frame` | Error en el format de la trama |
| -710 | `EBUF_IdGate` | Error en el identificador de la puerta |
| -711 | `EBUF_IdEncoder` | Error en el identificador del encoder |
| -712 | `EBUF_File` | Error de archivo, acceso al archivo incorrecto |
| -713 | `EBUF_Acquiring` | Error, intento de lectura mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan() |
| -714 | `EBUF_DisabledDataFormat` | Error, formato de datos deshabilitado (Ver funciones ST_SetDataEnabled_XXX()) |
| -715 | `EBUF_MODULE_Number` | Error en el número de módulos |
| -716 | `EBUF_BASE_Number` | Error en el número de bases |
| -717 | `EBUF_CONFIG_NULL` | Config structure doesn't exist (Null Pointer) |
| -718 | `EBUF_IdVirtualChannel` | Error en el identificador del barrido 'scan' o canal virtual |
| -719 | `EBUF_IdBASE` | Error de número de base, la base a la que se desea acceder esta fuera de rango |
| -801 | `EENC_IdEncoder` | Error al acceder a un encoder que no existe |
| -802 | `EENC_Parameter` | Error: Rango de datos no válido |
| -803 | `EENC_NoMemory` | Error en la reserva de memoria dinámica |
| -804 | `EENC_DataInconsistency` | Error, inconsistencia entre los datos |
| -805 | `EENC_FunctionNotAvailable` | Error, funcionalidad no disponible en esta versión |
| -806 | `EENC_File` | Error accessing file |
| -1001 | `EFL_PARAMETER` | Error: Parameter value error |
| -1002 | `EFL_FILE` | Error: File access error |
| -1003 | `EFL_PA_Z` | ERROR: Invalid PA transducer Z position |
| -1004 | `EFL_PA_X` | ERROR: Invalid PA transducer X position |
| -1005 | `EFL_PA_NULL` | ERROR: The 'PA' transducer has not been created |
| -1006 | `EFL_WEDGE_NULL` | ERROR: The 'Wedge' has not been created |
| -1007 | `EFL_SPECIMEN_NULL` | ERROR: The 'Specimen' has not been created |
| -1008 | `EFL_SCAN_NULL` | ERROR: The 'Scan' has not been created |
| -1009 | `EFL_DDF_APERTURE_NULL` | ERROR: The 'Reception Aperture' has not been created |
| -1010 | `EFL_NO_MEM` | ERROR: Not enough dinamic memory space |
| -1011 | `EFL_NO_SCAN` | ERROR: Scan not found in the list of scan assigned to the component |
| -1012 | `EFL_ANGLE_OVF` | ERROR: Incident angle is greater than or equal 90 degrees |
| -1013 | `EFL_XYZ_SIZE` | ERROR: X, Y and Z size coordinates is different |
| -1014 | `EFL_IANGLE_CRITICAL` | ERROR: The incidence angle is greater than 90º |
| -1015 | `EFL_APERTURE_SIZE` | ERROR Size aperture |
| -1016 | `EFL_FOCAL_LAW_SIZE` | ERROR emission an reception focal law number are different |
| -1017 | `EFL_PA_COORD_SIZE` | ERROR PA coordinates size and element number are different |
| -1018 | `EFL_NO_WEDGE_SURF` | ERROR Calculating surface limits without wedge |
| -1019 | `EFL_SURFACE` | ERROR Surface type |
| -1020 | `EFL_VIRTUAL_ARRAY_SIZE` | ERROR virtual aperture size and real aperture size are differenc |
| -1021 | `EFL_DDF_NFOCUS` | ERROR en el número de focos |
| -1022 | `EFL_X_AP_SIZE` | ERROR: The aperture size is null or negative distance |
| -1023 | `EFL_PQ` | DDF ERROR: Error en la codificación de los parámetros P y Q |
| -1024 | `EFL_ACQ_RANGE` | DDF ERROR: Error en la rango de la adquisición |
| -1025 | `EFL_SQRT_NEG` | ERROR: Raiz de un número negativo. |
| -1026 | `EFL_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -1027 | `EFL_PAElement` | ERROR: The element doesn't exist. |
| -1028 | `EFL_IFAZ_ERR` | ERROR: Interface commponent not exists |
| -1029 | `EFL_DataInconsistency` | Error, inconsistencia entre los datos |
| -1030 | `EFL_ASIN` | Error, Cálculo de un arco cuyo seno es mayor que 1. |
| -1031 | `EFL_SurfacePoints_Izqd` | Error, El número de puntos de la superficie es insuficiente por la izquierda |
| -1032 | `EFL_SurfacePoints_Drch` | Error, El número de puntos de la superficie es insuficiente por la derecha |
| -1033 | `EFL_SAD_NoGate` | Surface Auto Detect Error, Error en la definición en la puerta de busqueda. |
| -1034 | `EFL_NoSurface` | No se ha deficido la superficie. |
| -1035 | `EFL_SCAN_LINES_NUMBER` | Error, el número de lineas total de la adquisición excede el número máximo |
| -1501 | `EVCH_Parameter` | Error: Rango de datos no válido |
| -1502 | `EVCH_NoMemory` | Error en la reserva de memoria dinámica |
| -1503 | `EVCH_File` | Error accessing file |
| -1504 | `EVCH_DataInconsistency` | Error, inconsistencia entre los datos |
| -1505 | `EVCH_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -1506 | `EVCH_MathFunction` | Error en el cálculo de una función matemática. |
| -1507 | `EVCH_PA_NULL` | PA structure doesn't exist (Null Pointer) |
| -1508 | `EVCH_WEDGE_NULL` | Wedge structure doesn't exist (Null Pointer) |
| -1509 | `EVCH_SPECIMEN_NULL` | Specimen structure doesn't exist (Null Pointer) |
| -1510 | `EVCH_SCAN_NULL` | Scan structure doesn't exist (Null Pointer) |
| -1511 | `EVCH_BASE_NULL` | BASE structure doesn't exist (Null Pointer) |
| -1512 | `EVCH_MODULE_NULL` | MODULE structure doesn't exist (Null Pointer) |
| -1513 | `EVCH_SYSTEM_NULL` | PA Sytem structure doesn't exist (Null Pointer) |
| -1514 | `EVCH_CONFIG_NULL` | Config structure doesn't exist (Null Pointer) |
| -1515 | `EVCH_NMU_Underflow` | Error en el número de muestras, no puede ser inferior a 31 después del diezmado |
| -1516 | `EVCH_NMU_Overflow` | Error en el número de muestras, no puede ser superior a 20478 después del diezmado |
| -1517 | `EVCH_AScanNumber` | Error, el número de leyes focales excede el máxio permitido (el número máximo de líneas es 255) |
| -1518 | `EVCH_IdMOD` | Error de número de módulo, el módulo al que se desea acceder esta fuera de rango |
| -1519 | `EVCH_IdBASE` | Error de número de base, la base a la que se desea acceder esta fuera de rango |
| -1520 | `EVCH_IdPASystem` | Error de número de sub-sistema, el sub-sistema al que se desea acceder esta fuera de rango |
| -1521 | `EVCH_USER_MODE` | Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode() |
| -1522 | `EVCH_ACQUISITION_TYPE` | Error, tipo de adquisición incorrecto (MC, PA, TFM) |
| -1523 | `EVCH_PinToChannelSize` | pin_to_channel: longitud incompatible con los elementos configurados (BASES_NUMBER x 32 para sonda completa); se admite un patron valido de 128 para 256. |
| -1701 | `EBASE_Parameter` | Error: Rango de datos no válido |
| -1702 | `EBASE_NoMemory` | Error en la reserva de memoria dinámica |
| -1703 | `EBASE_File` | Error accessing file |
| -1704 | `EBASE_DataInconsistency` | Error, inconsistencia entre los datos |
| -1705 | `EBASE_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -1706 | `EBASE_MathFunction` | Error en el cálculo de una función matemática. |
| -1707 | `EBASE_IdBASE` | Error de número de base, la base a la que se desea acceder esta fuera de rango |
| -1708 | `EBASE_NUM_CPB` | Error de número de coeficiente del filtro, el número de coeficiente al que se desea acceder está fuera de rango |
| -1709 | `EBASE_NUM_FocalLaw` | Error, el número de leyes focales excede el máxio permitido (el número máximo de líneas es 255) |
| -1710 | `EBASE_MODULE_Number` | Error en el número de módulos |
| -1711 | `EBASE_BASE_Number` | Error en el número de bases |
| -1712 | `EBASE_AMPLIACommand_Number` | Error en el número de comandos AMPLIA |
| -1901 | `ES2_BASE_TEST` | One BASE device has not been configured correctly |
| -1902 | `ES2_MOD_TEST` | One MODULE device has not been configured correctly |
| -1903 | `ES2_NOINIT` | Error: Se ha llamado a ST_ConfigSys sin una llamada previa a ST_OpenSys |
| -1904 | `ES2_NOCONNECT` | Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys |
| -1905 | `ES2_RANGO` | Error: Rango de datos no válido |
| -1906 | `ES2_TIMEOUT` | Se ha salido de un bucle por timeout |
| -1907 | `ES2_CH` | Error número de canales, el numero de canales es distinto al configurado por la función ST_OpenSys() |
| -1908 | `ES2_NO_MEM` | Error en la reserva de memoria dinámica |
| -1909 | `ES2_BUFFER_FTR_NDAT_3` | Error trama incorrecta en el buffer del canal, la trama leida del buffer del canal indica que contiene un numero de datos distinto al programado |
| -1910 | `ES2_BUFFER_ADQ_OVF_1` | ERROR Se ha llenado el buffer de adquisición |
| -1911 | `ES2_BUFFER_ADQ_OVF_2` | Error desbordamiento de buffer (durante la adquisicion), el numero de datos leídos es superior al tamaño del buffer temporal de adquisición |
| -1912 | `ES2_FILE` | Error accessing a file |
| -1913 | `ES2_ZERO_DIVISION` | Se va a intentar dividir por cero. Se aborta la operación |
| -1914 | `ES2_EmptyBuffer` | Intento de lectura de Buffer vacío |
| -1915 | `ES2_NOCONECT` | Error: Se ha llamado a alguna función sin una llamada previa a ST_ConfigSys() |
| -1916 | `ES2_SCAN_ENC` | Error de Disparo: El sistema se encuentra en un estado de barrido por encoder ST_RunScanENC() |
| -1917 | `ES2_SCAN` | Error de Programación: Intento de programación mientras el sistema encuentra en un estado de barrido configurado con ST_RunScanSW() o ST_RunScanENC() |
| -1918 | `ES2_INVALID_ACCESS` | Error Se ha intentado acceder a una posición no válida del vector |
| -1919 | `ES2_INVALID_LINES` | Error No se han calculado correctamente las líneas del barrido |
| -1920 | `ES2_FL_FILE` | Error en el formato del archivo de leyes focales |
| -1921 | `ES2_NMU_LOW` | Error en el número de muestras, no puede ser inferior a 31 después del diezmado |
| -1922 | `ES2_NMU_HIGH` | Error en el número de muestras, no puede ser superior a 20478 después del diezmado |
| -1923 | `ES2_EEPROM_CFG` | Error de formato en la información del sistema leida de la memoria EEPROM |
| -1924 | `ES2_FILE_CFG` | Error accediendo al archivo de configuración del sistema 'SITAU.XML' |
| -1925 | `ES2_AD_TEST` | Error en los conversores AD de un módulo con el patrón de test |
| -1926 | `ES2_NOT_PA_SYSTEM` | Intento de obtener datos de un sistema phased array en un sistema de tipo multicanal |
| -1927 | `ES2_SCAN_LINES_NUMBER` | Error, el número de lineas total de la adquisición excede el número máximo |
| -1928 | `ES2_USER_MODE` | Error, función no disponible en este modo de usuario. Ver función ST_SetUserMode() |
| -1929 | `ES2_DataInconsistency` | Error, inconsistencia entre los datos |
| -1930 | `ES2_InterfaceDetect` | Error en la detección d ela interfaz |
| -1931 | `ES2_NoDataAcquired` | Error en la adquisición de datos |
| -1932 | `ES2_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -1933 | `ES2_MathFunction` | Error en el cálculo de una función matemática. |
| -1934 | `ES2_FunctionNotAvailable` | Error function not available. |
| -1935 | `ES2_NoBuffer` | Error, el buffer interno ha sido creado |
| -1936 | `ES2_IdThreadNULL` | Error Thread handle is NULL. |
| -1937 | `ES2_ThreadResume` | Error starting thread. |
| -1938 | `ES2_IdVirtualChannel` | Error en el identificador del barrido 'scan' o canal virtual |
| -1939 | `ES2_IdGate` | Error en el identificador de la puerta |
| -1940 | `ES2_IdEncoder` | Error en el identificador del encoder |
| -1941 | `ES2_Acquiring` | Error, intento de disparo mientras el sistema encuentra en un modo de disparo configurado con las funciones ST_StartPRFScan(), ST_StartEncoderScan() ó ST_StartTRGScan() |
| -1942 | `ES2_RUN_NO_DATA` | Error intento de disparo con cero muestras programado |
| -1943 | `ES2_BUFFER_RD_WORD_INI` | Error trama incorrecta en el buffer del canal, la trama leida del buffer no tiene la palabra de control inicial |
| -1944 | `ES2_MEM_UCI_OVERFLOW` | Error desbordamiento de la memoria interna del equipo |
| -1945 | `ES2_BUFFER_ADD_OVF_1` | Error desbordamiento de buffer TIPO 1, el numero de datos leidos es superior a la capacidad del buffer |
| -1946 | `ES2_BUFFER_ADD_OVF_2` | Error desbordamiento de buffer TIPO 2, el numero de datos leidos es superior al tamaño del buffer de los canales |
| -1947 | `ES2_BUFFER_ADD_OVF_3` | Error desbordamiento de buffer TIPO 3, el numero de datos leidos es superior al tamaño del buffer de los canales |
| -1948 | `ES2_BUFFER_RD_WR1` | Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura |
| -1949 | `ES2_BUFFER_RD_WR2` | Error desbordamiento de buffer TIPO 1, el puntero de lectura supera al de escritura |
| -1950 | `ES2_SYSTEM_NULL` | PA Sytem structure doesn't exist (Null Pointer) |
| -1951 | `ES2_MODULE_Number` | Error en el número de módulos |
| -1952 | `ES2_BASE_Number` | Error en el número de bases |
| -1953 | `ES2_IdConnector` | Error en el identificador del conector |
| -1954 | `ES2_TriggerSource` | Error, la fuente de disparo no existe. |
| -1955 | `ES2_File` | Error accessing file |
| -1956 | `ES2_Acquire` | Error al intentar acceder a una sección crítica |
| -3001 | `EHW_Parameter` | Error: Rango de datos no válido |
| -3002 | `EHW_Status_FSM` | Error, UCI Status |
| -3003 | `EHW_XML_Format` | Error en el formato del archivo de configuración 'SITAU.XML' |
| -3003 | `EHW_File` | Error accessing a file |
| -3004 | `EHW_PtrAccess_NULL` | ERROR: Acceso a puntero NULL. |
| -3005 | `EHW_BASE_Number` | Error en el número de bases |
| -3006 | `EHW_NoMemory` | Error en la reserva de memoria dinámica |
| -3007 | `EHW_NoXMLData` | Error no existen datos de configuración hardware XML |
| -3008 | `EHW_DataInconsistency` | Error, inconsistencia entre los datos |
| -3009 | `EHW_IdEncoder` | SITAU XML Config File: Error en la configuración de los encoders |
| -3010 | `EHW_FRONTEND_NULL` | SITAU XML Config File: Error en la configuración del FORNTEND |

## Fuentes del catálogo

- `trunk/src/lib/stlib2/src/st_error_code.h`
- `trunk/src/lib/stlib2/src/c_buffer/buffer_error_code.h`
- `trunk/src/lib/stlib2/src/c_virtual_channel/vch_error_code.h`
- `trunk/src/lib/stlib2/src/HW/sitau_error_code.h`
- `trunk/src/lib/stlib2/src/TOOLS/common_error_code.h`
- `trunk/src/lib/stlib2/src/c_focal_law/c_fl/fl_error_code.h`
- `trunk/src/lib/stlib2/src/HW/MK32/base_error_code.h`
- `trunk/src/lib/stlib2/src/HW/UCI/tcp_error_code.h`
- `trunk/src/lib/api_pcie/src/pcie_error_code.h`
- `trunk/src/lib/stlib2/src/TOOLS/XML/xml_err.h`
- `trunk/src/lib/stlib2/src/st_encoder_error_code.h`
- `trunk/src/lib/stlib2/src/HW/c_sitau_hw_err.h`
