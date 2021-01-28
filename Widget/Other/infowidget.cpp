#include "infowidget.h"

InfoWidget::InfoWidget(QWidget *parent) : QWidget(parent)
{
    updateHeight();
}

InfoWidget::InfoWidget(const QString &text, QColor col, QWidget *parent) : InfoWidget(parent)
{
    setData(text, col);
}

void InfoWidget::setData(const QString &_text, QColor _col) {
    paint = true;
    text = _text;
    col = _col;
    update();
}
void InfoWidget::clear() {
    paint = false;
    update();
}
void InfoWidget::setPointSize(int pointSize) {
    j::SetPointSize(this, pointSize);
    updateHeight();
    update();
}

void InfoWidget::updateHeight() {
    j::LimitHeight(this, QFontMetrics(font()).height() + 4);
}

void InfoWidget::paintEvent(QPaintEvent *) {
    if(!paint)
        return;

    QPainter p(this);

    QRect rect(0, 0, width(), height());

    p.fillRect(rect, col);
    p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
}
