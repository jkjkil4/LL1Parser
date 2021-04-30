#include "outputwidget.h"

OutputWidget::OutputWidget(QWidget *parent) : QWidget(parent)
{
    for(const Tab &tab : tabs)
        mTabWidget->addTab(tab.p, tab.fn());

    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(mTabWidget);
    setLayout(layMain);
}

void OutputWidget::setCurrentWidget(QWidget *widget) {
    mTabWidget->setCurrentWidget(widget);
}

void OutputWidget::clear() {
    mErrListWidget->clear();
    mOutputListWidget->clear();
}
