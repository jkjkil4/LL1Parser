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

    void open(const QString &projPath);
    int count() { return tabWidget->count(); }
    ProjWidget* current() { return (ProjWidget*)tabWidget->currentWidget(); }

private slots:
    void onTabCloseRequested(int index);

private:
    QTabWidget *tabWidget = new QTabWidget;
};

