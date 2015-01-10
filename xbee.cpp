#include "xbee.h"

/// Linux sleep
#include <unistd.h>

/// Printing
#include <QDebug>

XBee::XBee(QObject *parent, QString uartFileName, QString pan, QString destinationHigh, QString destinationLow) :
    QObject(parent)
{

    qDebug() << "Configuring XBee... " << uartFileName;

    // Assign the filename
    m_UartFileName = uartFileName;

    // Get the pan
    m_Pan = pan;

    // Get the high and low destination addresses
    m_DestinationHighAddress = destinationHigh;
    m_DestinationLowAddress = destinationLow;

    // Setup the Port
    m_SerialPort.setPortName(m_UartFileName);
    m_SerialPort.setReadBufferSize(111111*2);
    //int serialPortBaudRate = QSerialPort::Baud9600;
    qint32 serialPortBaudRate = 111111; // Apparently this is closer to XBee internal baud rate
    //qint32 serialPortBaudRate = QSerialPort::Baud115200;

    // Use Hardware Flow Control
    m_SerialPort.setFlowControl(QSerialPort::HardwareControl);
    //m_SerialPort.setFlowControl(QSerialPort::NoFlowControl);

    // Try 2 stop bits
    bool didSetStopBits = m_SerialPort.setStopBits(QSerialPort::TwoStop);
    qDebug() << "QSerialPort Successfully Did Set Stop Bits: " << didSetStopBits;

    if ( uartFileName == "/dev/ttyUSB1" ) {
        bool didConnect = connect(&m_SerialPort, SIGNAL(readyRead()), this, SLOT(receivedDataInSerialPort()));
        qDebug() << "*** Successfully connected read signal!" << didConnect;
    }

    if (!m_SerialPort.open(QIODevice::ReadWrite)) {

        qDebug() << QString("XBee Serial Port: Failed to open port %1, error: %2").arg(m_UartFileName).arg(m_SerialPort.error());

        // Failure to setup
        m_Configured = false;

    }
    else {

        // Not receiving packets
        m_ReceivingPackets = false;

        // Set the baud rate
        m_SerialPort.setBaudRate(serialPortBaudRate);
        m_SerialPort.flush();

        qDebug() << "XBee Serial Port: Baud rate set to: " << m_SerialPort.baudRate();

        // Guard band is 1 second
        sleep(1.0);

        // Setup the XBee
        qDebug("XBee enter command mode...");
        writeToXBee("+++");
        sleep(1.0);
        QString response = readFromXbee();
        qDebug() << "Response is: " << response;
        while ( response.trimmed() != "OK" ) {
            //qDebug() << "Trying different Baud Rate: " << QSerialPort::Baud115200;
            //m_SerialPort.setBaudRate(QSerialPort::Baud115200);
            sleep(1.0);
            writeToXBee("+++");
            response = readFromXbee();
            continue;
        }

//        QString baud = "7"; // 115200
//        qDebug() << QString("Setting up Baud Rate to: %1").arg(baud);
//        writeToXBee(QString("ATBD %1").arg(baud));
//        sleep(1.0);
//        qDebug() << readFromXbee();

//        sleep(2.0);
//        m_SerialPort.flush();
//        m_SerialPort.close();
//        m_SerialPort.setBaudRate(serialPortBaudRate);
//        //m_SerialPort.setBaudRate(QSerialPort::Baud115200);
//        m_SerialPort.open(QIODevice::ReadWrite);
//        sleep(1.0);

        // Time to wait
        float timeToWait = 0.15;

        qDebug("Enable Flow Control");
        writeToXBee("ATD6 1");
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug("Query Radio High Address...");
        writeToXBee("ATSH");
        sleep(timeToWait);
        m_HighAddress = readFromXbee();
        qDebug() << m_HighAddress;

        qDebug("Query Radio Low Address...");
        writeToXBee("ATSL");
        sleep(timeToWait);
        m_LowAddress = readFromXbee();
        qDebug() << m_LowAddress;

        qDebug() << QString("Setting up PAN to: %1").arg(m_Pan);
        writeToXBee(QString("ATID %1").arg(m_Pan));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug() << QString("Setting up Destination High Address to: %1").arg(m_DestinationHighAddress);
        writeToXBee(QString("ATDH %1").arg(m_DestinationHighAddress));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug() << QString("Setting up Destination Low Address to: %1").arg(m_DestinationLowAddress);
        writeToXBee(QString("ATDL %1").arg(m_DestinationLowAddress));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug() << QString("Writing settings to flash...");
        writeToXBee(QString("ATWR"));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug() << QString("Using this Firmware:");
        writeToXBee(QString("ATVR"));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        qDebug() << QString("Exiting Command Mode...");
        writeToXBee(QString("ATCN"));
        sleep(timeToWait);
        qDebug() << readFromXbee();

        // Success setting up
        m_Configured = true;

        m_AudioRateTimer = new QElapsedTimer();
        m_AudioRateTimer->start();
        m_AudioBytesReceived = 0;
        m_AudioCounter = 0;

        // If this is the router
        if ( uartFileName == "/dev/ttyUSB1" ) {

            // Start receiving packets
            m_ReceivingPackets = true;

        }

    }

}

int XBee::writeToXBee(QString cmdToSend) {

    // Append carriage return if not +++ command
    if ( cmdToSend != "+++" ) {
        cmdToSend.append("\r\n");
    }
    qint64 bytesWritten = m_SerialPort.write(cmdToSend.toStdString().c_str(),cmdToSend.length());
    if (bytesWritten == -1) {
        qDebug() << QString("Failed to write the data to port %1, error: %2").arg(m_UartFileName).arg(m_SerialPort.errorString()) << endl;
    } else if (bytesWritten != cmdToSend.size()) {
        qDebug() << QString("Failed to write all the data to port %1, error: %2").arg(m_UartFileName).arg(m_SerialPort.errorString()) << endl;
    } else if (!m_SerialPort.waitForBytesWritten(5000)) {
        qDebug() << QString("Operation timed out or an error occurred for port %1, error: %2").arg(m_UartFileName).arg(m_SerialPort.errorString()) << endl;
    }
    return bytesWritten;

}

QString XBee::readFromXbee() {

    // Now attempt to read from port
    QString readData;
    while ( m_SerialPort.waitForReadyRead(1500) ) {
        readData.append( m_SerialPort.readAll() );
    }
    if (m_SerialPort.error() == QSerialPort::ReadError) {
        qDebug() << QString("XBee Read Failure: Failed to read from port %1, error: %2").arg(m_UartFileName).arg(m_SerialPort.errorString());
        return readData;
    } else if (m_SerialPort.error() == QSerialPort::TimeoutError && readData.isEmpty()) {
        qDebug() << QString("XBee Read Failure: No data was currently available for reading from port %1").arg(m_UartFileName);
        return readData;
    }
    else {
        return readData.trimmed();
    }

}

void XBee::receivedDataInSerialPort() {

    // Data is ready from the port
    if ( m_ReceivingPackets ) {

        // Time just the Audio Data itself
        m_AudioBytesReceived += m_SerialPort.bytesAvailable();
        m_SerialPort.readAll();
        m_AudioCounter++;
        if ( m_AudioCounter % 10000 == 0 ) {
            int audioBytes = (int)m_AudioBytesReceived;
            //int allBytes = (int)m_BytesReceived;
            int seconds = (int)(m_AudioRateTimer->elapsed() / 1000.0f);
            if ( audioBytes == 0 || seconds == 0 ) {
                return;
            }
            qDebug() << QString("Received Audio Bytes / Sec = %1 / %2 == %3").arg(audioBytes).arg(seconds).arg(audioBytes/seconds);
        }
    }
}
