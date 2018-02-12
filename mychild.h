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
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);

    QString pureCurrentFile();
    QString currentFile() { return curFile; }

protected:
    void closeEvent(QCloseEvent *event);
private:

    QString strippedName(const QString &fullFileName);
    void setCurrentFile(const QString &fileName);
    bool maybeSave();

    QString curFile;
    bool isUntitled;
private slots:
    void documentWasModified();
};

#endif // MYCHILD_H
