#include <cstdio>
#include <cstring>
#include <iostream>
#include <QString>
#include <QBuffer>
#include <QDebug>
#include <QDataStream>
#include <QTime>
#include "singleapplication.h"
#include "mainwindow.h"
#include "debug.h"

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#define _4K (0x1000)
#define NOTESHKEY   "JSMulNote"
#define SHBUFSIZE   (1024 * 4)

static char *g_shareBuff = nullptr;
static bool isOver {false};

/**
  * @brief 构造函数
  * @param
  *     arg1：main函数参数个数
  *     arg2：main函数各个参数
  * @return none
  * @auther JSCao
  * @date   2018-11-24
  */
singleApplication::singleApplication(int & argc, char *argv[])
    :QApplication(argc, argv), m_sharedMemory(QString(NOTESHKEY)), isRunning(false)
{
    int pos = 0, index = 0;
    char *p_temp = nullptr;
    try {
        g_shareBuff  = new char[_4K];
        p_temp       = new char[_4K];
    } catch (std::bad_alloc &memExc) {
        qDebug() << memExc.what();
        printLog(DEBUG, "singleApplication init failed!!!");
        return;
    }

    char *p_tmpShareBuff = nullptr;

    // 1、extract file name from parameter
    pos = sprintf(g_shareBuff, "%d,", argc - 1);
    while (--argc > 0)
    {
        pos += sprintf(g_shareBuff + pos, "%s,", *(argv + (++index)));
    }

    if (m_sharedMemory.isAttached()) {
        if (!m_sharedMemory.detach()) {
            qDebug() << "unable to detach shared memory segment!";
        }
    }

    if (!m_sharedMemory.create(pos + 1)) {
        qDebug() << "Unable to create shared memory segment!";
        if (m_sharedMemory.error() == QSharedMemory::AlreadyExists) {
            if (!m_sharedMemory.attach()) {
                qDebug() << "attach failed!";
            }
            isRunning = true;
            int i = 0;
            while (i < 30) {
                i++;
                m_sharedMemory.lock();

                p_tmpShareBuff = reinterpret_cast<char *>(m_sharedMemory.data());
                memcpy(p_temp, p_tmpShareBuff, static_cast<size_t>(m_sharedMemory.size()));
                index = -1;
                if(pos == 2) {
                     *p_tmpShareBuff = '$';
                     index = 0;
                } else if ((*(p_temp + 1) == ',') && (*(p_temp + 0) == '0')) {
                     // if index = 0 that means share data has been extract
                     memset(p_tmpShareBuff, 0, static_cast<size_t>(m_sharedMemory.size()));
                     memcpy(p_tmpShareBuff, g_shareBuff, static_cast<size_t>(pos + 1));
                     index = 0;
                }
                m_sharedMemory.unlock();
                if (0 == index) {
                    break;
                }
                QTime tarTimer = QTime::currentTime().addMSecs(60);
                while( QTime::currentTime() < tarTimer )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
            }

            m_sharedMemory.detach();
            delete [] p_temp;
            p_temp = nullptr;
        }
    } else {   
        m_sharedMemory.lock();
        char *to = reinterpret_cast<char *>(m_sharedMemory.data());
        memcpy(to, g_shareBuff, static_cast<size_t>(pos + 1));
        m_sharedMemory.unlock();
    }
}

/**
  * @brief 析构函数
  * @param none
  * @return none
  * @auther JSCao
  * @date   2018-12-05
  */
 singleApplication::~singleApplication()
 {
     delete [] g_shareBuff;
     g_shareBuff = nullptr;
     isOver = true;
     if (m_thread.get_id() != std::thread::id {}) {
        m_thread.join();
     }
 }

 /**
   * @brief 线程函数
   * @param 线程参数
   * @return none
   * @auther JSCao
   * @date   2018-12-05
   */
 void fetchSharemem(void *mw)
 {
    struct share_arg *p_arg = reinterpret_cast<struct share_arg *>(mw);
    class singleApplication * p_sa = p_arg->p_sa;
    class MainWindow *p_mw = p_arg->p_mw;

    char str[256] = {0};
    // 必须这样做，如果直接调用openAssignFile会导致所创建窗口在主窗口外部
    p_mw->connect(p_sa, &singleApplication::fileName, p_mw, &MainWindow::openAssignFile);

    while (1) {
        if (isOver == true) {
            break;
        }

        p_sa->m_sharedMemory.lock();
        char * p_tmpShareBuff = reinterpret_cast<char *>(p_sa->m_sharedMemory.data());
        int head  = 0, tail = 0, index = 0;

        while (*(p_tmpShareBuff + head) != ',') {
            if (!isdigit(*p_tmpShareBuff)) {
                break;
            }
            index = *(p_tmpShareBuff + head) - '0' + 10 * index;
            head++;
        }

        if (*p_tmpShareBuff == '$') { // 激活窗口
            p_mw->raise();
            p_mw->activateWindow();
            p_mw->setWindowState((p_mw->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            p_mw->show();
            *p_tmpShareBuff = '0';
        }

        if (index) {
            printLog(DEBUG, "The index is %d\n", index);
            p_mw->raise();
            p_mw->activateWindow();
            p_mw->setWindowState((p_mw->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            p_mw->show();
            /*************************************************************************************
             *   1   23456789a b
             *   ,   test1.txt ,  test2.txt ,
             *   ^           ^              ^
             *  head        tail           head
             * ***********************************************************************************
             */
            while (index--) {
                tail = head + 1;
                while (*(p_tmpShareBuff + tail++) != ',')
                    ;
                memcpy(str, p_tmpShareBuff + head + 1, static_cast<size_t>(tail - head - 2));
                *(str + tail - head - 2) = 0;
                head = tail - 1;
                emit p_sa->fileName(QString::fromLocal8Bit(str));
            }

            *p_tmpShareBuff = '0';
            *(p_tmpShareBuff + 1) = ',';
        }
        p_sa->m_sharedMemory.unlock();

        QTime tarTimer = QTime::currentTime().addMSecs(30);
        while( QTime::currentTime() < tarTimer )
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
 }
