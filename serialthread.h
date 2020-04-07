#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>

#include "Render/types.h"



///
/// \brief Hilo para la lectura del estado del IMU.
///
class SerialThread : public QThread
{
    Q_OBJECT

public:
    explicit SerialThread(const QSerialPortInfo& info, QObject* parent = 0);
    ~SerialThread();
    void run();
    const QString& getUID() const;
    void setMode(IMUMode mode);
    void recalibrate(const QMatrix4x4& acc, const QMatrix4x4& mag);

signals:
    void readOrientation(QQuaternion ori);
    void readForce(QVector4D force);
    void readRawSensors(QVector3D gyr, QVector3D acc, QVector3D mag);
    void readRawAnalog(float values[6]);

private:
    QSerialPortInfo m_info;
    QSerialPort* m_port;
    QString m_uid;
    QMatrix4x4 m_acc_calib, m_mag_calib;
    bool m_write_calib, m_change_mode;
    IMUMode m_mode;

    QStringList sendCommand(const QByteArray& command);
};
