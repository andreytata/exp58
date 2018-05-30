# Using third party library "libjson":
#       - tp_src_libjson.pri   # "libjson" lib source code.
#       - gx_src_json.pri      # "libjson to gx" adapter.

QT += core
QT -= gui

TARGET = test_gx_src_json
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include (../tp_src_libjson/tp_src_libjson.pri)
include (../gx_src_json/gx_src_json.pri)
