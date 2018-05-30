# Abstract acync TCP server base classes
# Usage:
#   include(../gx_src_async_server/gx_src_async_server.pri)

!contains(INCLUDEPATH,$$PWD){

  INCLUDEPATH += $$PWD

  HEADERS += $$PWD/gx_async_tcp_server.h
  HEADERS += $$PWD/gx_json_tcp_thread.h

  SOURCES += $$PWD/gx_async_tcp_server.cpp

  !contains(QT, core)    {  QT += core    }
  !contains(QT, network) {  QT += network }

  include(../tp_src_http_headers/tp_src_http_headers.pri)
}
