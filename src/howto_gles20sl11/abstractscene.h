#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>    // glm::mat4 // #include <glm/glm.hpp>
#include <glm/vec3.hpp>      // glm::vec3 // #include <glm/vec4.hpp>  // glm::vec4
#include <glm/gtc/matrix_transform.hpp>   // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <gx_gui.h>

class AbstractScene
{
    // имя сервера эффекта (имя объекта содержащего зависимости)
    // gx_file_depends объект - файловый сервер эффекта
public:
    AbstractScene(): gl_context(NULL), glext(NULL), overwrite("phong") {}
    virtual ~AbstractScene() {}
    virtual void initialize( QOpenGLContext* context )
    {
        // ВЫЗЫВАЕТСЯ ИЗ ВИДЖЕТА ОКНА, И ТОЛЬКО ЕСЛИ ВЫПОЛНЕНЫ УСЛОВИЯ:
        // context->makeCurrent(..) => TRUE (КОНТЕКСТ GL УСПЕШНО ПЕРЕКЛЮЧЕН)
        // context->functions() != 0 (GL-РАСШИРЕНИЯ УСПЕШО ИНИЦИАЛИЗИРОВАНЫ)
        gl_context = context;
        glext = context->functions();
    }
    virtual void update(float t) = 0;
    virtual void render() = 0;
    virtual void resize(int width, int height) = 0;

    void set_uniform_mat4x4(const GLuint id, const glm::mat4x4& src) const
    {
        glext->glUniformMatrix4fv( id, 1, 0, glm::value_ptr(src) );
    }
protected:
    QOpenGLContext*       gl_context;  //
    QOpenGLFunctions*     glext;       //

public:
    // НОВОЕ: СНОСИМ В ЭТУ СЕКЦИЮ ОБЩЕЕ ИЗ уроков "basicusagescene" и "gx_gui_text_item"
    gx::gl_shared_effect overwrite;
};

#endif // ABSTRACTSCENE_H
