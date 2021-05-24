#include "projtabbar.h"

void ProjTabBar::mousePressEvent(QMouseEvent *ev) {
    QTabBar::mousePressEvent(ev);
    if(ev->button() == Qt::LeftButton) {
        mPressPos = ev->pos();
        mDragIndex = tabAt(ev->pos());
        mMovedOut = false;
    }
}
void ProjTabBar::mouseMoveEvent(QMouseEvent *ev) {
    QTabBar::mouseMoveEvent(ev);
    if(ev->buttons() & Qt::LeftButton) {
        if(mDragIndex != -1 && !mMovedOut && (ev->pos() - mPressPos).manhattanLength() >= QApplication::startDragDistance()) {
            emit dragRequested(mDragIndex);
            mMovedOut = true;
        }
    }
}
