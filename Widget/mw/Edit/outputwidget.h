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
    void clear();

    QListWidget* errListWidget() const { return mErrListWidget; }
    QListWidget* outputListWidget() const { return mOutputListWidget; }

private:
    QTabWidget *mTabWidget = new QTabWidget;

    QListWidget *mErrListWidget = new QListWidget;
    QListWidget *mOutputListWidget = new QListWidget;

    typedef QString(*TrFn)();
    struct Tab
    {
        QWidget *p;
        TrFn fn;
    };
    Tab tabs[2] {
        { mErrListWidget, []()->QString { return tr("Issues"); } },
        { mOutputListWidget, []()->QString { return tr("Output"); } }
    };
};

