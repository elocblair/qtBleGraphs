#include "shockclockreader.h"
#include <qDebug>
#include <math.h>
#include <QDateTime>
#include <algorithm>
#include <vector>
#include <QTGlobal>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QString>
#include <QList>

qint64 previousSample = 0;
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};
int z = 0;
double num;


ShockClockReader::ShockClockReader()
{
    serialPort = new QSerialPort;

}

float truncate(float value);
#define CC2650_DEVICE_UUID  "{38192FF6-D763-4793-B9D4-6E58894954BB}"
#define SIMPLE_BLE_PERIPHERAL "{0000F00D-1212-EFDE-1523-785FEF13D123}"

namespace util
{
    std::string to_narrow(const wchar_t *s, char def = '?', const std::locale& loc = std::locale())
    {
        std::ostringstream stm;
        while (*s != L'\0')
        {
            stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, def);
        }
        return stm.str();
    }
}
ShockClockReader *selfReference;
void HandleBLENotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
    //QList<QSerialPortInfo> ports =  info.availablePorts();
    selfReference->serialPort->setPortName("COM3");
    selfReference->serialPort->setBaudRate(QSerialPort::Baud9600);
    if(!selfReference->serialPort->isOpen()){
        selfReference->serialPort->open(QIODevice::ReadWrite);
    }
    QVector<double> values;
    QVector<double> doubleValuesX, doubleValuesY;
    //printf("notification obtained ");
    PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;
    QVector<UCHAR> char1;
    HRESULT hr;
    if (0 == ValueChangedEventParameters->CharacteristicValue->DataSize) {
        hr = E_FAIL;
       // printf("datasize 0\n");
    }
    else {
        //printf("got value\n");
        for(int i=0; i<ValueChangedEventParameters->CharacteristicValue->DataSize;i++) {
            char1.push_back(ValueChangedEventParameters->CharacteristicValue->Data[i]);
            //printf("SUP %0x",ValueChangedEventParameters->CharacteristicValue->Data[i]);
            Sleep(1);
        }
       // qDebug() << char1.size();
        if (char1.size()>15){

            if(selfReference->serialPort->waitForReadyRead(1)){
                QByteArray numberRead;
                numberRead = selfReference->serialPort->readAll();
                QByteArray currentNumX,currentNumY;


                qDebug() << numberRead;
                int flag = 2;
                for( int i = 0; i < numberRead.size(); i++){
                    char temp = numberRead.at(i);
                    if(i == (numberRead.size() - 1)){
                        if(flag == 1){
                            currentNumY.push_back(numberRead.at(i));
                            double currentDub = QString::fromLatin1(currentNumY).indexOf('.') > -1 ? currentNumY.toDouble() : 0.0;
                            qDebug() << "y serial data: " << currentDub << "\n";

                            doubleValuesY.push_back(currentDub);
                        }
                        if(flag == 0){
                            currentNumX.push_back(numberRead.at(i));
                            double currentDub = QString::fromLatin1(currentNumX).indexOf('.') > -1 ? currentNumX.toDouble() : 0.0;
                            qDebug() << "x serial data: " << currentDub << "\n";
                            doubleValuesX.push_back(currentDub);
                        }
                    }
                    if(temp > 130){
                        flag = 2;
                        i = numberRead.size();
                    }
                    if (temp == 120){
                        //m
                        if(currentNumY.size() != 0){
                            double currentDub = QString::fromLatin1(currentNumY).indexOf('.') > -1 ? currentNumY.toDouble() : 0.0;                            //values.at(1) = currentDub;
                            qDebug() << "y serial data: " << currentDub << "\n";
                            doubleValuesY.push_back(currentDub);
                            currentNumY.clear();
                        }
                        flag = 0;
                    }
                    if (temp == 121){
                        //make double
                        if(currentNumX.size() != 0){
                            double currentDub = QString::fromLatin1(currentNumX).indexOf('.') > -1 ? currentNumX.toDouble() : 0.0;                            //values.at(0) = currentDub;
                            qDebug() << "x serial data: " << currentDub << "\n";
                            doubleValuesX.push_back(currentDub);
                            currentNumX.clear();
                        }
                        flag = 1;
                    }
                    if ((temp > 47) & (temp < 58)){
                        if(flag ==1){
                            currentNumY.push_back(numberRead.at(i));
                        }
                        if(!flag){
                            currentNumX.push_back(numberRead.at(i));
                        }
                    }
                    if (temp == 46){
                        if(flag==1){
                            currentNumY.push_back(numberRead.at(i));
                        }
                        if(!flag){
                            currentNumX.push_back(numberRead.at(i));
                        }
                    }
                    if (temp == 45){
                        if(flag==1){
                            currentNumY.push_back(numberRead.at(i));
                        }
                        if(!flag){
                            currentNumX.push_back(numberRead.at(i));
                        }
                    }

                }
            }
            short az = (char1.at(3) << 8) | (char1.at(2) & 0xff);

            short gx = (char1.at(9) << 8) | (char1.at(8) & 0xff);
            short gy = (char1.at(11) << 8) | (char1.at(10) & 0xff);
            short gz = (char1.at(13) << 8) | (char1.at(12) & 0xff);

            short gx2 = (char1.at(15) << 8) | (char1.at(14) & 0xff);
            short gy2 = (char1.at(6) << 8) | (char1.at(7) & 0xff);
            short gz2 = (char1.at(4) << 8) | (char1.at(5) & 0xff);


            double azScaled, gxScaled, gyScaled, gzScaled, gx2Scaled, gy2Scaled, gz2Scaled;

            azScaled = ((double)az)*0.00111;

            gxScaled = ((double)gx)*0.00111;
            gyScaled = ((double)gy)*0.00111;
            gzScaled = ((double)gz)*0.00111;

            gx2Scaled = ((double)gx2)*0.00111;
            gy2Scaled = ((double)gy2)*0.049;
            gz2Scaled = ((double)gz2)*0.049;
            //qDebug() << gy2Scaled << gz2Scaled;
            double sizeX = (double)doubleValuesX.size();
            double sizeY = (double)doubleValuesY.size();
            values.clear();
            values << sizeX << sizeY;
            if(doubleValuesX.size() != 0){
                //qDebug() "before\n" << doubleValuesX << doubleValuesY;
            }

            for (int i = 0; i < sizeX; i++){
                values << doubleValuesX.at(i);
            }
            for(int i = 0; i < sizeY; i++){
                values << doubleValuesY.at(i);
            }
            //qDebug() << values;
            values << azScaled  << gxScaled  << gyScaled  << gzScaled << gx2Scaled  << gy2Scaled  << gz2Scaled;
            //readShockClock();
            emit selfReference->newValues(values);

        }
        // if the first bit is set, then the value is the next 2 bytes.  If it is clear, the value is in the next byte
        //The Heart Rate Value Format bit (bit 0 of the Flags field) indicates if the data format of
        //the Heart Rate Measurement Value field is in a format of UINT8 or UINT16.
        //When the Heart Rate Value format is sent in a UINT8 format, the Heart Rate Value
        //Format bit shall be set to 0. When the Heart Rate Value format is sent in a UINT16
        //format, the Heart Rate Value Format bit shall be set to 1
        //from this PDF https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=239866
        //unsigned heart_rate;
        //if (0x01 == (ValueChangedEventParameters->CharacteristicValue->Data[0] & 0x01)) {
        //	heart_rate = ValueChangedEventParameters->CharacteristicValue->Data[1] * 256 + ValueChangedEventParameters->CharacteristicValue->Data[2];
        //}
        //else {
        //	heart_rate = ValueChangedEventParameters->CharacteristicValue->Data[1];
        //}
        //printf("%d\n", heart_rate);
    }
}

// This function works to get a handle for a BLE device based on its GUID
// Copied from http://social.msdn.microsoft.com/Forums/windowshardware/en-US/e5e1058d-5a64-4e60-b8e2-0ce327c13058/erroraccessdenied-error-when-trying-to-receive-data-from-bluetooth-low-energy-devices?forum=wdk
// From https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/bad452cb-4fc2-4a86-9b60-070b43577cc9/is-there-a-simple-example-desktop-programming-c-for-bluetooth-low-energy-devices?forum=wdk
// Credits to Andrey_sh
HANDLE ShockClockReader::GetBLEHandle(__in GUID AGuid)
{
    selfReference = this;

    HDEVINFO hDI;
    SP_DEVICE_INTERFACE_DATA did;
    SP_DEVINFO_DATA dd;
    GUID BluetoothInterfaceGUID = AGuid;
    HANDLE hComm = NULL;

    hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (hDI == INVALID_HANDLE_VALUE) return NULL;

    did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    dd.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
    {
        SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

        DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        DWORD size = 0;

        if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0))
        {
            int err = GetLastError();

            if (err == ERROR_NO_MORE_ITEMS) break;

            PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

            pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd))
                break;

            hComm = CreateFile(
                pInterfaceDetailData->DevicePath,
                GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            GlobalFree(pInterfaceDetailData);
        }
    }

    SetupDiDestroyDeviceInfoList(hDI);
    return hComm;
}

void ShockClockReader::ConnectBLEDevice()
{

    // Step 1: find the BLE device handle from its

    // GUID can be constructed from "{xxx....}" string using CLSID
    CLSIDFromString(TEXT(SIMPLE_BLE_PERIPHERAL), &AGuid);
    // Get the handle
    hLEDevice = GetBLEHandle(AGuid);

    // Step 2: Get a list of services that the device advertises
    // first send 0, NULL as the parameters to BluetoothGATTServices inorder to get the number of
    // services in serviceBufferCount
    USHORT serviceBufferCount;
    ////////////////////////////////////////////////////////////////////////////
    // Determine Services Buffer Size
    ////////////////////////////////////////////////////////////////////////////

    HRESULT hr = BluetoothGATTGetServices(
        hLEDevice,
        0,
        NULL,
        &serviceBufferCount,
        BLUETOOTH_GATT_FLAG_NONE);

    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
       // printf("BluetoothGATTGetServices - Buffer Size %d\n", hr);
    }
    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
            std::cout << "BluetoothGATTGetServices - Error = 0x" << std::hex << hr;
    }

    PBTH_LE_GATT_SERVICE pServiceBuffer = (PBTH_LE_GATT_SERVICE)
        malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

    if (NULL == pServiceBuffer) {
      //  printf("pServiceBuffer out of memory\n");
    }
    else {
        RtlZeroMemory(pServiceBuffer,
            sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Retrieve Services
    ////////////////////////////////////////////////////////////////////////////

    USHORT numServices;
    hr = BluetoothGATTGetServices(
        hLEDevice,
        serviceBufferCount,
        pServiceBuffer,
        &numServices,
        BLUETOOTH_GATT_FLAG_NONE);

    if (S_OK != hr) {
       // printf("BluetoothGATTGetServices - Buffer Size %d\n", hr);
    }
    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
        std::cout << "BluetoothGATTGetServices - Error = 0x" << std::hex << hr;
    }

    // Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
    ////////////////////////////////////////////////////////////////////////////
    // Determine Characteristic Buffer Size
    ////////////////////////////////////////////////////////////////////////////

    hr = BluetoothGATTGetCharacteristics(
        hLEDevice,
        pServiceBuffer,
        0,
        NULL,
        &charBufferSize,
        BLUETOOTH_GATT_FLAG_NONE);

    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
       // printf("BluetoothGATTGetCharacteristics - Buffer Size %d\n", hr);
    }

    //PBTH_LE_GATT_CHARACTERISTIC pCharBuffer;
    if (charBufferSize > 0) {
        pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
            malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

        if (NULL == pCharBuffer) {
           // printf("pCharBuffer out of memory\n");
        }
        else {
            RtlZeroMemory(pCharBuffer,
                charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
        }

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve Characteristics
        ////////////////////////////////////////////////////////////////////////////
        /// \brief numChars
        USHORT numChars;
        hr = BluetoothGATTGetCharacteristics(
            hLEDevice,
            pServiceBuffer,
            charBufferSize,
            pCharBuffer,
            &numChars,
            BLUETOOTH_GATT_FLAG_NONE);

        if (S_OK != hr) {
            //printf("BluetoothGATTGetCharacteristics - Actual Data %d\n", hr);
        }

        if (numChars != charBufferSize) {
           // printf("buffer size and buffer size actual size mismatch\n");
        }
    }

    // Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
    // descriptors are required as we descriptors that are notification based will have to be written
    // once IsSubcribeToNotification set to true, we set the appropriate callback function
    // need for setting descriptors for notification according to
    //http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
    PBTH_LE_GATT_CHARACTERISTIC currGattChar;
    for (int ii = 0; ii <charBufferSize; ii++) {
        currGattChar = &pCharBuffer[ii];
        USHORT charValueDataSize;
        PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;

        ///////////////////////////////////////////////////////////////////////////
        // Determine Descriptor Buffer Size
        ////////////////////////////////////////////////////////////////////////////
        USHORT descriptorBufferSize;
        hr = BluetoothGATTGetDescriptors(
            hLEDevice,
            currGattChar,
            0,
            NULL,
            &descriptorBufferSize,
            BLUETOOTH_GATT_FLAG_NONE);

        if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
            //printf("BluetoothGATTGetDescriptors - Buffer Size %d\n", hr);
        }

        PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer;
        if (descriptorBufferSize > 0) {
            pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)
                malloc(descriptorBufferSize
                * sizeof(BTH_LE_GATT_DESCRIPTOR));

            if (NULL == pDescriptorBuffer) {
               // printf("pDescriptorBuffer out of memory\n");
            }
            else {
                RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
            }

            ////////////////////////////////////////////////////////////////////////////
            // Retrieve Descriptors
            ////////////////////////////////////////////////////////////////////////////

            USHORT numDescriptors;
            hr = BluetoothGATTGetDescriptors(
                hLEDevice,
                currGattChar,
                descriptorBufferSize,
                pDescriptorBuffer,
                &numDescriptors,
                BLUETOOTH_GATT_FLAG_NONE);

            if (S_OK != hr) {
                //printf("BluetoothGATTGetDescriptors - Actual Data %d\n", hr);
            }

            if (numDescriptors != descriptorBufferSize) {
                //printf("buffer size and buffer size actual size mismatch\n");
            }

            for (int kk = 0; kk<numDescriptors; kk++) {
                PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer[kk];
                ////////////////////////////////////////////////////////////////////////////
                // Determine Descriptor Value Buffer Size
                ////////////////////////////////////////////////////////////////////////////
                USHORT descValueDataSize;
                hr = BluetoothGATTGetDescriptorValue(
                    hLEDevice,
                    currGattDescriptor,
                    0,
                    NULL,
                    &descValueDataSize,
                    BLUETOOTH_GATT_FLAG_NONE);

                if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
                    //printf("BluetoothGATTGetDescriptorValue - Buffer Size %d\n", hr);
                }

                PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

                if (NULL == pDescValueBuffer) {
                   // printf("pDescValueBuffer out of memory\n");
                }
                else {
                    RtlZeroMemory(pDescValueBuffer, descValueDataSize);
                }

                ////////////////////////////////////////////////////////////////////////////
                // Retrieve the Descriptor Value
                ////////////////////////////////////////////////////////////////////////////

                hr = BluetoothGATTGetDescriptorValue(
                    hLEDevice,
                    currGattDescriptor,
                    (ULONG)descValueDataSize,
                    pDescValueBuffer,
                    NULL,
                    BLUETOOTH_GATT_FLAG_NONE);
                if (S_OK != hr) {
                   // printf("BluetoothGATTGetDescriptorValue - Actual Data %d\n", hr);
                }
                // you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
                // we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
                if (currGattDescriptor->AttributeHandle < 255) {
                    BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

                    RtlZeroMemory(&newValue, sizeof(newValue));

                    newValue.DescriptorType = ClientCharacteristicConfiguration;
                    newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

                    hr = BluetoothGATTSetDescriptorValue(
                        hLEDevice,
                        currGattDescriptor,
                        &newValue,
                        BLUETOOTH_GATT_FLAG_NONE);
                    if (S_OK != hr) {
                        //printf("BluetoothGATTGetDescriptorValue - Actual Data %d\n", hr);
                    }
                }

            }
        }

        // set the appropriate callback function when the descriptor change value
        BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

        if (currGattChar->IsNotifiable) {
            //printf("Setting Notification for ServiceHandle %d\n", currGattChar->ServiceHandle);
            BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

            BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
            EventParameterIn.Characteristics[0] = *currGattChar;
            EventParameterIn.NumCharacteristics = 1;
            hr = BluetoothGATTRegisterEvent(
                hLEDevice,
                EventType,
                &EventParameterIn,
                HandleBLENotification,
                NULL,
                &EventHandle,
                BLUETOOTH_GATT_FLAG_NONE);

            if (S_OK != hr) {
               // printf("BluetoothGATTRegisterEvent - Actual Data %d\n", hr);
            }
        }
        else if (currGattChar->IsReadable) {
            ////////////////////////////////////////////////////////////////////////////
            // Determine Characteristic Value Buffer Size
            ////////////////////////////////////////////////////////////////////////////
            hr = BluetoothGATTGetCharacteristicValue(
                hLEDevice,
                currGattChar,
                0,
                NULL,
                &charValueDataSize,
                BLUETOOTH_GATT_FLAG_NONE);

            if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
               // printf("BluetoothGATTGetCharacteristicValue - Buffer Size %d\n", hr);
            }

            pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

            if (NULL == pCharValueBuffer) {
                //printf("pCharValueBuffer out of memory\n");
            }
            else {
                RtlZeroMemory(pCharValueBuffer, charValueDataSize);
            }

            ////////////////////////////////////////////////////////////////////////////
            // Retrieve the Characteristic Value
            ////////////////////////////////////////////////////////////////////////////

            hr = BluetoothGATTGetCharacteristicValue(
                hLEDevice,
                currGattChar,
                (ULONG)charValueDataSize,
                pCharValueBuffer,
                NULL,
                BLUETOOTH_GATT_FLAG_NONE);

            if (S_OK != hr) {
               // printf("BluetoothGATTGetCharacteristicValue - Actual Data %d\n", hr);
            }

            // print the characeteristic Value
           // printf("\n Printing a read characterstic ");
            for (int iii = 0; iii < (int)pCharValueBuffer->DataSize; iii++) {// ideally check ->DataSize before printing
               // printf("%d ", pCharValueBuffer->Data[iii]);
            }
           // printf("\n");

            // Free before going to next iteration, or memory leak.
            free(pCharValueBuffer);
            pCharValueBuffer = NULL;
        }

    }

    // go into an inf loop that sleeps. you will ideally see notifications from the HR device
    while (1){
      //  printf("sleep\n");
        Sleep(1000);
    }

    CloseHandle(hLEDevice);

    /*if (GetLastError() != NO_ERROR &&
        GetLastError() != ERROR_NO_MORE_ITEMS)
    {
        // Insert error handling here.
        return 1;
    }

    return 0;*/

    // go into an inf loop that sleeps. you will ideally see notifications from the HR device
    /*while (1){
        //printf("sleep\n");
        Sleep(1000);
    }

    CloseHandle(hLEDevice);

    if (GetLastError() != NO_ERROR &&
        GetLastError() != ERROR_NO_MORE_ITEMS)
    {
        // Insert error handling here.
        return 1;
    }
*/

    //readShockClock(); //ADD this back to read using gattcharreadfxn

}

void ShockClockReader::ScanBLEDevices()
{

    HDEVINFO hDI;
    SP_DEVINFO_DATA did;
    DWORD i;

    // Create a HDEVINFO with all present devices.
    hDI = SetupDiGetClassDevs(&GUID_DEVCLASS_BLUETOOTH, NULL, NULL, DIGCF_PRESENT);

    if (hDI == INVALID_HANDLE_VALUE)
    {
        return;
    }

    // Enumerate through all devices in Set.
    did.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i = 0; SetupDiEnumDeviceInfo(hDI, i, &did); i++)
    {
        bool hasError = false;

        DWORD nameData;
        LPTSTR nameBuffer = NULL;
        DWORD nameBufferSize = 0;

        while (!SetupDiGetDeviceRegistryProperty(
            hDI,
            &did,
            SPDRP_FRIENDLYNAME,
            &nameData,
            (PBYTE)nameBuffer,
            nameBufferSize,
            &nameBufferSize))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (nameBuffer) delete(nameBuffer);
                nameBuffer = new wchar_t[nameBufferSize * 2];
            }
            else
            {
                hasError = true;
                break;
            }
        }

        DWORD addressData;
        LPTSTR addressBuffer = NULL;
        DWORD addressBufferSize = 0;

        while (!SetupDiGetDeviceRegistryProperty(
            hDI,
            &did,
            SPDRP_HARDWAREID,
            &addressData,
            (PBYTE)addressBuffer,
            addressBufferSize,
            &addressBufferSize))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (addressBuffer) delete(addressBuffer);
                addressBuffer = new wchar_t[addressBufferSize * 2];
            }
            else
            {
                hasError = true;
                break;
            }
        }

        LPTSTR deviceIdBuffer = NULL;
        DWORD deviceIdBufferSize = 0;

        while (!SetupDiGetDeviceInstanceId(
            hDI,
            &did,
            deviceIdBuffer,
            deviceIdBufferSize,
            &deviceIdBufferSize))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (deviceIdBuffer) delete(deviceIdBuffer);
                deviceIdBuffer = new wchar_t[deviceIdBufferSize * 2];
            }
            else
            {
                hasError = true;
                break;
            }
        }

        if (hasError)
        {
            continue;
        }

        std::string name = util::to_narrow(nameBuffer);
        std::string address = util::to_narrow(addressBuffer);
        std::string deviceId = util::to_narrow(deviceIdBuffer);
        std::cout << "Found " << name << " (" << deviceId << ")" << std::endl;
    }
    ConnectBLEDevice();
}
QVector<UCHAR> ShockClockReader::readCharacteristic(int charNumber)
{
    PBTH_LE_GATT_CHARACTERISTIC currGattChar;


    currGattChar = &pCharBuffer[charNumber];
    USHORT charValueDataSize = 56;
    PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;

    hr = BluetoothGATTGetCharacteristicValue(
        hLEDevice,
        currGattChar,
        0,
        NULL,
        &charValueDataSize,
        BLUETOOTH_GATT_FLAG_FORCE_READ_FROM_DEVICE);

    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
        //printf("BluetoothGATTGetCharacteristicValue - Buffer Size %d\n", hr);
    }

    pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

    if (NULL == pCharValueBuffer) {
        printf("pCharValueBuffer out of memory\n");
    }
    else {
        RtlZeroMemory(pCharValueBuffer, charValueDataSize);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Retrieve the Characteristic Value
    ////////////////////////////////////////////////////////////////////////////

    hr = BluetoothGATTGetCharacteristicValue(
        hLEDevice,
        currGattChar,
        (ULONG)charValueDataSize,
        pCharValueBuffer,
        NULL,
        BLUETOOTH_GATT_FLAG_FORCE_READ_FROM_DEVICE);

    if (S_OK != hr) {
        printf("BluetoothGATTGetCharacteristicValue - Actual Data %d\n", hr);
    }
    QVector<UCHAR> returnVector;
    QString string = "";
    QString comma;
    for (int i = 0; i < (int)pCharValueBuffer->DataSize; i++) {// ideally check ->DataSize before printing

        returnVector.push_back(pCharValueBuffer->Data[i]);
    }
    string.append('\n');

    if(pCharValueBuffer->DataSize > 0)
    {
        free(pCharValueBuffer);
        pCharValueBuffer = NULL;
    }
    return returnVector;
}


void ShockClockReader::readShockClock()
{

    QVector<UCHAR> CHAR1 = readCharacteristic(0);
    if(CHAR1.size() > 11)
    {
        short ax = (CHAR1.at(1) << 8) | (CHAR1.at(0) & 0xff);
        short ay = (CHAR1.at(3) << 8) | (CHAR1.at(2) & 0xff);
        short az = (CHAR1.at(5) << 8) | (CHAR1.at(4) & 0xff);

        short gx = (CHAR1.at(7) << 8) | (CHAR1.at(6) & 0xff);
        short gy = (CHAR1.at(9) << 8) | (CHAR1.at(8) & 0xff);
        short gz = (CHAR1.at(11) << 8) | (CHAR1.at(10) & 0xff);

        double axScaled, ayScaled, azScaled, gxScaled, gyScaled, gzScaled, mxScaled, myScaled, mzScaled;

        axScaled = ((double)ax)*0.049;
        ayScaled = ((double)ay)*0.049;
        azScaled = ((double)az)*0.049;

        gxScaled = ((double)gx)*0.049;
        gyScaled = ((double)gy)*0.049;
        gzScaled = ((double)gz)*0.049;

        QVector<double> values;
        values << axScaled  << ayScaled << azScaled  << gxScaled  << gyScaled  << gzScaled << mxScaled  << myScaled  << mzScaled;
        emit newValues(values);

    }
    else
    {
       // qDebug() <<  "WRONG";
    }
    QTimer::singleShot(0,this, SLOT(readShockClock()));
}
