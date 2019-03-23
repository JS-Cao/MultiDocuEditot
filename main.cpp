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
    w.show();
    struct share_arg saAndmw;
    saAndmw.p_mw = &w;
    saAndmw.p_sa = &a;
    void * ptr = narrow_cast<void *>(&saAndmw);
    a.m_thread = std::thread{fetchSharemem, ptr};

    return a.exec();
}
