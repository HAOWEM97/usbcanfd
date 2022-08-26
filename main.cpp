#include "mainwindow.h"
#include "canfdinterface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    canfdinterface canfd;
    canfd.start();
    MainWindow w;
    w.show();
    return a.exec();
}
