#ifndef MAINWINDOW_H
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
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    inline void setBit(unsigned int &index, const int bit)
    {
        index |= ((unsigned int)1 << bit);
    }
    inline void clearBit(unsigned int &index, const int bit)
    {
        index &= ~((unsigned int)1 << bit);
    }
    inline bool isSet(unsigned int &index, const int bit)
    {
        return index & ((unsigned int)1 << bit);
    }
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
private:
    unsigned int fileBitIndex;

    void createActions(void);
    void readSetting(void);
    void writeSetting(void);
    MyChild *activeMyChild(void);
    QMdiSubWindow *findMyChild(const QString &fileName);
    QWidget *findTagMyChild(const QString &fileName);

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

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
    QAction *ptileAct;
    QAction *pcascadeAct;

    QAction *pseparatorAct;

    QMenu *paboutMenu;

private slots:
    void about();
    void newChild();
    void openFile();
    void fileSave();
    void fileSaveAs();

    void updateMenus();
    MyChild *createMyChild();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void clearFileBit(int index);
signals:
    void subIdRestore(int id);
};

#endif // MAINWINDOW_H
