#pragma once

#include <QTabWidget>
#include <QListWidget>
#include <QAction>

#include <QHBoxLayout>

#include <Lib/header.h>

class OutputWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OutputWidget(QWidget *parent = nullptr);

    void setCurrentWidget(QWidget *widget);

    QListWidget* errListWidget() { return mErrListWidget; }

private:
    QTabWidget *tabWidget = new QTabWidget;

    QListWidget *mErrListWidget = new QListWidget;

    typedef QString(*TrFn)();
    struct Tab
    {
        QWidget *p;
        TrFn fn;
    };
    Tab tabs[1] {
        { mErrListWidget, []()->QString { return tr("Issues"); } }
    };
};

