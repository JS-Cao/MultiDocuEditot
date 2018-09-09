#include "mychild.h"
#include <cstdlib>
#include <QFile>
#include <QFileInfo>
#include <QCloseEvent>
#include <QString>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileDialog>
#include <QStatusBar>
#include <QPushButton>
#include <QStringLiteral>
#include <QDebug>
#include <QWidget>
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
#include <QRect>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPlainTextEdit>
#include <QtWidgets>
#include "linenumberarea.h"
#include "mainwindow.h"

#pragma execution_character_set("utf-8")

MyChild::MyChild()
{
    setAttribute(Qt::WA_DeleteOnClose);
    lineNumberArea = new LineNumberArea(this);

    scrollToBlockNum = -1;
    scrollToBlockStep = -1;
    blockNumsPerPage = -1;
    idIsChanged = false;

    this->verticalScrollBar()->setSingleStep(17);
    this->verticalScrollBar()->setPageStep(17);
    this->verticalScrollBar()->connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyChild::FirstVisibleBlockNum);
    this->verticalScrollBar()->connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &MyChild::scrollMapToBlock);

    connect(this, &MyChild::textChanged, this, &MyChild::updateLineNumberAreaWidth);
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(document(), &QTextDocument::blockCountChanged, this, &MyChild::_updateLineNumberArea);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MyChild::_updateLineNumberArea);

    updateLineNumberAreaWidth();
    highlightCurrentLine();
    isUntitled = true;
}

MyChild::~MyChild()
{
    //qDebug() << "deconstructor";
}

/**
  * @brief 设置是否接受关闭事件
  * @param
  *     arg1：0-接受  1-忽略
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MyChild::setClosedFlag(int flag)
{
    isReceivedClosedFlag = flag;
}

/**
  * @brief 返回是否接受关闭事件标志位
  * @param none
  * @return 0：接受
  *         1：忽略
  * @auther JSCao
  * @date   2018-08-25
  */
int MyChild::closedFlag(void)
{
    return isReceivedClosedFlag;
}

void MyChild::newFile()
{
    static int sequenceNumber = 1;
    isUntitled = true;

    curFile = tr("新建文档%1").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
    connect(document(), &QTextDocument::contentsChanged, this, &MyChild::documentWasModified);
}

void MyChild::documentWasModified()
{
    setWindowModified(document()->isModified());
}

QString MyChild::pureCurrentFile()
{
    return strippedName(curFile);
}

QString MyChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MyChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
        setClosedFlag(0);
    } else {
        event->ignore();
        setClosedFlag(1);
        idIsChanged = false;    // 在restoreId的时候不改变该ID
    }
}

bool MyChild::loadFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("多文档编辑器"),tr("无法打开文件%1\n%2").arg(fileName).arg(file.errorString()));
            return false;
        }

        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();
        setCurrentFile(fileName);
        connect(document(), &QTextDocument::contentsChanged, this, &MyChild::documentWasModified);
        return true;
    }
    return false;
}

void MyChild::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();  // 返回绝对路劲
    isUntitled = false;
    document()->setModified(false);
    setWindowModified(false);   // 去*
    setWindowTitle(pureCurrentFile() + tr("[*]"));
}

bool MyChild::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MyChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), curFile);

    if (fileName.isEmpty()) {
        return false;
    } else {
        return saveFile(fileName);
    }
}

bool MyChild::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("多文档编辑器"), tr("无法写入文件%1\n%2").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);

    return true;
}

bool MyChild::maybeSave()
{
    if (!document()->isModified()) {
        return true;
    }
    QMessageBox box;
    box.setWindowTitle(tr("保存"));
    box.setText(tr("是否保存对文件%1的修改!!!").arg(pureCurrentFile()));
    box.setIcon(QMessageBox::Warning);

    QPushButton *yesBtn = box.addButton(tr("是"), QMessageBox::YesRole);
    QPushButton *noBtn = box.addButton(tr("否"), QMessageBox::DestructiveRole);
    QPushButton *cancleBtn = box.addButton(tr("取消"), QMessageBox::RejectRole);

    box.exec();
    if (box.clickedButton() == yesBtn) {
        return save();
    } else if (box.clickedButton() == noBtn) {
        return true;
    } else if (box.clickedButton() == cancleBtn) {
        return false;
    } else {
        //
    }
    return false;
}

void MyChild::closefile(int index)
{
    if (index != myId) {
        if (index < myId) {
            myId--;
            idIsChanged = true;
        }
        return;
    }


    MyChild::close();
}

void MyChild::setId(const int Id)
{
    myId = Id;
}

const int MyChild::fileId(void)
{
    return myId;
}

int MyChild::lineNumberAreaWidth(void)
{
    int digits = 1;

    int max = qMax(1, document()->blockCount());
    while (max >= 10) { // 计算行号是几位数
        max /= 10;
        ++digits;
    }

    // QLatin1Char class is only useful to construct a QChar with 8-bit character.
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void MyChild::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void MyChild::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void MyChild::_updateLineNumberArea(void)
{
    //qDebug() << __FUNCTION__;
    if (document()->blockCount() * cursorRect().size().height() > frameRect().height()) {
        lineNumberArea->scroll(0, frameRect().height());
    }else

    lineNumberArea->update(0, 0, lineNumberArea->width(), lineNumberArea->height());
    updateLineNumberAreaWidth();
}

void MyChild::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void MyChild::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);

    QRect cr = contentsRect();

    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

/**
  * @brief 绘制行号
  * @param
  *     arg1：绘图事件指针，记录绘图/更新区域
  * @return none
  * @auther JSCao
  * @date   2018-08-12
  */
void MyChild::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    int _blockNumber = 1;
    QTextBlock block = document()->firstBlock();

    blockNumsPerPage = frameGeometry().bottom() / document()->documentLayout()->blockBoundingRect(block).height();

    if ((scrollToBlockNum != -1) && (scrollToBlockStep != -1)) {
        block = document()->findBlockByLineNumber(scrollToBlockNum);
        if (block.isValid()) {
            _blockNumber = block.blockNumber() + 1;
        }
        //qDebug() << "Line start is :" << _blockNumber;
    }

    int top = (int)document()->documentLayout()->blockBoundingRect(document()->firstBlock()).topLeft().ry();
    int bottom = top + (int)document()->documentLayout()->blockBoundingRect(document()->firstBlock()).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(_blockNumber);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)document()->documentLayout()->blockBoundingRect(block).height();
        ++_blockNumber;
    }

}

/**
  * @brief 【slot】建立scrollValue与blocknums的对应关系
  * @param
  *     arg1：无名
  *     arg2：滚动条最大值
  * @return none
  * @auther JSCao
  * @date   2018-08-12
  */
void MyChild::scrollMapToBlock(int, int max)
{
    double A = document()->blockCount() - blockNumsPerPage;
    double B = max;
    if (document()->blockCount() - blockNumsPerPage > 0 && max != 0) {
        scrollToBlockStep = A / B;
    }
    else {
        scrollToBlockNum = -1;
        scrollToBlockStep = -1;
    }
}

/**
  * @brief 【slot】将scrollValue的值映射为blocknums的值
  * @param
  *     arg1：scrollValue
  * @return none
  * @auther JSCao
  * @date   2018-08-12
  */
void MyChild::FirstVisibleBlockNum(int index)
{
    if (scrollToBlockStep > 0) {
        scrollToBlockNum = index * scrollToBlockStep;
    }
}

/**
  * @brief 【slot】恢复上一次变更的ID值
  * @param
  *     arg1：Id
  * @return none
  * @auther JSCao
  * @date   2018-08-25
  */
void MyChild::restoreId(const int id)
{
    if (id <= myId && idIsChanged) {
        myId++;
        idIsChanged = false;
    }
}
