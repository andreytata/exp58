#include <QDebug>
#include <QList>
#include <QString>
#include <QNetworkInterface>
#include <QThreadPool>
#include <QJsonParseError>

#include <sstream>
#include <string>

#include "gx_async_tcp_server.h"

gx_async_tcp_socket::gx_async_tcp_socket(QObject *parent)
:QObject(parent)
,mp_async_tcp_thread(NULL)
,m_socket_descriptor(0)
{
}

void gx_async_tcp_socket::set_socket_descriptor(qintptr descriptor)
{
    qDebug()<< QThread::currentThreadId() << this << "set_socket_descriptor(" << descriptor << ")";
    m_socket_descriptor = static_cast<unsigned int>(descriptor);
    mp_socket = new QTcpSocket(this); // make a new socket, use 'this' as parent
    connect(mp_socket, SIGNAL( disconnected() ), this, SLOT( slot_disconnected() ), Qt::DirectConnection);
    connect(mp_socket, SIGNAL(    readyRead() ), this, SLOT( slot_ready_read()   ), Qt::DirectConnection);
    mp_socket->setSocketDescriptor(descriptor);
}

void gx_async_tcp_server::start()
{
    QString ipAddress;

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

    for (int i = 0; i < ipAddressesList.size(); ++i)  // find the first non-localhost IPv4 address
    {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address())
        {
                ipAddress = ipAddressesList.at(i).toString();
                break;
        }
    }

    if (ipAddress.isEmpty())  // if we did not find one, use IPv4 localhost
    {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    if(listen(QHostAddress::Any, m_port))
    {
        qDebug() << "Server: started" << ipAddress << m_port;
    }
    else
    {
        qDebug() << "Server: not started!" << ipAddress << m_port;
    }
}

// After a task performed a time consuming task, grab the result here, and send it to client
void gx_async_tcp_socket::send_results()
{
    qDebug()<< QThread::currentThreadId() << "send_results()" ;
    QByteArray Buffer;
    std::ostringstream http_out;

    http_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "  <head>\n"
    "     <title>Testing XML Example</title>\n"
    "  </he0ad>\n"
    "  <body>\n"
    "  <h1>Persons </h1>\n"
    "    <ul>\n"
    "      <li>Ismincius, Morka</li>\n"
    "      <li>Smith, John</li>\n"
    "    </ul>\n"
    "  </body>\n"
    "</html>\n";

    /*
    const char* http_200 =
            "HTTP/1.1 200 OK\r\n"
    Server: nginx/1.2.1
    Date: Sat, 08 Mar 2014 22:53:46 GMT
            "Content-Type: text/yaml\r\n"    //Content-Type: application/octet-stream\r\n"
            "Content-Length: 7\r\n"
    Last-Modified: Sat, 08 Mar 2014 22:53:30 GMT
    Accept-Ranges: bytes
            "\r\n"
            "\"JSON\"\n";
    */

    Buffer.append("HTTP/1.1 200 OK\r\n");
    Buffer.append("Content-Type: text/html\r\n");
    Buffer.append("Content-Length: ");
    Buffer.append( QString::number( http_out.str().size() ) );
    Buffer.append("\r\n");
    Buffer.append("Connection: keep-alive\r\n");
    Buffer.append("\r\n");
    Buffer.append(http_out.str().c_str());
    mp_socket->write(Buffer);
}

void gx_async_tcp_socket::set_socket_behaviour(gx_async_tcp_thread* connection_behaviour)
{
    mp_async_tcp_thread = connection_behaviour;
    connection_behaviour->set_gx_async_tcp_socket(this);
}

void gx_async_tcp_socket::slot_disconnected() { mp_async_tcp_thread->on_disconnected (); }
void gx_async_tcp_socket::slot_ready_read()   { mp_async_tcp_thread->on_ready_read   (); }
void gx_async_tcp_socket::slot_success(const QString& value_type) { mp_async_tcp_thread->on_success( value_type ); }
void gx_async_tcp_socket::slot_failure(const QString& error_info) { mp_async_tcp_thread->on_failure( error_info ); }
