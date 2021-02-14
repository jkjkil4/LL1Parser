#pragma once

#include "../mainwindowview.h"
#include <QLabel>
#include <QListWidget>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <Lib/PlainButton/plainbutton.h>

#include "recentfilelistwidget.h"
#include "Widget/Dialog/newprojdialog.h"

class HomePage : public MainWindowView
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage() override = default;

    void updateTr();

    RFLWidget* recentFileListWidget() { return mRflWidget; }
    PlainButton* btnNew() { return mBtnNewProj; }
    PlainButton* btnOpen() { return mBtnOpenProj; }

private:
    QLabel *mLabRfl = new QLabel;
    RFLWidget *mRflWidget = new RFLWidget;

    QLabel *mLabAdd = new QLabel;
    PlainButton *mBtnNewProj = new PlainButton;
    PlainButton *mBtnOpenProj = new PlainButton;
};
