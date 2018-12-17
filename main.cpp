#include <iostream>
#include <QApplication>
#include <QTime>
#include "singleapp.h"
#include "mainwindow.h"
#include "debug.h"

QTime g_time;
debug * g_debug = NULL;

int main(int argc, char *argv[])
{
    singleApplication a(argc, argv);

    if (a.appIsRunning() == true) {
        return 0;
    }

    g_debug = new debug(0xff);
    MainWindow w(argc, argv);
    w.show();

    struct share_arg saAndmw;
    saAndmw.p_mw = &w;
    saAndmw.p_sa = &a;
    a.m_thread = std::thread{fetchSharemem, (void *)&saAndmw};

    return a.exec();
}
