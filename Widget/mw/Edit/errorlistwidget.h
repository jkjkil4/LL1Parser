#pragma once

#include <QWidget>
#include <QListWidget>

#include <Lib/header.h>

class ErrorListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorListWidget(QWidget *parent = nullptr);

    QListWidget* listWidget() { return mListWidget; }

private:
    QListWidget *mListWidget = new QListWidget;
};
