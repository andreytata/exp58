# Support QT4.8 "smart" deprecated classes
#   QHttpHeader
#   QHttpResponseHeader
#   QHttpRequestHeader
# Usage:
#   include(../tp_src_http_headers/tp_src_http_headers.pri)

!contains ( INCLUDEPATH, $$PWD ) {
  HEADERS     += $$PWD/http_headers.h
  SOURCES     += $$PWD/http_headers.cpp
  INCLUDEPATH += $$PWD
}
