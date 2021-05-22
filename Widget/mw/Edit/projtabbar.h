#pragma once

#include <QTabBar>

#include <QMouseEvent>

class ProjTabBar : public QTabBar
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

public:
    ProjTabBar(QWidget *parent = nullptr);

signals:
    void dragRequested(int index);

private slots:
    void onTabMoved(int from, int to);

private:
    int dragIndex = -1;
    bool movedOut = false;
};