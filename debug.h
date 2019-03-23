#ifndef DEBUG_H_
#define DEBUG_H_

#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QString>
#include <QDebug>
#include <QtDebug>
#include <QTextStream>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QDir>
#include <QStandardPaths>

#ifdef QT_NO_DEBUG
#define RELEASE_ 1
#else
#define RELEASE_ 0
#endif
class debug;
extern QTime g_time;
extern debug * g_debug;

enum DEBUG_MODE {
    DEBUG   = 0x01 << 0,
    INFO    = 0x01 << 1,
    WARN    = 0x01 << 2,
    ERR     = 0x01 << 3,
    FATAL   = 0x01 << 4,
    RELEASE = 0x01 << 5,
};

/* Usage：
 *  printLog(DEBUG, "The DEBUG is starting......");
 *  printLog(INFO, "The INFO is starting......");
 *  printLog(WARN, "The WARN is starting......");
 *  printLog(ERROR, "The ERROR is starting......");
 *  printLog(FATAL, "The FATAL is starting......");
 *  printLog(RELEASE, "The RELEASE is starting......");
 */
#define printLog(mode,str,...) \
    if (g_debug->getLevel() & (mode)) {\
        if (!RELEASE_) {\
            qDebug() << __FUNCTION__ << "(" << __LINE__ << ")," \
                     << g_debug->debugPrint(str,##__VA_ARGS__);\
        } else\
        {\
            *g_debug->getOutStream() << g_time.currentTime().toString("[hh:mm:ss(zzz)] ")\
                                     << __FUNCTION__ << "(" << __LINE__ << ")," \
                                     << g_debug->debugPrint(str,##__VA_ARGS__) << "\n";\
            g_debug->logFlush();\
        }\
    }


class debug
{
public:
    debug(const unsigned int level);
    ~debug();

    void setLevel(const unsigned int level)
    {
        m_level = level;
    }
    unsigned int getLevel(void) { return m_level; }
    QTextStream *getOutStream(void) {return m_logStream; }
    const char* debugPrint(const char *,...);
    void logFlush(void);
    void clearLogFile(int numDaysAgo = 30);

private:
    unsigned int m_level;    // debug level
    QString m_logName;
    QFile *m_logFile;
    QTextStream *m_logStream;
    QDir *m_logDir;
};

#endif // DEBUG_H
