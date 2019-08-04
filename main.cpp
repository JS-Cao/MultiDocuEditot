#include <iostream>
#include "mainwindow.h"
#include "singleapplication.h"
#include "debug.h"
#include "common.h"

int main(int argc, char *argv[])
{
    singleApplication a(argc, argv);
    if (a.appIsRunning() == true) {
        return 0;
    }
    g_debug = new debug(0xff);
    MainWindow w;
    a.m_mw = &w;
    w.show();

    return a.exec();
}


