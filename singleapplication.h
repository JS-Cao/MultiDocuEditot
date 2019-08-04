#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>

QT_BEGIN_NAMESPACE
class MainWindow;
class QLocalServer;
QT_END_NAMESPACE

class singleApplication : public QApplication
{
    Q_OBJECT
public:
    singleApplication(int & argc, char *argv[]);
    ~singleApplication();
    bool appIsRunning(void) { return isRunning; }
    MainWindow * m_mw;
signals:
    void fileName(QString fileName);
private:
    /* function */
    void initLocalServer(void);
    void newLocalServer(void);
    /* variable */
    QLocalServer *m_localServer;
    QString m_serverName;
    bool isRunning;
private slots:
    void newLocalConnection(void);
};

#endif // SINGLEAPPLICATION_H__
