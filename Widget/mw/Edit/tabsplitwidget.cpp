#include "tabsplitwidget.h"

TabSplitWidget::TabSplitWidget(ProjTabManager *pPtManager, QWidget *parent)
    : QSplitter(parent), pPtManager(pPtManager)
{
    ProjTabWidget *ptWidget = new ProjTabWidget;
    mManager << ptWidget;
    if(pPtManager)
        pPtManager->append(ptWidget);
    addWidget(ptWidget);
    connect(ptWidget, &ProjTabWidget::splitRequested, this, TabSplitWidget::onSplitRequested);

    setChildrenCollapsible(false);
}
TabSplitWidget::~TabSplitWidget() {
    if(pPtManager) {
        for(ProjTabWidget *ptWidget : mManager)
            pPtManager->removeOne(ptWidget);
    }
}
#include <QDebug>
void TabSplitWidget::onSplitRequested(Qt::Orientation orientation) {
    qDebug() << "Split Requested";
    
    ProjTabWidget *ptWidget = (ProjTabWidget*)sender();
    if(!ptWidget) return;

    int index = indexOf(ptWidget);
    if(this->orientation() == orientation) {
        ProjTabWidget *other = new ProjTabWidget;
        mManager << ptWidget;
        if(pPtManager)
            pPtManager->append(other);
        insertWidget(index + 1, other);
        connect(other, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);
    } else {
        TabSplitWidget *widget = new TabSplitWidget(pPtManager);
        widget->setOrientation(orientation);
        replaceWidget(index, widget);
        widget->insertWidget(0, ptWidget);
        disconnect(ptWidget, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);
        connect(ptWidget, &ProjTabWidget::splitRequested, widget, &TabSplitWidget::onSplitRequested);
    }
}