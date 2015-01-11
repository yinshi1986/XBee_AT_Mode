#-------------------------------------------------
#
# Project created by QtCreator 2014-12-12T08:36:03
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
#QT       += multimedia
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xbee
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    xbee.cpp

HEADERS  += mainwindow.h \
    xbee.h

target.path = /home/pi
INSTALLS += target
