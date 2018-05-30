#ifndef GX_GDOM_TCP_SERVER_H
#define GX_GDOM_TCP_SERVER_H

#include <gx_json_tcp_thread.h>  // HTTP стили, JSON async parse & build, Async TCP server and reactor.
#include <gx_gdom.h>
#include <gx_name.h>

#define PRINT_SOCKET_INFO(S) qDebug() << this->socket() << S.str().c_str();

class gx_gdom_tcp_reactor: public gx_json_tcp_thread
{
    std::string query_interface;
    std::ostringstream oss;  // query string stream
    std::ostringstream err;  // extended error info
    gx::json::DICT* p_root;  // has valid value after success "json_query_contain_interface"
    gx::json*       p_json;  // temporary variable

public:
    void json_ready_read()
    {
        // qDebug() << "ready_read OK JSON from" << qstr_peer() << "at" << *get_parsed().begin();
        // ready read json ("event-loop-thread" process event "async-json-parser-are-finished")
        while( get_parsed().begin() != get_parsed().end() )
        {
            // Store front pointer, from the parsed queue, to temporary variable:
            gx::json* p_gx_json_parsed_tree = *get_parsed().begin();
            // Delete first item of the parsed queue (here used std::list)
            get_parsed().pop_front();              // free list item
            json_reactor(p_gx_json_parsed_tree);   // react on json
        }
    }

    virtual void on_query_is_http_get_request();

    bool json_query_contain_interface(gx::json* j)
    {
        oss << gx::json_dumps(j).c_str();
        PRINT_SOCKET_INFO(oss);

        if( false == j->is_dict() ) {
            err << "query root must be DICT";
            PRINT_SOCKET_INFO(err);
            return false;
        }

        p_root = (gx::json::DICT*)j;
        p_json = p_root->get_item("Json");

        if( NULL == p_json ) {
            err << "query root missed 'Json' field";
            PRINT_SOCKET_INFO(err);
            return false;
        }

        if( false == p_json->is_cstr() ) {
            err << "query root 'Json' field not STR";
            PRINT_SOCKET_INFO(err);
            return false;
        }

        gx::json::CSTR* p_cstr = (gx::json::CSTR*) p_json;
        query_interface = p_cstr->value;
        return true;
    }

    void json_reactor(gx::json* j)  // return
    {
        //if( false == oss.empty() )
        oss.str(""); oss.clear();

        //if( false == err.empty() )
        err.str(""); err.clear();

        if( NULL == j ) {
            err << "query is not JSON";
            PRINT_SOCKET_INFO(err);
            socket()->write(mo_json_info.c_str());
            return;
        }

        if ( json_query_contain_interface( j ) )
        {
            if ( query_interface == "HTTP_GET_REQUEST" )
            {
                qDebug() << this->socket() << "HTTP_GET_REQUEST";
                on_query_is_http_get_request();
            }
            else if ( query_interface == "JSON_GET_REQUEST" )
            {
                qDebug() << this->socket() << "JSON_GET_REQUEST";
                socket()->write(mo_json_info.c_str());
            }
            else if ( query_interface == "JSON_SET_REQUEST" )
            {
                qDebug() << this->socket() << "JSON_SET_REQUEST";
                socket()->write(mo_json_info.c_str());
            }
            else
            {
                err << "query type not supported \"" << query_interface.c_str() << "\"";
                qDebug() << this->socket() << err.str().c_str();
                socket()->write(mo_json_info.c_str());
            }
        }
        else
        {
            socket()->write(mo_json_info.c_str());
        }
        gx::json_erase(j);  // delete parsed JSON from memory
        return;
    }
};

class gx_gdom_tcp_server : public gx_json_tcp_server
{
public:

    gx_gdom_tcp_server(int p) : gx_json_tcp_server(p, NULL)
    {
    }

    virtual~gx_gdom_tcp_server()
    {
    }

    virtual gx_async_tcp_reactor* get_tcp_socket_behaviour()
    {
        return new gx_gdom_tcp_reactor;
    }
};

//USAGE: gx_gdom_tcp_server tcp_server(4680); tcp_server.start();

#endif // GX_GDOM_TCP_SERVER_H
