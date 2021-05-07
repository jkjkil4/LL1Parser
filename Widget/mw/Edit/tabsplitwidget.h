#pragma once

#include <QSplitter>

#include "projtabwidget.h"

class TabSplitWidget : public QSplitter
{
    Q_OBJECT
public:
    TabSplitWidget(ProjTabManager *pPtManager = nullptr, QWidget *parent = nullptr);
    ~TabSplitWidget() override;

    void setCurrentPtWidget(ProjTabWidget *ptWidget);

signals:
    void removeableRequested(bool &able);  //用于检查是否可移除
    void splitRemoveRequested();    //用于发送移除请求

private slots:
    void onPtFocused();
    void onSplitRequested(Qt::Orientation orientation);
    void onRemoveRequested();
    void onRemoveableRequested(bool &able);
    void onSplitRemoveRequested();

private:
    void connectPtWidget(ProjTabWidget *ptWidget);
    void disconnectPtWidget(ProjTabWidget *ptWidget);

    QVector<ProjTabWidget*> mPtWidgets;
    ProjTabManager *pPtManager;
};