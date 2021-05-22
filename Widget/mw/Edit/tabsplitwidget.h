#pragma once

#include <QSplitter>

#include "projtabwidget.h"

class TabSplitWidget : public QSplitter
{
    Q_OBJECT
public:
    TabSplitWidget(ProjTabManager *pPtManager = nullptr, QWidget *parent = nullptr);
    ~TabSplitWidget() override;

    void setCurrentPtWidget(ProjTabWidget *ptWidget) { setCurrentPtWidget(pPtManager, ptWidget); }
    static void setCurrentPtWidget(ProjTabManager *pPtManager, ProjTabWidget *ptWidget);

signals:
    void changeViewRequested();
    void removeableRequested(bool &able);  //用于检查是否可移除
    void splitRemoveRequested();    //用于发送移除请求
    // void windowCreated(TabSplitWidget *tsWidget);   //用于发送创建了新窗口的信息

private slots:
    void onPtFocused();
    void onSplitRequested(Qt::Orientation orientation);
    void onRemoveRequested();
    void onRemoveableRequested(bool &able);
    void onSplitRemoveRequested();
    // void onNewWindowRequested();

private:
    void connectPtWidget(ProjTabWidget *ptWidget);
    void disconnectPtWidget(ProjTabWidget *ptWidget);

    QVector<ProjTabWidget*> mPtWidgets;
    ProjTabManager *pPtManager;
};