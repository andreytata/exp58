/** ����� � ���������� ������:
   - �� ���, ������� �� ����� �� ����?
   - � ��, ������ ������� - ������ �������!

   � ������ ���������� ������ ����� ��� ��������:
   - � ���������� �� ������ "������" � ����� ���� �������?
   - � ���� ��, �� ��� � ��� �� �� �������� ����?
   - � ���� ������ ��� �������� "������"?
   - � ���� �����?
    ...
   ���� �� ��������� ���� ������������ ��� ��������
   � ���� ������, ����� � ����� ����� �������� ��� !
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

    /** �������������� ������������� ������:
       ������� ��� ���������� ������� � ������, ������ � ���,
       ������� �� �� "QT" �� ������� "GL"? ���� ���, �� �����
       �������� ������ "����������� ��������", �������� ���
       �� ������� QOpenGLTexture::textureId() �������� OpenGL-
       ���� ���������� ����� ������� � �� ������� � ��������
       ���������. ��� ������ ����� ������� ���� ����� on-paint.
    */
    for(int i=0; i<256; ++i){
        delete font_textures_array[i];
    }
}

void gxGuiTextItem::initialize(QOpenGLContext* context)
{
    AbstractScene::initialize(context);  // gl_context, glext

    // ��� � � "basicusagescene" ������ ���������� �� "VAO",
    // � ������ � ����������� �� ���������� �� �������, ����
    // � �� �������� � ����������� � ����������:

    // ���������� ������� �� � ������� ��������� ����������� ���������� "VAO":
    // "GL_ARB_vertex_array_object"  // ��� ��� ����� ���������� �� ATI � NVIDIA
    // "GL_OES_vertex_array_object"  // � ��� �� NVIDIA, ATI � ANDROID MALI ...
    // ... ������� - ���� � ������, � ���������� � � ������ �������������.
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


    ///    �������������� ������������� ������:
    // ��������� ������ ��������������� �������, � ������� �������
    // �������� 0..255 ����� ��������� � ������������ ����� OPENGL
    // �������� �� ������� �����. ���������� QOpenGLTexture ������
    // ������������� �������. ���������� ��� ����� "QGLWidget", ��
    // ������� ����� ��, ��� ����������� � ������� QT-5.6 ��������
    // ����������� ��� "QGLWidget" ������ �� ����� ��������������,
    // � ����� QOpenGLTexture �������� ��� GL-��������� �� ������.
    for(int i=0; i<256; ++i)
    {
        // ������� ����������� �� ������ (const char* const[] XPM)
        // XPM-(��. ��������� ������� UNIX ����������� ������)
        QImage image( gx_code_font_32(i) );  // ������ ����� �� XPM
                                             // ��. "gx_font_32.cpp"
        image = image.convertToFormat(QImage::Format_RGBA8888);
        // ������ �������� �� ������, ��� ��������� MipMaps
        QOpenGLTexture* curr_gl_texture =
                new QOpenGLTexture( image.mirrored(),
                                    QOpenGLTexture::DontGenerateMipMaps);

        // ��������� �������� � ��������������, � ������������ � ASCII.
        font_textures_array[i] = curr_gl_texture;

        ///TODO: ���� ����������� ��������� ������:
        // WARNING! ��� ������ �� ����� QOpenGLTexture::create ��������
        // ����� �������� �� ������� GL. (���� ��� ���������, �� id ���
        // �� �������� ������� ��������, ��� � ���������. � ������������
        // �������, ��� ���� �������� ��� �� ������� "GL", �� ����������
        // QOpenGLTexture � ��������� ������ ������. ������ �������� ��
        // � "id" � ����������� � �� �������� ����� "glTexParameteri")
        // ( ��� �������: destroy() => ��������� ��������� => create() )
        GLuint id = curr_gl_texture->textureId();
        if( id )
        {
            qDebug() << "success loaded font item" << i << "gluid ="<<id;
            GLuint id = curr_gl_texture->textureId();

            // ��� ����, ���� ����������� �������� �������� �� ����� �����������
            // � �� ����� ��������� ������������ �� ���� ��������� � "GLES2SL1":
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
        /// ������ ���������, ��������� � ������ ��� ������� ������.
        /// ����� ���������� ���������� � ������ UNIFORM-��������.

        /// ������ ������ �������� � ������ �������� ���� glm::mat4x4 :
        ///   - ������� QMatrix4x4 mmm �� glm::mat4x4
        ///   - ���������� mShaderProgram.setUniformValue("���_uniform_��������", mmm);
        ///  ��� ����� ��������� ������, ��������� ������ �������:
        ///  1. ����������� QMatrix4x4 �������� 16 ����� �� glm::mat4x4.
        ///  2. ������ ����� ���������� ����� "id" uniform_�������� �� ��� �����.
        /// ------------------------------------>
        /// QMatrix4x4 m( glm::value_ptr(mGlmSampleMatrix) );
        /// mShaderProgram.setUniformValue("gxu_model_view_projection", m);

        /// ������ ������ �������� � ������ �������� ���� glm::mat4x4 :
        ///   - ������� ��������� id "���_uniform_��������"
        ///   - ������� QMatrix4x4 temp �� glm::mat4x4
        ///   - ���������� mShaderProgram.setUniformValue(int ID, temp );
        ///  ��� �������, �� ������ ��������:
        ///  1. ����������� QMatrix4x4 �������� 16 ����� �� glm::mat4x4.
        /// ------------------------------------>
        /// QMatrix4x4 m( glm::value_ptr(mZeroPosition) );
        /// mShaderProgram.setUniformValue(gxu_model_view_projection_id, m);

        /// ������ ������ �������� � ������ �������� ���� glm::mat4x4 :
        ///   - ������� ��������� id "���_uniform_��������"
        ///   - ������� �������������� "�����������" ������� GLES2
        ///   - �������� GLfloat* mptr �� glm::mat4x4 ����� glm::value_ptr
        ///   - �������� QOpenGLFunctions* ext ����� this->glext()
        ///   - �������� �������� ������� glUniformMatrix4fv(id, 1, 0, mptr)
        /// ��������� ������ ���:
        /// ------------------------------------>
        /// const GLfloat* mptr = glm::value_ptr(mGlmSampleMatrix);
        /// QOpenGLFunctions* ext = glext();
        /// ext->glUniformMatrix4fv(gxu_model_view_projection_id,1,0,mptr);

        /// ��������� ������������ ������� �� ��� ����� ����� ������� ������ �
        /// �������� ��� � DEFINE ��� � ���������� inline �������. ��� �� ���:
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
            mVAO.bind(); // ������ �������� � ��������, �������� ������ ���������
        }
        else
        {   // ������ �������� � ��������, ��������� ��� "VAO"
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

        // ������ ������������, ������� � ������� �������, ����� ������
        glAssert( glDrawArrays(GL_TRIANGLES, 0, 6) );

        const int doit = 1; // ����� �������� MVP, � ������� ��� �� ����� �٨ ���
        if( 1 == doit ){
            glm::mat4x4 m2;
            m2 = glm::translate(mGlmSampleMatrix, glm::vec3(0.5,0.1,0.1));
            m2 = glm::rotate(m2, glm::degrees(90.f), glm::vec3(0,0,1));
            set_uniform_mat4x4(gxu_model_view_projection_id, m2);
            // ����� ������������� ��������
            glAssert((font_textures_array[mLastSymbol+1])->bind());
            glAssert( glDrawArrays(GL_TRIANGLES, 0, 6) );
        }
        // ��� �������� ����� ������������ ��� ������ ������ ������ � 3D
        // ��������� "old school style text console", ������� � ������ ������
        // ������������� � �������� ��������� ����� UNIX-���������� � MS-DOS.
        //( ���� ��������� �������� ���� � ���� ����� ���������� ������� )

        // �������� �Ѩ ��� ������ ���������� ��������.
        if(gl_has_vertex_array_object_extention)
        {
            mVAO.release();        // �������� ������ ������
        }
        else
        {
            if( gxa_vertex_texture_uv_id > -1)
                mShaderProgram.disableAttributeArray(gxa_vertex_texture_uv_id);

            if( gxa_vertex_color_id > -1 )     // ���� ���� ������� ��������, ���������
                mShaderProgram.disableAttributeArray(gxa_vertex_color_id);

            if( gxa_vertex_position_id > -1 )  // ���� ���� ������� ��������, ���������
                mShaderProgram.disableAttributeArray(gxa_vertex_position_id);
        }
        mShaderProgram.release();  // �������� ��������� �������
    }
    glDisable(GL_BLEND);           // �������� ����� ������������

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
