#include "homepage.h"
#include <QCoreApplication>

#include <QDebug>

HomePage::HomePage(QWidget *parent)
    : MainWindowView(parent)
{
    rflWidget->setPath(APP_DIR + "/Config/rfl.txt");
    rflWidget->updateList();
    rflWidget->resize(400, 300);
    rflWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    rflWidget->setVerticalScrollMode(QListView::ScrollPerPixel);
    connect(rflWidget, SIGNAL(itemClicked(const QString&)), this, SLOT(onOpenProject(const QString&)));

    QFont titleFont = labRfl->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);

    labRfl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labRfl->setFont(titleFont);

    labAdd->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labAdd->setFont(titleFont);

    j::LimitSize(btnNewProj, 200, 50);
    j::SetPointSize(btnNewProj, 11);

    j::LimitSize(btnOpenProj, 200, 50);
    j::SetPointSize(btnOpenProj, 11);


    //创建布局
    QVBoxLayout *layRfl = new QVBoxLayout;
    layRfl->setSpacing(10);
    layRfl->addWidget(labRfl);
    layRfl->addWidget(rflWidget);

    QVBoxLayout *layBtns = new QVBoxLayout;
    layBtns->setSpacing(10);
    layBtns->addWidget(labAdd);
    layBtns->addWidget(btnNewProj);
    layBtns->addWidget(btnOpenProj);
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
    labRfl->setText(tr("Recent Projects"));
    labAdd->setText(tr("Add Project"));
    btnNewProj->setText(tr("New Project"));
    btnOpenProj->setText(tr("Open Project"));
}

void HomePage::onOpenProject(const QString& path) {

}

void HomePage::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
