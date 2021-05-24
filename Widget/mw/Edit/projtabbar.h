#pragma once

#include <QTabBar>

#include <QMouseEvent>
#include <QApplication>

class ProjTabBar : public QTabBar
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

public:
    using QTabBar::QTabBar;

signals:
    void dragRequested(int index);

private:
    QPoint mPressPos;           //用于记录拖动开始的位置
    int mDragIndex = -1;        //用于记录拖动开始的index
    bool mMovedOut = false;     //用于防止重复触发拖动
};