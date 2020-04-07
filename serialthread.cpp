#include "serialthread.h"

#include <QDebug>

const char* COMMAND_RESET = "reset";
const char* COMMAND_READ_UID = "read uid";
const char* COMMAND_READ_ACC = "read acc";
const char* COMMAND_READ_MAG = "read mag";
const char* COMMAND_WRITE_ACC = "write acc %f %f %f %f %f %f %f %f %f %f %f %f";
const char* COMMAND_WRITE_MAG = "write mag %f %f %f %f %f %f %f %f %f %f %f %f";
const char* COMMAND_START_ORI = "start ori";
const char* COMMAND_START_CAL = "start cal";
const char* COMMAND_STOP = "stop";



///
/// \brief Constructor.
/// \param info Datos del puerto serie a usar.
/// \param parent Objeto padre.
///
SerialThread::SerialThread(const QSerialPortInfo& info, QObject* parent) : QThread(parent), m_port(nullptr)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_info = info;
    m_mode = Disconnected;
    m_write_calib = false;
    m_change_mode = false;
}



///
/// \brief Destructor, cierra el puerto serie.
///
SerialThread::~SerialThread()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_mode = Disconnected;
    this->wait();
}



///
/// \brief Bucle de comunicación con el IMU.
///
void SerialThread::run()
{
    qDebug() << __PRETTY_FUNCTION__;

    // Abre el puerto serie
    m_port = new QSerialPort(m_info, this);
    //m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setBaudRate(460800);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_port->open(QIODevice::ReadWrite)) {
        qDebug() << "The selected port couldn't be opened";
        delete m_port;
        return;
    }
    QThread::msleep(100);

    // Lee el identificador del IMU
    sendCommand(COMMAND_RESET);
    const QStringList foo = sendCommand(COMMAND_READ_UID);
    qDebug() << foo;
    for( auto f : foo ) {
        const QStringList bar = f.split(' ');
        if(bar[0] == "uid") {
            m_uid = bar[1];
            qDebug() << "IMU unique id:" << m_uid;
        }
    }

    // Bucle de lectura
    while(m_mode != Disconnected) {
        // Escribe la nueva calibración
        if(m_write_calib) {
            // Prepara los comandos
            QString acc_command, mag_command;
            acc_command.sprintf(COMMAND_WRITE_ACC,
                                m_acc_calib(0,0), m_acc_calib(0,1), m_acc_calib(0,2), m_acc_calib(0,3),
                                m_acc_calib(1,0), m_acc_calib(1,1), m_acc_calib(1,2), m_acc_calib(1,3),
                                m_acc_calib(2,0), m_acc_calib(2,1), m_acc_calib(2,2), m_acc_calib(2,3));
            mag_command.sprintf(COMMAND_WRITE_MAG,
                                m_mag_calib(0,0), m_mag_calib(0,1), m_mag_calib(0,2), m_mag_calib(0,3),
                                m_mag_calib(1,0), m_mag_calib(1,1), m_mag_calib(1,2), m_mag_calib(1,3),
                                m_mag_calib(2,0), m_mag_calib(2,1), m_mag_calib(2,2), m_mag_calib(2,3));

            // Envía los comandos
            sendCommand(acc_command.toUtf8());
            sendCommand(acc_command.toUtf8());
            sendCommand(acc_command.toUtf8());
            sendCommand(mag_command.toUtf8());
            sendCommand(mag_command.toUtf8());
            sendCommand(mag_command.toUtf8());
            m_write_calib = false;
        }

        if(m_change_mode) {
            switch(m_mode) {
                case Waiting: sendCommand(COMMAND_STOP); break;
                case Compass: sendCommand(COMMAND_START_ORI); break;
                case Calibration: sendCommand(COMMAND_START_CAL); break;
                default: break;
            }
            m_change_mode = false;
        }

        if(m_port->waitForReadyRead(10)) {
            while( m_port->canReadLine() ) {
                auto foo = m_port->readLine();
                QString header;
                QList<float> values;
                std::tie(header, values) = ParseLine(foo);

                //qDebug() << foo;

                if((header == "wxyz") && (values.size() == 4)) {
                    QQuaternion ori(values[0], values[1], values[2], values[3]);
                    emit readOrientation(ori);
                }
                else if((header == "force") && (values.size() == 4)) {
                    QVector4D force(values[0], values[1], values[2], values[3]);
                    emit readForce(force);
                }
                else if((header == "raw_adc") && (values.size() == 6)) {
                    float v[6];
                    for(int i=0 ; i<6 ; ++i) v[i] = values[i];
                    emit readRawAnalog(v);
                }
                else if((header == "raw_gam") && (values.size() == 9)) {
                    auto gyr = QVector3D(values[0], values[1], values[2]);
                    auto acc = QVector3D(values[3], values[4], values[5]);
                    auto mag = QVector3D(values[6], values[7], values[8]);
                    emit readRawSensors(gyr, acc, mag);
                }
            }
        }
    }

    // Cierra el puerto serie
    if( m_port ) {
        sendCommand(COMMAND_STOP);
        m_port->close();
        delete m_port;
        qDebug() << "Closed serial port";
    }
}



///
/// \brief Devuelve el identificador único del IMU.
/// \return Identificador del IMU.
///
const QString& SerialThread::getUID() const
{
    qDebug() << __PRETTY_FUNCTION__;
    while( m_uid.isEmpty() || m_uid.isNull() ) {}
    return m_uid;
}



///
/// \brief Envía los nuevos parámetros de calibración al IMU.
/// \param acc Calibración del acelerómetro.
/// \param mag Calibración del magnetómetro.
///
void SerialThread::recalibrate(const QMatrix4x4& acc, const QMatrix4x4& mag)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_acc_calib = acc;
    m_mag_calib = mag;
    m_write_calib = true;
}



///
/// \brief Envía un comando por el puerto serie y espera a que el IMU responda.
/// \param command Cadena con el comando. La función incluye automáticamente el fin de línea.
/// \return
///
QStringList SerialThread::sendCommand(const QByteArray& command)
{
    qDebug() << __PRETTY_FUNCTION__;

    qDebug() << "\tSending:" << command.trimmed();
    if( true ) {
        m_port->write(command.trimmed() + "\r\n");
        m_port->waitForBytesWritten(-1);
    }
    else {
        for( auto c : (command.trimmed() + "\r\n" )) {
            m_port->putChar(c);
            m_port->waitForBytesWritten(-1);
            QThread::msleep(10);
        }
        m_port->write(command.trimmed() + "\r\n");
        m_port->waitForBytesWritten(-1);
        QThread::msleep(10);
    }
    QThread::msleep(10);

    QString line;
    QStringList response;
    while(true) {
        m_port->waitForReadyRead(-1);
        while(m_port->canReadLine()) {
            line = m_port->readLine().trimmed().toLower();
            if(line.isNull() || line.isEmpty()) continue;
            else if(line=="ready") break;
            else response.append(line);
            qDebug() << "\tReceived:" << line;
        }
        if(line == "ready") {
            qDebug() << "\tREADY";
            break;
        }
    }
    QThread::msleep(10);
    return response;
}



///
/// \brief Cambia el modo de funcionamiento del IMU.
/// \param mode Nuevo modo.
///
void SerialThread::setMode(IMUMode mode)
{
    qDebug() << __PRETTY_FUNCTION__;

    if(mode != m_mode) {
        m_mode = mode;
        m_change_mode = true;
    }
}
