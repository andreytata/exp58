#include <sstream>
#include <string>

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include <gx_json.h>

gx::json* gx::json_loads(const char* pch)
{
    struct
    {
        gx::json* on_numb(const QJsonValue& o) const
        {
            double d = o.toDouble();
            int i = int(d);
            if ( d == i )
            {
                if ( i > 0 ) {
                    return new gx::json::UINT(i);
                }
                return new gx::json::SINT(i);
            }
            return new gx::json::REAL(i);
        }
        gx::json* operator()(const QJsonValue& o) const
        {
            if( o.isObject () ) return (*this)(o.toObject());
            if( o.isArray  () ) return (*this)(o.toArray());
            if( o.isString () ) return new gx::json::CSTR(o.toString().toStdString().c_str());
            if( o.isBool   () ) return new gx::json::BOOL(o.toBool());
            if( o.isNull   () ) return new gx::json::NONE;
            if( o.isDouble () ) return on_numb(o);
            return new gx::json::NONE;
        }
        gx::json* operator()(const QJsonArray& o) const
        {
            gx::json::LIST* p_list = new gx::json::LIST;
            for( QJsonArray::const_iterator it = o.constBegin(); it != o.constEnd(); ++it )
            {   // const QJsonValue &val = *it;
                p_list->val.push_back((*this)(*it));
            }
            return p_list;
        }
        gx::json* operator()(const QJsonObject& o) const
        {
            gx::json::DICT* p_dict = new gx::json::DICT;
            for( QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it )
            {   // const QJsonValue &val = it.value();  const QString    &key = it.key();
                p_dict->val[ it.key().toStdString() ] = (*this)(it.value());
            }
            return p_dict;
        }
        gx::json* operator()(const QJsonDocument& o) const {
            if (o.isArray()) return (*this)(o.array());
            if (o.isObject()) return (*this)(o.object());
            return new gx::json::NONE;
        }
    } parser;
    QString src(pch);
    QJsonParseError err;
    QJsonDocument doc = doc.fromJson(src.toUtf8(), &err);
    if(err.error) return NULL;
    return parser(doc);
}

// return compact JSON dump
std::string gx::json_dumps(gx::json* o)
{
    struct dumps_t: gx::json::proc
    {
        std::ostringstream& oss;
        dumps_t(std::ostringstream& ss):oss(ss){}
        void on(gx::json::DICT*o)
        {
            typedef std::map<std::string,gx::json*>::iterator iter_t;
            oss << "{";
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                oss << ( ( it != o->val.begin() ) ? "," : "" )
                    << "\"" << it->first << "\":"
                    << gx::json_dumps(it->second);
            oss << "}";
        }
        void on(gx::json::LIST*o)
        {
            typedef std::list<gx::json*>::iterator iter_t;
            oss << "[";
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                oss << ( ( it != o->val.begin() ) ? ",":"" ) << gx::json_dumps(*it);
            oss << "]";
        }
        void on(gx::json::NONE*){oss<<"null";}
        void on(gx::json::CSTR*o){oss<<"\""<<o->value<<"\"";}
        void on(gx::json::SINT*o){oss<<o->value;}
        void on(gx::json::UINT*o){oss<<o->value;}
        void on(gx::json::REAL*o){oss<<o->value;}
        void on(gx::json::BOOL*o){oss<<o->value;}
    };
    if( NULL == o ) return "null";
    std::ostringstream oss;
    dumps_t dumps(oss);
    o->on(&dumps);
    return oss.str();
}

// return formatted multiline JSON dump item
std::string gx::json_text_dump_item(gx::json* o, std::string head, std::string endl, bool root )
{
    struct dumps_t: gx::json::proc
    {
        std::ostringstream          &oss; std::string head; std::string endl;  bool root;
        dumps_t( std::ostringstream &oss, std::string head, std::string endl,  bool root)
            :                    oss(oss),       head(head),       endl(endl), root(root)
        {
        }

        void on(gx::json::DICT*o)
        {
            // IS RAW JSON, READABLE MEEN ONLY DICTS ARE MULTILINE,
            // LEADING DICT WITHOUT endl
            if ( 0 == o->val.size() ) {
                oss << "{}";
                return;
            }
            bool multiline = ( o->val.size() > 1 ) ? true : false;
            std::string pref = root ? "": endl;
            std::string dict_head = multiline ? pref + head + "{" : pref + head + "{";
            std::string dict_next = multiline ? endl + head + "," : "";
            std::string dict_tail = multiline ? endl + head + "}" : "}";

            typedef std::map<std::string,gx::json*>::iterator iter_t;
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                oss << ( ( it != o->val.begin() ) ? dict_next : dict_head )
                    << "\"" << it->first << "\":"
                    << gx::json_text_dump_item( it->second, head + "  ", endl, false );
            oss << dict_tail;
        }

        void on(gx::json::LIST*o)
        {
            if ( 0 == o->val.size() ) {
                oss << "[]";
                return;
            }
            typedef std::list<gx::json*>::iterator iter_t;
            oss << "[";
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                oss << ( ( it != o->val.begin() ) ? ",":"" ) << gx::json_text_dump_item(*it, head+"++", endl, false);
            oss << "]";
        }
        void on(gx::json::NONE*) {oss<<"null";}
        void on(gx::json::CSTR*o){oss<<"\""<<o->value<<"\"";}
        void on(gx::json::SINT*o){oss<<o->value;}
        void on(gx::json::UINT*o){oss<<o->value;}
        void on(gx::json::REAL*o){oss<<o->value;}
        void on(gx::json::BOOL*o){oss<<o->value;}
    };
    if( NULL == o ) return "null";
    std::ostringstream oss;
    dumps_t dumps(oss, head, endl, root);
    o->on(&dumps);
    return oss.str();
}

// return formatted multiline JSON dump
std::string gx::json_text_dump( gx::json* o, std::string line_head, std::string line_end )
{
    return json_text_dump_item(o, line_head, line_end, true );
}

// return formatted multiline JSON dump
std::string gx::json_text_dump( gx::json* o, std::string line_head)
{
    return json_text_dump(o, line_head, "\n");
}

void gx::json_erase(gx::json* o)
{
    struct : gx::json::proc
    {
        void on(gx::json::DICT*o)
        {
            typedef std::map<std::string,gx::json*>::iterator iter_t;
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                gx::json_erase(it->second);
        }
        void on(gx::json::LIST*o)
        {
            typedef std::list<gx::json*>::iterator iter_t;
            for(iter_t it = o->val.begin(); it != o->val.end(); ++it)
                gx::json_erase(*it);
        }
        void on(gx::json::NONE*o){ delete o; }
        void on(gx::json::CSTR*o){ delete o; }
        void on(gx::json::SINT*o){ delete o; }
        void on(gx::json::UINT*o){ delete o; }
        void on(gx::json::REAL*o){ delete o; }
        void on(gx::json::BOOL*o){ delete o; }
    } erase;
    o->on(&erase);
}

