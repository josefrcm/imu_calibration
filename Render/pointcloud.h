#pragma once

#include "types.h"



///
/// \brief The PointCloud class
///
class PointCloud : protected QGLFunctions
{
public:
    PointCloud();
    ~PointCloud();

    void update( const std::vector<QVector3D>& points );
    void render( const QMatrix4x4& pvmMatrix );

private:
    GLuint m_point_count;
    GLuint m_point_buffer;
    QGLShaderProgram m_shader;
};
