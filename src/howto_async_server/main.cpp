#include <QCoreApplication>
#include <QJsonParseError>

#include <howto_async_server.h>
#include <gx_json.h>

#include <gx_json_tcp_thread.h>

int main(int argc, char *argv[])
{
    const char* j0= "{\"key\" : \"value\"  , \"sint\" :13 }";
    const char* j1= "[\"12\",11]";
    const char* j2= "[]";
    const char* j3= "{}";
    const char* j4= "";
    const char* j5= "null";

    gx::json* t;
    t = gx::json_loads(j0); qDebug() << gx::json_dumps(t).c_str();
    t = gx::json_loads(j1); qDebug() << gx::json_dumps(t).c_str();
    t = gx::json_loads(j2); qDebug() << gx::json_dumps(t).c_str();
    t = gx::json_loads(j3); qDebug() << gx::json_dumps(t).c_str();
    t = gx::json_loads(j4); qDebug() << gx::json_dumps(t).c_str();
    t = gx::json_loads(j5); qDebug() << gx::json_dumps(t).c_str();

    QCoreApplication app(argc, argv);
    //howto_async_server tcp_server(4680);
    gx_json_tcp_server  tcp_server(4680);
    //gx_async_tcp_server tcp_server(4680);   // abstract compile crash test

    tcp_server.start();
    return app.exec();
}
