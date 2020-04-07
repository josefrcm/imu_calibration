#include "axes.h"

#include <stdint.h>



struct PointType {
    float x, y, z;
    uint8_t r, g, b, a;

    PointType(float _1, float _2, float _3, int _4, int _5, int _6, int _7){
        x = _1;
        y = _2;
        z = _3;
        r = _4;
        g = _5;
        b = _6;
        a = _7;
    }
};

static const PointType points[] = {
    PointType(-10.0f, 0.0f, 0.0f, 255,0,0,255),
    PointType(+10.0f, 0.0f, 0.0f, 255,0,0,255),
    PointType(0.0f, -10.0f, 0.0f, 0,255,0,255),
    PointType(0.0f, +10.0f, 0.0f, 0,255,0,255),
    PointType(0.0f, 0.0f, -10.0f, 0,0,255,255),
    PointType(0.0f, 0.0f, +10.0f, 0,0,255,255)
};

static const char* vertex =
    "#version 330\n"
    "uniform mat4 proj_view_model_matrix;\n"
    "in vec4 a_position;\n"
    "in vec4 a_color;\n"
    "out vec4 v_color;\n"
    "void main() { v_color = a_color; gl_Position = proj_view_model_matrix * a_position; }\n";

static const char* fragment =
    "#version 330\n"
    "in vec4 v_color;\n"
    "void main() { gl_FragColor = v_color; }\n";



///
/// \brief Constructor.
///
Axes::Axes()
{
    initializeGLFunctions();

    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(PointType), points, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m_shader.addShaderFromSourceCode(QGLShader::Vertex, vertex)) throw "wtf";
    if (!m_shader.addShaderFromSourceCode(QGLShader::Fragment, fragment)) throw "wtf";
    if (!m_shader.link()) throw "wtf";
}



///
/// \brief Destructor.
///
Axes::~Axes()
{
    glDeleteBuffers(1, &m_buffer);
}



///
/// \brief Renderiza los ejes de coordenadas.
///
void Axes::render(const QMatrix4x4& pvmMatrix)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);

    m_shader.bind();
    m_shader.setUniformValue("proj_view_model_matrix", pvmMatrix);

    int positionLocation = m_shader.attributeLocation("a_position");
    m_shader.enableAttributeArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(PointType), (const void *)offsetof(PointType, x));

    int colorLocation = m_shader.attributeLocation("a_color");
    m_shader.enableAttributeArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(PointType), (const void *)offsetof(PointType, r));

    glDrawArrays(GL_LINES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
