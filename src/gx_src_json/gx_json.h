#ifndef GX_JSON_WRAPPER_H_INCLUDED
#define GX_JSON_WRAPPER_H_INCLUDED

#include <stdexcept>
#include <list>
#include <map>

// struct gx::json::type_id const
#define GX_JSON( ... ) #__VA_ARGS__
#define GX_JSON_NONE 0
#define GX_JSON_DICT 1
#define GX_JSON_LIST 2
#define GX_JSON_CSTR 3
#define GX_JSON_SINT 4
#define GX_JSON_UINT 5
#define GX_JSON_REAL 6
#define GX_JSON_BOOL 7

namespace gx{
    struct json{
        struct DICT;   // JSON => objectValue
        struct LIST;   // JSON => arrayValue
        struct NONE;   // JSON => NULL
        struct CSTR;   // JSON => STRING
        struct SINT;   // JSON => SIGNED INT
        struct UINT;   // JSON => UNSIGNED INT
        struct REAL;   // JSON => REAL
        struct BOOL;   // JSON => BOOL
        struct proc{   // Abstract Visitor (visitor pattern)
            virtual void on(gx::json::DICT*)=0;
            virtual void on(gx::json::LIST*)=0;
            virtual void on(gx::json::NONE*)=0;
            virtual void on(gx::json::CSTR*)=0;
            virtual void on(gx::json::SINT*)=0;
            virtual void on(gx::json::UINT*)=0;
            virtual void on(gx::json::REAL*)=0;
            virtual void on(gx::json::BOOL*)=0;
            virtual~proc(){}
        };
        struct type_guard;
        struct type_name;
        struct type_id;

        static void free(gx::json*);

        virtual ~json() {}

        virtual void on(gx::json::proc*)=0; /*visitor support*/

        virtual bool is_dict(){ return false; }
        virtual bool is_list(){ return false; }
        virtual bool is_null(){ return false; }
        virtual bool is_cstr(){ return false; }
        virtual bool is_sint(){ return false; }
        virtual bool is_uint(){ return false; }
        virtual bool is_real(){ return false; }
        virtual bool is_bool(){ return false; }
    };

    // Return visitable JSON tree parsed from argument, or NULL if error.
    // To detect what error is occured use some other JSON library&tools.
    json* json_loads(const char*);

    // Return compact JSON dump
    std::string json_dumps(gx::json*);

    // Return formatted multiline JSON dump item
    std::string json_text_dump_item(gx::json* o, std::string head, std::string endl, bool root = false );

    // Return formatted multiline JSON dump
    std::string json_text_dump( gx::json* o, std::string line_head, std::string line_end = "\n" );

    // Return formatted multiline JSON dump
    std::string json_text_dump( gx::json* o, std::string line_head = "");

    // Delete visitable tree from memory
    void json_erase(gx::json*);

    struct json::DICT: gx::json
    {
        std::map<std::string, gx::json*> val;
        gx::json* get_item(const char* cc) {
            std::map<std::string, gx::json*>::iterator it = val.find(cc);
            return (it == val.end()) ? NULL : it->second;
        }
        bool is_dict(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        DICT(){}
    };

    struct json::LIST: gx::json
    {
        typedef std::list<gx::json*>::iterator iterator;
        iterator begin(){ return val.begin(); }
        iterator end(){ return val.end(); }
        std::list<gx::json*> val;
        bool is_list(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        std::size_t size() const { return val.size(); }
        LIST(){}
    };

    struct json::NONE: gx::json
    {
        bool is_null(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        NONE(){}
    };

    struct json::CSTR: gx::json
    {
        bool is_cstr(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        CSTR(const char* src):value(src){}
        std::string value;
    };

    struct json::SINT: gx::json
    {
        bool is_sint(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        SINT(const int& v):value(v){}
        int value;
    };

    struct json::UINT: gx::json
    {
        bool is_uint(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        UINT(const unsigned int& v):value(v){}
        unsigned int value;
    };

    struct json::REAL: gx::json
    {
        bool is_real(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        REAL(const double& v):value(v){}
        double value;
    };

    struct json::BOOL: gx::json
    {
        bool is_bool(){ return true; }
        void on(gx::json::proc*o){ o->on(this); }
        BOOL(const bool& v):value(v){}
        bool value;
    };
}

struct gx::json::type_name:gx::json::proc{
    std::string ret;
    virtual void on(gx::json::DICT*){ret="DICT";}
    virtual void on(gx::json::LIST*){ret="LIST";}
    virtual void on(gx::json::NONE*){ret="NONE";}
    virtual void on(gx::json::CSTR*){ret="CSTR";}
    virtual void on(gx::json::SINT*){ret="SINT";}
    virtual void on(gx::json::UINT*){ret="UINT";}
    virtual void on(gx::json::REAL*){ret="REAL";}
    virtual void on(gx::json::BOOL*){ret="BOOL";}
    std::string operator()(gx::json*o){o->on(this); return ret;}
};

struct gx::json::type_id:gx::json::proc{
    int ret;
    virtual void on(gx::json::DICT*){ret=GX_JSON_DICT;}
    virtual void on(gx::json::LIST*){ret=GX_JSON_LIST;}
    virtual void on(gx::json::NONE*){ret=GX_JSON_NONE;}
    virtual void on(gx::json::CSTR*){ret=GX_JSON_CSTR;}
    virtual void on(gx::json::SINT*){ret=GX_JSON_SINT;}
    virtual void on(gx::json::UINT*){ret=GX_JSON_UINT;}
    virtual void on(gx::json::REAL*){ret=GX_JSON_REAL;}
    virtual void on(gx::json::BOOL*){ret=GX_JSON_BOOL;}
    int operator()(gx::json*o){o->on(this);return ret;}
};

struct gx::json::type_guard:gx::json::proc{
    struct unexpected{ std::string str; unexpected(const char* s):str(s){} };
    virtual void on(gx::json::DICT*){throw unexpected("DICT");}
    virtual void on(gx::json::LIST*){throw unexpected("LIST");}
    virtual void on(gx::json::NONE*){throw unexpected("NONE");}
    virtual void on(gx::json::CSTR*){throw unexpected("CSTR");}
    virtual void on(gx::json::SINT*){throw unexpected("SINT");}
    virtual void on(gx::json::UINT*){throw unexpected("UINT");}
    virtual void on(gx::json::REAL*){throw unexpected("DICT");}
    virtual void on(gx::json::BOOL*){throw unexpected("BOOL");}
    virtual ~type_guard(){}
};

//is first step of build gdom instance - parse json tree and build builders

struct gx_json_no_operation_t: gx::json::proc
{
    virtual void on(gx::json::DICT *){}
    virtual void on(gx::json::LIST *){}
    virtual void on(gx::json::NONE *){}
    virtual void on(gx::json::CSTR *){}
    virtual void on(gx::json::SINT *){}
    virtual void on(gx::json::UINT *){}
    virtual void on(gx::json::REAL *){}
    virtual void on(gx::json::BOOL *){}
    virtual ~gx_json_no_operation_t(){}
};

struct gx_json_type_guard_t: gx::json::proc
{
    const char* error_c_str;
    virtual ~gx_json_type_guard_t(){}
    virtual void on(gx::json::DICT *){error_c_str= "Unexpected DICT";}
    virtual void on(gx::json::LIST *){error_c_str= "Unexpected LIST";}
    virtual void on(gx::json::NONE *){error_c_str= "Unexpected NONE";}
    virtual void on(gx::json::CSTR *){error_c_str= "Unexpected CSTR";}
    virtual void on(gx::json::SINT *){error_c_str= "Unexpected SINT";}
    virtual void on(gx::json::UINT *){error_c_str= "Unexpected UINT";}
    virtual void on(gx::json::REAL *){error_c_str= "Unexpected REAL";}
    virtual void on(gx::json::BOOL *){error_c_str= "Unexpected BOOL";}
};

struct gx_json_type_error_t: gx::json::proc
{
    const char* unexpected_type_name;
    gx_json_type_error_t():unexpected_type_name(NULL){}
    virtual ~gx_json_type_error_t(){}
    virtual void on(gx::json::DICT *){ unexpected_type_name = "DICT";}
    virtual void on(gx::json::LIST *){ unexpected_type_name = "LIST";}
    virtual void on(gx::json::NONE *){ unexpected_type_name = "NONE";}
    virtual void on(gx::json::CSTR *){ unexpected_type_name = "CSTR";}
    virtual void on(gx::json::SINT *){ unexpected_type_name = "SINT";}
    virtual void on(gx::json::UINT *){ unexpected_type_name = "UINT";}
    virtual void on(gx::json::REAL *){ unexpected_type_name = "REAL";}
    virtual void on(gx::json::BOOL *){ unexpected_type_name = "BOOL";}
    const char* type_error(){ return unexpected_type_name; }
};

#endif // GX_JSON_WRAPPER_H_INCLUDED
