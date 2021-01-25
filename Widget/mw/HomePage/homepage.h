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

signals:
    void openProj(const QString &filePath);

private slots:
    void onNewProj();
    void onOpenProj();

private:
    QLabel *labRfl = new QLabel;
    RFLWidget *rflWidget = new RFLWidget;

    QLabel *labAdd = new QLabel;
    PlainButton *btnNewProj = new PlainButton;
    PlainButton *btnOpenProj = new PlainButton;
};
