#include <QGuiApplication>
#include <QDebug>
#include <QDir>

#include "mainwindow.h"
#include <gx_gui.h>

#include <gx_gdom_tcp_server.h>  // HTTP �����, JSON async parse & build,
                                 // Async TCP server and reactor.

static gx::view::gxGuiText TextData;

/** ������ GUI MainWindow.
 *
 * VTSS - ��������� �������� �������. ����� � ���� - ��������� ���������,
 * �� ���������� ������� VTSS ������ ������ ������� �������������� EVENT.
 * ����� ������� ��� VTSS - ������ ���������� ������������� ������������
 * �������� ������ - ��-����� ������� - ��������� ������.
 *
 * �������������, ������ �� ������ ��������� �������, ����� ������� ���
 * ��� ��������� �������������-������. ������ ��� VTSS ������������ ���
 * ��������������, ���������� � ��������� ������� �������� ��� ��������
 * ����������, ����������� ���� ������ VTSS ������ � ����. ������������
 * ������ �������������� �������� ������ - ���� ������, ��� �������, ���
 * ������. ������������ ���������� ��������� ������ �� ����� �����������
 * ������, � ������� ���� ��������� ������ � �������� ���������. �������
 * ��� VTSS �������� � ����� ��������� �������� ������ ���� ������ VTSS.
 * BEGIN "������ �����" => "������� ������" => "������ ������" END. �Ѩ.
 * �Ѩ ���� ���������� ������������� � ��������� ���������. ��� �������
 * ��������? - ��. �� ��� �� ������������ ���������� - ������ �� ��� �
 * ���� �����������, ��� � ��� �� ������ �� ���������������.
 *
 * GUI - ��������� �������� �������������(VIEW) VTSS ������(MODEL).
 *  ������������� (VIEW)
 * RAIDER ������������ ������������ ����������� ������������� ������.
 * �������� ����� ��� ���� VIEW ��� ����� � ���-�� ��������� MODEL.
 * ������ VIEW ����� ������� � ������������ ��� ����������� �������������
 * �������� MODEL, ������ ����, ��� ���������� MODEL � ��������������
 * ��������������. ���� ������� ������������� ����� ������, �� VIEW ������ ������ ���������. ��� �����
 *    "����������� ������������� MODEL", ������� �������� ��������� ������������
 *    MODEL, ������ MODEL ������ ����� ���� ������:
 *      -- ������ ��������� ��������, ������� ����������� ������������� "MODEL"
 *         ������ ��������� "������" ������������� � "�������" ���������� "MODEL"
 *
 */

int main(int argc, char *argv[]) {
    QGuiApplication a(argc, argv);

    // DETECT DATA FOLDER LOCATION AND CREATE IF IS SOME FIRST START ON MOBILE DEVICE
    QString app_name = a.applicationName();        qDebug() << "application Name:" << app_name;
    QString app_path = a.applicationFilePath();    qDebug() << "application File:" << app_path;
    QDir app_data_dir(a.applicationDirPath());     qDebug() << "app_data_dir    :" << app_data_dir;
    QDir mobile_files; mobile_files.currentPath(); qDebug() << "mobile_files    :" << mobile_files.absolutePath();
    if( !app_data_dir.cd("../dat") )
    {
        qDebug() << "unreachable ../dat from" << app_data_dir << " mobile?";
        app_data_dir = mobile_files;
        if( app_data_dir.cd("dat") )
        {
            qDebug() << "make data path 'dat'" << app_data_dir.mkdir("dat");
            if( !app_data_dir.cd("dat") )
            {
                qDebug() << "ERROR:can't create or open data dir 'dat'";
            }
        }
    }
    qDebug()<<"DATA ROOT AT" << app_data_dir.absoluteFilePath("dat");
    qDebug()<<"ARGUMENTS BEGIN"<< a.arguments() << "ARGUMENTS END";
    qDebug()<<" app_name :"<< app_name;
    qDebug()<<" app_path :"<< app_path;

    // SETUP MAIN WINDOW, WHERE GLES CAN DRAW SCENE VIEW
    gx::gui::MainWindow w;
    w.set_data_path(app_data_dir);  // HERE WE CAN SET DETECTED DATA FOLDER LOCATION
    w.setMinimumSize(QSize(200, 200 ));
    w.setWidth(400); w.setHeight(400); w.setPosition(4,4); w.show();
    // w.showFullScreen();

    // qDebug<<QThread::yieldCurrentThread()
    QThreadPool::globalInstance()->setMaxThreadCount(4);

    /** ���������� ����� �������, ������������� ����� �������� �� ������� ��������������
     *  TCP-����������. ����������� ����� "json_ready_read" ���������� � ���������� �����
     *  ����������, ����� ����������� ������ ��� �������� ������. �.�. � ������ (�������)
     *  ���������� �� �������� ��������� ��������� ���� ��� ��������� ����� �������. ���
     *  ��������� � ������� ��� "gx::json*". �� ����� �����, JSON ��� HTTP ��������� ����
     *  ������� - ��������� ������� ��� json, ������ ������� ������� ��� NULL.
     *
     *  ����� "json_ready_read" ���������� �� ���� ��� � ����� ����� ������� �����, ���
     *  ���������� ����� ���������. � ���������� ������ ���������� ������� �� �������
     *  ������, ������� �� ������ ���������, � ��������� ����������. ��� ������� ��������
     *  ����, ��� ���������� ��� �� ��������� �� ������� �������.

    class tcp_server_t : public gx_json_tcp_server
    {
    public:
        gx::gui::MainWindow* pWindow;
        tcp_server_t(int p) : gx_json_tcp_server(p, 0 ) { }

        virtual gx_async_tcp_reactor* get_tcp_socket_behaviour()
        {
            custom_reactor_t* p_reactor = new custom_reactor_t;
            p_reactor->pWindow = pWindow;
            return p_reactor;
        }
    } tcp_server(4680);
    **/

    /* ��� ���� gx::name ��������� - ����� �������
    GX_KEYS("maman","mylan","raman") src;
    const char** curr = src();
    unsigned size = 0;
    if (curr)
        for(; *curr; ++curr, ++size)
            qDebug() << QString().sprintf("<<%p>>%s", *curr, *curr );
    qDebug() << QString().sprintf("<END> size=%d", size);
    qDebug() << QString().sprintf("<END> len=%d", src.len());
    */

    //screen
    // os /fs /������ ������� ����� �������� �������� ���������� � �����, ��� dropbox url ��� wiki url
    // ����� ����� ������������ ������� ������� �� ��������� ��������� ������� �� ����� ������� � �����
    // ��� ����� ������ ������ �����������, � ��������� ������� DG ����� ������������ � ������, ����
    // ������� ����, ������� ����� ���������� �� ��������� ��������� ����� ����� (������ ��� url).
    // ���� ������ ������ "file://aliases/my/game/mordo-demona-12.jpeg" �� ��� ����������� ����.
    // ���� ������ ������ "file://aliases/my/game/mordo-demona-12.auto" �� �������������� ���������.
    /// C��������� �����  "file://aliases/my/game/mordo-demona-12.auto"
    /// { type: "GDOM_FILE_ACTOR"
    /// , refs: 700
    /// , done: [ "python2", "-f load_file_done.py to_static_jpeg" ]
    /// , load: [ "python2", "-f load_file_proc.py" ]
    /// , versions:
    ///   [ { "name": "file://md/EDRFAE0034011", "refs": 0, "load": "1001010100010001" }
    ///   , { "name": "file://md/234005DFAE304", "refs":23, "load": "1000010101001000" }
    ///   , { "name": "file://md/UT235D00F0AET", "refs":13, "load": "0"}
    ///   ]
    /// }
    // ���� ���� ������ �� ��������, � ��� ��� � ������, �� � ���� ������ ��� ������������ � ����������
    // ��������� ������. ������ � ����� � ���� "auto" ������������ ������� ��� ������������ �� �������,
    // ��� ������ ��� ���������� ���� �������� ����� ���� ������� � ������������. ���� ��������� �����
    // ���������� ������ ���������� ��� ���������� ������� - ������ ������ �� ����� ����������, � �����
    // ��������� ������ ���� ����� ������, �������������, �� ���� ���������� ������ ������ ������ ����.
    // fs is dict of file names w/o extentions. File name can't contain utf8|utf16|cp1251 etc. Why? Brain
    // Fack It Is Dakara. ASCII names only. Files with arabian names or with russian cyrylic? Nonsens.
    // ���� ���� ���� � ������ - ������ �� ��� ���������� ���-�� �������, ����������� is a hardref incref
    // for active scanners, or event send for passive waiters.
    // fs (dict) virtual const char* get_type_name() files_dir. file_load(command, actor)
    // load started failure success
    // save started failure success

    gx::dict* p_root   = gx::gdom::root();     // root of the GDOM tree ( is like some file-system tree )
    gx::dict* p_gles   = new gx::dict;         // new screen dict GDOM "dict" object ptr
    gx::dict* p_tx2d   = new gx::dict;         // opengl textures storage, each item texture object loaded
                                               // (or ready to load) to GPU memory, and has weekref to file
                                               // info, used only at "LOAD TEXTURE ACTION TIME"
    gx::dict* p_glsl   = new gx::dict;         // opengl shaders storage, each item is compiled or ready to
                                               // compile shaders. With weekref to file info. Files is only
                                               // files in files tree. Realy used only at
                                               // "LOAD FRAGMENT OR LOAD VERTEX SHADER ACTION TIME"
    gx::dict* p_cube   = new gx::dict;         // opengl cube textures storage
    gx::dict* p_file   = new gx::dict;         // real files or auto generators
    gx::dict* p_vars   = new gx::dict;         // shared variables with dynamically load
    gx::dict* p_pact   = new gx::dict;         // shared type declarations (interfaces)
    gx::dict* p_heap   = new gx::dict;         // gb and heap allocated variables (key here is uid)

    // GLGPU
    p_gles->set_item("tx2d", p_tx2d);
    p_gles->set_item("cube", p_cube);
    p_gles->set_item("glsl", p_glsl);

    // root
    p_root->set_item("gles", p_gles);  // defacto is one from PC GPU selected in start
    p_root->set_item("file", p_file);  // each key is file or folder
    p_root->set_item("heap", p_heap);  // defacto is one from PC GPU selected in start
    p_root->set_item("vars", p_vars);  // shared variables path
    p_root->set_item("pact", p_pact);  // shared prototypes path (is not some as variables tree)
                                       // here is "path" dual specific - built-in types and user
                                       // types specified using "path" as dependency graph "path"
                                       // both (types and vars) using GDOM types

    /* ��� ���� gx::json_text_dump
    //    gx::json* subj = gx::json_loads("[]");
    //    std::string dump;
    //    dump = gx::json_text_dump(subj);
    //    qDebug() << dump.c_str();
    //    return 0;
    */

    gx_gdom_tcp_server tcp_server(4680);       // Server used to async manipulations with GDOM data GRAPH
    tcp_server.start();
    return a.exec();
}
