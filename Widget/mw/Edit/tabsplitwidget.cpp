#include "tabsplitwidget.h"

TabSplitWidget::TabSplitWidget(ProjTabManager *pPtManager, QWidget *parent)
    : QSplitter(parent), pPtManager(pPtManager)
{
    //默认包含一个ptWidget
    ProjTabWidget *ptWidget = new ProjTabWidget(pPtManager);
    mPtWidgets << ptWidget;
    if(pPtManager) {
        pPtManager->vec << ptWidget;
        if(!pPtManager->current)
            setCurrentPtWidget(ptWidget);
    }
    addWidget(ptWidget);
    connectPtWidget(ptWidget);

    setChildrenCollapsible(false);  //设置分割部分不可因拖动而消失
    setHandleWidth(2);  //设置空隙宽度

    //设置背景透明
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0, 0));
    setPalette(pal);
    setAttribute(Qt::WA_TranslucentBackground);
}
TabSplitWidget::~TabSplitWidget() {
    if(pPtManager) {    //在析构时从pPtManager中移除所有该控件包含的ptWidget
        for(ProjTabWidget *ptWidget : mPtWidgets)
            pPtManager->vec.removeOne(ptWidget);

        //检查是否移除了pPtManager->current
        if(mPtWidgets.contains(pPtManager->current)) {
            if(pPtManager->vec.isEmpty()) {
                pPtManager->current = nullptr;
            } else {
                setCurrentPtWidget(pPtManager->vec[0]);
            }
        }
    }
}

void TabSplitWidget::setCurrentPtWidget(ProjTabManager *pPtManager, ProjTabWidget *ptWidget) {
    if(pPtManager && pPtManager->current != ptWidget) {
        if(pPtManager->current) {
            pPtManager->current->tabBarWidget()->setColor(Qt::lightGray);
            j::SetPaletteColor(pPtManager->current, QPalette::Background, QColor(230, 230, 230));
        }
        pPtManager->current = ptWidget;
        ptWidget->tabBarWidget()->setColor(QColor(170, 170, 170));
        j::SetPaletteColor(ptWidget, QPalette::Background, QColor(240, 240, 240));
    }
}

void TabSplitWidget::onSplitRequested(Qt::Orientation orientation) {
    //得到发出信号的ptWidget
    ProjTabWidget *ptWidget = (ProjTabWidget*)sender();
    if(!ptWidget) return;

    int index = indexOf(ptWidget);
    if(this->orientation() == orientation) {    //如果分割方向与该控件相同
        QList<int> lSizes = sizes();

        //直接创建控件到ptWidget后面
        ProjTabWidget *other = new ProjTabWidget(pPtManager);
        mPtWidgets << other;
        if(pPtManager) {
            pPtManager->vec << other;
            if(!pPtManager->current)
                pPtManager->current = other;
        }
        insertWidget(index + 1, other);
        connectPtWidget(other);

        //设置大小
        lSizes.insert(index + 1, lSizes[index]);
        int i = 0;
        for(auto iter = lSizes.begin(); iter < lSizes.end(); ++iter) {
            if(i != index && i != index + 1)
                *iter *= 2;
            i++;
        }
        setSizes(lSizes);
    } else {    //如果分割方向与该控件不同
        //创建新的TabSplitWidget
        TabSplitWidget *widget = new TabSplitWidget(pPtManager);
        widget->setOrientation(orientation);
        replaceWidget(index, widget);   //将当前控件分割部分的ptWidget替换为widget
        connect(widget, SIGNAL(changeViewRequested()), this, SIGNAL(changeViewRequested()));
        connect(widget, &TabSplitWidget::removeableRequested, this, &TabSplitWidget::onRemoveableRequested);
        connect(widget, &TabSplitWidget::splitRemoveRequested, this, &TabSplitWidget::onSplitRemoveRequested);
        // connect(widget, SIGNAL(windowCreated()), this, SIGNAL(windowCreated()));

        //将ptWidget转交到widget中
        widget->insertWidget(0, ptWidget);  //将ptWidget作为widget分割部分的第一个
        disconnectPtWidget(ptWidget);   //信号与槽的转交
        widget->connectPtWidget(ptWidget);
        mPtWidgets.removeOne(ptWidget);   //mPtWidgets的转交
        widget->mPtWidgets << ptWidget;

        //设置大小
        widget->setSizes(QList<int>() << 1 << 1);
    }
}

void TabSplitWidget::onPtFocused() {
    //得到发出信号的widget
    ProjTabWidget *ptWidget = (ProjTabWidget*)sender();
    if(!ptWidget) return;

    setCurrentPtWidget(ptWidget);       //设置
}

void TabSplitWidget::onRemoveRequested() {
    //得到发出信号的widget
    ProjTabWidget *ptWidget = (ProjTabWidget*)sender();
    if(!ptWidget) return;

    if(!ptWidget->closeAll())
        return;

    //如果控件数仅为1，则向上发送请求处理，否则将ptWidget从分割部分中移除
    if(count() <= 1) {
        emit splitRemoveRequested();
    } else {
        ptWidget->setParent(nullptr);
        ptWidget->deleteLater();
        mPtWidgets.removeOne(ptWidget);

        //检查是否移除了pPtManager->current
        if(pPtManager) {
            pPtManager->vec.removeOne(ptWidget);
            if(pPtManager->current == ptWidget) {
                if(pPtManager->vec.isEmpty()) {
                    pPtManager->current = nullptr;
                } else {
                    setCurrentPtWidget(pPtManager->vec[0]);
                }
            }
        }
    }
}

void TabSplitWidget::onRemoveableRequested(bool &able) {
    //如果控件数仅为1，则向上进行检测，否则标记为可移除
    if(count() <= 1) {
        emit removeableRequested(able);
    } else {
        able = true;
    }
}
void TabSplitWidget::onSplitRemoveRequested() {
    //得到发出信号的widget
    TabSplitWidget *widget = (TabSplitWidget*)sender();
    if(!widget) return;

    //如果控件数仅为1，则向上发送请求处理，否则将widget从分割部分中移除
    if(count() <= 1) {
        emit splitRemoveRequested();
    } else {
        widget->setParent(nullptr);
        widget->deleteLater();
    }
}

// void TabSplitWidget::onNewWindowRequested() {
//     TabSplitWidget *widget = new TabSplitWidget(pPtManager);
//     widget->setAttribute(Qt::WA_DeleteOnClose);
//     widget->show();
//     emit windowCreated(widget);
// }

void TabSplitWidget::connectPtWidget(ProjTabWidget *ptWidget) {
    connect(ptWidget, &ProjTabWidget::focused, this, &TabSplitWidget::onPtFocused);
    connect(ptWidget, SIGNAL(changeViewRequested()), this, SIGNAL(changeViewRequested()));
    connect(ptWidget, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);
    connect(ptWidget, &ProjTabWidget::removeRequested, this, &TabSplitWidget::onRemoveRequested);
    connect(ptWidget, &ProjTabWidget::removeableRequested, this, &TabSplitWidget::onRemoveableRequested);
    // connect(ptWidget, &ProjTabWidget::newWindowRequested, this, &TabSplitWidget::onNewWindowRequested);
}
void TabSplitWidget::disconnectPtWidget(ProjTabWidget *ptWidget) {
    disconnect(ptWidget, &ProjTabWidget::focused, this, &TabSplitWidget::onPtFocused);
    disconnect(ptWidget, SIGNAL(changeViewRequested()), this, SIGNAL(changeViewRequested()));
    disconnect(ptWidget, &ProjTabWidget::splitRequested, this, &TabSplitWidget::onSplitRequested);
    disconnect(ptWidget, &ProjTabWidget::removeRequested, this, &TabSplitWidget::onRemoveRequested);
    disconnect(ptWidget, &ProjTabWidget::removeableRequested, this, &TabSplitWidget::onRemoveableRequested);
    // disconnect(ptWidget, &ProjTabWidget::newWindowRequested, this, &TabSplitWidget::onNewWindowRequested);
}
