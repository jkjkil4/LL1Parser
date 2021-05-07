#include "projtabwidget.h"

ProjTabWidget::ProjTabWidget(ProjTabManager *pPtManager, QWidget *parent) 
    : QWidget(parent), pPtManager(pPtManager)
{
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setExpanding(false);
    mTabBar->setDrawBase(false);
    mTabBar->setMinimumWidth(32);
    repeat(int, i, 5) {
        mTabBar->addTab(QString::number(i));
    }

    mTabBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
    mTabBtn->setIconSize(QSize(10, 10));
    j::LimitSize(mTabBtn, 24, 24);
    connect(mTabBtn, &QPushButton::clicked, this, &ProjTabWidget::onTabBtnClicked);
    
    mStackedWidget->setMinimumHeight(32);

    QHBoxLayout *layTabBar = new QHBoxLayout;
    layTabBar->setMargin(0);
    layTabBar->setSpacing(2);
    layTabBar->addWidget(mTabBar);
    layTabBar->addWidget(mTabBtn);

    mTabBarWidget->setLayout(layTabBar);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(mTabBarWidget);
    layMain->addWidget(mStackedWidget);
    setLayout(layMain);

    setFocusPolicy(Qt::ClickFocus);

    j::SetPaletteColor(this, QPalette::Background, QColor(230, 230, 230));      //设置背景颜色
    setAutoFillBackground(true);
}

void ProjTabWidget::focusInEvent(QFocusEvent *) {
    emit focused();
}

void ProjTabWidget::onTabBtnClicked() {
    QMenu menu;
    
    //向右分割
    QAction actSplitRight(tr("Split right"));
    menu.addAction(&actSplitRight);
    connect(&actSplitRight, &QAction::triggered, [this] { emit splitRequested(Qt::Horizontal); });

    //向下分割
    QAction actSplitDown(tr("Split down"));
    menu.addAction(&actSplitDown);
    connect(&actSplitDown, &QAction::triggered, [this] { emit splitRequested(Qt::Vertical); });

    menu.addSeparator();    //分割线

    //移除
    QAction actRemove(tr("Remove"));
    menu.addAction(&actRemove);
    connect(&actRemove, &QAction::triggered, [this] { emit removeRequested(); });
    
    //设置"移除"按钮的可用性
    bool removeable = false;
    emit removeableRequested(removeable);
    actRemove.setEnabled(removeable);
    
    menu.move(mTabBtn->mapToGlobal(QPoint(0, mTabBtn->height())));
    menu.exec();
}
