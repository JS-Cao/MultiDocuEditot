#include <iostream>
#include "mainwindow.h"
#include "singleapplication.h"
#include "debug.h"
#include "common.h"
#include <QTest>

int main(int argc, char *argv[])
{
    textedit::singleApplication * p_sa = textedit::singleApplication::instance(argc, argv);
    if (p_sa->appIsRunning() == true) {
        return 0;
    }

    g_debug = new debug(0xff);
    MainWindow w;
    p_sa->set_mw(&w);
    w.show();

    return p_sa->exec();
}


