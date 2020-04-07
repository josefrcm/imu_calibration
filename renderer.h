#pragma once

#include <QOpenGLWidget>

#include "Render/types.h"

class Axes;
class PointCloud;
class StaticMesh;



///
/// \brief The Renderer class
///
class Renderer : public QOpenGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    explicit Renderer(QWidget *parent = 0);
    ~Renderer();

public slots:
    void setOrientation(QMatrix4x4 ori);
    void setAccCloud(const std::vector<QVector3D>& cloud);
    void setMagCloud(const std::vector<QVector3D>& cloud);
    void setMode(IMUMode mode);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

private:
    int m_width, m_height;
    float m_aspect;
    IMUMode m_mode;
    QMatrix4x4 m_perspective;
    QMatrix4x4 m_ortho;

    Axes* m_axes;
    PointCloud* m_acc_cloud;
    PointCloud* m_mag_cloud;
    StaticMesh* m_mesh;
    QMatrix4x4 m_orientation;

    void renderMesh();
    void renderClouds();
};
