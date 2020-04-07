#pragma once

#include "types.h"



///
/// \brief Malla estática, sin animaciones de ningún tipo.
///
class StaticMesh : protected QGLFunctions
{
public:
    StaticMesh();
    ~StaticMesh();

    void load( const QString& fileName );
    void update( const std::vector<VertexData>& vertices, const std::vector<TriangleData>& faces );
    void render( const QMatrix4x4& camMatrix, const QMatrix4x4& modelMatrix );

private:
    GLuint m_vertex_count;
    GLuint m_vertex_buffer;
    GLuint m_face_count;
    GLuint m_face_buffer;

    QGLShaderProgram m_shader;
};
