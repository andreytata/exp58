/** Слышу в транспорте диалог:
   - Ну как, получил от мамки по жопе?
   - А то, мамарх сказало - мамарх сделало!

   В голове немедленно возник целый рой вопросов:
   - А существует ли термин "папарх" в языке этих дебилов?
   - И если да, то оно у них то же среднего рода?
   - А кота своего они величают "котарх"?
   - А если кошка?
    ...
   Даже не пытайтесь себе представлять что творится
   с моим мозгом, когда я читаю чужой исходный код !
                                                 (^_^)
 */

#include "glassert.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <glm/gtc/type_ptr.hpp>

#include "gx_gui_text_item.h"
#include "gxcodefont32.h"

gxGuiTextItem::gxGuiTextItem()
    : mShaderProgram()
    , mVertexPositionBuffer           (QOpenGLBuffer::VertexBuffer)
    , mVertexTextureCoordinatesBuffer (QOpenGLBuffer::VertexBuffer)
    , mVertexColorBuffer              (QOpenGLBuffer::VertexBuffer)
    , gxu_model_view_projection_id(0)
{
    mSymbolTime.start();      //
    mLastSymbol = 'e';        //

    qDebug()<<"++<gxGuiTextItem>++ at "<<(void*)this;
}

gxGuiTextItem::~gxGuiTextItem(){
    qDebug()<<"--<gxGuiTextItem>-- at "<<(void*)this;

    /** ЗНАКОГЕНЕРАТОР МОНОШИРИННОГО ШРИФТА:
       УДАЛЯЕМ ВСЕ ТЕКСТУРНЫЕ ОБЪЕКТЫ В ПАМЯТИ, ВОПРОС В ТОМ,
       УДАЛЯЕТ ЛИ ИХ "QT" НА СТОРОНЕ "GL"? Если нет, то нужно
       добавить объект "отложенного удаления", передать все
       не нулевые QOpenGLTexture::textureId() главному OpenGL-
       окну приложения через событие с их списком в качестве
       аргумента. Тот должен будет удалить этот мусор on-paint.
    */
    for(int i=0; i<256; ++i){
        delete font_textures_array[i];
    }
}

void gxGuiTextItem::initialize(QOpenGLContext* context)
{
    AbstractScene::initialize(context);  // gl_context, glext

    // КАК И В "basicusagescene" УЗНАЁМ ПОДДЕРЖДАН ЛИ "VAO",
    // и рисуем в зависимости от результата по разному, ХОТЯ
    // И НЕ ВДАВАЯСЬ В ПОДРОБНОСТИ И КОМПАКТНЕЕ:

    // ОПРЕДЕЛЯЕМ ИМЕЕТСЯ ЛИ В НАЛИЧИИ ПОДДЕРЖКА КОНКРЕТНОГО РАСШИРЕНИЯ "VAO":
    // "GL_ARB_vertex_array_object"  // ТАК ОНА МОЖЕТ НАЗЫВАТЬСЯ НА ATI И NVIDIA
    // "GL_OES_vertex_array_object"  // А ТАК НА NVIDIA, ATI И ANDROID MALI ...
    // ... ПРИЧИНА - ХАОС В ЖЕЛЕЗЕ, В СТАНДАРТАХ И В МОЗГАХ РАЗРАБОТЧИКОВ.
    bool exists = false;
    if( context->hasExtension("GL_ARB_vertex_array_object") ) exists = true;
    if( context->hasExtension("GL_OES_vertex_array_object") ) exists = true;

    qDebug()<<"USING_VAO:"<<( exists ? "[ENABLED]" : "[DISABLED]" );

    gl_has_vertex_array_object_extention = exists;

    if(gl_has_vertex_array_object_extention)
    {
        prepareShaderProgram();
        prepareVertexBuffers();
    }
    else
    {
        prepareShaderProgram();
    }


    ///    ЗНАКОГЕНЕРАТОР МОНОШИРИННОГО ШРИФТА:
    // ЗАПОЛНЯЕМ МАССИВ ИДЕНТИФИКАТОРОВ ТЕКСТУР, В КОТОРОМ КАЖДОМУ
    // СМЕЩЕНИЮ 0..255 будет поставлен в соответствие адрес OPENGL
    // ТЕКСТУРЫ НА СТОРОНЕ ВИДЕО. ИСПОЛЬЗУЕМ QOpenGLTexture СПОСОБ
    // ИНИЦИАЛИЗАЦИИ ТЕКСТУР. КОМПАКТНЕЕ ЧЕМ ЧЕРЕЗ "QGLWidget", НО
    // ГЛАВНОЕ ЗДЕСЬ ТО, ЧТО КОММЕНТАРИИ И ПРИМЕРЫ QT-5.6 СОДЕРЖАТ
    // УТВЕРЖДЕНИЯ ЧТО "QGLWidget" БОЛЬШЕ НЕ БУДЕТ ПОДДЕРДИВАТЬСЯ,
    // А НОВЫЙ QOpenGLTexture УПРОСТИТ КОД GL-РИСОВАНИЯ НА ТЕКСТУ.
    for(int i=0; i<256; ++i)
    {
        // СОЗДАЁМ ИЗОБРАЖЕНИЕ ИЗ СТРОКИ (const char* const[] XPM)
        // XPM-(см. википедия древний UNIX графический формат)
        QImage image( gx_code_font_32(i) );  // Создаём имидж из XPM
                                             // см. "gx_font_32.cpp"
        image = image.convertToFormat(QImage::Format_RGBA8888);
        // Создаём текстуру из имиджа, без генерации MipMaps
        QOpenGLTexture* curr_gl_texture =
                new QOpenGLTexture( image.mirrored(),
                                    QOpenGLTexture::DontGenerateMipMaps);

        // Сохраняем текстуру в знакогенератор, в соответствии с ASCII.
        font_textures_array[i] = curr_gl_texture;

        ///TODO: НАДО ИССЛЕДОВАТЬ СЛЕДУЮЩИЙ ВОПРОС:
        // WARNING! Без намёков на вызов QOpenGLTexture::create текстура
        // подло создаётся на стороне GL. (Если это произошло, то id уже
        // не содержит нулевое значение, что и проверяем. В документации
        // сказано, что если текстура уже на стороне "GL", то средствами
        // QOpenGLTexture её параметры менять поздно. Биндим текстуру по
        // её "id" и настраеваем её по старинке через "glTexParameteri")
        // ( Как вариант: destroy() => настроить параметры => create() )
        GLuint id = curr_gl_texture->textureId();
        if( id )
        {
            qDebug() << "success loaded font item" << i << "gluid ="<<id;
            GLuint id = curr_gl_texture->textureId();

            // БЕЗ ВСЕХ, НИЖЕ ПРИВЕДЕННЫХ НАСТРОЕК ТЕКСТУРА НЕ БУДЕТ ЗАВЕРШЕННОЙ
            // И НЕ БУДЕТ КОРРЕКТНО ОТОБРАЖАТЬСЯ НА РЯДЕ УСТРОЙСТВ С "GLES2SL1":
            glBindTexture(GL_TEXTURE_2D, id );
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        }
    }
    glCheckError();
}

void gxGuiTextItem::update(float t)
{
    Q_UNUSED(t);
}

void gxGuiTextItem::render()
{
    // qDebug()<<qPrintable("gx-gui-layer-renderer-begin");

    glext->glEnable(GL_BLEND);
    glext->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if(mShaderProgram.bind())
    {
        /// ШЕЙДЕР СКОМПИЛЕН, СЛИНКОВАН И ВЫБРАН КАК ТЕКУЩИЙ ШЕЙДЕР.
        /// ЗДЕСЬ ПОЛАГАЕТСЯ ПЕРЕДАВАТЬ В ШЕЙДЕР UNIFORM-ЗНАЧЕНИЯ.

        /// ПЕРВЫЙ СПОСОБ ПЕРЕДАТЬ В ШЕЙДЕР ПАРАМЕТР ТИПА glm::mat4x4 :
        ///   - СОЗДАЁМ QMatrix4x4 mmm из glm::mat4x4
        ///   - используем mShaderProgram.setUniformValue("имя_uniform_значения", mmm);
        ///  ЭТО САМЫЙ МЕДЛЕННЫЙ СПОСОБ, ОЧЕВИДНЫЕ ПОТЕРИ ВРЕМЕНИ:
        ///  1. Конструктор QMatrix4x4 КОПИРУЕТ 16 ЧИСЕЛ ИЗ glm::mat4x4.
        ///  2. КАЖДЫЙ ВЫЗОВ ПРОИСХОДИТ ПОИСК "id" uniform_значения ПО ЕГО ИМЕНИ.
        /// ------------------------------------>
        /// QMatrix4x4 m( glm::value_ptr(mGlmSampleMatrix) );
        /// mShaderProgram.setUniformValue("gxu_model_view_projection", m);

        /// ВТОРОЙ СПОСОБ ПЕРЕДАТЬ В ШЕЙДЕР ПАРАМЕТР ТИПА glm::mat4x4 :
        ///   - ЗАРАНЕЕ СОХРАНЯЕМ id "имя_uniform_значения"
        ///   - СОЗДАЁМ QMatrix4x4 temp из glm::mat4x4
        ///   - используем mShaderProgram.setUniformValue(int ID, temp );
        ///  ЭТО БЫСТРЕЕ, НО ПОТЕРИ ОСТАЛИСЬ:
        ///  1. Конструктор QMatrix4x4 КОПИРУЕТ 16 ЧИСЕЛ ИЗ glm::mat4x4.
        /// ------------------------------------>
        /// QMatrix4x4 m( glm::value_ptr(mZeroPosition) );
        /// mShaderProgram.setUniformValue(gxu_model_view_projection_id, m);

        /// ТРЕТИЙ СПОСОБ ПЕРЕДАТЬ В ШЕЙДЕР ПАРАМЕТР ТИПА glm::mat4x4 :
        ///   - ЗАРАНЕЕ СОХРАНЯЕМ id "имя_uniform_значения"
        ///   - ЗАРАНЕЕ ИНИЦИАЛИЗИРУЕМ "РАСШИРЕННЫЕ" ФУНКЦИИ GLES2
        ///   - ПОЛУЧАЕМ GLfloat* mptr из glm::mat4x4 через glm::value_ptr
        ///   - ПОЛУЧАЕМ QOpenGLFunctions* ext через this->glext()
        ///   - НАПРЯМУЮ ВЫЗЫВАЕМ ФУНКЦИЮ glUniformMatrix4fv(id, 1, 0, mptr)
        /// ОЧЕВИДНЫХ ПОТЕРЬ НЕТ:
        /// ------------------------------------>
        /// const GLfloat* mptr = glm::value_ptr(mGlmSampleMatrix);
        /// QOpenGLFunctions* ext = glext();
        /// ext->glUniformMatrix4fv(gxu_model_view_projection_id,1,0,mptr);

        /// ИНСТИНКТЫ ПРОГРАММИСТА ТРЕБУЮТ ОТ НАС ВЗЯТЬ САМЫЙ БЫСТРЫЙ СПОСОБ И
        /// СВЕРНУТЬ ЕГО В DEFINE ИЛИ В КОМПАКТНУЮ inline ФУНКЦИЮ. КАК ТО ТАК:
        set_uniform_mat4x4(gxu_model_view_projection_id, mGlmSampleMatrix);

        mGlmSampleMatrix = glm::rotate(mGlmSampleMatrix, glm::degrees(0.0002f), glm::vec3(0,0,1));
        mGlmSampleMatrix = glm::rotate(mGlmSampleMatrix, glm::degrees(0.0004f), glm::vec3(0,1,0));

        if( mSymbolTime.elapsed() > 500 )
        {
            mSymbolTime.restart();
            mLastSymbol = 0 + rand() % 127;
        }
        glAssert((font_textures_array[mLastSymbol])->bind());

        if(gl_has_vertex_array_object_extention)
        {
            mVAO.bind(); // ШЕЙДЕР НАЗНАЧЕН И НАСТРОЕН, ВЫБИРАЕМ ОБЪЕКТ ГЕОМЕТРИИ
        }
        else
        {   // ШЕЙДЕР НАЗНАЧЕН И НАСТРОЕН, ОБХОДИМСЯ БЕЗ "VAO"
            if( -1 < gxa_vertex_position_id ){
                float* vertex_position_data = gxa_vertex_position_data();
                mShaderProgram.setAttributeArray(gxa_vertex_position_id, vertex_position_data, 3);
                mShaderProgram.enableAttributeArray(gxa_vertex_position_id);
            }  // else assert(false); glCheckError();

            if( -1 < gxa_vertex_color_id ){
                float* vertex_color_data = gxa_vertex_color_data();
                mShaderProgram.setAttributeArray(gxa_vertex_color_id, vertex_color_data, 3);
                mShaderProgram.enableAttributeArray(gxa_vertex_color_id);
            }  // else assert(false); glCheckError();

            if( -1 < gxa_vertex_texture_uv_id ){
                float* vertex_texture_data = gxa_vertex_texture_uv_data();
                mShaderProgram.setAttributeArray(gxa_vertex_texture_uv_id, vertex_texture_data, 2);
                mShaderProgram.enableAttributeArray(gxa_vertex_texture_uv_id);
            }  // else assert(false); glCheckError();
        }

        // РИСУЕМ ТРЕУГОЛЬНИКИ, НАЧИНАЯ С НУЛЕВОЙ ВЕРШИНЫ, ШЕСТЬ ВЕРШИН
        glAssert( glDrawArrays(GL_TRIANGLES, 0, 6) );

        const int doit = 1; // МОЖЕМ СМЕСТИТЬ MVP, И ВЫВЕСТИ ТОТ ЖЕ БУФЕР ЕЩЁ РАЗ
        if( 1 == doit ){
            glm::mat4x4 m2;
            m2 = glm::translate(mGlmSampleMatrix, glm::vec3(0.5,0.1,0.1));
            m2 = glm::rotate(m2, glm::degrees(90.f), glm::vec3(0,0,1));
            set_uniform_mat4x4(gxu_model_view_projection_id, m2);
            // МОЖЕМ ПЕРЕНАЗНАЧИТЬ ТЕКСТУРУ
            glAssert((font_textures_array[mLastSymbol+1])->bind());
            glAssert( glDrawArrays(GL_TRIANGLES, 0, 6) );
        }
        // ЭТО СВОЙСТВО МОЖНО ИСПОЛЬЗОВАТЬ ДЛЯ ВЫВОДА ЛЮБОГО ТЕКСТА В 3D
        // ИММИТИРУЯ "old school style text console", который а памяти юзеров
        // ассоциируется с древними хацкерами времён UNIX-терминалов и MS-DOS.
        //( Этот стереотип насмерть вбит в мозг юзера множеством фильмов )

        // ОТМЕНИТЬ ВСЁ ЧТО ДАННЫЙ ИНСТРУМЕНТ НАЗНАЧАЛ.
        if(gl_has_vertex_array_object_extention)
        {
            mVAO.release();        // ОТМЕНЯЕМ МАССИВ ВЕРШИН
        }
        else
        {
            if( gxa_vertex_texture_uv_id > -1)
                mShaderProgram.disableAttributeArray(gxa_vertex_texture_uv_id);

            if( gxa_vertex_color_id > -1 )     // ЕСЛИ ЭТОТ АТРИБУТ ВКЛЮЧАЛИ, ВЫКЛЮЧИТЬ
                mShaderProgram.disableAttributeArray(gxa_vertex_color_id);

            if( gxa_vertex_position_id > -1 )  // ЕСЛИ ЭТОТ АТРИБУТ ВКЛЮЧАЛИ, ВЫКЛЮЧИТЬ
                mShaderProgram.disableAttributeArray(gxa_vertex_position_id);
        }
        mShaderProgram.release();  // ОТМЕНЯЕМ ПРОГРАММУ ШЕЙДЕРА
    }
    glDisable(GL_BLEND);           // ОТМЕНЯЕМ РЕЖИМ ПРОЗРАЧНОСТИ

   // qDebug() << qPrintable("gx-gui-layer-renderer-end!");
}

void gxGuiTextItem::resize(int width, int height)
{
    glAssert( glViewport(0, 0, width, height) );

    mProjection = glm::ortho( 0.0f, (float)width, (float)height, 0.0f ); //, 0.0f, 100.0f);
    // mZeroPosition = in identity by default;
}

void gxGuiTextItem::prepareShaderProgram()
{
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/glyphs.vsh"))
    {
        qCritical() << "error";
    }
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/glyphs.fsh"))
    {
        qCritical() << "error";
    }
    if (!mShaderProgram.link())
    {
        qCritical() << "error";
    }

    gxa_vertex_position_id =
            mShaderProgram.attributeLocation("gxa_vertex_position");
    gxa_vertex_color_id =
            mShaderProgram.attributeLocation("gxa_vertex_color");
    gxa_vertex_texture_uv_id =
            mShaderProgram.attributeLocation("gxa_vertex_texture_uv");

    gxu_model_view_projection_id =
            mShaderProgram.uniformLocation("gxu_model_view_projection");

    glCheckError();
}

float* gxGuiTextItem::gxa_vertex_position_data()
{
    const float dy = .6f;
    const float dx = .3f;
    static float static_position_data[] = {
       -dx,  dy, 0.0f,
        dx,  dy, 0.0f,
        dx, -dy, 0.0f,

       -dx,  dy, 0.0f,
        dx, -dy, 0.0f,
       -dx, -dy, 0.0f
    };
    return static_position_data;
}

float* gxGuiTextItem::gxa_vertex_color_data()
{
   static float static_color_data[] = {
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       0.0f, 0.5f, 1.0f,

       1.0f, 1.0f, 0.0f,
       0.0f, 0.5f, 1.0f,
       0.0f, 0.5f, 1.0f
   };
   return static_color_data;
}

float* gxGuiTextItem::gxa_vertex_texture_uv_data()
{
    static float uvData[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        1.0f,  0.0f,

        0.0f,  1.0f,
        1.0f,  0.0f,
        0.0f,  0.0f,
    };
    return uvData;
}

void gxGuiTextItem::prepareVertexBuffers()
{
    /* For the desktop, VAOs are supported as a core feature in "OpenGL 3.0"
     * or newer and by the "GL_ARB_vertex_array_object" for older versions.
     * On "OpenGL ES 2", VAOs are provided by the optional
     * "GL_OES_vertex_array_object extension". You can check the version of
     * "OpenGL" with "QOpenGLContext::surfaceFormat()" and check for the
     * presence of extensions with "QOpenGLContext::hasExtension()".
    */

    mVAO.create();
    mVAO.bind();

    mVertexPositionBuffer.create();
    mVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexPositionBuffer.bind();
    float* vertex_position_data = gxa_vertex_position_data();
    mVertexPositionBuffer.allocate(vertex_position_data, 2 * 3 * 3 * sizeof(float));

    mVertexTextureCoordinatesBuffer.create();
    mVertexTextureCoordinatesBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexTextureCoordinatesBuffer.bind();
    float* txuv = gxa_vertex_texture_uv_data();
    mVertexTextureCoordinatesBuffer.allocate(txuv, 2 * 3 * 2 * sizeof(float));

    mVertexColorBuffer.create();
    mVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexColorBuffer.bind();
    float* color_data = this->gxa_vertex_color_data();
    mVertexColorBuffer.allocate(color_data, 2 * 3 * 3 * sizeof(float));

    mShaderProgram.bind();

    mVertexPositionBuffer.bind();
    mShaderProgram.enableAttributeArray("gxa_vertex_position");
    mShaderProgram.setAttributeBuffer  ("gxa_vertex_position", GL_FLOAT, 0, 3);

    mVertexTextureCoordinatesBuffer.bind();
    mShaderProgram.enableAttributeArray("gxa_vertex_texture_uv");
    mShaderProgram.setAttributeBuffer  ("gxa_vertex_texture_uv", GL_FLOAT, 0, 2);

    mVertexColorBuffer.bind();
    mShaderProgram.enableAttributeArray("gxa_vertex_color");
    mShaderProgram.setAttributeBuffer  ("gxa_vertex_color", GL_FLOAT, 0, 3);

    // UNBIND ALL:
    mVertexPositionBuffer.release();
    mVertexTextureCoordinatesBuffer.release();
    mVertexColorBuffer.release();
    mShaderProgram.release();
    mVAO.release();

    glCheckError();
}
