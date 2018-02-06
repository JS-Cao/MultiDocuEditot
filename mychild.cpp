#include "mychild.h"
#include <QFile>
#include <QFileInfo>
#include <QCloseEvent>

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
