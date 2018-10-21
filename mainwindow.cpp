#include <cstdio>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <QMdiArea>
#include <QSettings>
#include <QCoreApplication>
#include <QByteArray>
#include <QRect>
#include <QDesktopWidget>
#include <QPoint>
#include <QSize>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <QList>
#include <QString>
#include <QWidget>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QStatusBar>
#include <QTabWidget>
#include <QDebug>
#include <QLabel>
#include <QTextBlock>
#include <QDockWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include "mainwindow.h"
#include "mychild.h"

#pragma execution_character_set("utf-8")

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    mdiArea = new QMdiArea;

    fileTab = new QTabWidget(this);
    setWindowTitle(tr("多文档编辑器"));

    setCentralWidget(fileTab);
    fileTab->setMovable(true);
    fileTab->setContextMenuPolicy(Qt::CustomContextMenu);
    fileTab->setTabsClosable(true);
    fileTab->setTabShape(QTabWidget::Rounded);

    createActions();
    updateMenus();

    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::updateMenus);

    readSetting();

    if (2 == argc) {
        openAssignFile(*(argv + 1));
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window) {
        return;
    }
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::updateMenus()
{
    //qDebug() << __FUNCTION__;
    bool hasMyChild = (activeMyChild() != 0);
    psaveAct->setEnabled(hasMyChild);
    psaveAsAct->setEnabled(hasMyChild);
    ppasteAct->setEnabled(hasMyChild);
    ptileAct->setEnabled(hasMyChild);
    pcascadeAct->setEnabled(hasMyChild);

    bool hasSelection = (activeMyChild() && activeMyChild()->textCursor().hasSelection());
    pcopyAct->setEnabled(hasSelection);
    pcutAct->setEnabled(hasSelection);

    bool hasChanged = (activeMyChild() && activeMyChild()->document()->isUndoAvailable());
    pundoAct->setEnabled(hasChanged);
    hasChanged = (activeMyChild() && activeMyChild()->document()->isRedoAvailable());
    predoAct->setEnabled(hasChanged);

    MyChild *p_childText = qobject_cast<MyChild *>(activeMyChild());
    if (p_childText != NULL) {
        int lineNum = p_childText->document()->lineCount();
        statusBar()->showMessage(tr("Line %1").arg(lineNum), 0);
        //qDebug() << lineNum;
    }
}

MyChild *MainWindow::activeMyChild()
{
    if (QWidget *activeSubWindow = fileTab->currentWidget()) {
        return qobject_cast<MyChild *>(activeSubWindow);
    }

    return NULL;
}

MyChild * MainWindow::createMyChild()
{
    MyChild * child = new MyChild;

    connect(child, &MyChild::copyAvailable, pcutAct,  &QAction::setEnabled);
    connect(child, &MyChild::copyAvailable, pcopyAct, &QAction::setEnabled);
    connect(child, &MyChild::redoAvailable, predoAct, &QAction::setEnabled);
    connect(child, &MyChild::undoAvailable, pundoAct, &QAction::setEnabled);

    return child;
}


void MainWindow::newChild()
{
    //qDebug() << __FUNCTION__;
    MyChild * child = createMyChild();
    child->newFile();
    int index = fileTab->addTab(child, child->pureCurrentFile());
    fileTab->setCurrentIndex(index);
    child->setId(index);
    connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::closefile);
    connect(this, &MainWindow::subIdRestore, child, &MyChild::restoreId);

    child->show();
}

void MainWindow::createActions(void)
{
    // 创建主菜单以及各个子菜单
    // 文件菜单
    pfileMenu = menuBar()->addMenu(tr("文件(&F)"));
    pnewAct = new QAction(tr("新建(&N)"), this);
    pnewAct->setShortcuts(QKeySequence::New);
    pnewAct->setStatusTip(tr("新建文件"));
    connect(pnewAct, &QAction::triggered, this, &MainWindow::newChild);
    pfileMenu->addAction(pnewAct);

    popenAct = new QAction(tr("打开(&O)"), this);
    popenAct->setShortcuts(QKeySequence::Open);
    popenAct->setStatusTip(tr("打开一个文件"));
    connect(popenAct, &QAction::triggered, this, &MainWindow::openFile);
    pfileMenu->addAction(popenAct);

    psaveAct = new QAction(tr("保存(&S)"), this);
    psaveAct->setShortcuts(QKeySequence::Save);
    psaveAct->setStatusTip(tr("保存该文件"));
    connect(psaveAct, &QAction::triggered, this, &MainWindow::fileSave);
    pfileMenu->addAction(psaveAct);

    psaveAsAct = new QAction(tr("另存为(&A)..."), this);
    psaveAsAct->setShortcuts(QKeySequence::SaveAs);
    psaveAsAct->setStatusTip(tr("另存为该文件"));
    connect(psaveAsAct, &QAction::triggered, this, &MainWindow::fileSaveAs);
    pfileMenu->addAction(psaveAsAct);

    pexitAct = new QAction(tr("退出(&X)"), this);
    pexitAct->setShortcuts(QKeySequence::Quit);
    pexitAct->setStatusTip(tr("退出应用程序"));
    connect(pexitAct, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    pfileMenu->addAction(pexitAct);

    // 编辑菜单
    peditMenu = menuBar()->addMenu(tr("编辑(&E)"));
    pundoAct = new QAction(tr("撤销(&U)"), this);
    pundoAct->setShortcuts(QKeySequence::Undo);
    pundoAct->setStatusTip(tr("撤销当前操作"));
    connect(pundoAct, &QAction::triggered, this, &MainWindow::undo);
    peditMenu->addAction(pundoAct);

    predoAct = new QAction(tr("恢复(&R)"), this);
    predoAct->setShortcuts(QKeySequence::Redo);
    predoAct->setStatusTip(tr("恢复上一次操作"));
    connect(predoAct, &QAction::triggered, this, &MainWindow::redo);
    peditMenu->addAction(predoAct);
    peditMenu->addSeparator();

    pcutAct = new QAction(tr("剪切(&T)"), this);
    pcutAct->setShortcuts(QKeySequence::Cut);
    pcutAct->setStatusTip(tr("剪切"));
    connect(pcutAct, &QAction::triggered, this, &MainWindow::cut);
    peditMenu->addAction(pcutAct);

    pcopyAct = new QAction(tr("复制(&C)"), this);
    pcopyAct->setShortcuts(QKeySequence::Copy);
    pcopyAct->setStatusTip(tr("复制"));
    connect(pcopyAct, &QAction::triggered, this, &MainWindow::copy);
    peditMenu->addAction(pcopyAct);

    ppasteAct = new QAction(tr("粘贴(&P)"), this);
    ppasteAct->setShortcuts(QKeySequence::Paste);
    ppasteAct->setStatusTip(tr("粘贴"));
    connect(ppasteAct, &QAction::triggered, this, &MainWindow::paste);
    peditMenu->addAction(ppasteAct);

    // 窗口菜单
    pwindowMenu = menuBar()->addMenu(tr("窗口(&W)"));
    //updateWindowMenu();
    connect(pwindowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);
    ptileAct = new QAction(tr("平铺(&T)"), this);
    ptileAct->setStatusTip(tr("平铺子窗口"));
    //connect(ptileAct, &QAction::triggered, MyMdi, &MyMdi);
    pwindowMenu->addAction(ptileAct);

    pcascadeAct = new QAction(tr("层叠(&C)"), this);
    pcascadeAct->setStatusTip(tr("层叠子窗口"));
    pwindowMenu->addAction(pcascadeAct);
    updateWindowMenu();

    // 帮助菜单
    paboutMenu = menuBar()->addMenu(tr("帮助(A)"));
    QAction *aboutAct = paboutMenu->addAction(tr("关于"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("简要说明"));
}

void MainWindow::updateWindowMenu()
{
    pwindowMenu->clear();
    pwindowMenu->addAction(ptileAct);
    pwindowMenu->addAction(pcascadeAct);
    pwindowMenu->addSeparator();
    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    for (int i = 0; i < windows.size(); i++) {
        MyChild * child = qobject_cast<MyChild *>(windows.at(i)->widget());
        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1).arg(child->pureCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1).arg(child->pureCurrentFile());
        }
        QAction *action = pwindowMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMyChild());

        void (QSignalMapper::*pmap_void)(void) = &QSignalMapper::map;
        connect(action, &QAction::triggered, windowMapper, pmap_void);

        windowMapper->setMapping(action, windows.at(i));
    }
}

void MainWindow::readSetting()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    int Dwidth = QApplication::desktop()->width();
    int Dheight = QApplication::desktop()->height();
    //qDebug() << Dwidth << " " << Dheight;
    QPoint pos = settings.value("pos", QPoint(int(Dwidth * 0.2),int(Dheight * 0.1))).toPoint();
    QSize size = settings.value("size", QSize(int(Dwidth * 0.6), int(Dheight * 0.75))).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSetting()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

/**
  * @brief 设置子窗口是否接收关闭事件
  * @param
  *     arg1：0-接收   1-忽略
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::closeEvent(QCloseEvent *event)
{
    int i = fileTab->count();
    int targetIndex = 0;

    for (; i > 0; i--) {
       MyChild *target = qobject_cast<MyChild *>(fileTab->widget(targetIndex));
       target->setClosedFlag(-1);
       emit fileTab->tabCloseRequested(targetIndex);

       while (-1 == target->closedFlag()) ;
       if (0 == target->closedFlag()) {
           fileTab->removeTab(targetIndex);
           delete target;
       } else {
           emit subIdRestore(targetIndex);
           targetIndex++;
       }
    }

    if (fileTab->count() != 0) {
        event->ignore();
    } else {
        event->accept();
    }

    writeSetting();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("说明"), tr("本<b>多文档编辑器</b>"
                                            "是曹靖松专属\n"
                                            "邮箱：cjsmcu@gmail.com"));
}

void MainWindow::undo()
{
    if (activeMyChild()) {
        activeMyChild()->undo();
    }
}

void MainWindow::redo()
{
    if (activeMyChild()) {
        activeMyChild()->redo();
    }
}

void MainWindow::copy()
{
    if (activeMyChild()) {
        activeMyChild()->copy();
    }
}

void MainWindow::cut()
{
    if (activeMyChild()) {
        activeMyChild()->cut();
    }
}

void MainWindow::paste()
{
    if (activeMyChild()) {
        activeMyChild()->paste();
    }
}

QMdiSubWindow *MainWindow::findMyChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MyChild *myChild = qobject_cast<MyChild *>(window->widget());
        if (myChild->currentFile() == canonicalFilePath) {
            return window;
        }
    }
    return 0;
}

QWidget *MainWindow::findTagMyChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    QWidget *tagSubWindow = NULL;
    for (int i = 0; i < fileTab->count(); i++) {
        tagSubWindow = fileTab->widget(i);
        MyChild *myChild = qobject_cast<MyChild *>(tagSubWindow);
        if (myChild->currentFile() == canonicalFilePath) {
            return tagSubWindow;
        }
    }
    return NULL;
}

void MainWindow::openAssignFile(QString fileName)
{
    if (!fileName.isEmpty()) {
        //QMdiSubWindow *existing = findMyChild(fileName);
        QWidget *existing = findTagMyChild(fileName);
        if (existing) {
            //mdiArea->setActiveSubWindow(existing);
            fileTab->setCurrentWidget(existing);
            return;
        }
        MyChild *child = createMyChild();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("文件已打开"), 2000);
            int index = fileTab->addTab(child, child->pureCurrentFile());
            fileTab->setCurrentIndex(index);
            child->setId(index);
            connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::closefile);
            //connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::updateId);
            child->show();
        } else {
            child->close();
        }
    }
}


void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty()) {
        //QMdiSubWindow *existing = findMyChild(fileName);
        QWidget *existing = findTagMyChild(fileName);
        if (existing) {
            //mdiArea->setActiveSubWindow(existing);
            fileTab->setCurrentWidget(existing);
            return;
        }
        MyChild *child = createMyChild();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("文件已打开"), 2000);
            int index = fileTab->addTab(child, child->pureCurrentFile());
            fileTab->setCurrentIndex(index);
            child->setId(index);
            connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::closefile);
            //connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::updateId);
            child->show();
        } else {
            child->close();
        }
    }
}

void MainWindow::fileSave()
{
    if (activeMyChild() && activeMyChild()->save()) {
        statusBar()->showMessage(tr("文件保存成功"), 2000);
    }
}

void MainWindow::fileSaveAs()
{
    if (activeMyChild() && activeMyChild()->saveAs()) {
        statusBar()->showMessage(tr("文件保存成功"), 2000);
    }
}

void MainWindow::clearFileBit(int index)
{
    clearBit(fileBitIndex, index);
}
