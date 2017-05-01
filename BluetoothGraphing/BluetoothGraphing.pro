#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T09:28:26
#
#-------------------------------------------------

QT       += core gui opengl widgets
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BluetoothGraphing
TEMPLATE = app
CONFIG += qwt

INCLUDEPATH += C:/qwt-6.1.2/include
DEPENDPATH += C:/qwt-6.1.2/lib
LIBS += -LC:/qwt-6.1.2/lib
win32 {
     CONFIG(debug, debug|release) {
         LIBS += -lqwtd
     } else {
         LIBS += -lqwt
     }
}
DEFINES += QWT_DLL


SOURCES += main.cpp\
        mainwindow.cpp \
    shockclockreader.cpp

HEADERS  += mainwindow.h \
    shockclockreader.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$quote(C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10240.0\\um)
INCLUDEPATH += $$quote(C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10240.0\\shared)
LIBS += -L$$quote(C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\um\x86) -lBluetoothApis -lbthprops -lOle32

