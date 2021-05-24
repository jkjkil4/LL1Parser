#pragma once

#include <QPushButton>
#include <QStackedWidget>
#include <QMenu>
#include <QMessageBox>

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QStyle>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Widget/Other/colorwidget.h"
#include "projtabbar.h"
#include "projwidget.h"
#include <Lib/paint.h>

class ProjTabWidget;
struct ProjTabManager
{
    QVector<ProjTabWidget*> vec;
    QMap<CanonicalFilePath, QTextDocument*> documents;
    ProjTabWidget *current = nullptr;
};

class ProjTabWidget : public QWidget
{
    Q_OBJECT
protected:
    void focusInEvent(QFocusEvent *) override;

    void dragEnterEvent(QDragEnterEvent *ev) override;
    void dropEvent(QDropEvent *ev) override;

public:
    ProjTabWidget(ProjTabManager *pPtManager = nullptr, QWidget *parent = nullptr);

    bool contains(const CanonicalFilePath &cFilePath) const;
    ProjWidget* open(const CanonicalFilePath &cFilePath);
    int count() const { return mStackedWidget->count(); }
    ProjWidget* current() const { return (ProjWidget*)mStackedWidget->currentWidget(); }
    ProjWidget* widget(int index) const { return (ProjWidget*)mStackedWidget->widget(index); }
    void setCurrent(ProjWidget *w);

    bool confirmClose(int index);
    bool closeAll();

    VAR_GET_FUNC(tabBar, mTabBar, QTabBar*)
    VAR_GET_FUNC(stackedWidget, mStackedWidget, QStackedWidget*)
    VAR_GET_FUNC(tabBarWidget, mTabBarWidget, ColorWidget*)

signals:
    void focused();
    void changeViewRequested();
    void splitRequested(Qt::Orientation orientation);   //用于发送分割请求
    void removeableRequested(bool &able);   //用于检查是否可移除
    void removeRequested();     //用于发送移除请求
    // void newWindowRequested();  //用于发送 在新窗口中打开 请求

private slots:
    void onTabMoved(int from, int to);
    void onCurrentChanged(int index);
    void onTabCloseRequested(int index);
    void onTabBtnClicked();
    void onTabDragRequested(int index);

    void onProjWidgetSaveStateChanged(bool isSaved);

private:
    ProjTabBar *mTabBar = new ProjTabBar;
    QPushButton *mTabBtn = new QPushButton;
    QStackedWidget *mStackedWidget = new QStackedWidget;

    ColorWidget *mTabBarWidget = new ColorWidget(Qt::lightGray);

    ProjTabManager *pPtManager;
};