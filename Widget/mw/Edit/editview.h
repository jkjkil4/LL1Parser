#pragma once

#include "../mainwindowview.h"
#include <QTabWidget>

#include <QHBoxLayout>

#include "projwidget.h"

class EditView : public MainWindowView
{
public:
    explicit EditView(QWidget *parent = nullptr);

    void open(const QString &projPath);

private:
    QTabWidget *tabWidget = new QTabWidget;
};

