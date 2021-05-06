#include "tabsplitwidget.h"

TabSplitWidget::TabSplitWidget(ProjTabManager *pPtManager, QWidget *parent)
    : QSplitter(parent), pPtManager(pPtManager)
{
    //默认包含一个ptWidget
    ProjTabWidget *ptWidget = new ProjTabWidget;
    mManager << ptWidget;
    if(pPtManager)
        pPtManager->append(ptWidget);
    addWidget(ptWidget);
    connect(ptWidget, &ProjTabWidget::splitRequested, this, TabSplitWidget::onSplitRequested);

    setChildrenCollapsible(false);  //设置分割部分不可因拖动而消失
    setHandleWidth(1);  //设置空隙宽度

    //设置背景透明
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0, 0));
    setPalette(pal);
    setAttribute(Qt::WA_TranslucentBackground);
}
TabSplitWidget::~TabSplitWidget() {
    if(pPtManager) {    //在析构时从pPtManager中移除所有该控件包含的ptWidget
        for(ProjTabWidget *ptWidget : mManager)
            pPtManager->removeOne(ptWidget);
    }
}

void TabSplitWidget::onSplitRequested(Qt::Orientation orientation) {
    //得到发出信号的ptWidget
    ProjTabWidget *ptWidget = (ProjTabWidget*)sender();
    if(!ptWidget) return;

    int index = indexOf(ptWidget);
    if(this->orientation() == orientation) {    //如果分割方向与该控件相同
        //直接创建控件到ptWidget后面
        ProjTabWidget *other = new ProjTabWidget;
        mManager << ptWidget;
        if(pPtManager)
            pPtManager->append(other);
        insertWidget(index + 1, other);
        connect(other, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);
    } else {    //如果分割方向与该控件不同
        //创建新的TabSplitWidget
        TabSplitWidget *widget = new TabSplitWidget(pPtManager);
        widget->setOrientation(orientation);

        //将ptWidget转交到widget中
        replaceWidget(index, widget);   //将当前控件分割部分的ptWidget替换为widget
        widget->insertWidget(0, ptWidget);  //将ptWidget作为widget分割部分的第一个
        disconnect(ptWidget, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);  //信号与槽的转交
        connect(ptWidget, &ProjTabWidget::splitRequested, widget, &TabSplitWidget::onSplitRequested);
        mManager.removeOne(ptWidget);   //mManager的转交
        widget->mManager << ptWidget;
    }
}