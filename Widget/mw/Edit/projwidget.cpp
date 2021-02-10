#include "projwidget.h"

ProjWidget::ProjWidget(const QString &projPath, QWidget *parent) : QWidget(parent), projPath(projPath)
{
    ColorWidget *bottomWidget = new ColorWidget;


    //创建布局
    QHBoxLayout *layBtn = new QHBoxLayout;
    layBtn->addWidget(btnParse, 0, Qt::AlignRight);
    bottomWidget->setLayout(layBtn);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->setSpacing(0);
    layMain->addWidget(edit);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);


    updateTr();
}

void ProjWidget::updateTr() {
    btnParse->setText(tr("Parse"));
}

void ProjWidget::changeEvent(QEvent *ev) {
    if(ev->type() == QEvent::LanguageChange) {
        updateTr();
    }
}
