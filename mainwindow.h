#pragma once

#include <QBasicTimer>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>

#include "serialthread.h"



namespace Ui {
    class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    virtual void timerEvent(QTimerEvent* e);

private:
    Ui::MainWindow* ui;
    SerialThread* m_thread;

    QList<QSerialPortInfo> m_serialPortInfos;
    QComboBox m_serialPortList;
    QLabel m_status;
    IMUMode m_mode;
    QBasicTimer m_timer;

    std::vector<QVector3D> m_acc_measurements;
    std::vector<QVector3D> m_mag_measurements;

private slots:
    void actionConnect();
    void actionDisconnect();
    void actionCompass();
    void actionCalibration();
    void actionDone();
    void actionCancel();

    void setMode(IMUMode mode);

public slots:
    void readOrientation(QQuaternion ori);
    void readForce(QVector4D force);
    void readRawSensors(QVector3D gyr, QVector3D acc, QVector3D mag);
    void readRawAnalog(float values[6]);
};
