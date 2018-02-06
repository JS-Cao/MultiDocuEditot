#ifndef MYCHILD_H
#define MYCHILD_H

#include <QWidget>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class MyChild : public QTextEdit
{
    Q_OBJECT
public:
    MyChild();
    void newFile();     // 新建文件

    QString pureCurrentFile();
    QString currentFile() { return curFile; }

protected:
    void closeEvent(QCloseEvent *event);
private:

    QString strippedName(const QString &fullFileName);
    QString curFile;
    bool isUntitled;
private slots:
    void documentWasModified();
};

#endif // MYCHILD_H
