#include "mychild.h"
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

MyChild::MyChild()
{
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
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
    } else {
        event->ignore();
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
}
