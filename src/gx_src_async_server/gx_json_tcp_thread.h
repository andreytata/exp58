#ifndef GX_ASYNC_THREAD_H
#define GX_ASYNC_THREAD_H

#include <sstream>

#include <QByteArray>
#include <QDebug>
#include <QThreadPool>
#include <QRunnable>
#include <QUrlQuery>

#include <gx_json.h>
#include <gx_async_tcp_server.h>
#include <http_headers.h>

#define __GX_NODE_INFO_FAVICON_ICO \
    "<link href=\"data:image/x-icon;base64,AAABAAEAEBAQAAAAAAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAg"\
    "AAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAEhEQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"\
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"\
    "AAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD"\
    "//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP7/AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA\""\
    " rel=\"icon\" type=\"image/x-icon\" />"

#define __GX_NODE_INFO_HTML_STYLE \
    "<style>"\
      ".example {"\
        "border: 1px solid black;"  /* Параметры рамки */\
        "background: #fffff5; "     /* Цвет фона */\
        "font-family: monospace;"   /* Шрифт текста */\
        "padding: 3 0 0 7;"         /* Поля вокруг текста */\
        "margin: 1px 0 1em;"        /* Отступы вокруг */\
        "white-space: pre;"         /* Учитывать переносы и пробелы */\
      "}"\
      ".exampleTitle {"\
        "border: 1px solid black;"   /* Параметры рамки */\
        "border-bottom: none;"       /* Убираем линию снизу */\
        "padding: 3px 7px 2px 7px;"              /* Поля вокруг текста */\
        "display: inline;"           /* Устанавливаем как встроенный элемент */\
        "background: #efecdf;"       /* Цвет фона */\
        "font-family: monospace;"    /* Шрифт текста */\
        "font-weight: bold;"\
        "font-size: 90%;"\
        "margin: 0; "                /* Убираем отступы вокруг */\
        "white-space: nowrap;"       /* Отменяем переносы текста */\
      "}"\
    "</style>"

// Use abstract "gx_async_tcp_server" class to create concrete "howto_async_server"
// Use abstract "gx_async_tcp_reactor" to define initial connection behaviour "gx_async_tcp_reactor"
class gx_json_tcp_thread: public gx_async_tcp_reactor
{
    class abstract_state
    {
    // Each instance of the gx_json_tcp_thread class delegate each call of self interface methods to
    // instance of self current state, with pointer to self as first parameter. (GOF, State Machine).
    // where current state defined as gx_json_tcp_thread's member with type "pointer to abstract_state"

    public:
        virtual ~abstract_state(){}
        virtual void disconnect(gx_json_tcp_thread*)=0;
        virtual void ready_read(gx_json_tcp_thread*)=0;
        virtual void on_success(gx_json_tcp_thread*, const QString&){}
        virtual void on_failure(gx_json_tcp_thread*, const QString&){}
    };

    class sess_init_state_t: public abstract_state
    {
    public:
        void disconnect(gx_json_tcp_thread* p_reactor){ p_reactor->disconnect(0); }
        void ready_read(gx_json_tcp_thread* p_reactor){
            p_reactor->set_wait_size();
            p_reactor->on_ready_read();
        }
    } sess_init_state;

    class wait_size_state_t: public abstract_state
    {
        // This method are not part of the State Machine signal interface.
        // Normal way to use gx_json_tcp_thread: JSON request => JSON result.
        // This is some utilite method to convert HTTP-GET-REQUEST to JSON request.
       QString json_dumps_get_request(const QHttpRequestHeader& request)
       {
           QString dump;  // JSON text accomulate here ( COW supported? )
           dump += "{\"Json\":\"HTTP_GET_REQUEST\"\n";        // set base interface

           QStringList keys = request.keys();
           for(QStringList::iterator i = keys.begin(); i != keys.end(); ++i)
           {
               QString key(*i);
               if ( key == "Accept-Language" ) key = "Lang";  // rename
               else if ( key == "Connection" ) key = "Keep";  // rename
               else if ( key == "User-Agent" ) key = "User";  // rename
               else if ( key == "Host" )                   ;  // as is
               else                                continue;  // ignore all unsupported

               dump += ",\""; dump += key ; dump += "\":\""; dump += request.value(*i); dump += "\"\n";
           }

           QUrl url(request.path());
           QStringList path = url.path().split("/");
           dump += ",\"Path\":[";
           unsigned int path_len = 0;
           for(QStringList::iterator i = path.begin(); i != path.end(); ++i)
           {
               if( (*i).size() )
               {
                   path_len += 1;
                   dump += "\""; dump += *i; dump += "\",";
               }
           }
           if ( path_len )
               dump.replace(dump.size()-1,1,"]");
           else
               dump+="]";
           dump += "\n";

           QUrlQuery url_query(url.query());
           QList<QPair<QString,QString> > query_tokens = url_query.queryItems();
           QList<QPair<QString,QString> >::iterator iter = query_tokens.begin();
           QList<QPair<QString,QString> >::iterator stop = query_tokens.end();
           QString url_query_as_json;
           url_query_as_json += "{";
           for(; iter != stop; ++iter)
           {
              url_query_as_json += "\"";
              url_query_as_json += iter->first + "\":\"";
              url_query_as_json += iter->second  + "\",";
           }
           if( url_query_as_json.size() > 1 )
               url_query_as_json.replace(url_query_as_json.size()-1,1,"}");
           else
               url_query_as_json += "}";

           dump += ",\"Args\":"; dump += url_query_as_json; dump += "\n}";
           return dump;
       }

    public:
        void disconnect(gx_json_tcp_thread* p_reactor){ p_reactor->disconnect(1); }
        void ready_read(gx_json_tcp_thread* p_reactor){
            // qDebug() << QThread::currentThreadId() << p_reactor << "ready_read <wait_size>" << p_reactor->qstr_peer();
            if( p_reactor->socket()->bytesAvailable() < 4 )
            {
                //qDebug() << "NEED 4 BYTES, BUT bytesAvailable() =>" << p_reactor->mp_socket->socket()->bytesAvailable();
                return;  // so, continue in wait size mode
            }
            char head[5] = {'J','F','F','F','\0'};
            p_reactor->socket()->read(head, 4);
            switch (head[0])
            {
                case 'J':
                { // parse size
                    head[4]=0;
                    QString str_size(head+1);
                    bool success;
                    quint32 sz = str_size.toULong(&success, 16);
                    if (success)
                    {
                        // qDebug() << "-> WAIT JSON WITH SIZE" << sz;
                        p_reactor->set_body_size(sz);
                        p_reactor->set_wait_body();
                        p_reactor->on_ready_read();
                    }
                    else
                    {
                        // qDebug() << "-> FAIL READ SIZE FROM '" << str_size << "'";
                        p_reactor->socket()->disconnectFromHost();
                        p_reactor->mp_socket->deleteLater();
                    }
                }
                break;

                case 'G':  // CLIENT IS SOME BROWSER, QUERY IS SOME HTTP GET (builtin debug mode)
                {          // REACTOR STATE SKIP SWITCH TO WAIT BODY STATE
                           // PARSE/CONVERT REQUEST TO JSON
                           // SWITCH REACTOR STATE TO ASYNC PARSE JSON;

                        // read all available bytes from TCP socket
                        QByteArray tail = p_reactor->socket()->readAll();
                        head[4]=0;            // zero terminate first 4 readed bytes
                        QString query(head);  // create query buff from head
                        query += tail;        // add tail to buff


                        // parse query, using old deprecated QHttpRequestHeader parser
                        // QHttpRequestHeader request(query);
                        // qDebug() << "REQUEST KEYS:" << request.keys();
                        // qDebug() << request.path();
                        // QUrl url = QUrl(request.path());
                        // QUrlQuery url_query(url);
                        p_reactor->mo_body_buff = json_dumps_get_request(query).toLocal8Bit();
                        p_reactor->set_wait_task();
                        p_reactor->run_task();

                        /*
                        // INSERT ICON - IS ONLY PREVENT SEPARETE GET FOR "favicon.ico" (DMFD).
                        // or using "<link href=\"favicon.ico\" rel=\"icon\" type=\"image/x-icon\" />"
                        std::ostringstream html;
                        html <<"<html><head>";
                        html <<   "<title>" << "url.path().toStdString()" << "</title>";
                        html <<   GX_NODE_INFO_FAVICON_ICO;
                        html <<   GX_NODE_INFO_HTML_STYLE;
                        html <<"</head><body>";
                        html <<   "<p class=\"exampleTitle\">" << "GX_DIR_PATH" << "</p><p class=\"example\">";
                        html <<    json_dumps_get_request(query).toStdString() << "\n";
                        html <<   "</p>";
                        html <<"</body></html>";

                        // GENERATE RESPONSE
                        QString message_body = html.str().c_str();
                        QHttpResponseHeader response(200, "OK");
                        response.setContentLength(message_body.size());  // response.setValue("Content-Length", message_body.size() ); //response.setValue("Status-Line", "OK");
                        response.setContentType("text/html");
                        response.setValue("Cache-Control","no-cache");   // response.setValue("Date", "Sun, 15 Mar 2009 05:00:48 GMT");
                        response.setValue("Server", "nginx/1.21");       // TODO: is needed ??
                        response.setValue("Connection", "keep-alive");   // Keep socket open


                        QString message = response.toString() + message_body;
                        //qDebug() << message;
                        p_reactor->mp_socket->socket()->write(message.toStdString().c_str());
                        */
                        // if not keep alive:
                        // p_reactor->mp_socket->socket()->disconnectFromHost();
                        // p_reactor->mp_socket->deleteLater();
                }
                break;

                default:
                    p_reactor->mp_socket->socket()->disconnectFromHost();
                    p_reactor->mp_socket->deleteLater();
            }
        }
    } wait_size_state;

    class wait_body_state_t: public abstract_state
    {
    public:
        void disconnect(gx_json_tcp_thread* p_reactor){ p_reactor->disconnect(2); }
        void ready_read(gx_json_tcp_thread* p_reactor){
            if ( p_reactor->mp_socket->socket()->bytesAvailable() < p_reactor->mo_body_size )
            {
                return;  // not available bytes to start parse JSON so, continue in wait size mode
            }
            p_reactor->mo_body_buff = p_reactor->mp_socket->socket()->read(p_reactor->mo_body_size);
            p_reactor->set_wait_task();
            p_reactor->run_task();
        }
    } wait_body_state;

    class wait_task_state_t: public abstract_state
    {
    public:
        void disconnect(gx_json_tcp_thread* p_reactor){ p_reactor->disconnect(3); }
        void ready_read(gx_json_tcp_thread*){}
        void on_success(gx_json_tcp_thread* p_reactor, const QString& info) {
            // qDebug() << QThread::currentThreadId() << p_reactor << "success on <wait task>" << info;
            p_reactor->mo_json_info = (QString("Ok ") + info).toStdString();
            //p_reactor->mp_socket->socket()->write(echo.toStdString().c_str());
            p_reactor->fin_task(); //on_run_finished(p_reactor);
        }
        void on_failure(gx_json_tcp_thread* p_reactor, const QString& info) {
            // qDebug() << QThread::currentThreadId() << p_reactor << "failure on <wait_task>" << info;
            p_reactor->mo_json_info = (QString("?? ") + info).toStdString();
            //p_reactor->mp_socket->socket()->write(echo.toStdString().c_str());
            p_reactor->fin_task(); // on_run_finished(p_reactor);
        }
    } wait_task_state;

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
            mp_parsed = gx::json_loads(mp_readed->constData());
            if( mp_parsed )
                QMetaObject::invokeMethod( mp_report, "slot_success", Qt::QueuedConnection, Q_ARG(QString,"JSON"));
            else
                QMetaObject::invokeMethod( mp_report, "slot_failure", Qt::QueuedConnection, Q_ARG(QString,"JSON ERROR"));
        }
    };

    abstract_state*       mp_state;      // Current Connection State
    async_parser*         mp_parser;     // Async object sheduled to threadpool.
    std::list<gx::json*>  mo_parsed;     // Parsed Data Chunks.
    quint32               mo_body_size;  // Input Json Size
    QByteArray            mo_body_buff;  // Input Json Buff
protected:
    std::string           mo_json_info;  // Copy of the json parser info

public:
    gx_json_tcp_thread()
        :mp_state(&sess_init_state)
        ,mp_parser(NULL)             // Async method is not created and not sheduled to threadpool.
    {   // qDebug() << QThread::currentThreadId()  << this << "RUNNABLE CREATED";
    }
    virtual~gx_json_tcp_thread(){}

    virtual void run_task()
    {
        mp_parser = new async_parser(&mo_body_buff, mp_socket);
        QThreadPool::globalInstance()->start(mp_parser);
    }

    virtual void fin_task()
    {
        mo_parsed.push_back(mp_parser->get_result());
        delete mp_parser;  mp_parser = NULL;
        json_ready_read();  // here can be sended responce socket()->write
        set_wait_size();  // continue in wait header state
        if( mp_socket->socket()->bytesAvailable() )
            mp_state->ready_read(this);  // call self ready read if bytes available
    }

    /*
    virtual void on_json_parser_is_finished( bool is_json, const QString& info )
    {
        if ( is_json ) { // void on_success(gx_json_tcp_thread* p_reactor, const QString& info) {
            // qDebug() << QThread::currentThreadId() << p_reactor << "success on <wait task>" << info;
            QString echo = QString("Ok ") + info;
            mp_socket->socket()->write(echo.toStdString().c_str());
            fin_task(); //on_run_finished(p_reactor);
        }
        else
        {
        void on_failure(gx_json_tcp_thread* p_reactor, const QString& info) {
            // qDebug() << QThread::currentThreadId() << p_reactor << "failure on <wait_task>" << info;
            QString echo = QString("?? ") + info;
            p_reactor->mp_socket->socket()->write(echo.toStdString().c_str());
            p_reactor->fin_task(); // on_run_finished(p_reactor);
        }
    }
    */

    void on_success(const QString& _type)  { mp_state->on_success(this, _type); }
    void on_failure(const QString& _info)  { mp_state->on_failure(this, _info); }
    void on_ready_read()                   { mp_state->ready_read(this);        }
    void on_disconnected()                 { mp_state->disconnect(this);        }

    virtual void set_sess_init()           {}
    virtual void set_wait_size()           { mp_state = &wait_size_state; }
    virtual void set_wait_body()           { mp_state = &wait_body_state; }
    virtual void set_wait_task()           { mp_state = &wait_task_state; }
    virtual void set_body_size(quint32 sz) { mo_body_size = sz;}

    std::list<gx::json*>& get_parsed()     { return mo_parsed; }
    virtual void json_ready_read()         {
        qDebug() <<         "ready_read OK JSON from" << qstr_peer() << "at" << *get_parsed().begin();
        qDebug() << gx::json_dumps(*get_parsed().begin()).c_str();
        socket()->write(mo_json_info.c_str());
        if( *get_parsed().begin() )
        {
            gx::json_erase(*get_parsed().begin());
        }
        get_parsed().pop_front();
    }

    virtual void disconnect(const int& st) {
        switch (st)
        {
        case 0: qDebug() << "disconnect on sess init" << qstr_peer(); break;
        case 1: qDebug() << "disconnect on wait size" << qstr_peer(); break;
        case 2: qDebug() << "disconnect on wait body" << qstr_peer(); break;
        case 3: qDebug() << "disconnect on wait task" << qstr_peer(); break;
        }
    }
};

class gx_json_tcp_server: public gx_async_tcp_server
{
public:
    gx_json_tcp_server(int port, QObject* parent = NULL)
        : gx_async_tcp_server(port, parent)
    {
    }
    virtual gx_async_tcp_reactor* get_tcp_socket_behaviour()
    {
        return new gx_json_tcp_thread;
    }
    virtual~gx_json_tcp_server(){}
};

#endif // GX_ASYNC_THREAD_H
