#pragma once

#include "types.h"



///
/// \brief Clase auxiliar para renderizar los ejes de coordenadas globales.
///
/// Eje X en rojo, eje Y en verde, eje Z en azul, convenio de la mano derecha.
///
class Axes : protected QGLFunctions
{
public:
    Axes();
    ~Axes();
    void render(const QMatrix4x4& pvmMatrix);

private:
    GLuint m_buffer;
    QGLShaderProgram m_shader;
};
