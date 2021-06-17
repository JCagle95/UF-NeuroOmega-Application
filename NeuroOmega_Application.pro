#-------------------------------------------------
#
# Project created by QtCreator 2021-01-18T16:24:36
#
#-------------------------------------------------

QT       += core gui charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NeuroOmega_Application
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    NeuroOmega_SDK/Include/AOSystemAPI_TEST.cpp \
    electrodeconfigurations.cpp \
    jsonstorage.cpp \
    mainwindow.cpp \
    macaddressdialog.cpp \
    controllerform.cpp \
    channelselectiondialog.cpp \
    detailchannelslist.cpp \
    realtimestream.cpp \
    recordingannotation.cpp \
    manuallabelentry.cpp \
    novelstimulationconfiguration.cpp \
    streamdatahandler.cpp
    NeuroOmega_SDK/Include/AOSystemAPI_TEST.cpp \

HEADERS  += mainwindow.h \
    electrodeconfigurations.h \
    jsonstorage.h \
    macaddressdialog.h \
    controllerform.h \
    channelselectiondialog.h \
    detailchannelslist.h \
    realtimestream.h \
    recordingannotation.h \
    manuallabelentry.h \
    novelstimulationconfiguration.h \
    NeuroOmega_SDK/Include/AOSystemAPI.h \
    NeuroOmega_SDK/Include/AOSystemAPI_TEST.h \
    NeuroOmega_SDK/Include/AOTypes.h \
    NeuroOmega_SDK/Include/StreamFormat.h \
    streamdatahandler.h

FORMS    += mainwindow.ui \
    electrodeconfigurations.ui \
    macaddressdialog.ui \
    controllerform.ui \
    channelselectiondialog.ui \
    detailchannelslist.ui \
    realtimestream.ui \
    recordingannotation.ui \
    manuallabelentry.ui \
    novelstimulationconfiguration.ui

RC_ICONS = $$PWD/resources/logo_icon.ico

win32: LIBS += -L$$PWD/NeuroOmega_SDK/ -lNeuroOmega_x64

INCLUDEPATH += $$PWD/NeuroOmega_SDK/Include
DEPENDPATH += $$PWD/NeuroOmega_SDK

DISTFILES += \
    DefaultStimulationConfiguration_4Contacts.json \
    InterfaceConfigurations.json
