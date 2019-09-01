#include <cstdio>
#include <cstring>
#include <iostream>
#include <QString>
#include <QDebug>
#include <QDataStream>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include "singleapplication.h"
#include "mainwindow.h"
#include "debug.h"
#include "common.h"

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

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
    :QApplication(argc, argv), isRunning(false), m_localServer(nullptr), m_mw(nullptr)
{
    m_serverName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    qDebug() << "localservername: " << m_serverName;
    initLocalServer();
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
     if (m_localServer) {
         m_localServer->close();
         delete m_localServer;
     }
     if (g_debug) {
         delete g_debug;
     }
 }

 /**
   * @brief 初始化localserver
   * @param none
   * @return none
   * @auther JSCao
   * @date   2019-08-04
   */
 void singleApplication::initLocalServer(void)
 {
    QLocalSocket writeEndpoint;
    writeEndpoint.connectToServer(m_serverName);
    if (writeEndpoint.waitForConnected(250)) {  // 成功连接到服务器
        isRunning = true;
        QTextStream stream(&writeEndpoint);
        QStringList args = QCoreApplication::arguments();

        if (args.empty())
            stream << QString();
        else {
            for (auto p = ++args.begin(); p != args.end(); p++) {
                qDebug() << "write: "<< *p;
                stream << *p << ";";
            }
        }
        stream.flush();

        writeEndpoint.waitForBytesWritten();
        writeEndpoint.close();
        return;
    }

    newLocalServer();
 }

 /**
   * @brief 创建新的服务器
   * @param none
   * @return none
   * @auther JSCao
   * @date   2019-08-04
   */
 void singleApplication::newLocalServer(void)
 {
    m_localServer = new QLocalServer(this);
    connect(m_localServer, &QLocalServer::newConnection, this, &singleApplication::newLocalConnection);
    if (!m_localServer->listen(m_serverName) &&
            (m_localServer->serverError() == QAbstractSocket::AddressInUseError)) {
        QLocalServer::removeServer(m_serverName); // 程序上次退出有残留
        m_localServer->listen(m_serverName);
    }
 }

 /**
   * @brief 处理连接请求
   * @param none
   * @return none
   * @auther JSCao
   * @date   2019-08-04
   */
 void singleApplication::newLocalConnection(void)
 {
    QLocalSocket * readEndpoint = m_localServer->nextPendingConnection();
    if (!readEndpoint) {
        return;
    }
    connect(readEndpoint, &QLocalSocket::disconnected, readEndpoint, &QLocalSocket::deleteLater);
    readEndpoint->waitForReadyRead();
    QByteArray param = readEndpoint->readAll();
    if (!param.isEmpty()){
        m_mw->connect(this, &singleApplication::fileName, m_mw, &MainWindow::openAssignFile);
        std::string str = param.toStdString();
        std::string::size_type first = 0, second = 0;
        while (second != std::string::npos) {
            first  = second;
            second = str.find(';', first);
            std::string subStr = str.substr(first, second - first);
            if (!subStr.empty()) {
                emit fileName(QString::fromLocal8Bit(subStr.c_str()));
                second++;
            }
        }
        disconnect(this, &singleApplication::fileName, m_mw, &MainWindow::openAssignFile);
    }

    if (m_mw) {
        m_mw->raise();
        m_mw->activateWindow();
        m_mw->setWindowState((m_mw->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        m_mw->show();
    }
    readEndpoint->disconnectFromServer();
    disconnect(readEndpoint, &QLocalSocket::disconnected, readEndpoint, &QLocalSocket::deleteLater);
    delete readEndpoint;
 }
