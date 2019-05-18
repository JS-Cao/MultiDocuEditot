﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
class QMdiArea;
class MyChild;
class QMdiSubWindow;
class QSignalMapper;
class QWidget;
class QTabWidget;
class QLabel;
class QStatusBar;
class QTextEdit;
class QComboBox;
class QFontComboBox;
class QTextCharFormat;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    /* function */
    void createActions(void);
    void readSetting(void);
    void writeSetting(void);
    MyChild *activeMyChild(void);
    QMdiSubWindow *findMyChild(const QString &fileName);
    QWidget *findTagMyChild(const QString &fileName);
    void createStatusBar(QStatusBar *p_statusBar);
    void setupTextActions(void);
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    /* variable */
    QTabWidget *fileTab;
    QLabel *lineLabel;

    QMenu *pfileMenu;
    QAction *pnewAct;
    QAction *popenAct;
    QAction *psaveAct;
    QAction *psaveAsAct;
    QAction *pexitAct;

    QMenu *peditMenu;
    QAction *pundoAct;
    QAction *predoAct;
    QAction *pcutAct;
    QAction *pcopyAct;
    QAction *ppasteAct;

    QMenu *pwindowMenu;
    QSignalMapper *pwinMapper;

    QAction *pseparatorAct;

    QMenu *paboutMenu;
    QAction *paboutQt;
    /* StatusBar */
    QString lineAndColCount;
    QString totalCountStr;
    QLabel *countLabel;
    QLabel *totalLabel;
    /* format */
    QComboBox *pcomboSize;
    QFontComboBox *pcomboFont;

    int totalCount;
    int lineNum;
    int colNum;
    int selectContent;
    int totalLines;

private slots:
    void about();
    void newChild();
    void openFile();
    void fileSave();
    void fileSaveAs();

    void updateMenus();
    void updateWinMenus();
    MyChild *createMyChild();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void aboutQt();

    void lineAndColmessage(void);
    void textTotalCount(void);
    void setWinFileTitle(void);
    void setTitlePostfix(bool isChanged);
    void setActiveTab(const int index);
    void textSize(const QString &p);
    void textFamily(const QString &f);
    void setComboIndex(void);
    void setComboFont(void);
public slots:
    void openAssignFile(QString fileName);
signals:
    void subIdRestore(int id);
};

#endif // MAINWINDOW_H
