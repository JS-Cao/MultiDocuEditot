#ifndef MYCHILD_H
#define MYCHILD_H

#include <QWidget>
#include <QTextEdit>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
class QTextCursor;
QT_END_NAMESPACE

class MyChild : public QTextEdit
{
    Q_OBJECT
public:
    MyChild(QWidget *parent);
    ~MyChild();
    void newFile();     // 新建文件
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    void setId(const int Id);
    int fileId(void);

    void setClosedFlag(int flag);
    int closedFlag(void);
    QString pureCurrentFile();
    QString currentFile() { return curFile; }
    void lineNumberAreaPaintEvent(QPaintEvent *event);  // draw line number
    int lineNumberAreaWidth(void);
protected:
    void closeEvent(QCloseEvent *event);
    //void resizeEvent(QResizeEvent *event) override;
private:
    bool idIsChanged;
    bool isRichtext;
    int myId;
    int isReceivedClosedFlag;
    double scrollToBlockStep;  // 滚动条到步长的计算
    int scrollToBlockNum;   // 滚动条到blockNum的转换
    int blockNumsPerPage;   // 每页有多少块
    QString strippedName(const QString &fullFileName);
    void setCurrentFile(const QString &fileName);
    bool maybeSave();

    QString curFile;
    QWidget *lineNumberArea;    // show line numbers
    bool isUntitled;
private slots:
    void documentWasModified();
    void updateLineNumberAreaWidth();
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void _updateLineNumberArea();
    void scrollMapToBlock(int, int max);
    void FirstVisibleBlockNum(int index);
public slots:
    void closefile(int index);
    void restoreId(const int id);
};

#endif // MYCHILD_H
