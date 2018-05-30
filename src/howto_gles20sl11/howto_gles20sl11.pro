QT       += core gui #opengl
QT       += network # its only for recv TCP event from lockalhost
CONFIG   += thread  # its only for use real acync thread for each TCP-client

greaterThan(QT_MAJOR_VERSION, 4){
  QT += widgets
  #CONFIG += console
}

TARGET   = howto_gles20sl11
TEMPLATE = app

SOURCES += main.cpp \
    gx_gdom_tcp_server.cpp
SOURCES += mainwindow.cpp
SOURCES += basicusagescene.cpp
SOURCES += gx_font_32.cpp
SOURCES += gx_gui_text_item.cpp

HEADERS += mainwindow.h \
    gx_gdom_tcp_server.h
HEADERS += abstractscene.h
HEADERS += basicusagescene.h
HEADERS += glassert.h
HEADERS += gxcodefont32.h
HEADERS += gx_gui_text_item.h

win32:LIBS+=libopengl32

CONFIG(debug, debug|release){
    TARGET = howto_gles20sl11_d
    DESTDIR = ../../bin
}

CONFIG(release, debug|release){
    TARGET = howto_gles20sl11
    DESTDIR = ../../bin
}

PROJ_TMP_DIR = $$sprintf("../../tmp/%1", $$TARGET )

OBJECTS_DIR = $$PROJ_TMP_DIR/.obj
MOC_DIR     = $$PROJ_TMP_DIR/.moc
RCC_DIR     = $$PROJ_TMP_DIR/.qrc
UI_DIR      = $$PROJ_TMP_DIR/.u

include (../gx_cfg_templates/gx_app_template.pri)
include (../gx_src_json/gx_src_json.pri)
include (../gx_src_gles20sl11/gx_src_gles20sl11.pri)
include (../gx_src_gui/gx_src_gui.pri)

include (../gx_src_async_server/gx_src_async_server.pri)
include (../gx_src_json/gx_src_json.pri)
include (../gx_src_gdom/gx_src_gdom.pri)

