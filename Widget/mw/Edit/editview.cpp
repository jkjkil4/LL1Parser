#include "editview.h"

EditView::EditView(QWidget *parent) : MainWindowView(parent)
{
    //设置tabWidget的属性
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(tabWidget);

    setLayout(layMain);
}

void EditView::open(const QString &projPath) {
    QFileInfo info(projPath);
    QString canonicalProjPath = info.canonicalFilePath();

    int count = tabWidget->count();
    bool has = false;
    repeat(int, i, count) { //遍历所有已打开的项目
        ProjWidget *widget = (ProjWidget*)tabWidget->widget(i);
        if(widget->getProjPath() == canonicalProjPath) {    //如果项目已打开
            tabWidget->setCurrentIndex(i);  //切换到该项目
            has = true;
            break;
        }
    }

    if(!has) {  //如果项目未打开，则添加页面打开项目
        ProjWidget *widget = new ProjWidget(canonicalProjPath);
        tabWidget->addTab(widget, info.completeBaseName());
        tabWidget->setCurrentWidget(widget);
    }
}
