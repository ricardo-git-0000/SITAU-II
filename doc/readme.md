# SDK SITAU-II

Este es el código fuente con el que se genera el SDK de SITAU-II.

Está formado por dos librerías:

## Librería stlib2.dll

Es la librería de contfol del SITAU-II que gestiona la configuración del equipo y la comunicación Giga-Ethernet.

## Librería api_pcie.dll

Gestiona la comunicación PICe del SITAU-II y ademñas es un wrapper de la libraría stlib2.dll
En este código hay un ejemplo de aplicación basada en esta libraría 'pcie_vc.exe'.

# Estructura de directorios

trunk/
│
├── src/	--> Archivos de los proyectos (.vcxproj) y fuentes
│   ├── app/
│   │   ├── MatLab/
│   │   │   ├── Examples/		--> scripts ejemplo de MatLab
│   │   │   └── Toolbox_sitau2/	--> Toolbox de MatLab
│   │   └── pcie_vc/			--> Archivos del proyecto
│   │       ├── src/			--> Código fuente del proyecto
│   │       ├── AppFiles/		--> Archivos de la aplicación
│   │       └── SystemFiles/	--> Archivos del hardware
│   │
│   └── lib/
│       ├── api_pcie/
│       │   ├── packages/		--> Paquetes nuget necesarios para compilación
│       │   └── src/			--> Código fuente del proyecto
│       └── stlib2/				--> Archivos del proyecto
│           └── src/			--> Código fuente del proyecto
│
├── doc/	--> Documentación
│   
└── make/	--> Archivos de las soluciones (.sln) y archivos generados durante la compilación
    ├── pcie_vc/
    │   ├── app/	--> (Sólo si es una aplicación) Archivos de salida de la compilación.
    │   └── make/	--> Archivos intermedios generados durante la compilación
    │
    └── stlib/
        └── make/	--> Archivos intermedios generados durante la compilación

# Entorno de desarrollo

## Librería stlib2.dll

Microsoft Visual Studio Community 2022 (64 bits) - Current Versión 17.14.32

## Librería api_pcie.dll

Microsoft Visual Studio Community 2017 - Current Versión 15.9.55

---