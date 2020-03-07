#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QMutexLocker>
#include "debug.h"

QT_BEGIN_NAMESPACE
class MainWindow;
class QLocalServer;
QT_END_NAMESPACE

namespace textedit {

class singleApplication : public QApplication
{
    Q_OBJECT
public:
    // 单例
    static singleApplication *instance(int & argc, char **argv);
    // 析构
    ~singleApplication();

    void set_mw(MainWindow *mw) { m_mw = mw; }

    bool appIsRunning(void) { return isRunning; }

signals:
    void fileName(QString fileName);

private:
    // 构造
    singleApplication(int & argc, char *argv[]);

    // 垃圾工人，回收单例
    class Garbo {
    public:
        // 析构回收
        ~Garbo() {
            qDebug() << "garbage recycle.";
            if (singleApplication::_instance) {
                delete _instance;
            }
        }
    };

    void initLocalServer(void);
    void newLocalServer(void);

private:
    static singleApplication * _instance;
    static QMutex _mutex;
    static Garbo _garbo;

    QLocalServer *m_localServer;
    QString m_serverName;
    bool isRunning;
    MainWindow * m_mw;

private slots:
    void newLocalConnection(void);
};

} // namespace

#endif // SINGLEAPPLICATION_H__
