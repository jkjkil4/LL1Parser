#include "notewidget.h"

NoteWidget::NoteWidget(QWidget *parent) : QWidget(parent)
{
    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void NoteWidget::onTimer() {
    mAph -= 4;
    if(mAph < 0) {
        mAph = 0;
        mTimer->stop();
    }
    update();
}

void NoteWidget::start() {
    mAph = 600;
    mTimer->start(16);
    update();
}

void NoteWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    QColor col(mColor);
    col.setAlpha(qMin(mAph, 255));
    p.setPen(col);
    p.drawText(QRect(0, 0, width(), height()), Qt::AlignLeft | Qt::AlignVCenter, mText);
}
