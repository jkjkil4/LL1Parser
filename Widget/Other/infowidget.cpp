#include "infowidget.h"

InfoWidget::InfoWidget(QWidget *parent) : QWidget(parent)
{
    updateHeight();
}

InfoWidget::InfoWidget(const QString &text, QColor col, QWidget *parent) : InfoWidget(parent)
{
    setData(text, col);
}

void InfoWidget::setData(const QString &text, QColor col) {
    mPaint = true;
    mText = text;
    mCol = col;
    update();
}
void InfoWidget::clear() {
    mPaint = false;
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
    if(!mPaint)
        return;

    QPainter p(this);

    QRect rect(0, 0, width(), height());

    p.fillRect(rect, mCol);
    p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, mText);
}
