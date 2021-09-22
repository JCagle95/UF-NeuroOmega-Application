# UF NeuroOmega Application (temporary)

## Build from Source
The source code of this application is provided as a QT project file. Import the project using the [QT Project File](https://github.com/JCagle95/UF-NeuroOmega-Application/blob/main/NeuroOmega_Application.pro) and build with [qmake](https://doc.qt.io/qt-5/qmake-overview.html). 

One essential component currently not provided in the repo is the NeuroOmega SDK 2.0. This should be provided by the Alpha Omega representatives. Install the NeuroOmega SDK 2.0 and reference to the static library in the [QT Project File](https://github.com/JCagle95/UF-NeuroOmega-Application/blob/main/NeuroOmega_Application.pro).

The source codes are written and tested in QT Creator 4.15.0, built with [Desktop QT 6.1.0 MinGW 32-bit], the most updated version at the time of writing. Newer Beta QT versions were not tested, if you found error while compiling, try rolling back to QT 6.1.0. 

## Pre-compiled Binary
A pre-compiled binary installer for Windows is available in [Release](https://github.com/JCagle95/UF-NeuroOmega-Application/releases/tag/v0.2.0). All runtime libraries, including the NeuroOmega DLL) are packaged into the installer so no additional installations are needed.

## Build from Source
The source code of this application is provided as a QT project file. The source codes are written and tested in QT Creator 4.15.0, built with [Desktop QT 6.1.0 MinGW 64-bit] (https://wiki.qt.io/Qt_6.1_Release). QT 5 series are not longer supported in this branch.
