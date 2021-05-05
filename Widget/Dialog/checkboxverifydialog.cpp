#include "checkboxverifydialog.h"

CheckBoxVerifyDialog::CheckBoxVerifyDialog(const QString &text, const QString &cbbText, QWidget *parent) 
    : QDialog(parent)
{
    mCheckBox->setText(cbbText);
    QWidget *topWidget = new QWidget;   //顶部控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);

    //信号与槽
    connect(mBtnCancel, &QPushButton::clicked, [this] { done(Cancel); });
    connect(mBtnNo, &QPushButton::clicked, [this] { done(No); });
    connect(mBtnYes, &QPushButton::clicked, [this] { done(Yes); });

    //创建布局
    QVBoxLayout *layTop = new QVBoxLayout;
    layTop->addWidget(new QLabel(text));
    layTop->addSpacing(6);
    layTop->addWidget(mCheckBox, 0, Qt::AlignLeft);
    topWidget->setLayout(layTop);

    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addStretch();
    layBottom->addWidget(mBtnCancel);
    layBottom->addWidget(mBtnNo);
    layBottom->addWidget(mBtnYes);
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