#include "outputwidget.h"

OutputWidget::OutputWidget(QWidget *parent) : QWidget(parent)
{
    for(const Tab &tab : tabs)
        tabWidget->addTab(tab.p, tab.fn());

    QHBoxLayout *layMain = new QHBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(tabWidget);
    setLayout(layMain);
}

void OutputWidget::setCurrentWidget(QWidget *widget) {
    tabWidget->setCurrentWidget(widget);
}
