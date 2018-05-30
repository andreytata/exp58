#include <vector>

#include "basicusagescene.h"
#include "glassert.h"

BasicUsageScene::BasicUsageScene()
    : mShaderProgram(),
      mVertexPositionBuffer(QOpenGLBuffer::VertexBuffer),
      mVertexColorBuffer(QOpenGLBuffer::VertexBuffer)
{
}

void BasicUsageScene::init_use_vertex_array_object()
{
    prepareShaderProgram();
    prepareVertexBuffers();
}

void BasicUsageScene::draw_use_vertex_array_object()
{
    mShaderProgram.bind();  // vertex & pixel shaders
    mVAO.bind();            // mesh+normals+uv

    glAssert( glext->glDrawArrays(GL_TRIANGLES, 0, 3) );

    mShaderProgram.release();
    mVAO.release();
}

void BasicUsageScene::update(float t)
{
    Q_UNUSED(t);
}

void BasicUsageScene::init_use_vertex_attrib_array()
{

/* Ñ ÁËÀÃÎÃÎÂÅÍÈÅÌ ÂÇÈĞÀÅÌ ÊÀÊ İÒÎ ÄÅËÀËÎÑÜ Â ÄĞÅÂÍÎÑÒÈ ( ëåò 8 íàçàä ):
#pragma pack(push)
#pragma pack(1)
struct xy_dist
{
    float x;
    float y;
    float dist;
};
#pragma pack(pop)

std::vector<xy_dist> vertices;
std::vector<char> indices;

glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
glEnableVertexAttribArray(m_posAttr);
glEnableVertexAttribArray(m_distLocation);

int      posAttrSize           = sizeof(float) * 2;
int      distLocationAttrSize  = sizeof(float) * 1;
GLintptr posAttrStart          = reinterpret_cast<GLintptr>(&vertices[0]);
GLintptr distLocationAttrStart = posAttrStart + posAttrSize;
int      stride                = posAttrSize + distLocationAttrSize;

glVertexAttribPointer(m_posAttr,      2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid*>(posAttrStart));
glVertexAttribPointer(m_distLocation, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid*>(distLocationAttrStart));

glBindAttribLocation(m_program.programId(), m_posAttr, "posAttr" );
glBindAttribLocation(m_program.programId(), m_distLocation, "distance" );

glUniform1i(m_patternLocation, -1);
glUniform4fv(m_colorAttr, 1, vColor);
glUniform1f(m_depthAttr, 0.2f);

glDrawElements(GL_TRIANGLES,indices.size(), GL_UNSIGNED_SHORT, &indices[0] );
*/
    // À ÑÅÉ×ÀÑ ÏĞÎÑÒÎ ÃÎÒÎÂÈÌ ÏĞÎÃĞÀÌÌÓ "mShaderProgram" ÊÀÊ Â ÑËÓ×ÀÅ Ñ "VAO"
    // È ÈÑÏÎËÜÇÓÅÌ ÏÀĞÓ ÂÛÇÎÂÎÂ mShaderProgram ÄËß ÊÀÆÄÎÃÎ ÈÇ gxa_ ÀÒÒĞÈÁÓÒÎÂ
    // ÍÎ ÍÅ ÇÄÅÑÜ Â init_ , à â ïàğíîì ê íåìó draw_:
    // 1. mShaderProgram.setAttributeArray(gxa_name_id, gxa_name_data_ptr, SZ)
    // 2. mShaderProgram.enableAttributeArray(gxa_position_id)
    //    È ÂÑ¨( ÕÂÀËÀ ÍÎÂÎÌÓ "QOpenGLShaderProgram" )
    prepareShaderProgram();
}


void BasicUsageScene::draw_use_vertex_attrib_array()
{
    // ÁÈÍÄÈÌ ÑÎÁĞÀÍÍÓŞ È ÑËÈÍÊÎÂÀÍÍÓŞ ÏĞÎÃĞÀÌÌÓ
    if(mShaderProgram.bind())
    {
        if( -1 < gxa_position_id ){  // ÅÑËÈ Â ÏĞÎÃĞÀÌÌÅ ÒÀÊÎÉ ÀÒĞÈÁÓÒ ÅÑÒÜ:
            // ÏÎËÓ×ÀÅÌ ÓÊÀÇÀÒÅËÜ ÍÀ ÄÀÍÍÛÅ È ÂÛÇÛÂÀÅÌ ÄÂÀ ÂÎËØÅÁÍÛÕ ÌÅÒÎÄÀ:
            float* gxa_position_data_ptr = gxa_position_data();
            mShaderProgram.setAttributeArray(gxa_position_id, gxa_position_data_ptr, 3);
            mShaderProgram.enableAttributeArray(gxa_position_id);
        }  // åñëè íàäî, ïàğàíîèäàëüíî ïğîâåğÿåì: else assert(false); glCheckError();

        if( gxa_color_id > -1 ){  // ÀÍÀËÎÃÈ×ÍÎ ÅÑËÈ Â ÏĞÎÃĞÀÌÌÅ ÒÀÊÎÉ ÀÒĞÈÁÓÒ ÅÑÒÜ:
            float* gxa_color_data_ptr = gxa_color_data();
            mShaderProgram.setAttributeArray(gxa_color_id, gxa_color_data_ptr, 3);
            mShaderProgram.enableAttributeArray(gxa_color_id);
        }  // åñëè íàäî, ïàğàíîèäàëüíî ïğîâåğÿåì: else assert(false); glCheckError();

        // ÂÑ¨ ÏĞÈÑÎÅÄÈÍÅÍÎ: - GL! ĞÈÑÓÉ ÒĞÅÓÃÎËÜÍÈÊÈ, ÒĞÈ ÂÅĞØÈÍÛ ÍÀ×ÈÍÀß Ñ ÍÓËÅÂÎÉ
        glAssert( glext->glDrawArrays(GL_TRIANGLES, 0, 3));

        if( gxa_color_id > -1 )     // ÅÑËÈ İÒÎÒ ÀÒĞÈÁÓÒ ÂÊËŞ×ÀËÈ, ÂÛÊËŞ×ÈÒÜ
            mShaderProgram.disableAttributeArray(gxa_color_id);

        if( gxa_position_id > -1 )  // ÅÑËÈ İÒÎÒ ÀÒĞÈÁÓÒ ÂÊËŞ×ÀËÈ, ÂÛÊËŞ×ÈÒÜ
            mShaderProgram.disableAttributeArray(gxa_position_id);

        mShaderProgram.release();   // Î×ÈÙÀÅÌ ÂÑ¨ ×ÒÎ ÏÎÄÊËŞ×ÈËÈ
        // åñëè íàäî, ïàğàíîèäàëüíî ïğîâåğÿåì: glCheckError();
    }
    // åñëè íàäî, ïàğàíîèäàëüíî ïğîâåğÿåì: else assert(false);
}

void BasicUsageScene::resize(int width, int height)
{
    glAssert( glViewport(0, 0, width, height) );
}

void BasicUsageScene::prepareShaderProgram()
{
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/simplest.vsh"))
    {
        qCritical() << "error";
    }
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/simplest.fsh"))
    {
        qCritical() << "error";
    }
    if (!mShaderProgram.link())
    {
        qCritical() << "error";
    }

    gxa_position_id = mShaderProgram.attributeLocation("gxa_position");
    qDebug()<<"gx-BasicUsageScene-program-gxa_position_id"<<gxa_position_id;

    gxa_normal_id = mShaderProgram.attributeLocation("gxa_normal");
    qDebug()<<"gx-BasicUsageScene-program-gxa_normal_id"<<gxa_normal_id;

    gxa_color_id = mShaderProgram.attributeLocation("gxa_color");
    qDebug()<<"gx-BasicUsageScene-program-gxa_colol_id"<<gxa_color_id;

    gxu_mvp_matrix_id = mShaderProgram.uniformLocation("gxu_mvp_matrix");
    qDebug()<<"gx-BasicUsageScene-program-gxu_mvp_matrix_id"<<gxu_mvp_matrix_id;

    glCheckError();
}

float* BasicUsageScene::gxa_position_data(){
    static float positionData[] = {
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f,  0.8f, 0.0f,
    };
    return positionData;
}

float* BasicUsageScene::gxa_color_data(){
    static float colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    return colorData;
}

void BasicUsageScene::prepareVertexBuffers()
{
    mVAO.create();
    mVAO.bind();

    mVertexPositionBuffer.create();
    mVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexPositionBuffer.bind();
    mVertexPositionBuffer.allocate(gxa_position_data(), 3 * 3 * sizeof(float));

    mVertexColorBuffer.create();
    mVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexColorBuffer.bind();
    mVertexColorBuffer.allocate(gxa_color_data(), 3 * 3 * sizeof(float));

    mShaderProgram.bind();

    mVertexPositionBuffer.bind();
    mShaderProgram.enableAttributeArray(gxa_position_id); // replace shader var "vertexPosition"
    mShaderProgram.setAttributeBuffer(gxa_position_id, GL_FLOAT, 0, 3);

    mVertexColorBuffer.bind();
    mShaderProgram.enableAttributeArray(gxa_color_id);    // replace shader var "gxa_vertex_color"
    mShaderProgram.setAttributeBuffer(gxa_color_id, GL_FLOAT, 0, 3);

    // UNBIND ALL:
    mVertexPositionBuffer.release();
    mVertexColorBuffer.release();
    mShaderProgram.release();
    mVAO.release();

    glCheckError();
}
