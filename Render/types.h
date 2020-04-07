#pragma once

#include <cstdint>
#include <tuple>

//#include <QFile>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
//#include <QTextStream>

#include <QGLFunctions>
#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>



///
/// \brief Color RGBA de 8 bits por canal.
///
struct RGBA
{
    uint8_t m_red, m_green, m_blue, m_alpha;

    RGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
        m_red(red),
        m_green(green),
        m_blue(blue),
        m_alpha(alpha) {}
};



///
/// \brief Tipo propio de vértices.
///
struct VertexData
{
    QVector3D m_position;
    QVector3D m_normal;
    RGBA m_color;

    VertexData(QVector3D position, QVector3D normal, RGBA color) :
        m_position(position),
        m_normal(normal),
        m_color(color) {}
};



///
/// \brief Índices que forman un triángulo.
///
struct TriangleData
{
    GLushort m_v1, m_v2, m_v3;

    TriangleData(GLushort v1, GLushort v2, GLushort v3) :
        m_v1(v1),
        m_v2(v2),
        m_v3(v3) {}
};



///
/// \brief Modo de funcionamiento de la aplicación.
///
enum IMUMode { Disconnected, Waiting, Compass, Calibration };



QMatrix4x4 FitAlignedEllipsoid(const std::vector<QVector3D>& data);
QMatrix4x4 FitOrientedEllipsoid(const std::vector<QVector3D>& data);
std::tuple<QString, QList<float>> ParseLine(const QString& line);
