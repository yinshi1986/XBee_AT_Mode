#ifndef XBEE_H
#define XBEE_H

#include <QObject>

/// Use Qt5 Serial Port
#include <QtSerialPort/QtSerialPort>

class XBee : public QObject
{
    Q_OBJECT

public:

    // Construct with Filename
    explicit XBee(QObject *parent = 0,QString uartFileName = "/dev/ttyUSB0", QString pan = "2001", QString destinationHigh = "0013A200", QString destinationLow = "00000000");

    // The XBee was setup properly
    bool m_Configured;

    // The XBee high and low addresses
    QString m_HighAddress;
    QString m_LowAddress;

    // The XBee destination high and low addresses
    QString m_DestinationHighAddress;
    QString m_DestinationLowAddress;

    // Read from the XBee
    QString readFromXbee();

    // Write to the XBee
    int writeToXBee(QString cmdString);

    // This is the Serial Port connection to the XBee radio
    QSerialPort m_SerialPort;

public slots:

    // XBee Data Available
    void receivedDataInSerialPort();

private:

    // This is the UART filename
    QString m_UartFileName;

    // The PAN network
    QString m_Pan;

    // We are now receiving packets
    bool m_ReceivingPackets;

    // Receive a packet
    QByteArray m_RxDataLeftovers;
    bool m_HaveStartByte;
    int m_ByteIndexRead;

    // Measure the incoming data
    QElapsedTimer *m_AudioRateTimer;
    int m_AudioBytesReceived;
    int m_AudioCounter;

signals:

    // Received data on XBee
    void receivedData(QString);

};

#endif // XBEE_H
