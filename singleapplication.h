#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <thread>
#include <QApplication>
#include <QSharedMemory>

extern void fetchSharemem(void *mw);
QT_BEGIN_NAMESPACE
class MainWindow;
QT_END_NAMESPACE

struct share_arg
{
    class MainWindow *p_mw;
    class singleApplication *p_sa;
};

class singleApplication : public QApplication
{
    Q_OBJECT
public:
    singleApplication(int & argc, char *argv[]);
    ~singleApplication();
    bool appIsRunning(void) { return isRunning; }
    friend void fetchSharemem(void *mw);

    std::thread m_thread;
signals:
    void fileName(QString fileName);
private:
    /* function */
    /* variable */
    QSharedMemory m_sharedMemory;
    bool isRunning;
private slots:
};

#endif // SINGLEAPPLICATION_H__
