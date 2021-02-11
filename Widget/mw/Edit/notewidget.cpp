#include "notewidget.h"

NoteWidget::NoteWidget(QWidget *parent) : QWidget(parent)
{
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void NoteWidget::onTimer() {
    aph -= 4;
    if(aph < 0) {
        aph = 0;
        timer->stop();
    }
    update();
}

void NoteWidget::start() {
    aph = 400;
    timer->start(16);
    update();
}

void NoteWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    QColor col(color);
    col.setAlpha(qMin(aph, 255));
    p.setPen(col);
    p.drawText(QRect(0, 0, width(), height()), Qt::AlignLeft | Qt::AlignVCenter, text);
}
