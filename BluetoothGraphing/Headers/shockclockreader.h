#include <qt_windows.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <BthLEDef.h>
#include <BluetoothApis.h>
#include <bluetoothleapis.h>

#pragma warning (disable: 4068)
//#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <bluetoothleapis.h>

#include <iostream>
#include <sstream>
#include <string>
#include <locale>


#include <QFile>
#include <QTextStream>
#include <QVariantList>
#include <QVector>
#include <QTextStream>
#include <QSerialPort>
#include <QSerialPortInfo>
#pragma comment(lib,"SetupAPI")

#ifndef SHOCKCLOCKREADER_H
#define SHOCKCLOCKREADER_H


class ShockClockReader : public QObject
{
    Q_OBJECT
public:
    ShockClockReader();
    HANDLE GetBLEHandle(__in GUID AGuid);
    HANDLE hLEDevice;
    GUID AGuid;
    HRESULT hr;
    PBTH_LE_GATT_CHARACTERISTIC pCharBuffer;
    USHORT charBufferSize;
    QVector<UCHAR> readCharacteristic(int charNumber);

    float iq0, iq1, iq2, iq3;
    float exInt, eyInt, ezInt;  // scaled integral error
    float twoKp;      // 2 * proportional gain (Kp)
    float twoKi;      // 2 * integral gain (Ki)
    float q0, q1, q2, q3; // quaternion of sensor frame relative to auxiliary frame
    float integralFBx,  integralFBy, integralFBz;
    float invSqrt(float number);
    bool noAccelReadings;
    double accelxBias, accelyBias, accelzBias;

    QVector<long> gyroAccX, gyroAccY, gyroAccZ;
    long gyroBiasX, gyroBiasY, gyroBiasZ;
    int gyroBiasSamples;
    bool gyroBiasCalculated;

    QVector<long> accelAccX, accelAccY, accelAccZ;
    long accelBiasX, accelBiasY, accelBiasZ;
    int accelBiasSamples;
    bool accelBiasCalculated;

    QVector<long> magAccX, magAccY, magAccZ;
    long magBiasX, magBiasY, magBiasZ;
    int magBiasSamples;
    bool magBiasCalculated;
//    void HandleBLENotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);
    QSerialPort* serialPort;
    QString serialPortName;
    QByteArray data;
    //char* ptr = &data;



public slots:
    void readShockClock();
    void ScanBLEDevices();
    void ConnectBLEDevice();


signals:
    void broadcastQuaternions(QVariantList quaternions);
    void newValues(QVector<double> values);
};

#endif // SHOCKCLOCKREADER_H
