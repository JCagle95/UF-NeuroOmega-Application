# UF NeuroOmega Application (temporary)

## Build from Source
The source code of this application is provided as a QT project file. Import the project using the [QT Project File](https://github.com/JCagle95/UF-NeuroOmega-Application/blob/main/NeuroOmega_Application.pro) and build with [qmake](https://doc.qt.io/qt-5/qmake-overview.html). 

One essential component currently not provided in the repo is the NeuroOmega SDK 2.0. This should be provided by the Alpha Omega representatives. Install the NeuroOmega SDK 2.0 and reference to the static library in the [QT Project File](https://github.com/JCagle95/UF-NeuroOmega-Application/blob/main/NeuroOmega_Application.pro).

The source codes are written and tested in QT Creator 4.20, built with [Desktop QT 5.8.0 MinGW 32-bit] (https://doc.qt.io/archives/qt-5.8/index.html). Newer QT versions were not tested, if you found error while compiling, try rolling back to QT 5.8. 

## Pre-compiled Binary
A pre-compiled binary installer for Windows is available in [Release](https://github.com/JCagle95/UF-NeuroOmega-Application/releases). All runtime libraries, including the NeuroOmega DLL) are packaged into the installer so no additional installations are needed.
