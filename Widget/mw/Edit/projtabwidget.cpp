#include "projtabwidget.h"

ProjTabWidget::ProjTabWidget(QWidget *parent) : QWidget(parent)
{
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setExpanding(false);
    mTabBar->setDrawBase(false);
    mTabBar->setMinimumWidth(32);

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

    ColorWidget *tabBarWidget = new ColorWidget(Qt::lightGray);
    tabBarWidget->setLayout(layTabBar);

    QHBoxLayout *layStackedWidget = new QHBoxLayout;
    layStackedWidget->setMargin(0);
    layStackedWidget->addWidget(mStackedWidget);

    ColorWidget *stackedWidgetBackground = new ColorWidget(QColor(240, 240, 240));
    stackedWidgetBackground->setLayout(layStackedWidget);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(tabBarWidget);
    layMain->addWidget(stackedWidgetBackground);
    setLayout(layMain);

    setFocusPolicy(Qt::ClickFocus);
}

void ProjTabWidget::focusInEvent(QFocusEvent *) {
    emit focused();
}

void ProjTabWidget::onTabBtnClicked() {
    QMenu menu;
    
    QAction actSplitRight(tr("Split right"));
    menu.addAction(&actSplitRight);
    connect(&actSplitRight, &QAction::triggered, [this] { emit splitRequested(Qt::Horizontal); });

    QAction actSplitDown(tr("Split down"));
    menu.addAction(&actSplitDown);
    connect(&actSplitDown, &QAction::triggered, [this] { emit splitRequested(Qt::Vertical); });
    
    menu.move(mTabBtn->mapToGlobal(QPoint(0, mTabBtn->height())));
    menu.exec();
}
