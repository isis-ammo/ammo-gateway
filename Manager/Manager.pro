#-------------------------------------------------
#
# Project created by QtCreator 2012-08-13T13:43:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Manager
TEMPLATE = app

win32 {
    #DEFINES += WIN32 _WIN32 _WINDOWS JSON_DLL_BUILD
    DEFINES += WIN32 _WIN32 _WINDOWS
}

#LIBS += -L$$PWD/../build/lib/ -ljson -lShell32
LIBS += -lShell32

#INCLUDEPATH += $$PWD/../LibJSON
#DEPENDPATH += $$PWD/../LibJSON

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
    ConfigFileWidget.cpp

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
    ConfigFileWidget.h

FORMS    += MainWindow.ui \
    LogFileWidget.ui \
    ServiceWidget.ui \
    ConfigFileWidget.ui

OTHER_FILES += \
    ManagerConfig.json \
    icons/README.txt

RESOURCES += \
    Resources.qrc
