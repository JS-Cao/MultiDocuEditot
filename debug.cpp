#include "debug.h"

#define BUF_SIZE 256
char g_buf[BUF_SIZE] = {0};
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
    m_logName = "./log/" + QDateTime::currentDateTime().toString("cj's'logyyyy_MMdd_HHmmss");

    try {
        m_logDir    = new QDir("./");
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
    m_logFile->close();
    delete m_logStream;
    delete m_logFile;
    delete m_logDir;
}

/**
* @brief 打印函数
* @param
*   arg1：待打印的字符串
* @return none
* @auther JSCao
* @date   2018-10-28
*/
const char* debug::debugPrint(const char *str, ...)
{
    memset(g_buf, 0, BUF_SIZE);
    va_list va;
    va_start(va, str);
    vsprintf_s(g_buf, BUF_SIZE, str, va);
    va_end(va);

    return g_buf;
}


 /**************************************overide function*************************************/


 /*************************************end of overide function*******************************/
