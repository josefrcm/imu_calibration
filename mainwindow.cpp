#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "renderer.h"



///
/// \brief Constructor.
/// \param parent Objeto padre.
///
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_thread(nullptr)
{
    // Inicializa la interfaz gráfica
    ui->setupUi(this);

    // Inicializa la barra de menú
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::actionConnect);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::actionDisconnect);
    connect(ui->actionCompass, &QAction::triggered, this, &MainWindow::actionCompass);
    connect(ui->actionCalibration, &QAction::triggered, this, &MainWindow::actionCalibration);
    connect(ui->actionDone, &QAction::triggered, this, &MainWindow::actionDone);
    connect(ui->actionCancel, &QAction::triggered, this, &MainWindow::actionCancel);

    // Añade la lista de puertos series
    ui->mainToolBar->insertWidget(ui->actionConnect, &m_serialPortList);
    m_serialPortInfos = QSerialPortInfo::availablePorts();
    for( const auto& info : m_serialPortInfos ) {
        const QString name = info.portName() + " - " + info.description() + " - " + info.manufacturer();
        m_serialPortList.addItem(name);
    }

    // Inicializa la barra de estado
    ui->statusBar->addWidget(&m_status);
    m_status.setFont(QFont("Courier", 10));
    setMode(Disconnected);
    m_timer.start(1000/60, this);
}



///
/// \brief Destructor.
///
MainWindow::~MainWindow()
{
    delete m_thread;
    delete ui;
}



///
/// \brief Abre la conexión con el IMU.
///
void MainWindow::actionConnect()
{
    const int index = m_serialPortList.currentIndex();
    if (index >= 0) {
        m_thread = new SerialThread(m_serialPortInfos[index], this);
        m_thread->start();
        connect(m_thread, &SerialThread::readOrientation, this, &MainWindow::readOrientation);
        connect(m_thread, &SerialThread::readForce, this, &MainWindow::readForce);
        connect(m_thread, &SerialThread::readRawSensors, this, &MainWindow::readRawSensors);
        connect(m_thread, &SerialThread::readRawAnalog, this, &MainWindow::readRawAnalog);
        setMode(Compass);
    }
}



///
/// \brief Cierra la conexión con el IMU.
///
void MainWindow::actionDisconnect()
{
    setMode(Disconnected);
}



///
/// \brief Modo brújula.
///
void MainWindow::actionCompass()
{
    setMode(Compass);
}



///
/// \brief Inicia el proceso de calibración.
///
void MainWindow::actionCalibration()
{
    setMode(Calibration);
    m_acc_measurements.clear();
    m_acc_measurements.reserve(100000);
    m_mag_measurements.clear();
    m_mag_measurements.reserve(100000);
}



///
/// \brief Termina la calibración.
///
void MainWindow::actionDone()
{
    // Detiene la captura de datos
    setMode(Waiting);

    // Calculamos los factores de corrección
    auto accCalib = FitAlignedEllipsoid(m_acc_measurements);
    auto magCalib = FitAlignedEllipsoid(m_mag_measurements);
    m_thread->recalibrate(accCalib, magCalib);
    m_acc_measurements.clear();
    m_mag_measurements.clear();
}



///
/// \brief Cancela la calibración.
///
void MainWindow::actionCancel()
{
    setMode(Compass);
    m_acc_measurements.clear();
    m_mag_measurements.clear();
}



///
/// \brief Recibe la orientación calculada por el IMU.
/// \param quat Orientación en referencia al sistema ENU.
///
void MainWindow::readOrientation(QQuaternion quat)
{
    if(m_mode == Compass) {
        QMatrix4x4 m;
        m.rotate(quat);
        ui->openGLWidget->setOrientation(m);
    }
}



///
/// \brief Recibe la fuerza ejercida sobre la muñeca.
/// \param force Flexión XY, compresión, torsión.
///
void MainWindow::readForce(QVector4D force)
{
    if(m_mode == Compass) {
        QString msg;
        msg.sprintf("Flexión: (%+f, %+f) | Compresión: %+f | Torsión: %+f", force.x(), force.y(), force.z(), force.w());
        m_status.setText(msg);
    }
}



///
/// \brief Lectura de los sensores del IMU, sin procesar.
/// \param gyr Giróscopo, radianes/s.
/// \param acc Acelerómetro, x/g₀
/// \param mag Magnetómetro, x/45µT
///
void MainWindow::readRawSensors(QVector3D gyr, QVector3D acc, QVector3D mag)
{
    if(m_mode == Calibration) {
        m_acc_measurements.push_back(acc);
        m_mag_measurements.push_back(mag);
        ui->openGLWidget->setAccCloud(m_acc_measurements);
        ui->openGLWidget->setMagCloud(m_mag_measurements);

        /*QString msg;
        msg.sprintf("Gyr: (%+f, %+f, %+f) | Acc: (%+f, %+f, %+f) | Mag: (%+f, %+f, %+f)",
                gyr.x(), gyr.y(), gyr.z(),
                acc.x(), acc.y(), acc.z(),
                mag.x(), mag.y(), mag.z());
        m_status.setText(msg);*/
    }
}



///
/// \brief Lectura de los valores del ADC, sin procesar.
/// \param values Los 6 canales del ADC, en milivoltios.
///
void MainWindow::readRawAnalog(float values[6])
{
    if(m_mode == Calibration) {
        QString msg;
        msg.sprintf("ADC: (%+f, %+f, %+f, %+f, %+f, %+f)", values[0], values[1], values[2], values[3], values[4], values[5]);
        m_status.setText(msg);
    }
}



///
/// \brief Cambia el modo de funcionamiento de la aplicación.
/// \param mode Nuevo modo de funcionamiento.
///
void MainWindow::setMode(IMUMode mode)
{
    m_mode = mode;
    switch(m_mode) {
    case Disconnected:
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->actionCalibration->setEnabled(false);
        ui->actionDone->setEnabled(false);
        ui->actionCancel->setEnabled(false);
        m_status.setText("Disconnected");
        ui->openGLWidget->setMode(Disconnected);
        if(m_thread) {
            m_thread->setMode(Disconnected);
            delete m_thread;
            m_thread = nullptr;
        }
        break;
    case Waiting:
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionCompass->setEnabled(true);
        ui->actionCalibration->setEnabled(true);
        ui->actionDone->setEnabled(false);
        ui->actionCancel->setEnabled(false);

        m_status.setText("Waiting");
        ui->openGLWidget->setMode(Compass);
        m_thread->setMode(Waiting);
        break;
    case Compass:
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionCompass->setEnabled(true);
        ui->actionCalibration->setEnabled(true);
        ui->actionDone->setEnabled(false);
        ui->actionCancel->setEnabled(false);

        m_status.setText("Compass mode");
        ui->openGLWidget->setMode(Compass);
        m_thread->setMode(Compass);
        break;
    case Calibration:
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionCompass->setEnabled(true);
        ui->actionCalibration->setEnabled(false);
        ui->actionDone->setEnabled(true);
        ui->actionCancel->setEnabled(true);

        m_status.setText("Calibration mode");
        ui->openGLWidget->setMode(Calibration);
        m_thread->setMode(Calibration);
        break;
    }
}



void MainWindow::timerEvent(QTimerEvent* e)
{
    if(m_mode != Disconnected) {
        ui->openGLWidget->update();
    }
}
