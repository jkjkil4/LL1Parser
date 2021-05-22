#include "projtabbar.h"

ProjTabBar::ProjTabBar(QWidget *parent) : QTabBar(parent)
{
    connect(this, &ProjTabBar::tabMoved, this, &ProjTabBar::onTabMoved);
}

void ProjTabBar::mousePressEvent(QMouseEvent *ev) {
    QTabBar::mousePressEvent(ev);
    if(ev->button() == Qt::LeftButton) {
        dragIndex = tabAt(ev->pos());
        movedOut = false;
    }
}
void ProjTabBar::mouseMoveEvent(QMouseEvent *ev) {
    QTabBar::mouseMoveEvent(ev);
    if(ev->buttons() & Qt::LeftButton) {
        if(!movedOut && dragIndex != -1 && !QRect(0, 0, width(), height()).contains(ev->pos())) {
            emit dragRequested(dragIndex);
            movedOut = true;
        }
    }
}

void ProjTabBar::onTabMoved(int from, int to) {
    if(to == dragIndex)
        dragIndex = from;
}