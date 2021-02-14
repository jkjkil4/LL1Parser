#include "editview.h"

EditView::EditView(QWidget *parent) : MainWindowView(parent)
{
    //设置tabWidget的属性
    mTabWidget->setTabsClosable(true);
    mTabWidget->setMovable(true);

    connect(mTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(mTabWidget);

    setLayout(layMain);
}

void EditView::open(const QString &projPath) {
    QFileInfo info(projPath);
    QString canonicalProjPath = info.canonicalFilePath();

    int count = mTabWidget->count();
    bool has = false;
    repeat(int, i, count) { //遍历所有已打开的项目
        ProjWidget *widget = (ProjWidget*)mTabWidget->widget(i);
        if(widget->getProjPath() == canonicalProjPath) {    //如果项目已打开
            mTabWidget->setCurrentIndex(i);  //切换到该项目
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
        mTabWidget->addTab(widget, " " + widget->getProjName() + " ");
        mTabWidget->setCurrentWidget(widget);
        connect(widget, &ProjWidget::stateChanged, [this, widget](bool isSaved){
            mTabWidget->setTabText(mTabWidget->indexOf(widget), " " + (isSaved ? widget->getProjName() : widget->getProjName() + "*") + " ");
        });
    }
}


bool EditView::confirmClose(int index) {
    ProjWidget *widget = (ProjWidget*)mTabWidget->widget(index);

    int res = QMessageBox::information(this, "", tr("Do you want to save the project \"%1\"?").arg(widget->getProjName()),
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

    if(res == QMessageBox::Cancel)  //如果选择了取消，则return
        return false;
    if(res == QMessageBox::Yes && !widget->save()) {    //如果选择了保存
        //如果保存失败，则提示是否强制关闭
        int res2 = QMessageBox::warning(this, tr("Error"), tr("Cannot save the project \"%1\", force it to close?").arg(widget->getProjName()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(res2 == QMessageBox::No) //如果不强制关闭，则return
            return false;
    }

    return true;
}

bool EditView::closeAll() {
    bool viewChanged = false;
    int count = mTabWidget->count();
    for(int i = count - 1; i >= 0; i--) {     //遍历所有打开的项目
        ProjWidget *widget = (ProjWidget*)mTabWidget->widget(i);
        if(!widget->getIsSaved()) {     //如果该项目未保存
            //将视图切换到该项目
            mTabWidget->setCurrentIndex(i);
            if(!viewChanged) {
                changeView("Edit");
                viewChanged = true;
            }

            //关闭项目
            if(!confirmClose(i))
                return false;
        }

        mTabWidget->removeTab(i);    //从tabWidget中移除该控件
    }
    return true;
}



void EditView::onTabCloseRequested(int index) {
    ProjWidget *widget = (ProjWidget*)mTabWidget->widget(index);
    if(!widget->getIsSaved() && !confirmClose(index))
        return;

    mTabWidget->removeTab(index);    //从tabWidget中移除该控件

    if(mTabWidget->count() == 0) {
        emit changeView("HomePage");
    }
    return;
}

