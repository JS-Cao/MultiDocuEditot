#include <iostream>
#include <QApplication>
#include <QTime>
#include "mainwindow.h"
#include "debug.h"

QTime g_time;
debug * g_debug = NULL;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    g_debug = new debug(0xff);
    MainWindow w(argc, argv);
    w.show();
    return a.exec();
}
