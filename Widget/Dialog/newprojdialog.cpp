#include "newprojdialog.h"

NewProjDialog::NewProjDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *labTitle = new QLabel(tr("New Project"));
    j::SetPointSize(labTitle, 18);

    QWidget *centralWidget = new QWidget;
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);

    QVBoxLayout *layCentral = new QVBoxLayout;
    layCentral->setMargin(24);
    layCentral->addWidget(labTitle);
    layCentral->addStretch();
    centralWidget->setLayout(layCentral);

    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addStretch();
    layBottom->addWidget(btnOK);
    layBottom->addWidget(btnCancel);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->setMargin(0);
    layMain->addWidget(centralWidget);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);
}
