#include <cstdio>
#include <algorithm>
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
#include <QStringList>
#include <QStringListIterator>
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
#include <QVariant>
#include <QComboBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QTextCharFormat>
#include <QToolBar>
#include <QSizePolicy>
#include <QLayout>
#include <QPixmap>
#include <QColor>
#include <QColorDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTextList>
#include <QMouseEvent>
#include <QEvent>
#include "mainwindow.h"
#include "mychild.h"
#include "debug.h"
#include "common.h"

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#define LINECOLCOUNT "Line:%d\tCol:%d\tsel(%d)\t"
#define TOTALCOUNT   " Total:%d  lines:%d "

#define WINSIZE      "windowSize"
#define NUMOFRESLAB  "NumOfResLabels"

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
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    printLog(DEBUG, "starting mainwindow......");
    totalCount = 0;
    totalLines = 0;
    setWindowTitle(tr("多文档编辑器"));

    try {
        fileTab = new QTabWidget(this);
        fileTab->installEventFilter(this);
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

    QCoreApplication::setOrganizationName("JS-Cao");
    //QCoreApplication::setOrganizationDomain("mysoft.com");
    QCoreApplication::setApplicationName("myNote");

    createStatusBar(statusBar());
    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::setWinFileTitle);
    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::textTotalCount);
    connect(fileTab, &QTabWidget::currentChanged, this, &MainWindow::lineAndColmessage);
    printLog(DEBUG, "starting mainwindow success......");
    printLog(DEBUG, "program path is: %s,app run path is: %s", narrow_cast<const char *>(QCoreApplication::applicationDirPath().toUtf8()), narrow_cast<const char *>(QDir::currentPath().toUtf8()));
    readSetting();
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
  * @brief 字体菜单栏对齐方式改变
  * @param none
  * @return none
  * @auther JSCao
  * @date   2019-07-22
  */
void MainWindow::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        pleft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        pcenter->setChecked(true);
    else if (a & Qt::AlignRight)
        pright->setChecked(true);
    else if (a & Qt::AlignJustify)
        pjustify->setChecked(true);
}

/**
  * @brief 字体菜单栏格式改变
  * @param none
  * @return none
  * @auther JSCao
  * @date   2019-07-22
  */
void MainWindow::fontChanged(const QFont & f)
{
    // 之所以需要手动设置是因为当用户点击action时，
    // 其会自动根据ischecked来设置setChecked（若为真设为假，若为假设为真，可以理解为状态反转）
    // 因此我们需要根据光标选择的内容手动更改
    pbold->setChecked(f.bold());
    pitalic->setChecked(f.italic());
    punderline->setChecked(f.underline());
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

    lineAndColCount.sprintf(LINECOLCOUNT,lineNum, colNum, selectContent);
    countLabel = new QLabel(lineAndColCount, this);
    p_statusBar->addPermanentWidget(countLabel, 0);
    totalCountStr.sprintf(TOTALCOUNT, totalCount, totalLines);
    totalLabel = new QLabel(totalCountStr, this);
    p_statusBar->addPermanentWidget(totalLabel, 0);
    printLog(DEBUG, "statusBar init success.");
}

/**
  * @brief 融合修改
  * @param none
  * @return none
  * @auther caojingsong
  * @date   2019-5-2
  */
void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (!p_activeSubWin) {
        return;
    }
    QTextCursor cursor = p_activeSubWin->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format); // 对光标选中的部分进行融合
    p_activeSubWin->mergeCurrentCharFormat(format);
}

/**
  * @brief 创建字体编辑行为
  * @param none
  * @return none
  * @auther JSCao
  * @date   2019-5-2
  */
void MainWindow::setupTextActions(void)
{
    QToolBar *tb = addToolBar(tr("Format"));
    // 设置工具栏可以移动到的区域
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    //addToolBar(tb);
    pcomboStyle = new QComboBox(tb);
    tb->addWidget(pcomboStyle);
    pcomboStyle->addItem(tr("Standard"));
    pcomboStyle->addItem(tr("Bullet List (Disc)"));
    pcomboStyle->addItem(tr("Bullet List (Circle)"));
    pcomboStyle->addItem(tr("Bullet List (Square)"));
    pcomboStyle->addItem(tr("Ordered List (Decimal)"));
    pcomboStyle->addItem(tr("Ordered List (Alpha lower)"));
    pcomboStyle->addItem(tr("Ordered List (Alpha upper)"));
    pcomboStyle->addItem(tr("Ordered List (Roman lower)"));
    pcomboStyle->addItem(tr("Ordered List (Roman upper)"));
    connect(pcomboStyle, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::textStyle);

    pcomboFont = new QFontComboBox;
    tb->addWidget(pcomboFont);
    connect(pcomboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textFamily);

    // create menubar
    pcomboSize = new QComboBox(tb);
    tb->addWidget(pcomboSize);
    pcomboSize->setObjectName("comboSize");
    pcomboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();

    std::for_each(standardSizes.begin(), standardSizes.end(),
                  [this](const int& size){ pcomboSize->addItem(QString::number(size)); });
    // 需要使用this指针因为comboSize为该类的成员变量

    pcomboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()/* 操作系统默认字体size */));
    connect(pcomboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::textSize);

    tb->addAction(pcolor);
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
    pfileMenu->addSeparator();

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
    pfileMenu->addSeparator();

#ifndef QT_NO_PRINTER
    QAction * tmpA = nullptr;
    tmpA = pfileMenu->addAction(tr("打印"), this, &MainWindow::filePrint);
    tmpA->setPriority(QAction::LowPriority);
    tmpA->setShortcut(QKeySequence::Print);

    tmpA = pfileMenu->addAction(tr("打印预览"), this, &MainWindow::filePrintPreview);

    tmpA = pfileMenu->addAction(tr("输出为PDF"), this, &MainWindow::filePrintPdf);
    tmpA->setPriority(QAction::LowPriority);
    tmpA->setShortcut(Qt::CTRL + Qt::Key_D);
    pfileMenu->addSeparator();
#endif

    pexitAct = new QAction(tr("退出(&X)"), this);
    pexitAct->setShortcuts(QKeySequence::Quit);
    pexitAct->setStatusTip(tr("退出应用程序"));
    connect(pexitAct, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    pfileMenu->addAction(pexitAct);
    pexitAct->setShortcut(Qt::CTRL + Qt::Key_Q);

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

    // 字体
    pformatMenu = menuBar()->addMenu(tr("字体"));
    pbold = pformatMenu->addAction(tr("加粗"), this, &MainWindow::textBold);
    pbold->setShortcut(Qt::CTRL + Qt::Key_B);
    pbold->setPriority(QAction::LowPriority);
    pbold->setStatusTip(tr("字体加粗"));
    QFont bold;
    bold.setBold(true);
    pbold->setFont(bold);
    pbold->setCheckable(true);

    pitalic = pformatMenu->addAction(tr("斜体"), this, &MainWindow::textItalic);
    pitalic->setShortcut(Qt::CTRL + Qt::Key_I);
    pitalic->setPriority(QAction::LowPriority);
    pitalic->setStatusTip(tr("字体斜体"));
    QFont italic;
    italic.setItalic(true);
    pitalic->setFont(italic);
    pitalic->setCheckable(true);

    punderline = pformatMenu->addAction(tr("下划线"), this, &MainWindow::textUnderline);
    punderline->setShortcut(Qt::CTRL + Qt::Key_U);
    punderline->setPriority(QAction::LowPriority);
    punderline->setStatusTip(tr("字体下划线"));
    QFont underline;
    underline.setUnderline(true);
    punderline->setFont(underline);
    punderline->setCheckable(true);

    pformatMenu->addSeparator();
    // 对齐方式
    pleft = new QAction(tr("左对齐"), this);
    pleft->setShortcut(Qt::CTRL + Qt::Key_L);
    pleft->setPriority(QAction::LowPriority);
    pleft->setStatusTip(tr("向左对齐"));
    pleft->setCheckable(true);
    pright = new QAction(tr("右对齐"), this);
    pright->setShortcut(Qt::CTRL + Qt::Key_R);
    pright->setPriority(QAction::LowPriority);
    pright->setStatusTip(tr("向右对齐"));
    pright->setCheckable(true);
    pcenter = new QAction(tr("居中对齐"), this);
    pcenter->setShortcut(Qt::CTRL + Qt::Key_E);
    pcenter->setPriority(QAction::LowPriority);
    pcenter->setStatusTip(tr("居中对齐"));
    pcenter->setCheckable(true);
    pjustify = new QAction(tr("两端对齐"), this);
    pjustify->setShortcut(Qt::CTRL + Qt::Key_J);
    pjustify->setPriority(QAction::LowPriority);
    pjustify->setStatusTip(tr("两端对齐"));
    pjustify->setCheckable(true);

    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &MainWindow::textAlign);
    // Make sure the alignLeft  is always left of the alignRight,主要针对往添加工具栏添加时有用
    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(pleft);
        alignGroup->addAction(pcenter);
        alignGroup->addAction(pright);
    } else {
        alignGroup->addAction(pright);
        alignGroup->addAction(pcenter);
        alignGroup->addAction(pleft);
    }
    alignGroup->addAction(pjustify);

    pformatMenu->addActions(alignGroup->actions());

    pformatMenu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    pcolor = pformatMenu->addAction(pix, tr("颜色"), this, &MainWindow::textColorset);

    // window menu
    pwindowMenu = menuBar()->addMenu(tr("窗口(&W)"));
    updateWinMenus();
    connect(pwindowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWinMenus);

    // 帮助菜单
    paboutMenu = menuBar()->addMenu(tr("帮助(&H)"));
    QAction *aboutAct = paboutMenu->addAction(tr("关于"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("简要说明"));
    paboutQt = new QAction(tr("关于Qt"), this);
    paboutMenu->addAction(paboutQt);
    connect(paboutQt, &QAction::triggered, this, &MainWindow::aboutQt);

    // format
    setupTextActions();
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

    return nullptr;
}

/**
  * @brief 设置活跃子窗口
  * @param 活跃子窗口Tab索引
  * @return none
  * @auther JSCao
  * @date   2019-01-20
  */
void MainWindow::setActiveTab(const int index)
{
    if (fileTab->count() < index) {
        return;
    }
    fileTab->setCurrentIndex(index);
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
    bool hasMyChild = (p_activeSubWin != nullptr);

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
    QSettings settings;
    printLog(DEBUG, "organizationName is %s, applicationName is %s", \
             narrow_cast<const char *>(QCoreApplication::organizationName().toUtf8()), narrow_cast<const char *>(QCoreApplication::applicationName().toUtf8()));
    int Dwidth  = QApplication::desktop()->width();
    int Dheight = QApplication::desktop()->height();

    QPoint pos = settings.value("pos", QPoint(int(Dwidth * 0.2),int(Dheight * 0.1))).toPoint();
    QSize size = settings.value("size", QSize(int(Dwidth * 0.6), int(Dheight * 0.75))).toSize();

    // read window size
    settings.beginGroup(WINSIZE);
    if (settings.contains("pos") && settings.contains("size")) {
        pos = settings.value("pos").toPoint();
        size = settings.value("size").toSize();
    }
    move(pos);
    resize(size);
    settings.endGroup();

    // read label of numbers and unclosed file's name
    settings.beginGroup(NUMOFRESLAB);
    int count = settings.value("labelofnumber").toInt();
    if (count != 0) {
        QStringList fileList = settings.value("fileNameList").toStringList();
        QStringList::const_iterator constIterator;
            for (constIterator = fileList.constBegin(); constIterator != fileList.constEnd(); ++constIterator) {
                printLog(DEBUG, (*constIterator).toLocal8Bit().constData());
                openAssignFile(*constIterator);
            }
    }
    settings.endGroup();

    printLog(DEBUG, "read configuration finish.");
}

/**
  * @brief 写入配置以供下回程序启动时读取
  * @param
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MainWindow::writeSetting()
{
    int tabCount = fileTab->count() - 1;
    printLog(DEBUG, "tabCount is %d", tabCount + 1);
    QStringList fileList;
    for (; tabCount >= 0; tabCount--) {
       MyChild *target = qobject_cast<MyChild *>(fileTab->widget(tabCount));
       printLog(DEBUG, "tabCount is %d, %s", tabCount, static_cast<const char *>(target->currentFile().toUtf8()));
       fileList += target->currentFile();
    }
    QSettings settings;
    // store window size
    settings.beginGroup(WINSIZE);
    settings.setValue("pos", QVariant(pos()));
    settings.setValue("size", QVariant(size()));
    settings.endGroup();

    // store label of numbers and unclosed file's name
    settings.beginGroup(NUMOFRESLAB);
    settings.setValue("labelofnumber", QVariant(tabCount));
    settings.setValue("fileNameList", QVariant(fileList));
    settings.endGroup();
}


/****************************** Place slot functiong in here ***************************************/
/**
  * @brief 【slot】设置排序
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-09-01
  */
void MainWindow::textStyle(int styleIndex)
{
    MyChild * p_child = activeMyChild();
    if (!p_child)
        return;

    QTextCursor cursor = p_child->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

/**
  * @brief 【slot】导出PDF
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-08-18
  */
void MainWindow::filePrintPdf(void)
{
#ifndef QT_NO_PRINTER
//! [0]
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    MyChild * p_child = activeMyChild();
    if (p_child)
        p_child->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
//! [0]
#endif
}
/**
  * @brief 【slot】文件打印预览
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-08-18
  */
void MainWindow::filePrintPreview(void)
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &MainWindow::printPreview);
    preview.exec();
#endif
}

/**
  * @brief 【slot】文件打印
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-08-18
  */
void MainWindow::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    MyChild * p_child = activeMyChild();
    if (p_child)
        p_child->print(printer);
#endif
}

/**
  * @brief 【slot】文件打印
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-08-17
  */
void MainWindow::filePrint(void)
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    MyChild * p_child = activeMyChild();
    if (p_child && p_child->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (p_child && dlg->exec() == QDialog::Accepted)
        p_child->print(&printer);
    delete dlg;
#endif
}

/**
  * @brief 【slot】字体状态更新
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-18
  */
void MainWindow::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void MainWindow::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    pcolor->setIcon(pix);
}
/**
  * @brief 【slot】字体颜色设置
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-18
  */
void MainWindow::textColorset(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    QColor col = QColorDialog::getColor(p_activeSubWin->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}
/**
  * @brief 【slot】文本对齐设置
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-14
  */
void MainWindow::textAlign(QAction *a)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (p_activeSubWin == nullptr || a == nullptr)
        return;

    if (a == pleft)
        p_activeSubWin->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == pcenter)
        p_activeSubWin->setAlignment(Qt::AlignHCenter);
    else if (a == pright)
        p_activeSubWin->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == pjustify)
        p_activeSubWin->setAlignment(Qt::AlignJustify);
}

/**
  * @brief 【slot】下滑线设置
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-14
  */
void MainWindow::textUnderline(void)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(punderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

/**
  * @brief 【slot】斜体字设置
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-14
  */
void MainWindow::textItalic(void)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(pitalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

/**
  * @brief 【slot】黑体字设置
  * @param  none
  * @return none
  * @auther QT
  * @date   2019-07-14
  */
void MainWindow::textBold(void)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(pbold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

/**
  * @brief 【slot】设置Combo的索引
  * @param  none
  * @return none
  * @auther caojingsong
  * @date   2019-05-04
  */
void MainWindow::setComboIndex(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (p_activeSubWin == nullptr || pcomboSize == nullptr)
        return;

    const QList<int> standardSizes = QFontDatabase::standardSizes();

    int size = p_activeSubWin->textCursor().charFormat().fontPointSize();
    if (size == 0)
        size = QApplication::font().pointSize();
    pcomboSize->setCurrentIndex(standardSizes.indexOf(size));

    alignmentChanged(p_activeSubWin->alignment());
}

/**
  * @brief 【slot】设置Combo的索引
  * @param  none
  * @return none
  * @auther caojingsong
  * @date   2019-05-18
  */
void MainWindow::setComboFont(void)
{
    MyChild *p_activeSubWin = activeMyChild();
    if (p_activeSubWin == nullptr || pcomboSize == nullptr)
        return;

    QFont font = p_activeSubWin->textCursor().charFormat().font();
    pcomboFont->setCurrentFont(font);
}

/**
  * @brief 【slot】设置字体格式
  * @param  none
  * @return none
  * @auther Qt
  * @date   2019-05-18
  */
void MainWindow::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

/**
  * @brief 【slot】设置字体size
  * @param  none
  * @return none
  * @auther Qt
  * @date   2019-05-04
  */
void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

/**
  * @brief 【slot】设置窗口标题后缀
  * @param  none
  * @return none
  * @auther JSCao
  * @date   2018-11-18
  */
void MainWindow::updateWinMenus(void)
{
    // clear windows
    pwindowMenu->clear();
    // traverse all tabs
    for (int i = 0; i < fileTab->count(); i++) {
        MyChild *myChild = qobject_cast<MyChild *>(fileTab->widget(i));
        QString subName = tr("&%1 %2").arg(i + 1).arg(myChild->pureCurrentFile());
        // add action to menu
        QAction *subWinAct = pwindowMenu->addAction(subName);
        subWinAct->setCheckable(true);
        subWinAct->setChecked(myChild == activeMyChild());
        connect(subWinAct, &QAction::triggered, [i, this](){setActiveTab(i);} );
    }
}

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
    totalLines = p_activeSubWin->document()->blockCount();
    totalCountStr.sprintf(TOTALCOUNT, totalCount, totalLines);
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
    MyChild * child = new MyChild(this);
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
    connect(child, &MyChild::textChanged, this, &MainWindow::setWinFileTitle);
    connect(child, &MyChild::cursorPositionChanged, this, &MainWindow::lineAndColmessage);
    connect(child, &MyChild::cursorPositionChanged, this, &MainWindow::setComboIndex);
    connect(child, &MyChild::cursorPositionChanged, this, &MainWindow::setComboFont);
    connect(child, &MyChild::currentCharFormatChanged, this, &MainWindow::currentCharFormatChanged);

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
        setWinFileTitle();
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

    writeSetting();

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
}

QWidget *MainWindow::findTagMyChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    QWidget *tagSubWindow = nullptr;
    for (int i = 0; i < fileTab->count(); i++) {
        tagSubWindow = fileTab->widget(i);
        MyChild *myChild = qobject_cast<MyChild *>(tagSubWindow);
        if (myChild->currentFile() == canonicalFilePath) {
            return tagSubWindow;
        }
    }
    return nullptr;
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
    printLog(DEBUG, "start opening file %s.", static_cast<const char *>(fileName.toUtf8()));

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
            totalCount = child->textCursor().document()->characterCount() - 1;
            totalLines = child->document()->blockCount();
            child->show();
            printLog(DEBUG, "load file success.");
        } else {
            child->close();
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == fileTab)
        if (event->type() == QEvent::MouseButtonDblClick) {
            newChild();
            return true;
        }
    return false;
}
