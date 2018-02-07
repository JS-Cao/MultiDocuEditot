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
#include "mainwindow.h"
#include "mychild.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    setWindowTitle(tr("多文档编辑器"));
    createActions();
    updateMenus();

    windowMapper = new QSignalMapper(this);
    void (QSignalMapper::*pmapped_qwidget)(QWidget *) = &QSignalMapper::mapped;
    connect(windowMapper, pmapped_qwidget, this, &MainWindow::setActiveSubWindow);

    connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenus);
    readSetting();
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
    bool hasMyChild = (activeMyChild() != 0);
    psaveAct->setEnabled(hasMyChild);
    psaveAsAct->setEnabled(hasMyChild);
    ppasteAct->setEnabled(hasMyChild);
    ptileAct->setEnabled(hasMyChild);
    pcascadeAct->setEnabled(hasMyChild);

    bool hasSelection = (activeMyChild() && activeMyChild()->textCursor().hasSelection());
    pcopyAct->setEnabled(hasSelection);
    pcutAct->setEnabled(hasSelection);
}

MyChild *MainWindow::activeMyChild()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow()) {
        return qobject_cast<MyChild *>(activeSubWindow->widget());
    }
    return 0;
}

MyChild * MainWindow::createMyChild()
{
    MyChild * child = new MyChild;
    mdiArea->addSubWindow(child);
    connect(child, &MyChild::copyAvailable, pcutAct, &QAction::setEnabled);
    connect(child, &MyChild::copyAvailable, pcopyAct, &QAction::setEnabled);
    return child;
}


void MainWindow::newChild()
{
    MyChild * child = createMyChild();
    child->newFile();
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
    //connect(psaveAct, &QAction::triggered, this, &MyMdi::saveFile);
    pfileMenu->addAction(psaveAct);

    psaveAsAct = new QAction(tr("另存为(&A)..."), this);
    psaveAsAct->setShortcuts(QKeySequence::SaveAs);
    psaveAsAct->setStatusTip(tr("另存为该文件"));
    //connect(psaveAsAct, &QAction::triggered, this, &MyMdi::saveFile);
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
    predoAct->setShortcuts(QKeySequence::Undo);
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

    ppasteAct = new QAction(tr("粘贴"), this);
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
    /*const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 2, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }*/
    QPoint pos = settings.value("pos", QPoint(200,200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSetting()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    //settings.value("geometry", saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        event->accept();
    }
    writeSetting();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("说明"), tr("本<b>多文档编辑器</b>"
                                            "这是本人的第一个编辑器"));
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

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        QMdiSubWindow *existing = findMyChild(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }
        MyChild *child = createMyChild();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("文件已打开"), 2000);
            child->show();
        } else {
            child->close();
        }
    }
}
