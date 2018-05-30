#ifndef GX_HOWTO_ASYNC_TCP_SERVER_H_INCLUDED
#define GX_HOWTO_ASYNC_TCP_SERVER_H_INCLUDED

/* Для создания любого протокола обмена данными, поверх абстрактного TCP-сервера,
 * достаточно декларировать класс реакции на каждое входящее соединение (Реактор)
 * Каждый реактор представляет собой инстанцию состояния произвольного конечного
 * автомата состояний. Для простейших случаев достаточно одного состояния, часто
 * без мультизадачности. Мультизадачность не нужна, когда запросы обрабатываются
 * за время, равное или меньшее чем время старта параллельного вычисления.
 * Мультизадачность необходима в случаях, когда обработка пакета включает парсер,
 * выполнение нетривиальных операций в системе и последующее подтверждение успеха
 * или провала этих действий через запись в клиентский сокет. Данный пример будет
 * мультизадачным. Будет проверять входящие пакеты на предмет соответствия языку
 * текстового формата обмена данными JSON (подмножеству языка JavaScript). Режим
 * обработки пакетов - строго последовательный: Соединение, через которое клиент
 * пришлёт следующий запрос, до завершения обработки предыдущего запроса,- будет
 * разорвано. Адрес запомнен (, на случай отказа в последующем соединении). Сама
 * обработка пакетов будет мультизадачной (QThreadPool, QRunnable).
*/

#include <QByteArray>
#include <QDebug>
#include <QThreadPool>
#include <QRunnable>

#include <gx_json.h>
#include <gx_async_tcp_server.h>

// Use abstract "gx_async_tcp_server" class to create concrete "howto_async_server"
// Use abstract "gx_async_tcp_reactor" to define initial connection behaviour "gx_async_tcp_reactor"
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
    std::list<gx::json*>  mo_parsed;  // Parsed Data Chunks.
    std::list<QByteArray>::iterator mi_readed;
    std::list<QByteArray>::iterator mi_latest;

public:
    //const static int max_stored = 5;  // max stored results count
    gx_async_tcp_thread()
        :mp_parser(NULL)             // Async method is not created and not sheduled to threadpool.
        ,mi_readed(mo_readed.end())  // Currently Parsed Chunk of the Readed Data
        ,mi_latest(mo_readed.end())  // Latest Readed Chunk of the Readed Data
    {
        // qDebug() << QThread::currentThreadId()  << this << "RUNNABLE CREATED";
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
        if ( mo_readed.size() > 1 )
        {
            mo_readed.pop_front();
            gx::json* garbage = *mo_parsed.begin();
            if (garbage) delete garbage;
            mo_parsed.pop_front();
        }
        */
        delete mp_parser; mp_parser = NULL;
        if ( mi_readed != mi_latest ) {
            ++ mi_readed;
            schedule_self_to_thread_pool();
        }
    }

    virtual void on_success(const QString& _media_type)
    {
        qDebug() << QThread::currentThreadId() << this << "on_success thread finished (" << _media_type << ")";
        QString echo = QString("Ok");  //+ mi_readed->data();

        mp_socket->socket()->write(echo.toStdString().c_str());

        on_run_finished();
    }

    virtual void on_failure(const QString& _error_info)
    {
        qDebug() << QThread::currentThreadId() << this << "on_failure thread finished (" << _error_info << ")";

        mp_socket->socket()->write("\"??\"");

        on_run_finished();
    }

    /*
     * QString qstr_host_port(const quint32 ip4, const quint32 port = 0) const
    {
        QString buff;
        buff += QString::number((ip4 & 0xFF000000) >> 24, 10); buff += ".";
        buff += QString::number((ip4 & 0x00FF0000) >> 16, 10); buff += ".";
        buff += QString::number((ip4 & 0x0000FF00) >> 8 , 10); buff += ".";
        buff += QString::number( ip4 & 0x000000FF, 10);
        if(port) {
            buff += ":";
            buff += QString::number(mp_socket->socket()->peerPort(), 10);
        }
        return buff;
    }*/

    virtual void on_ready_read()
    {
        // qDebug()<< QThread::currentThreadId() << this << "on_ready_read()";
        qDebug()<< QThread::currentThreadId()<< this << qstr_peer() << "SEND:";

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
    void on_disconnected() { qDebug()<< QThread::currentThreadId() << this << "on_disconnected()"; }
};

class howto_async_server: public gx_async_tcp_server
{
public:
    howto_async_server(int port, QObject* parent = NULL)
        : gx_async_tcp_server(port, parent)
    {
    }
    gx_async_tcp_reactor* get_tcp_socket_behaviour()
    {
        return new gx_async_tcp_thread;
    }
};

#endif  // GX_HOWTO_ASYNC_TCP_SERVER_H_INCLUDED
