#include "pointcloud.h"



static const char* vertex =
    "#version 330\n"
    "uniform mat4 proj_view_model_matrix;\n"
    "in vec4 v_position;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "f_color.rgb = 0.5*normalize(v_position.xyz) + 0.5;\n"
    "f_color.a = 1.0;\n"
    "gl_Position = proj_view_model_matrix * v_position;\n"
    "}\n";

static const char* fragment =
    "#version 330\n"
    "in vec4 f_color;\n"
    "void main() { gl_FragColor = f_color; }\n";



///
/// \brief PointCloud::PointCloud
///
PointCloud::PointCloud()
{
    initializeGLFunctions();
    glGenBuffers(1, &m_point_buffer);
    m_point_count = 0;

    if (!m_shader.addShaderFromSourceCode(QGLShader::Vertex, vertex)) throw "wtf";
    if (!m_shader.addShaderFromSourceCode(QGLShader::Fragment, fragment)) throw "wtf";
    if (!m_shader.link()) throw "wtf";
}



///
/// \brief PointCloud::~PointCloud
///
PointCloud::~PointCloud()
{
    glDeleteBuffers(1, &m_point_buffer);
}



///
/// \brief PointCloud::update
/// \param points
///
void PointCloud::update( const std::vector<QVector3D> &points )
{
    glBindBuffer(GL_ARRAY_BUFFER, m_point_buffer);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(QVector3D), points.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_point_count = points.size();
}



///
/// \brief PointCloud::render
/// \param shader
///
void PointCloud::render( const QMatrix4x4& pvmMatrix )
{
    m_shader.bind();
    m_shader.setUniformValue("proj_view_model_matrix", pvmMatrix);

    glBindBuffer(GL_ARRAY_BUFFER, m_point_buffer);
    int positionLocation = m_shader.attributeLocation("v_position");
    m_shader.enableAttributeArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
    glDrawArrays(GL_POINTS, 0, m_point_count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
