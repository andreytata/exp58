#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QWindow>
#include <QScopedPointer>

#include <gx_gui.h>

#include "abstractscene.h"

class QScreen;
class QOpenGLContext;
class QKeyEvent;
class QOpenGLFunctions;

namespace gx{
    namespace gui{
        // GX GLES2.0 GLSL1.0 MAIN WINDOW CLASS
        class MainWindow : public QWindow, public gx::gl
        {
            Q_OBJECT

        public:
            explicit MainWindow(QScreen* screen = 0);

            virtual bool is_created(){return mb_is_initited;}
            virtual bool is_current(){return mb_is_current;}
            virtual std::string gl_version(){return "";}
            virtual std::string gl_shaders_version(){return "";}
            virtual std::list<std::string> gl_extentions(){std::list<std::string> e;return e;}
            virtual bool has_extention(const char*){return false;}
            virtual int get_texture(const char*){return 0;}             //
            virtual int get_geometry(const char*){return 0;}            //
            virtual int get_compiled(const char*){return 0;}            //
            void set_data_path(const QDir& q_dir){mo_data_path=q_dir;}  // external data path
            virtual void on_file_from_data_path_are_changed(const QString& relative_file_path);

        protected:
             void keyPressEvent(QKeyEvent *);

        protected slots:
             void resizeGL();
             void paintGL();
             void updateScene();

        private:
             void initializeGL();
             void printContextInfos();
             QOpenGLContext *mp_context;  // GL-CONTEXT POINTER ( MAKE CURRENT BEFORE CALL GL-FUNCTIONS )
             QOpenGLFunctions *mp_funcs;  // GL-FUNCTIONS POINTER ( DEPEND GL-VERSION AND HARDWARE CONF )
             QDir mo_data_path;
             bool mb_is_initited;
             bool mb_is_current;

             // LESSONS:
             QScopedPointer<AbstractScene> mp_scene; // 1. SIMLEST COLOR TRIANGLE
             QScopedPointer<AbstractScene> mp_ui;    // 2. TEXT ITEM (TEXTURED RECTANGLE)
        };
    }
}
#endif // MAINWINDOW_H
