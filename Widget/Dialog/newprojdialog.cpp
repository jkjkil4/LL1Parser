#include "newprojdialog.h"
#include <QCoreApplication>

NewProjDialog::NewProjDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *labTitle = new QLabel(tr("New Project"));		//标题文本
    j::SetPointSize(labTitle, 18);

    QLabel *labEditPath = new QLabel(tr("Project Path") + ":");	//"Project Path"文本
    QLabel *labEditName = new QLabel(tr("Project Name") + ":"); //"Project Name"文本

    QWidget *centralWidget = new QWidget;	//中心控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);	//底部控件

    connect(btnBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));


    QGridLayout *layEdit = new QGridLayout;     //网格布局
    layEdit->addWidget(labEditPath, 0, 0);
    layEdit->addWidget(editPath, 0, 1);
    layEdit->addWidget(btnBrowse, 0, 2);
    layEdit->addWidget(labEditName, 1, 0);
    layEdit->addWidget(editName, 1, 1, 1, 2);

    QVBoxLayout *layCentral = new QVBoxLayout;	//中心布局
    layCentral->setMargin(24);
    layCentral->addWidget(labTitle);
    layCentral->addSpacing(5);
    layCentral->addLayout(layEdit);
    layCentral->addStretch();
    centralWidget->setLayout(layCentral);

    QHBoxLayout *layBottom = new QHBoxLayout;	//底部布局
    layBottom->addStretch();
    layBottom->addWidget(btnOK);
    layBottom->addWidget(btnCancel);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layMain = new QVBoxLayout;     //主布局
    layMain->setMargin(0);
    layMain->addWidget(centralWidget);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);


    resize(560, 380);	//设置大小
}

void NewProjDialog::onBrowse() {
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Path"), config.value("Path/NewProj").toString());
    if(path.isEmpty())
        return;
    config.setValue("Path/NewProj", path);
    editPath->setText(path);
}

void NewProjDialog::onAccept() {
    accept();
}

void NewProjDialog::onCancel() {
    reject();
}
