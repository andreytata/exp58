#ifndef BASICUSAGESCENE_H
#define BASICUSAGESCENE_H

#include "abstractscene.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

// CLASS BasicUsageScene. Use simplest shader w/o textures:
//  - Vertex shader stored in   ":shaders/simplest.vsh"
//  - Fragment shader stored in ":shaders/simplest.fsh"
class BasicUsageScene : public AbstractScene
{
public:
    BasicUsageScene();
    virtual ~BasicUsageScene(){}

    virtual void initialize(QOpenGLContext* context)
    {
        AbstractScene::initialize(context);  // gl_context, glext

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
            init_use_vertex_array_object();
        }
        else
        {
            init_use_vertex_attrib_array();
        }
    }

    virtual void render()
    {
        if( gl_has_vertex_array_object_extention )
        {
            draw_use_vertex_array_object();
        }
        else
        {
            draw_use_vertex_attrib_array();
        }
    }

    virtual void resize(int width, int height);
    virtual void update(float t);

private:
    void init_use_vertex_array_object();
    void draw_use_vertex_array_object();

    void init_use_vertex_attrib_array();
    void draw_use_vertex_attrib_array();

    int gxa_position_id;         // -1 => is not available in current shader program
    float* gxa_position_data();

    int gxa_normal_id;
    int gxu_mvp_matrix_id;

    // attribute vec3 gxa_color;
    int gxa_color_id;
    float* gxa_color_data();

    bool gl_has_vertex_array_object_extention;  // VALID ONLY AFTER "initialize"

    QOpenGLShaderProgram mShaderProgram;
    QOpenGLVertexArrayObject mVAO; //only if supported in current GL extentions
    QOpenGLBuffer mVertexPositionBuffer;
    QOpenGLBuffer mVertexColorBuffer;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // BASICUSAGESCENE_H
