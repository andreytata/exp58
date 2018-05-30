#ifndef GX_GUI_TEXT_ITEM_H
#define GX_GUI_TEXT_ITEM_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>    // glm::mat4 // #include <glm/glm.hpp>
#include <glm/vec3.hpp>      // glm::vec3 // #include <glm/vec4.hpp>  // glm::vec4
#include <glm/gtc/matrix_transform.hpp>   // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "abstractscene.h"

#include <QVector2D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>

class gxGuiTextItem: public AbstractScene
{
public:
    gxGuiTextItem();
    virtual ~gxGuiTextItem();

    virtual void initialize(QOpenGLContext*);
    virtual void update(float t);
    virtual void render();
    virtual void resize(int width, int height);

private:
    // int gxa_texture_coords_id;    virtual float* gxa_texture_coords_data();

    bool gl_has_vertex_array_object_extention;  // has valid value only after initialize

    QOpenGLShaderProgram mShaderProgram;
    QOpenGLVertexArrayObject mVAO;
    QOpenGLBuffer mVertexPositionBuffer;
    QOpenGLBuffer mVertexTextureCoordinatesBuffer;
    QOpenGLBuffer mVertexColorBuffer;

    void prepareShaderProgram();
    void prepareVertexBuffers();

    QOpenGLTexture* font_textures_array[256];

    // Shader Side Vertex Attributes Locations:
    //
    int    gxa_vertex_position_id;      // vec3 gxa_vertex_position
    float* gxa_vertex_position_data();

    int    gxa_vertex_color_id;         // vec3 gxa_vertex_color
    float* gxa_vertex_color_data();

    int    gxa_vertex_texture_uv_id;    // vec2 gxa_texture_coordinate
    float* gxa_vertex_texture_uv_data();

    // Shader Side Uniform Attributes Locations:
    //
    int gxu_model_view_projection_id;   // mat4 qx_ModelViewProjectionMatrix

    // Some set of variables for tests:
    //
    glm::mat4x4 mProjection;            // glm ortho for tests, resize
    glm::mat4x4 mZeroPosition;          // base glypf's position
    glm::mat4x4 mGlmSampleMatrix;       //
    QMatrix4x4 mSampleMatrix;           //
    QElapsedTimer mSymbolTime;          //
    int           mLastSymbol;          //
};

#endif // GX_GUI_TEXT_ITEM_H
