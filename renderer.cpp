#include "renderer.h"
#include "Render/axes.h"
#include "Render/pointcloud.h"
#include "Render/staticmesh.h"

QMatrix4x4 camSide, camFront, camTop, cam3D;



///
/// \brief Constructor.
/// \param parent Padre del widget, el widget se borrará cuando se borre el padre.
///
Renderer::Renderer(QWidget *parent) : QOpenGLWidget(parent), m_axes(nullptr), m_acc_cloud(nullptr), m_mag_cloud(nullptr), m_mesh(nullptr)
{
    // Vista lateral
    camSide.setToIdentity();
    camSide.translate(0.0f, 0.0f, -3.0f);
    camSide.rotate(-90.0f, QVector3D(0.0f, 0.0f, 1.0f));
    camSide.rotate(-90.0f, QVector3D(0.0f, 1.0f, 0.0f));

    // Vista frontal
    camFront.setToIdentity();
    camFront.translate(0.0f, 0.0f, -3.0f);
    camFront.rotate(-90.0f, 1.0f, 0.0f, 0.0f);

    // Vista superior
    camTop.setToIdentity();
    camTop.translate(0.0f, 0.0f, -3.0f);

    // Vista 3D
    cam3D.setToIdentity();
    cam3D.translate(0.0f, 0.0f, -3.0f);
    cam3D.rotate(-45.0f, 1.0f, 0.0f, 0.0f);
    cam3D.rotate(-45.0f, 0.0f, 0.0f, 1.0f);

    /*printf("%f %f %f %f\n", camRight(0,0), camRight(0,1), camRight(0,2), camRight(0,3));
    printf("%f %f %f %f\n", camRight(1,0), camRight(1,1), camRight(1,2), camRight(1,3));
    printf("%f %f %f %f\n", camRight(2,0), camRight(2,1), camRight(2,2), camRight(2,3));
    printf("%f %f %f %f\n", camRight(3,0), camRight(3,1), camRight(3,2), camRight(3,3));*/

    /*// Inicializa el puerto serie
    m_serial.setPort(ports[1]);
    m_serial.setBaudRate(115200);
    if (m_serial.open(QIODevice::ReadWrite)) {
        qDebug() << "Opened serial port";
    }*/
}



///
/// \brief Destructor.
///
Renderer::~Renderer()
{
    delete m_mesh;
    delete m_acc_cloud;
    delete m_mag_cloud;
    delete m_axes;
}



///
/// \brief Renderer::initializeGL
///
void Renderer::initializeGL()
{
    // Initialize OpenGL
    initializeGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Render objects
    m_axes = new Axes();
    m_acc_cloud = new PointCloud();
    m_mag_cloud = new PointCloud();
    m_mesh = new StaticMesh();
    m_mesh->load( QString(":/compassXYZ.mesh") );
}



///
/// \brief Renderer::resizeGL
/// \param w
/// \param h
///
void Renderer::resizeGL(int width, int height)
{
    // Set OpenGL viewport to cover whole widget
    m_width = width;
    m_height = height;
    m_aspect = qreal(width) / qreal(height ? height : 1);
    glViewport(0, 0, width, height);

    // Set perspective projection
    const qreal zNear = 0.1, zFar = 1000.0, fov = 45.0;
    m_perspective.setToIdentity();
    m_perspective.perspective(fov, m_aspect, zNear, zFar);

    // Set orthographic projection
    m_ortho.setToIdentity();
    m_ortho.ortho(-1.5*2.0/3.0*m_aspect, +1.5*2.0/3.0*m_aspect, -1.5, +1.5, -2000.0, +2000.0);
}



///
/// \brief Renderer::paintGL
///
void Renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    switch(m_mode) {
        case Compass: renderMesh(); break;
        case Calibration: renderClouds(); break;
        default: break;
    }
}



///
/// \brief Cambia el modo de funcionamiento del renderizador.
/// \param mode Nuevo modo.
///
void Renderer::setMode(IMUMode mode)
{
    m_mode = mode;
}



///
/// \brief Actualiza la orientación del IMU.
/// \param quat Nueva orientación.
///
void Renderer::setOrientation(QMatrix4x4 ori)
{
    m_orientation = ori;
}



///
/// \brief Actualiza la nube de puntos del acelerómetro.
/// \param cloud
///
void Renderer::setAccCloud(const std::vector<QVector3D>& cloud)
{
    m_acc_cloud->update(cloud);
}



///
/// \brief Actualiza la nube de puntos del magnetómetro.
/// \param cloud
///
void Renderer::setMagCloud(const std::vector<QVector3D>& cloud)
{
    m_mag_cloud->update(cloud);
}



///
/// \brief Renderiza la malla del IMU con la orientación calculada.
///
void Renderer::renderMesh()
{
    //QMatrix4x4 model;
    //model.rotate(m_orientation);

    // Cuadrante superior izquierdo / vista lateral
    glViewport(0, m_height / 2, m_width / 2, m_height / 2);
    m_axes->render(m_perspective * camSide);
    m_mesh->render(m_perspective * camSide, m_orientation);

    // Cuadrante superior derecho / vista frontal
    glViewport(m_width / 2, m_height / 2, m_width / 2, m_height / 2);
    m_axes->render(m_perspective * camFront);
    m_mesh->render(m_perspective * camFront, m_orientation);

    // Cuadrante inferior izquierdo / vista superior
    glViewport(0, 0, m_width / 2, m_height / 2);
    m_axes->render(m_perspective * camTop);
    m_mesh->render(m_perspective * camTop, m_orientation);

    // Cuadrante inferior derecho / vista 3D
    glViewport(m_width / 2, 0, m_width / 2, m_height / 2);
    m_axes->render(m_perspective * cam3D);
    m_mesh->render(m_perspective * cam3D, m_orientation);
}



///
/// \brief Renderiza las nube de puntos.
///
void Renderer::renderClouds()
{
    // Cuadrante superior izquierdo / vista superior
    glViewport(0 * m_width / 3, m_height / 2, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camTop);
    m_mag_cloud->render(m_ortho * camTop);

    // Cuadrante superior central / vista lateral
    glViewport(1 * m_width / 3, m_height / 2, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camSide);
    m_mag_cloud->render(m_ortho * camSide);

    // Cuadrante superior derecho / vista frontal
    glViewport(2 * m_width / 3, m_height / 2, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camFront);
    m_mag_cloud->render(m_ortho * camFront);

    // Cuadrante inferior izquierdo / vista superior
    glViewport(0 * m_width / 3, 0, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camTop);
    m_acc_cloud->render(m_ortho * camTop);

    // Cuadrante inferior central / vista lateral
    glViewport(1 * m_width / 3, 0, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camSide);
    m_acc_cloud->render(m_ortho * camSide);

    // Cuadrante inferior derecho / vista frontal
    glViewport(2 * m_width / 3, 0, m_width / 3, m_height / 2);
    m_axes->render(m_ortho * camFront);
    m_acc_cloud->render(m_ortho * camFront);
}
