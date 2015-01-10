#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

/// The XBee data type(s)
#include "xbee.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:

    // The coordinator
    XBee *m_Coordinator;

    // The router
    XBee *m_Router;

    // Measure Performance
    int m_BytesSent;
    int m_CycleCounter;
    QElapsedTimer m_RateTimer;

};

#endif // MAINWINDOW_H
