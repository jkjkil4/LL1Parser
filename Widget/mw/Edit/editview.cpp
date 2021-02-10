#include "editview.h"

EditView::EditView(QWidget *parent) : MainWindowView(parent)
{
    //设置tabWidget的属性
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));


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
        if(!widget->load()) {   //如果读取失败，则提示并return
            delete widget;
            QMessageBox::critical(this, tr("Error"), tr("Cannot load the project \"%1\"").arg(widget->getProjName()));
            return;
        }
        tabWidget->addTab(widget, widget->getProjName());
        tabWidget->setCurrentWidget(widget);
        connect(widget, &ProjWidget::stateChanged, [this, widget](bool isSaved){
            tabWidget->setTabText(tabWidget->indexOf(widget), isSaved ? widget->getProjName() : widget->getProjName() + "*");
        });
    }
}

void EditView::onTabCloseRequested(int index) {
    ProjWidget *widget = (ProjWidget*)tabWidget->widget(index);
    if(!widget->getIsSaved()) {
        int res = QMessageBox::information(this, "", tr("Do you want to save the project \"%1\"?").arg(widget->getProjName()),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

        if(res == QMessageBox::Cancel)  //如果选择了取消，则return
            return;
        if(res == QMessageBox::Yes && !widget->save()) {    //如果选择了保存
            //如果保存失败，则提示是否强制关闭
            int res2 = QMessageBox::warning(this, tr("Error"), tr("Cannot save the project \"%1\", force close?").arg(widget->getProjName()),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if(res2 == QMessageBox::No) //如果不强制关闭，则return
                return;
        }
    }

    tabWidget->removeTab(index);    //从tabWidget中移除该控件

    if(tabWidget->count() == 0) {
        emit changeView("HomePage");
    }
}
