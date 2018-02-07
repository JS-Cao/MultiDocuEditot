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
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
private:
    void createActions(void);
    void readSetting(void);
    void writeSetting(void);
    MyChild *activeMyChild(void);
    QMdiSubWindow *findMyChild(const QString &fileName);

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

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

    void updateMenus();
    MyChild *createMyChild();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
};

#endif // MAINWINDOW_H
