#-------------------------------------------------
#
# Project created by QtCreator 2015-01-15T10:07:59
#
#-------------------------------------------------

QT += core gui widgets opengl serialport

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iNEMO-Calibration

TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    renderer.cpp \
    serialthread.cpp \
    Render/staticmesh.cpp \
    Render/pointcloud.cpp \
    Render/axes.cpp \
    Render/types.cpp

HEADERS  += mainwindow.h \
    renderer.h \
    serialthread.h \
    Render/staticmesh.h \
    Render/pointcloud.h \
    Render/axes.h \
    Render/types.h

FORMS    += mainwindow.ui

RESOURCES += Render/data.qrc
