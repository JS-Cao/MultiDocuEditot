#include "mychild.h"
#include <QFile>
#include <QFileInfo>
#include <QCloseEvent>
#include <QString>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>

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
    event->accept();
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
