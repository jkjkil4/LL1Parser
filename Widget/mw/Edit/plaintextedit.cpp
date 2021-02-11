#include "plaintextedit.h"

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    updateTabStopDistance();    //更新tab距离
    setLineWrapMode(LineWrapMode::NoWrap);  //不自动换行
    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);  //总是隐藏横向滑动条
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);     //总是显示横向滑动条

}

void PlainTextEdit::updateTabStopDistance() {
    int fontWidth = fontMetrics().horizontalAdvance('_');
    if(prevFontWidth != fontWidth) {
        setTabStopDistance(fontWidth * 4);
        prevFontWidth = fontWidth;
    }
}

void PlainTextEdit::wheelEvent(QWheelEvent *ev) {
    if(ev->modifiers() == Qt::ControlModifier) {    //如果同时按下了Ctrl键
        int pointSize = font().pointSize();
        int offset = ev->delta() / 120;
        int res = qMax(1, pointSize + offset);
        if(res != pointSize) {
            j::SetPointSize(this, res);
            emit pointSizeChanged(res);
        }
        return;     //return是为了不调用父类wheelEvent
    }
    QPlainTextEdit::wheelEvent(ev);
}

void PlainTextEdit::paintEvent(QPaintEvent *ev) {
    updateTabStopDistance();

    QPlainTextEdit::paintEvent(ev);
}
