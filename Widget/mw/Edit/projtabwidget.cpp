#include "projtabwidget.h"

#include <QLabel>

ProjTabWidget::ProjTabWidget(ProjTabManager *pPtManager, QWidget *parent) 
    : QWidget(parent), pPtManager(pPtManager)
{
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setExpanding(false);
    mTabBar->setDrawBase(false);
    mTabBar->setMinimumWidth(32);
    connect(mTabBar, &QTabBar::tabMoved, this, &ProjTabWidget::onTabMoved);
    connect(mTabBar, &QTabBar::currentChanged, this, &ProjTabWidget::onCurrentChanged);
    connect(mTabBar, &QTabBar::tabCloseRequested, this, &ProjTabWidget::onTabCloseRequested);

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

bool ProjTabWidget::contains(const CanonicalFilePath &cFilePath) const {
    int count = mStackedWidget->count();
    repeat(int, i, count) {     //遍历所有项目
        ProjWidget *widget = (ProjWidget*)mStackedWidget->widget(i);
        if(widget->projPath() == cFilePath)
            return true;
    }
    return false;
}
ProjWidget* ProjTabWidget::open(const CanonicalFilePath &cFilePath) {
    QFileInfo info(cFilePath);
    if(!info.exists()) {
        QMessageBox::critical(this, tr("Error"), tr("\"%1\" does not exists").arg(cFilePath));
        return nullptr;
    }

    ProjWidget *projWidget = new ProjWidget(cFilePath);
    if(!projWidget->load()) {
        delete projWidget;
        QMessageBox::critical(this, tr("Error"), tr("Cannot load the project \"%1\"").arg(projWidget->projName()));
        return nullptr;
    }
    connect(projWidget, SIGNAL(focused()), this, SIGNAL(focused()));
    mStackedWidget->addWidget(projWidget);
    mTabBar->addTab(" " + info.completeBaseName() + " ");
    mTabBar->setCurrentIndex(mTabBar->count() - 1);
    connect(projWidget, &ProjWidget::stateChanged, [this, projWidget](bool isSaved) {
        mTabBar->setTabText(mStackedWidget->indexOf(projWidget), 
            " " + (isSaved ? projWidget->projName() : projWidget->projName() + "*") + " ");
    });
    return projWidget;
}

void ProjTabWidget::setCurrent(ProjWidget *w) {
    int index = mStackedWidget->indexOf(w);
    if(index != -1)
        mTabBar->setCurrentIndex(index);
}

bool ProjTabWidget::confirmClose(int index) {
    ProjWidget *widget = (ProjWidget*)mStackedWidget->widget(index);

    int res = QMessageBox::information(this, "", tr("Do you want to save the project \"%1\"?").arg(widget->projName()),
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

    if(res == QMessageBox::Cancel)  //如果选择了取消，则return
        return false;
    if(res == QMessageBox::Yes && !widget->save()) {    //如果选择了保存
        //如果保存失败，则提示是否强制关闭
        int res2 = QMessageBox::warning(this, tr("Error"), tr("Cannot save the project \"%1\", force it to close?").arg(widget->projName()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(res2 == QMessageBox::No) //如果不强制关闭，则return
            return false;
    }

    return true;
}
bool ProjTabWidget::closeAll() {
    bool viewChanged = false;
    int count = mStackedWidget->count();
    for(int i = count - 1; i >= 0; i--) {     //遍历所有打开的项目
        ProjWidget *widget = (ProjWidget*)mStackedWidget->widget(i);
        if(!widget->isSaved()) {     //如果该项目未保存
            //将视图切换到该项目
            mTabBar->setCurrentIndex(i);
            if(!viewChanged) {
                emit changeViewRequested();
                viewChanged = true;
            }

            //关闭项目
            if(!confirmClose(i))
                return false;
        }

        mStackedWidget->removeWidget(widget);    //移除该控件
        mTabBar->removeTab(i);
    }
    return true;
}

void ProjTabWidget::focusInEvent(QFocusEvent *) {
    if(QRect(0, 0, width(), height()).contains(mapFromGlobal(cursor().pos())))
        emit focused();
}

void ProjTabWidget::onTabMoved(int from, int to) {
    QSignalBlocker blocker(mStackedWidget);
    QWidget *w = mStackedWidget->widget(from);
    mStackedWidget->removeWidget(w);
    mStackedWidget->insertWidget(to, w);
}
void ProjTabWidget::onCurrentChanged(int index) {
    mStackedWidget->setCurrentIndex(index);
}
void ProjTabWidget::onTabCloseRequested(int index) {
    ProjWidget *widget = (ProjWidget*)mStackedWidget->widget(index);
    if(!widget->isSaved() && !confirmClose(index))
        return;
    
    mStackedWidget->removeWidget(widget);   //移除该控件
    mTabBar->removeTab(index);
}

void ProjTabWidget::onTabBtnClicked() {
    QMenu menu;
    
    //向右分割
    QAction actSplitRight(tr("Split right") + "(&X)");
    menu.addAction(&actSplitRight);
    connect(&actSplitRight, &QAction::triggered, [this] { emit splitRequested(Qt::Horizontal); });

    //向下分割
    QAction actSplitDown(tr("Split down") + "(&Y)");
    menu.addAction(&actSplitDown);
    connect(&actSplitDown, &QAction::triggered, [this] { emit splitRequested(Qt::Vertical); });

    // //打开新窗口
    // QAction actNewWnd(tr("Open new window"));
    // menu.addAction(&actNewWnd);
    // connect(&actNewWnd, SIGNAL(triggered()), this, SIGNAL(newWindowRequested()));

    menu.addSeparator();    //分割线

    //移除
    QAction actRemove(tr("Remove") + "(&R)");
    menu.addAction(&actRemove);
    connect(&actRemove, SIGNAL(triggered()), this, SIGNAL(removeRequested()));
    
    //设置"移除"按钮的可用性
    bool removeable = false;
    emit removeableRequested(removeable);
    actRemove.setEnabled(removeable);
    
    menu.move(mTabBtn->mapToGlobal(QPoint(0, mTabBtn->height())));
    menu.exec();
}
