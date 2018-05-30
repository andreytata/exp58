QT       += core
QT       += network
QT       -= gui

TARGET    = howto_async_tcp_server
DESTDIR   = ../../bin

CONFIG   += console
CONFIG   += thread
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
SOURCES += howto_async_server.cpp

HEADERS += howto_async_server.h

CONFIG(debug, debug|release)
{
    PROJ_TMP_DIR = $$sprintf("../../tmp/%1_d", $$TARGET )
}
CONFIG(release, debug|release)
{
    PROJ_TMP_DIR = $$sprintf("../../tmp/%1", $$TARGET )
}

OBJECTS_DIR = $$PROJ_TMP_DIR/.obj
MOC_DIR     = $$PROJ_TMP_DIR/.moc
RCC_DIR     = $$PROJ_TMP_DIR/.qrc
UI_DIR      = $$PROJ_TMP_DIR/.u

include (../gx_src_async_server/gx_src_async_server.pri)
include (../gx_src_json/gx_src_json.pri)
