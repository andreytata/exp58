#ifndef GX_ASYNC_TCP_SERVER_H_INCLUDED 
#define GX_ASYNC_TCP_SERVER_H_INCLUDED 

#include <list>

#include <QTcpServer>
#include <QTcpSocket>
//#include <QThreadPool>
//#include <QRunnable>
//#include <QJsonObject>

#include <gx_json.h>

class gx_async_tcp_reactor;
class gx_async_tcp_server;

// It is a QTcpSocket class wrapper. Some TCP sessions can be started and finished using
// only one connection. But some kinds of the TCP sessions can support disconnection and
// reconnection. And after each reconnection suchlike sessions use other socket and port.
// So, socket object is TCP-session's replaceable part.
class gx_async_tcp_socket : public QObject
{
    Q_OBJECT

private:
    QTcpSocket*           mp_socket;
    gx_async_tcp_reactor* mp_async_tcp_thread;
    unsigned int          m_socket_descriptor;

public:
    explicit gx_async_tcp_socket(QObject *parent = 0);  // usually, parent object is server
    QTcpSocket* socket() { return mp_socket; }
    void send_results();
    void set_socket_descriptor(qintptr descriptor);
    void set_socket_behaviour(gx_async_tcp_reactor* connection_behaviour);

public slots:
    // void slot_connected    ();
    void slot_disconnected ();
    void slot_ready_read   ();
    void slot_success(const QString& value_type);
    void slot_failure(const QString& error_info);
};

// Any TCP based protocol is some state machine included single or multiple states.
// Is may be even single state, so this API core class empty socket events reactor.
// In some cases is a session state
class gx_async_tcp_reactor
{
public:
    //virtual void on_connected    () = 0;
    virtual void on_disconnected () = 0;
    virtual void on_ready_read   () = 0;
    virtual void on_success      (const QString&) = 0;
    virtual void on_failure      (const QString&) = 0;
    virtual void set_gx_async_tcp_socket(gx_async_tcp_socket* _sock) { mp_socket = _sock; }
    virtual gx_async_tcp_reactor* get_state_by_state_name(const char*){ return NULL; }

    // structors :
    virtual ~gx_async_tcp_reactor(){}
    gx_async_tcp_reactor():mp_socket(NULL){}

    QString qstr_host_port(const quint32 ip4, const quint32 port = 0) const
    {
        QString buff;
        buff += QString::number((ip4 & 0xFF000000) >> 24, 10); buff += ".";
        buff += QString::number((ip4 & 0x00FF0000) >> 16, 10); buff += ".";
        buff += QString::number((ip4 & 0x0000FF00) >> 8 , 10); buff += ".";
        buff += QString::number( ip4 & 0x000000FF, 10);
        if(port) {
            buff += ":";
            buff += QString::number(socket()->peerPort(), 10);
        }
        return buff;
    }

    QString qstr_peer() const {
        return qstr_host_port( socket()->peerAddress().toIPv4Address()
                               , socket()->peerPort() );
    }
    QTcpSocket* socket() const { return mp_socket->socket(); }
protected:
    gx_async_tcp_socket*  mp_socket;  // Current connection's socket wrapper.
};

class gx_async_tcp_server : public QTcpServer
{
    // Q_OBJECT

public:

    explicit gx_async_tcp_server(int port, QObject* parent)
        : QTcpServer(parent)
        , m_port(port)
    {
    }

    virtual void start();

    virtual gx_async_tcp_reactor* get_tcp_socket_behaviour() = 0;

    virtual ~gx_async_tcp_server(){}

protected:

    virtual void incomingConnection(qintptr socketDescriptor)
    {
        // qDebug() << QThread::currentThreadId() << "incomingConnection";
        gx_async_tcp_socket* connection = new gx_async_tcp_socket;
        connection->set_socket_descriptor( socketDescriptor );
        gx_async_tcp_reactor* connection_behaviour = get_tcp_socket_behaviour();

        Q_ASSERT_X(connection_behaviour
                   , "gx_async_tcp_server"
                   , "get_tcp_socket_behaviour() => NULL: Reactor instanse expected.");

        connection->set_socket_behaviour( connection_behaviour );
    }
    unsigned short m_port;
};

#endif  // GX_ASYNC_TCP_SERVER_H_INCLUDED
