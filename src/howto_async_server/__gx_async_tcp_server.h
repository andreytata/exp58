#ifndef GX_ASYNC_TCP_SERVER_H_INCLUDED 
#define GX_ASYNC_TCP_SERVER_H_INCLUDED 

#include <list>

#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>
#include <QRunnable>
#include <QJsonObject>

#include <gx_json.h>

class gx_async_tcp_thread;
class gx_async_tcp_server;

// It is a QTcpSocket class wrapper. Some TCP sessions can be started and finished using
// only one connection. But some kinds of the TCP sessions can support disconnection and
// reconnection. And after each reconnection suchlike sessions use other socket and port.
// So, socket object is TCP-session's replaceable part.
class gx_async_tcp_socket : public QObject
{
    Q_OBJECT

private:
    QTcpSocket*          mp_socket;
    gx_async_tcp_thread* mp_async_tcp_thread;
    unsigned int         m_socket_descriptor;

public:
    explicit gx_async_tcp_socket(QObject *parent = 0);  // usually, parent object is server
    QTcpSocket* socket() { return mp_socket; }
    void send_results();
    void set_socket_descriptor(qintptr descriptor);
    void set_socket_behaviour(gx_async_tcp_thread* connection_behaviour);

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

protected:
    gx_async_tcp_socket*  mp_socket;  // Current connection's socket wrapper.
};

// This is example tcp-reactor implementation (Example Driven Development ED:D).
// Single state session with async thread pool JSON parser. Used In Async Tests.
class gx_async_tcp_thread: public gx_async_tcp_reactor
{
    class async_parser: public QRunnable
    {
        QByteArray*           mp_readed;   // async parsed raw_data pointer
        gx::json*             mp_parsed;   // async parsed result ptr, NULL if not parsed
        QObject*              mp_report;   // SUCCESS / FAILURE receiver

    public:
        async_parser(QByteArray* readed, QObject* report )
            :mp_readed(readed)
            ,mp_report(report)
        {
            setAutoDelete(false);
        }
        gx::json* get_result(){ return mp_parsed; }
        virtual ~async_parser(){}

    protected:
        virtual void run()   // ANY ASYNC TASK CAN BE DONE HERE.
        {
            mp_parsed = gx::json_loads( mp_readed->constData() );
            if( mp_parsed )
                QMetaObject::invokeMethod( mp_report, "slot_success", Qt::QueuedConnection, Q_ARG( QString, "JSON" ));
            else
                QMetaObject::invokeMethod( mp_report, "slot_failure", Qt::QueuedConnection, Q_ARG( QString, "JSON ERROR" ));
        }
    };
    async_parser*         mp_parser;  // Async object sheduled to threadpool.
    std::list<QByteArray> mo_readed;  // Readed Data Chunks.
    std::list<QByteArray>::iterator mi_readed;
    std::list<QByteArray>::iterator mi_latest;
    std::list<gx::json*>  mo_parsed;  // Parsed Data Chunks.

public:
    //const static int max_stored = 5;  // max stored results count
    gx_async_tcp_thread()
        :mp_parser(NULL)             // Async method is not created and not sheduled to threadpool.
        ,mi_readed(mo_readed.end())  // Currently Parsed Chunk of the Readed Data
        ,mi_latest(mo_readed.end())  // Latest Readed Chunk of the Readed Data
    {
        qDebug() << QThread::currentThreadId()  << this << "RUNNABLE CREATED";
    }

    virtual void schedule_self_to_thread_pool()
    {
        if ( mp_parser ) return;  // Is prevent multiple scheduling - async parser not finished yet.
                                  // parser events on_success|on_failure can call this method later.
                                  // This instance represent only one state of the session behavior,
                                  // and can be replaced on some another state when parser finished.

        // some preparations before shedule "this" as runnable to thread pool
        mp_parser = new async_parser( &(*mi_readed), mp_socket );
        QThreadPool::globalInstance()->start(mp_parser);  // schedule this object to thread pool
    }

    virtual void on_run_finished()
    {
        mo_parsed.push_back(mp_parser->get_result());
        /*
        if ( mo_parsed.size() > 10 )
        {
            mo_readed.pop_front();
            gx::json* garbage = *mo_parsed.begin();
            if (garbage) delete garbage;
            mo_parsed.pop_front();
        }
        */
        delete mp_parser;
        mp_parser = 0;
        if ( mi_readed != mi_latest ) {
            ++ mi_readed;
            schedule_self_to_thread_pool();
        }
    }

    virtual void on_success(const QString& _media_type)
    {
        qDebug() << QThread::currentThreadId() << this << "on_success thread finished (" << _media_type << ")";
        QString echo = QString("Ok") + mi_readed->data();
        mp_socket->socket()->write(echo.toStdString().c_str());
        on_run_finished();
    }

    virtual void on_failure(const QString& _error_info)
    {
        qDebug() << QThread::currentThreadId() << this << "on_failure thread finished (" << _error_info << ")";
        mp_socket->socket()->write("\"??");
        on_run_finished();
    }

    virtual void on_ready_read()
    {
        // NOW IS IMPORTANT TO DETECT THIS CONNECTION'S STATE:
        qDebug()<< QThread::currentThreadId() << this << "on_ready_read()";

        qDebug()<< "{{"
                << mp_socket->socket()->peerAddress() << ":"
                << mp_socket->socket()->peerPort()
                << "}}";
        QByteArray readed = mp_socket->socket()->readAll();
        qDebug()<< readed;
        mo_readed.push_back(readed);
        if( 1 == mo_readed.size() )
        {
            mi_readed = mo_readed.begin();
            mi_latest = mo_readed.begin();
        }
        else
        {
            ++ mi_latest;
        }
        schedule_self_to_thread_pool();
    }

    virtual void on_disconnected()
    {
        qDebug()<< QThread::currentThreadId() << this << "on_disconnected()";
    }

    virtual ~gx_async_tcp_thread(){}  // can be
};

class gx_async_tcp_server : public QTcpServer
{
    Q_OBJECT

public:

    explicit gx_async_tcp_server(unsigned short port, QObject* parent = NULL)
        : QTcpServer(parent)
        , m_port(port)
    {
    }

    virtual void start();

    virtual gx_async_tcp_thread* get_tcp_socket_behaviour()
    {
        return new gx_async_tcp_thread;
    }

protected:

    virtual void incomingConnection(qintptr socketDescriptor)
    {
        qDebug() << QThread::currentThreadId() << "incomingConnection";
        gx_async_tcp_socket* connection = new gx_async_tcp_socket;
        connection->set_socket_descriptor( socketDescriptor );
        gx_async_tcp_thread* connection_behaviour = get_tcp_socket_behaviour();
        if (connection_behaviour)
        {
            connection->set_socket_behaviour( connection_behaviour );
        }
        else
        {
            connection->set_socket_behaviour( new gx_async_tcp_thread );
        }
    }
    unsigned short m_port;
};

#endif  // GX_ASYNC_TCP_SERVER_H_INCLUDED
