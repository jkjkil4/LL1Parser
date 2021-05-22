#include "editview.h"

EditView::EditView(QWidget *parent) : MainWindowView(parent)
{
    //设置tabWidget的属性
    // mTabWidget->setTabsClosable(true);
    // mTabWidget->setMovable(true);
    // mTabWidget->setDocumentMode(true);

    // connect(mTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));


    //创建布局
    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    // layMain->addWidget(mTabWidget);
    layMain->addWidget(mTsWidget);

    setLayout(layMain);
}

ProjWidget* EditView::projContains(const CanonicalFilePath &cProjPath, ProjTabWidget **at) {
    for(ProjTabWidget *pt : ptManager.vec) {
        int count = pt->count();
        repeat(int, i, count) {
            ProjWidget *pw = pt->widget(i);
            if(pw->projPath() == cProjPath) {
                SET_PTR(at, pt);
                return pw;
            }
        }
    }
    SET_PTR(at, nullptr);
    return nullptr;
}

ProjWidget* EditView::open(const CanonicalFilePath &cProjPath) {
    // QFileInfo info(projPath);
    // if(!info.exists()) {
    //     QMessageBox::critical(this, tr("Error"), tr("\"%1\" does not exists").arg(projPath));
    //     return nullptr;
    // }
    // QString canonicalProjPath = info.canonicalFilePath();

    // int count = mTabWidget->count();
    // repeat(int, i, count) { //遍历所有已打开的项目
    //     ProjWidget *widget = (ProjWidget*)mTabWidget->widget(i);
    //     if(widget->projPath() == canonicalProjPath) {    //如果项目已打开
    //         mTabWidget->setCurrentIndex(i);  //切换到该项目
    //         return widget;
    //     }
    // }

    //如果项目未打开，则添加页面打开项目
    // ProjWidget *widget = new ProjWidget(cProjPath);
    // connect(widget, &ProjWidget::processItemDbClick, this, &EditView::onProcessItemDbClick);
    // connect(widget, &ProjWidget::beforeParse, this, &EditView::onBeforeParse);
    // if(!widget->load()) {   //如果读取失败，则提示并return
    //     delete widget;
    //     QMessageBox::critical(this, tr("Error"), tr("Cannot load the project \"%1\"").arg(widget->projName()));
    //     return nullptr;
    // }
    // mTabWidget->addTab(widget, " " + widget->projName() + " ");
    // mTabWidget->setCurrentWidget(widget);
    // connect(widget, &ProjWidget::stateChanged, [this, widget](bool isSaved){
    //     mTabWidget->setTabText(mTabWidget->indexOf(widget), " " + (isSaved ? widget->projName() : widget->projName() + "*") + " ");
    // });

    ProjTabWidget *at;
    ProjWidget *check = projContains(cProjPath, &at);
    if(!check)
        return ptManager.current->open(cProjPath);
    at->setFocus();
    TabSplitWidget::setCurrentPtWidget(&ptManager, at);
    at->setCurrent(check);
    return check;
}

bool EditView::hasProj() const {
    for(ProjTabWidget *pt : ptManager.vec) {
        if(pt->count() != 0)
            return true;
    }
    return false;
}

// bool EditView::confirmClose(int index) {
//     ProjWidget *widget = (ProjWidget*)mTabWidget->widget(index);

//     int res = QMessageBox::information(this, "", tr("Do you want to save the project \"%1\"?").arg(widget->projName()),
//                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

//     if(res == QMessageBox::Cancel)  //如果选择了取消，则return
//         return false;
//     if(res == QMessageBox::Yes && !widget->save()) {    //如果选择了保存
//         //如果保存失败，则提示是否强制关闭
//         int res2 = QMessageBox::warning(this, tr("Error"), tr("Cannot save the project \"%1\", force it to close?").arg(widget->projName()),
//                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
//         if(res2 == QMessageBox::No) //如果不强制关闭，则return
//             return false;
//     }

//     return true;
// }

bool EditView::closeAll() {
    // bool viewChanged = false;
    // int count = mTabWidget->count();
    // for(int i = count - 1; i >= 0; i--) {     //遍历所有打开的项目
    //     ProjWidget *widget = (ProjWidget*)mTabWidget->widget(i);
    //     if(!widget->isSaved()) {     //如果该项目未保存
    //         //将视图切换到该项目
    //         mTabWidget->setCurrentIndex(i);
    //         if(!viewChanged) {
    //             changeView("Edit");
    //             viewChanged = true;
    //         }

    //         //关闭项目
    //         if(!confirmClose(i))
    //             return false;
    //     }

    //     mTabWidget->removeTab(i);    //从tabWidget中移除该控件
    // }
    // return true;
    for(ProjTabWidget *pt : ptManager.vec) {
        if(!pt->closeAll())
            return false;
    }
    return true;
}

// void EditView::onTabCloseRequested(int index) {
//     ProjWidget *widget = (ProjWidget*)mTabWidget->widget(index);
//     if(!widget->isSaved() && !confirmClose(index))
//         return;

//     mTabWidget->removeTab(index);    //从tabWidget中移除该控件
//     delete widget;

//     if(mTabWidget->count() == 0) {
//         emit changeView("HomePage");
//     }
//     return;
// }

// void EditView::onProcessItemDbClick(ProjListWidgetItem *item) {
//     item->onDoubleClicked(this);
// }

// void EditView::onBeforeParse(bool &cancel) {
//     bool verify = false;

//     //遍历所有ProjWidget，如果有未保存的，则需要进行确认
//     int count = mTabWidget->count();
//     repeat(int, i, count) {
//         ProjWidget *projWidget = (ProjWidget*)mTabWidget->widget(i);
//         if(!projWidget->isSaved()) {
//             verify = true;
//             break;
//         }
//     }

//     if(verify) {    //如果需要确认
//         //从配置文件读取是否需要确认
//         QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
//         verify = !config.value("Config/AutoSaveBeforeParse", false).toBool();

//         //如果需要确认，则弹出提示框，否则自动保存
//         int res = CheckBoxVerifyDialog::Yes;
//         if(verify) {
//             CheckBoxVerifyDialog dialog(tr("Some projects have not yet saved,\ndo you want to save before parse?"),
//                                         tr("Auto save before parse"), this);
//             res = dialog.exec();

//             //如果结果为"Yes"且checkbox为checked，则将配置文件的AutoSaveBeforeParse设置为true
//             if(res == CheckBoxVerifyDialog::Yes && dialog.checked())
//                 config.setValue("Config/AutoSaveBeforeParse", true);
//         }
        
//         if(res == CheckBoxVerifyDialog::Cancel) {   //如果是取消，则取消分析
//             cancel = true;
//             return;
//         }
//         if(res == CheckBoxVerifyDialog::Yes) {  //如果是确定，则自动保存
//             repeat(int, i, count) {
//                 ProjWidget *projWidget = (ProjWidget*)mTabWidget->widget(i);
//                 if(!projWidget->isSaved())
//                     projWidget->save();
//             }
//         }
//     }
// }

