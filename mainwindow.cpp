
#include "mainwindow.h"

/// Async Operations
#include <QtConcurrent/QtConcurrentRun>

/// Sleep
#include <unistd.h>

/// Qt Includes
#include <QApplication>

/// Set priority
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>

/// The size of data to send
#define SIZE_TO_SEND 75

/// Screen Size
#define SCREEN_WIDTH 320.0f
#define SCREEN_HEIGHT 240.0f

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{

    qDebug() << "Welcome to the XBee Qt App...";

    // Get screen coords
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    qDebug() << "Setting Screen to Width: " << width << " and Height: " << height;

    // Resize
    resize(width, height); // Adafruit piTFT resolution

    // Setup font
    QFont newFont("FreeSans", 10, QFont::Bold, true);
    newFont.setPixelSize(10);
    QApplication::setFont(newFont);

    // Set niceness
    setpriority(PRIO_PROCESS, getpid(), -20);

    // Try to maximize performance
    int ret;

    // We'll operate on the currently running thread.
    pthread_t this_thread = pthread_self();

    // struct sched_param is used to store the scheduling priority
    struct sched_param params;

    // We'll set the priority to the maximum.
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    qDebug() << "Trying to set thread realtime prio = " << params.sched_priority;

    // Attempt to set thread real-time priority to the SCHED_FIFO policy
    ret = pthread_setschedparam(this_thread, SCHED_FIFO, &params);
    if (ret != 0) {
        // Print the error
        qDebug() << "Unsuccessful in setting thread realtime prio";
        return;
    }

    // Now verify the change in thread priority
    int policy = 0;
    ret = pthread_getschedparam(this_thread, &policy, &params);
    if (ret != 0) {
        qDebug() << "Couldn't retrieve real-time scheduling paramers";
        return;
    }

    // Check the correct policy was applied
    if(policy != SCHED_FIFO) {
        qDebug() << "Scheduling is NOT SCHED_FIFO!";
    } else {
        qDebug() << "SCHED_FIFO OK";
    }

    // Print thread scheduling priority
    qDebug() << "Thread priority is " << params.sched_priority;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Now make 2 XBee objects
    m_Coordinator = new XBee(this,"/dev/ttyUSB0","4321","0013A200","40C4570D"); //  408BAE82
    qDebug() << "Configured Coordinator correctly: " << m_Coordinator->m_Configured;
    m_Router = new XBee(this,"/dev/ttyUSB1","4321","0013A200","408BAE82"); //  40C4570D
    qDebug() << "Configured Router correctly: " << m_Router->m_Configured;

    // Get audio input
    if ( m_Router->m_Configured ) {

        // Measure Performance
        m_BytesSent = 0;
        m_RateTimer.start();
        m_CycleCounter = 0;

        // Send it
        while ( true ) {

            // Send it
            QByteArray toSend = QByteArray(QString("12345678910111213141516171819202122232425262728293031323334353637383940414243444546").toUtf8());
            //QByteArray toSend = QByteArray(QString("123").toUtf8());
            qint64 amount = m_Router->m_SerialPort.write(toSend,toSend.length());
            if (amount == -1) {
                qDebug() << QString("Failed to write the data to port %1, error: %2").arg(QString("Sender")).arg(m_Router->m_SerialPort.errorString()) << endl;
            } else if (amount != toSend.size()) {
                qDebug() << QString("Failed to write all the data to port %1, error: %2").arg(QString("Sender")).arg(m_Router->m_SerialPort.errorString()) << endl;
            } else if (!m_Router->m_SerialPort.waitForBytesWritten(5000)) {
                qDebug() << QString("Operation timed out or an error occurred for port %1, error: %2").arg(QString("Sender")).arg(m_Router->m_SerialPort.errorString()) << endl;
            }
            else {
                //qDebug() << "Wrote: " <<  amount << " Bytes...";
            }

            //usleep(10000); // 10 ms

            m_BytesSent += amount;
            m_CycleCounter++;
            if ( m_CycleCounter % 1000 == 0 ) {
                qDebug() << QString("Sent Bytes / Sec = %1 / %2 == %3").arg(m_BytesSent).arg((m_RateTimer.elapsed() / 1000.0f)).arg(m_BytesSent/(m_RateTimer.elapsed() / 1000.0f));
            }

        }

        return;

    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

MainWindow::~MainWindow()
{

}
