#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H
#pragma execution_character_set("utf-8")
#include <QWidget>
#include <QDebug>

QT_BEGIN_NAMESPACE
class MyChild;
class QSize;
class QPaintEvent;
QT_END_NAMESPACE

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(MyChild *parent) : QWidget(parent)
    {
        textEditor = parent;
    }

    QSize sizeHint() const override
    {   // 自定义窗口部件尺寸
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    MyChild *textEditor;
};


#endif // LINENUMBERAREA_H
