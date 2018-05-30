# Support JSON source loader in to visitable data tree.
# Contain visitable JSON nodes and base set of visitors.
# Is replace old TP "libjson" support with QJsonDocument.
# Usage:
#   include(../gx_src_json/gx_src_json.pri)

!contains ( INCLUDEPATH, $$PWD ) {
  HEADERS     += $$PWD/gx_json.h
  SOURCES     += $$PWD/gx_json.cpp
  INCLUDEPATH += $$PWD
}
