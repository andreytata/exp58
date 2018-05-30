#include <algorithm>
#include <iostream>
#include <vector>

#include "mainwindow.h"
#include "basicusagescene.h"
#include "gx_gui_text_item.h"
#include "glassert.h"
#include "gxcodefont32.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QThread>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QImage>

static struct gxFontImages
{
    std::vector<QImage*> char_images;
    gxFontImages()
    {
        for(std::size_t i=0; i<0x100; ++i)
        {
            char_images.push_back( new QImage( gx_code_font_32(i) ) );
        }
    }
    ~gxFontImages()
    {
        for(std::size_t i=0; i<0x100; ++i)
        {
           QImage* temp = char_images[i];
           delete temp;
        }
    }
    QImage* operator()(const unsigned char c){return char_images[c];}
}
gx_font_images;

static void infoGL()  //Send OpenGL Version Info To QT Debug
{
    const GLubyte *str;
    glCheckError();
    qDebug() << "OpenGL infos with gl functions";
    str = glGetString(GL_RENDERER);
    qDebug() << "Renderer : " << QString((const char*)str);
    str = glGetString(GL_VENDOR);
    qDebug() << "Vendor : " << QString((const char*)str);
    str = glGetString(GL_VERSION);
    qDebug() << "OpenGL Version : " << QString((const char*)str);
    str = glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "GLSL Version : " << QString((const char*)str);
    glCheckError();
}

gx::gui::MainWindow::MainWindow(QScreen* p_screen)
    :QWindow(p_screen)
    ,mb_is_initited(false)
    ,mb_is_current(false)
    ,mp_scene(new BasicUsageScene())
    ,mp_ui(new gxGuiTextItem())
{
    // Tell Qt we will use OpenGL for this window
    qDebug()<<"gx::gui::MainWindow::MainWindow BEGIN";
    setSurfaceType( OpenGLSurface );

    // Specify the format and create platform-specific surface
    QSurfaceFormat format;  //TRY OpenGL ES 2.0
    format.setDepthBufferSize( 24 );
    format.setMajorVersion( 2 );
    format.setMinorVersion( 0 );
    format.setSamples( 4 );
    format.setProfile( QSurfaceFormat::CoreProfile );

    setFormat(format);
    create();

    // Create an OpenGL context
    mp_context = new QOpenGLContext();
    mp_context->setFormat(format);
    mp_context->create();

    if(!mp_context->isValid()){
        std::cerr << "ERROR: MainWindow::MainWindow => The OpenGL context is invalid!" << std::endl;
    }
    else{
        std::cout<<"MainWindow::MainWindow => The OpenGL context is valid!" << std::endl;
    }

    // Make the context current on this window
    int max_try_count = 10;
    int sleep_interval_ms = 10;  // sleep time between trys in ms
    for(int i=0; i <= max_try_count; ++i){
        if (mp_context->makeCurrent(this))
            break;
        if (i==max_try_count){
            std::cerr<<"ERROR: m_pcontext->makeCurrent(this) FAILED!"<<std::endl;
            exit( 1 );
        }
        qDebug()<<"RETRY: mp_context->makeCurrent(this) after "<<sleep_interval_ms<<" ms";
        QThread::msleep(sleep_interval_ms);
    }
    qDebug()<<"mp_context->makeCurrent(this) SUCCESS";

    // Obtain a functions object and resolve all entry points
    // m_funcs is declared as: QOpenGLFunctions_4_3_Core* m_funcs
    mp_funcs = mp_context->functions(); //versionFunctions();
    if ( !mp_funcs ) {
         qDebug()<<"Could not obtain OpenGL versions object;";
         exit( 1 );
    }
    try{
         mp_funcs->initializeOpenGLFunctions();
    }
    catch(...) {
         qDebug()<<"Could not initializeOpenGLFunctions;";
         exit( 2 );
    }
    mb_is_initited = true;
    qDebug()<<"gx::gui::MainWindow::MainWindow END;";

    // QUERY ALL SUPPORTED GL EXTENTIONS: ЗАВИСИТ КАК ОТ ДРАЙВЕРОВ
    // ТАК И ОТ КОНКРЕТНОГО ВИДЕО-УСТРОЙСТВА ( ЧИПА )
    QList<QByteArray> extensions = mp_context->extensions().toList();
    std::sort( extensions.begin(), extensions.end() );
    qDebug() << "BEGIN: Supported extensions ("<< extensions.count() << "):";
    foreach ( const QByteArray &extension, extensions )
        qDebug() << "  " << extension;

    qDebug() << "  END: Supported extensions ("<< extensions.count() << ")";

    QThread::msleep(100);

    printContextInfos();

    initializeGL();  // ИНИЦИАЛИЗИРУЕМ ВСЕ ПОДКЛЮЧЕННЫЕ ПРИМЕРЫ

    resize(QSize(800,450));

    connect(this, SIGNAL(widthChanged(int)), this, SLOT(resizeGL()));
    connect(this, SIGNAL(heightChanged(int)), this, SLOT(resizeGL()));

    // ПРИВЯЗЫВАЕМ ЧАСТОТУ ПЕРЕРИСОВКИ К ТАЙМЕРУ.
    // (ЧАЩЕ РИСОВАТЬ - ТОЛЬКО БАТАРЕЙКИ САДИТЬ И ВИДЕОЧИП РАСКАЛЯТЬ)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(16);
}

void gx::gui::MainWindow::printContextInfos()
{
    if(!mp_context->isValid())
        qDebug()<< "The OpenGL context is invalid!";

    // Make the context current on this window (WIN32 is a pain, use QThread::msleep)
    int max_try_count = 10;
    int sleep_interval_ms = 10;  // sleep time between trys in ms
    for(int i=0; i <= max_try_count; ++i){
        if (mp_context->makeCurrent(this))
            break;
        if (i==max_try_count){
            qDebug()<<"FAIL:printContextInfos()=>m_pcontext->makeCurrent(this)";
            return;
        }
        qDebug()<<"WAIT:printContextInfos()=>mp_context->makeCurrent(this)"<<sleep_interval_ms;
        QThread::msleep(sleep_interval_ms);
    }

    qDebug() << "Window format version is: "
             << format().majorVersion() << "."
             << format().minorVersion();

    qDebug() << "Context format version is: "
             << mp_context->format().majorVersion() << "."
             << mp_context->format().minorVersion();

    qDebug() << "Format  R: " << mp_context->format().redBufferSize();
    qDebug() << "Format  G: " << mp_context->format().greenBufferSize();
    qDebug() << "Format  B: " << mp_context->format().blueBufferSize();
    qDebug() << "Format  A: " << mp_context->format().alphaBufferSize();
    qDebug() << "Ft  Depth: " << mp_context->format().depthBufferSize();
    qDebug() << "FtStencil: " << mp_context->format().stencilBufferSize();


    infoGL();
}

void gx::gui::MainWindow::initializeGL()
{
    // ТЕПЕРЬ ОКНУ ИЗВЕСТНЫ ПАРАМЕТРЫ GL, И МОЖНО ВЫЗЫВАТЬ
    // ИНСТРУМЕНТЫ РИСОВАНИЯ, КАЖДЫЙ ИЗ КОТОРЫХ МОЖЕТ ИМЕТЬ
    // СОБСТВЕННЫЙ СПОСОБ ПОДГОТОВКИ К ЭКСПЛУАТАЦИИ.
    mb_is_current = mp_context->makeCurrent(this);



    // ЗАПУСКАЕМ ИНИЦИАЛИЗАЦИЮ ПРИМЕРОВ ИСПОЛЬЗОВАНИЯ:
    mp_scene->initialize( mp_context );
    mp_ui->initialize( mp_context );

    glCheckError();
    mb_is_current = false;
}

void gx::gui::MainWindow::paintGL()
{
    if(!isExposed())
    {
        qDebug()<<"gx::gui::MainWindow::isExposed()=>false";
        return;
    }
    mb_is_current = mp_context->makeCurrent(this);
    if(!mb_is_current)
    {
        qDebug()<<"gx::gui::MainWindow mp_context->makeCurrent(this)=>false";
        return;
    }
    if(!mp_context->functions())
    {
        qDebug()<<"gx::gui::MainWindow mp_context->functions()=>NULL";
        return;
    }

    glAssert(glClearColor(0.5, 0.5, 0.5, 1.0)); //(GL_CLEAR_COLOROLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)));
    // ЧИСТИМ ЭКРАН И БУФФЕР ГЛУБИНЫ
    glAssert(glClear((GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)));


 //   glAssert(glActiveTexture(GL_TEXTURE0));
 //   glAssert(glEnable(GL_TEXTURE_2D)); //GL_TEXTURE0

    glCheckError();
    mp_scene->render();
    glCheckError();
    mp_ui->render();
    glCheckError();

    //    mp_ui->render();  // call a render of the "gxGuiTextItem" instance
                      //("gxGuiTextItem" use shader with color texture)

   // glAssert(glBegin(GL_QUADS_EXT));
   // glAssert(glEnd());

    glAssert();
    mp_context->swapBuffers(this);
    glCheckError();
    mb_is_current = false;
}

void gx::gui::MainWindow::resizeGL()
{
    mb_is_current = mp_context->makeCurrent(this);
    mp_scene->resize(width(), height());
    mp_ui->resize(width(), height());
    mb_is_current = false;
}

void gx::gui::MainWindow::updateScene()
{
    mp_scene->update(0.0f);
    mp_ui->update(0.0f);
    paintGL();
}

inline bool show_full_screen(QWindow*w){
    w->showFullScreen();
    return true;
}
inline bool show_normal(QWindow*w){
    w->show();
    return false;
}

void gx::gui::MainWindow::keyPressEvent( QKeyEvent* e )
{
    static bool isFullScreen = bool( QWindow::FullScreen & visibility() );

    switch ( e->key() )
    {
    case Qt::Key_Escape:
        QGuiApplication::quit();
        break;

    case Qt::Key_F11:
        isFullScreen = isFullScreen ? show_normal(this): show_full_screen(this);
        break;

    default:
        QWindow::keyPressEvent( e );
    }
}

void gx::gui::MainWindow::on_file_from_data_path_are_changed(const QString &relative_file_path)
{
    // important question are "This file already used as OpenGL buffer, or not loaded yet?"
    // в общем даже не надо чтобы используемые данные были файлом. Достаточно того что объект
    // является ресурсом на стороне видео. Это не то-же самое что изменения в сцене, в физике
    // в логике загружаемой из файла. Это касается исключительно отношения файла или буффера
    // к содержанию VBO или текстуры, которая потенциально могла быть использована в рисовании
    // сцены при помощи OpenGL.
    qDebug() << "FILE CHANGED " << relative_file_path;
}
