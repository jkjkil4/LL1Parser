#include "delayedverifydialog.h"

DelayedVerifyDialog::DelayedVerifyDialog(const QString &text, QWidget *parent) 
    : QDialog(parent) 
{
    mLabText->setText(text); //设置文字
    mTimer->setSingleShot(true);    //定时器
    QWidget *topWidget = new QWidget;   //顶部控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);  //底部控件

    //信号与槽
    connect(mTimer, &QTimer::timeout, this, &DelayedVerifyDialog::show);
    connect(mBtnAccept, &QPushButton::clicked, this, &DelayedVerifyDialog::accept);

    //创建布局
    QHBoxLayout *layTop = new QHBoxLayout;
    layTop->addWidget(mLabText);
    topWidget->setLayout(layTop);

    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addStretch();
    layBottom->addWidget(mBtnAccept);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(topWidget, 1);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);

    //窗口背景
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setPalette(pal);
}

void DelayedVerifyDialog::delayedVerify(int delayMsec) {
    mTimer->start(delayMsec);
}
