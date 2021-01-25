#include "newprojdialog.h"

NewProjDialog::NewProjDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *labTitle = new QLabel(tr("New Project"));
    j::SetPointSize(labTitle, 18);

    QLabel *labEditPath = new QLabel(tr("Project Path") + ":");
    QLabel *labEditName = new QLabel(tr("Project Name") + ":");

    QWidget *centralWidget = new QWidget;
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);


    QGridLayout *layEdit = new QGridLayout;
    layEdit->addWidget(labEditPath, 0, 0);
    layEdit->addWidget(editPath, 0, 1);
    layEdit->addWidget(btnBrowse, 0, 2);
    layEdit->addWidget(labEditName, 1, 0);
    layEdit->addWidget(editName, 1, 1, 1, 2);

    QVBoxLayout *layCentral = new QVBoxLayout;
    layCentral->setMargin(24);
    layCentral->addWidget(labTitle);
    layCentral->addSpacing(5);
    layCentral->addLayout(layEdit);
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


    resize(560, 380);
}
