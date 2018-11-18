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
#include <QLineEdit>
#include <QPlainTextEdit>
#include "mainwindow.h"
#include "mychild.h"
#include "debug.h"

#pragma execution_character_set("utf-8")

#define LINECOLCOUNT "Line:%d\tCol:%d\tsel(%d)\t"
#define TOTALCOUNT "Total:%d"

extern debug g_debug;

/**
  * @brief 构造函数
  * @param
  *     arg1：main函数参数个数
  *     arg2：main函数各个参数
  *     arg3：父组件
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    printLog(DEBUG, "starting mainwindow......");
    setWindowTitle(tr("多文档编辑器"));

    try {
        fileTab = new QTabWidget(this);
        // create menubar
        createActions();
    }
    catch(std::bad_alloc &memExc) {
        qDebug() << memExc.what();
        printLog(DEBUG, "mainwindow init failed!!!");
        return;
    }

    setCentralWidget(fileTab);
    fileTab->setMovable(true);
    fileTab->setContextMenuPolicy(Qt::CustomContextMenu);
    fileTab->setTabsClosable(true);
    fileTab->setTabShape(QTabWidget::Rounded);

    updateMenus();

    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::updateMenus);

    readSetting();

    // 如果参数个数等于2表示该程序通过某一文本文件打开，并将该文本文件路径传递给该应用
    if (2 == argc) {
        openAssignFile(*(argv + 1));
    }
    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::setWinFileTitle);
    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::textTotalCount);
    createStatusBar(statusBar());

    printLog(DEBUG, "starting mainwindow success......");
}

/**
  * @brief 析构函数
  * @param none
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
MainWindow::~MainWindow()
{
    delete fileTab;
}

/**
  * @brief 创建各个主菜单和子菜单
  * @param none
  * @return none
  * @auther JSCao
  * @date   2018-11-13
  */
void MainWindow::createStatusBar(QStatusBar *p_statusBar)
{
    lineNum = 1;
    colNum  = 1;
    selectContent = 0;
    totalCount = 0;
    lineAndColCount.sprintf(LINECOLCOUNT,lineNum, colNum, selectContent);
    countLabel = new QLabel(lineAndColCount, this);
    p_statusBar->addPermanentWidget(countLabel, 0);
    totalCountStr.sprintf(TOTALCOUNT, totalCount);
    totalLabel = new QLabel(totalCountStr, this);
    p_statusBar->addPermanentWidget(totalLabel, 0);
    printLog(DEBUG, "statusBar init success.");
}

/**
  * @brief 创建各个主菜单和子菜单
  * @param none
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::createActions(void)
{
    // create main menus and sub menus
    // file menu
    pfileMenu = menuBar()->addMenu(tr("文件(&F)"));
    pnewAct   = new QAction(tr("新建(&N)"), this);
    pnewAct->setShortcuts(QKeySequence::New);
    pnewAct->setStatusTip(tr("新建文件"));
    connect(pnewAct, &QAction::triggered, this, &MainWindow::newChild);
    pfileMenu->addAction(pnewAct);

    popenAct = new QAction(tr("打开(&O)"), this);
    popenAct->setShortcuts(QKeySequence::Open);
    popenAct->setStatusTip(tr("打开文件"));
    connect(popenAct, &QAction::triggered, this, &MainWindow::openFile);
    pfileMenu->addAction(popenAct);

    psaveAct = new QAction(tr("保存(&S)"), this);
    psaveAct->setShortcuts(QKeySequence::Save);
    psaveAct->setStatusTip(tr("保存文件"));
    connect(psaveAct, &QAction::triggered, this, &MainWindow::fileSave);
    pfileMenu->addAction(psaveAct);

    psaveAsAct = new QAction(tr("另存为(&A)..."), this);
    psaveAsAct->setShortcuts(QKeySequence::SaveAs);
    psaveAsAct->setStatusTip(tr("另存为文件"));
    connect(psaveAsAct, &QAction::triggered, this, &MainWindow::fileSaveAs);
    pfileMenu->addAction(psaveAsAct);

    pexitAct = new QAction(tr("退出(&X)"), this);
    pexitAct->setShortcuts(QKeySequence::Quit);
    pexitAct->setStatusTip(tr("退出应用程序"));
    connect(pexitAct, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    pfileMenu->addAction(pexitAct);

    // edit menu
    peditMenu = menuBar()->addMenu(tr("编辑(&E)"));
    pundoAct  = new QAction(tr("撤销(&U)"), this);
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

    // window menu
    pwindowMenu = menuBar()->addMenu(tr("窗口(&W)"));

    // 帮助菜单
    paboutMenu = menuBar()->addMenu(tr("帮助(&H)"));
    QAction *aboutAct = paboutMenu->addAction(tr("关于"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("简要说明"));
    paboutQt = new QAction(tr("关于Qt"), this);
    paboutMenu->addAction(paboutQt);
    connect(paboutQt, &QAction::triggered, this, &MainWindow::aboutQt);
}

/**
  * @brief 返回当前标签页下活跃子窗口
  * @param none
  * @return
  *        成功--当前活跃子窗口
  *        失败--NULL
  * @auther JSCao
  * @date   2018-08-25
  */
MyChild *MainWindow::activeMyChild()
{
    if (QWidget *activeSubWindow = fileTab->currentWidget()) {
        return qobject_cast<MyChild *>(activeSubWindow);
    }

    return NULL;
}

/**
  * @brief  更新各个子菜单状态
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::updateMenus()
{
    MyChild *p_activeSubWin = activeMyChild();
    bool hasMyChild = (p_activeSubWin != NULL);

    psaveAct->setEnabled(hasMyChild);
    psaveAsAct->setEnabled(hasMyChild);
    ppasteAct->setEnabled(hasMyChild);

    bool hasSelection = (p_activeSubWin && p_activeSubWin->textCursor().hasSelection());
    pcopyAct->setEnabled(hasSelection);
    pcutAct->setEnabled(hasSelection);

    bool hasChanged = (p_activeSubWin && p_activeSubWin->document()->isUndoAvailable());
    pundoAct->setEnabled(hasChanged);
    hasChanged = (p_activeSubWin && p_activeSubWin->document()->isRedoAvailable());
    predoAct->setEnabled(hasChanged);
    printLog(DEBUG, "update menu success......");
}

/**
  * @brief 读取初始化配置
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::readSetting()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    int Dwidth = QApplication::desktop()->width();
    int Dheight = QApplication::desktop()->height();
    QPoint pos = settings.value("pos", QPoint(int(Dwidth * 0.2),int(Dheight * 0.1))).toPoint();
    QSize size = settings.value("size", QSize(int(Dwidth * 0.6), int(Dheight * 0.75))).toSize();
    move(pos);
    resize(size);
    printLog(DEBUG, "read configuration finish.");
}

/**
  * @brief 写入配置以供下回程序启动时读取（待完善）
  * @param
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::writeSetting()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}


/****************************** Place slot functiong in here ***************************************/

/**
  * @brief 【slot】设置窗口标题后缀
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-11-18
  */
void MainWindow::setTitlePostfix(bool isChanged)
{
    setWindowModified(isChanged);
}
/**
  * @brief 【slot】设置窗口标题
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-11-18
  */
void MainWindow::setWinFileTitle(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (!p_activeSubWin) {
        return;
    }
    bool isChanged = p_activeSubWin->document()->isModified();
    setWindowModified(isChanged);
    setWindowTitle(p_activeSubWin->currentFile() + tr("[*]"));
}

/**
  * @brief 【slot】更新状态栏信息
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-11-16
  */
void MainWindow::textTotalCount(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (!p_activeSubWin) {
        return;
    }
    totalCount = p_activeSubWin->textCursor().document()->characterCount() - 1;
    totalCountStr.sprintf(TOTALCOUNT, totalCount);
    totalLabel->setText(totalCountStr);
}
/**
  * @brief 【slot】更新状态栏信息
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-11-14
  */
void MainWindow::lineAndColmessage(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (!p_activeSubWin) {
        return;
    }
    QTextCursor curTextCursor = p_activeSubWin->textCursor();
    lineNum = curTextCursor.blockNumber() + 1;
    colNum  = curTextCursor.columnNumber() + 1;
    selectContent = curTextCursor.selectionEnd() - curTextCursor.selectionStart();
    lineAndColCount.sprintf(LINECOLCOUNT,lineNum, colNum, selectContent);
    countLabel->setText(lineAndColCount);
}
/**
  * @brief 【slot】创建新的子窗口
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-08-12
  */
MyChild * MainWindow::createMyChild()
{
    MyChild * child = new MyChild;
    if (!child) {
        printLog(DEBUG, "create child failed!");
    }

    connect(child, &MyChild::copyAvailable, pcutAct,  &QAction::setEnabled);
    connect(child, &MyChild::copyAvailable, pcopyAct, &QAction::setEnabled);
    connect(child, &MyChild::redoAvailable, predoAct, &QAction::setEnabled);
    connect(child, &MyChild::undoAvailable, pundoAct, &QAction::setEnabled);
    connect(child, &MyChild::undoAvailable, this, &MainWindow::setTitlePostfix);
    connect(child, &MyChild::selectionChanged, this, &MainWindow::lineAndColmessage);
    connect(child, &MyChild::textChanged, this, &MainWindow::textTotalCount);
    connect(child, &MyChild::cursorPositionChanged, this, &MainWindow::lineAndColmessage);

    return child;
}

/**
  * @brief 【slot】创建新的子窗口
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-08-12
  */
void MainWindow::newChild()
{
    MyChild * child = createMyChild();
    child->newFile();
    int index = fileTab->addTab(child, child->pureCurrentFile());
    fileTab->setCurrentIndex(index);
    child->setId(index);
    connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::closefile);
    connect(this, &MainWindow::subIdRestore, child, &MyChild::restoreId);

    child->show();
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("说明"), tr("<b>多文档编辑器</b><br>"
                                            "<pre><b>作者：</b>    曹靖松<br>"
                                            "<b>邮箱：</b>    cjsmcu@gmail.com<br>"
                                            "<b>项目链接：</b> <a href=\"https://github.com/JS-Cao/MultiDocuEditot\">https://github.com/JS-Cao/MultiDocuEditot</a></pre>"));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
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

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this);

    if (!fileName.isEmpty()) {
        QWidget *existing = findTagMyChild(fileName);
        if (existing) {
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
/*****************************end of slot function**************************************************/

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

/**
  * @brief 打开指定路径文件
  * @param
  *     arg:    指定文件路径及文件名
  * @return none
  * @auther JSCao
  * @date   2018-10-21
  */
void MainWindow::openAssignFile(QString fileName)
{
    printLog(DEBUG, "start opening file %s.", (const char *)fileName.toUtf8());

    if (!fileName.isEmpty()) {
        QWidget *existing = findTagMyChild(fileName);
        if (existing) {
            fileTab->setCurrentWidget(existing);
            printLog(DEBUG, "setcurrentwidget.");
            return;
        }

        MyChild *child = createMyChild();
        if (!child) {
            printLog(DEBUG, "open file failed!");
        }
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("文件已打开"), 2000);
            int index = fileTab->addTab(child, child->pureCurrentFile());
            fileTab->setCurrentIndex(index);
            child->setId(index);
            connect(fileTab, &QTabWidget::tabCloseRequested, child, &MyChild::closefile);
            bool isChanged = child->document()->isModified();
            setWindowModified(isChanged);
            setWindowTitle(child->currentFile() + tr("[*]"));
            child->show();
        } else {
            child->close();
        }
    }
}

