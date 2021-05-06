#pragma once

#include <QTabBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QMenu>

#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Widget/Other/colorwidget.h"
#include <Lib/header.h>

class ProjTabWidget : public QWidget
{
    Q_OBJECT
protected:
    void focusInEvent(QFocusEvent *) override;

public:
    ProjTabWidget(QWidget *parent = nullptr);

    VAR_GET_FUNC(tabBar, mTabBar, QTabBar*)
    VAR_GET_FUNC(stackedWidget, mStackedWidget, QStackedWidget*)

signals:
    void focused();
    void splitRequested(Qt::Orientation orientation);

private slots:
    void onTabBtnClicked();

private:
    QTabBar *mTabBar = new QTabBar;
    QPushButton *mTabBtn = new QPushButton;
    QStackedWidget *mStackedWidget = new QStackedWidget;
};