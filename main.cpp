#include <iostream>
#include <QApplication>
#include "mainwindow.h"
#include "debug.h"

debug g_debug(0xf);
QTime g_time;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(argc, argv);
    w.show();
    return a.exec();
}
