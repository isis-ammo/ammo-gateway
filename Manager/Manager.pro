#-------------------------------------------------
#
# Project created by QtCreator 2012-08-13T13:43:44
#
#-------------------------------------------------

QT       += core gui

win32:CONFIG += embed_manifest_exe
win32:QMAKE_LFLAGS_WINDOWS += /manifestuac:level=\'requireAdministrator\'

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Manager
TEMPLATE = app

win32:DEFINES += WIN32 _WIN32 _WINDOWS WINDOWS
linux-g++:DEFINES += LINUX

win32:LIBS += -lAdvApi32 -lShell32

SOURCES += main.cpp\
    MainWindow.cpp \
    LogFileWidget.cpp \
    ServiceWidget.cpp \
    ManagerConfigurationManager.cpp \
    json/json_writer.cpp \
    json/json_valueiterator.inl \
    json/json_value.cpp \
    json/json_reader.cpp \
    json/json_internalmap.inl \
    json/json_internalarray.inl \
    LogFileAction.cpp \
    ConfigFileWidget.cpp \
    TabWidget.cpp \
    ServiceController.cpp

win32:SOURCES += ServiceControllerWindows.cpp
linux-g++:SOURCES += ServiceControllerLinux.cpp

HEADERS  += MainWindow.h \
    LogFileWidget.h \
    ServiceWidget.h \
    ManagerConfigurationManager.h \
    ConfigurationManager.h \
    ServiceDesc.h \
    json/writer.h \
    json/value.h \
    json/reader.h \
    json/json_batchallocator.h \
    json/json.h \
    json/forwards.h \
    json/features.h \
    json/config.h \
    json/autolink.h \
    LogFileAction.h \
    ConfigFileWidget.h \
    TabWidget.h \
    ServiceController.h \

win32:HEADERS += ServiceControllerWindows.h
linux-g++:HEADERS += ServiceControllerLinux.h

FORMS    += MainWindow.ui \
    LogFileWidget.ui \
    ServiceWidget.ui \
    ConfigFileWidget.ui

OTHER_FILES += \
    ManagerConfig.json \
    icons/README.txt \
    icons/tango-text-x-generic.svg \
    icons/tango-text-x-generic.png \
    icons/tango-system-log-out.svg \
    icons/tango-system-log-out.png \
    icons/tango-saveall.png \
    icons/tango-preferences-system.svg \
    icons/tango-preferences-system.png \
    icons/tango-media-stop.png \
    icons/tango-media-restart.png \
    icons/tango-media-playback-stop.svg \
    icons/tango-media-playback-stop.png \
    icons/tango-media-playback-set-2.svg \
    icons/tango-media-playback-set-2.png \
    icons/tango-media-play.png \
    icons/tango-media-floppy.svg \
    icons/tango-media-floppy.png \
    icons/README.txt \
    icons/ammo-bullet.png

RESOURCES += \
    Resources.qrc
