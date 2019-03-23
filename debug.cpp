#include <QCoreApplication>
#include "debug.h"

#define BUF_SIZE 256
static char g_buf[BUF_SIZE] = {0};
debug * g_debug = nullptr;
QTime g_time;

/**
  * @brief 构造函数
  * @param
  *     arg1：调试等级
  * @return none
  * @auther JSCao
  * @date   2018-10-28
  */
debug::debug(const unsigned int level)
{
    m_level = level;
    m_logName = QCoreApplication::applicationDirPath() + "/log/" + QDateTime::currentDateTime().toString("cj's'logyyyy_MMdd_HHmmss");

    try {
        m_logDir    = new QDir(QCoreApplication::applicationDirPath());
        m_logFile   = new QFile(m_logName);
        m_logStream = new QTextStream(m_logFile);
    } catch(std::bad_alloc &memExc) {
        qDebug() << memExc.what();
        return;
    }
    if (!m_logDir->exists(QString("log"))) {
        m_logDir->mkdir(QString("log"));
    }

    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text))
        return;
}

/**
* @brief 析构函数
* @param
* @return none
* @auther JSCao
* @date   2018-10-28
*/
debug::~debug(void)
{
    clearLogFile();
    m_logFile->close();
    delete m_logStream;
    delete m_logFile;
    delete m_logDir;
}

void debug::logFlush(void)
{
    m_logStream->flush();
}

/**
* @brief 清理日志文件函数
* @param
*   arg1：清理几天前的日志文件，0代表全部清理
* @return none
* @auther JSCao
* @date   2019-3-23
*/
void debug::clearLogFile(int numDaysAgo)
{
    bool starClear = false;
    qint32 currentYears = QDate::currentDate().year(), currentDays = QDate::currentDate().dayOfYear();
    qint32 days = 0;

    printLog(INFO, "start clear logs before %d days.", numDaysAgo);
    if (numDaysAgo >= 366) {
        printLog(ERR, "The days(%d) is out of range.", numDaysAgo);
    }

    m_logDir->cd(QString("log"));
    QFileInfoList logFileList = m_logDir->entryInfoList(QDir::Files, QDir::Time);
    QFileInfoList::const_iterator logFileIt;

    for (logFileIt = logFileList.begin(); logFileIt != logFileList.end(); ++logFileIt) {
        if (false == starClear) {
            QDate dateTmp = logFileIt->birthTime().date();
            if ((currentYears == dateTmp.year()) &&
                (dateTmp.daysTo(QDate::currentDate()) >= numDaysAgo)) {
                    starClear = true;
                    m_logDir->remove(logFileIt->canonicalFilePath());
            }
            else if (currentYears != dateTmp.year()) {
                if (dateTmp.isLeapYear(dateTmp.year())) {
                    days = 366 - dateTmp.dayOfYear() + currentDays;
                }
                else {
                    days = 365 - dateTmp.dayOfYear() + currentDays;
                }
                if (days >= numDaysAgo)
                    starClear = true;
            }
        }
        else {
            m_logDir->remove(logFileIt->canonicalFilePath());
            printLog(INFO, "remove file %s.", logFileIt->canonicalFilePath().toLocal8Bit());
        }
    }
}


/**
* @brief 打印函数
* @param
*   arg1：待打印的字符串
* @return none
* @auther JSCao
* @date   2018-10-28
*/
#ifdef linux
__attribute__((__format__ (__printf__, 2, 0)))
#endif
const char* debug::debugPrint(const char *str, ...)
{
    memset(g_buf, 0, BUF_SIZE);
    va_list va;
    va_start(va, str);
#ifdef WIN32
    vsprintf_s(g_buf, BUF_SIZE, str, va);
#endif
#ifdef linux
    vsprintf(g_buf, str, va);
#endif
    va_end(va);

    return g_buf;
}


 /**************************************overide function*************************************/


 /*************************************end of overide function*******************************/
