#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set number of threads
    QThreadPool::globalInstance()->setMaxThreadCount(25);

    // Setup the main window
    MainWindow w;
    w.show();

    return app.exec();
}
