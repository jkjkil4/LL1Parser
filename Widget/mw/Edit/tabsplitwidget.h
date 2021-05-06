#pragma once

#include <QSplitter>

#include "projtabwidget.h"

typedef QVector<ProjTabWidget*> ProjTabManager;

class TabSplitWidget : public QSplitter
{
public:
    TabSplitWidget(ProjTabManager *pPtManager = nullptr, QWidget *parent = nullptr);
    ~TabSplitWidget() override;

private slots:
    void onSplitRequested(Qt::Orientation orientation);

private:
    ProjTabManager mManager;
    ProjTabManager *pPtManager;
};