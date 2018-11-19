#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <QFile>
#include <QString>
#include <QDebug>
#include <QtDebug>
#include <QTextStream>
#include <QDateTime>
#include <QTime>
#include <QDir>
#include <QStandardPaths>
#include "error.h"

#ifdef QT_NO_DEBUG
#define _RELEASE 1
#else
#define _RELEASE 0
#endif

extern QTime g_time;

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
    if (g_debug.getLevel() & (mode)) {\
        if (!_RELEASE) {\
            qDebug() << __FUNCTION__ << "(" << __LINE__ << ")," \
                     << g_debug.debugPrint(str,__VA_ARGS__);\
        } else\
        {\
            *g_debug.getOutStream() << g_time.currentTime().toString("[hh:mm:ss(zzz)] ")\
                                    << __FUNCTION__ << "(" << __LINE__ << ")," \
                                    << g_debug.debugPrint(str,__VA_ARGS__) << "\n";\
            g_debug.logFlush();\
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
    int getLevel(void) { return m_level; }
    QTextStream *getOutStream(void) {return m_logStream; }
    const char* debugPrint(const char *,...);
    void logFlush(void);

private:
    unsigned int m_level;    // debug level
    QString m_logName;
    QFile *m_logFile;
    QTextStream *m_logStream;
    QDir *m_logDir;
};

#endif // DEBUG_H
