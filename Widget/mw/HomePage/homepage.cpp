#include "homepage.h"
#include <QCoreApplication>

//#include <QDebug>

HomePage::HomePage(QWidget *parent)
    : MainWindowView(parent)
{
    mRflWidget->setSelectionMode(RFLWidget::SelectionMode::NoSelection);
    mRflWidget->loadList();
    mRflWidget->resize(400, 300);
    mRflWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mRflWidget->setVerticalScrollMode(QListView::ScrollPerPixel);

    QFont titleFont = mLabRfl->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);

    mLabRfl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mLabRfl->setFont(titleFont);

    mLabAdd->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mLabAdd->setFont(titleFont);

    j::LimitSize(mBtnNewProj, 200, 50);
    j::SetPointSize(mBtnNewProj, 11);

    j::LimitSize(mBtnOpenProj, 200, 50);
    j::SetPointSize(mBtnOpenProj, 11);


    //创建布局
    QVBoxLayout *layRfl = new QVBoxLayout;
    layRfl->setSpacing(10);
    layRfl->addWidget(mLabRfl);
    layRfl->addWidget(mRflWidget);

    QVBoxLayout *layBtns = new QVBoxLayout;
    layBtns->setSpacing(10);
    layBtns->addWidget(mLabAdd);
    layBtns->addWidget(mBtnNewProj);
    layBtns->addWidget(mBtnOpenProj);
    layBtns->addStretch();

    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(30);
    layMain->setSpacing(20);
    layMain->addLayout(layRfl);
    layMain->addLayout(layBtns);

    setLayout(layMain);
    updateTr();
}

void HomePage::updateTr() {
    mLabRfl->setText(tr("Recent Projects"));
    mLabAdd->setText(tr("Add Project"));
    mBtnNewProj->setText(tr("New Project"));
    mBtnOpenProj->setText(tr("Open Project"));
}

void HomePage::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
