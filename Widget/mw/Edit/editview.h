#pragma once

#include "../mainwindowview.h"
#include <QTabWidget>
#include <QMessageBox>

#include <QHBoxLayout>

#include "projwidget.h"

class EditView : public MainWindowView
{
    Q_OBJECT
public:
    explicit EditView(QWidget *parent = nullptr);

    void open(const QString &projPath);     //打开项目
    int count() { return tabWidget->count(); }      //得到项目数量
    ProjWidget* current() { return (ProjWidget*)tabWidget->currentWidget(); }   //得到当前项目

    bool confirmClose(int index);    //对指定项目的关闭进行提示和确认，返回值为是否处理
    bool closeAll();    //当要关闭程序时，检查所有以打开的项目，返回值为是否全部处理

private slots:
    void onTabCloseRequested(int index);    //当tabWidget标签页的关闭按钮被点击时调用的槽

private:
    QTabWidget *tabWidget = new QTabWidget;
};

