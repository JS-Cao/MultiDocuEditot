#include <iostream>
#include <QApplication>
#include <QTime>
#include "mainwindow.h"
#include "debug.h"

QTime g_time;
debug g_debug(0xff);

int main(int argc, char *argv[])
{
    printLog(DEBUG, "start application......");
    QApplication a(argc, argv);
    MainWindow w(argc, argv);
    w.show();
    return a.exec();
}
