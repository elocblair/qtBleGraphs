#include "mainwindow.h"
#include <QApplication>
#include <QtSerialPort/QSerialPort>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*QSerialPort serial;
    serial.setPortName("COM3");
    serial.setBaudRate(QSerialPort::Baud9600);
    if(!serial.isOpen()){
        serial.open(QIODevice::ReadWrite);
    }
    while(1){
        serial.waitForReadyRead(55);
        QByteArray read = serial.readLine(10);
        qDebug() << read;

    }*/
    MainWindow w;
    w.show();

    return a.exec();
}
