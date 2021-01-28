#include "newprojdialog.h"
#include <QCoreApplication>

NewProjDialog::NewProjDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *labTitle = new QLabel(tr("New Project"));		//标题文本
    j::SetPointSize(labTitle, 18);
    j::SetBold(labTitle, true);

    QLabel *labEditPath = new QLabel(tr("Project Path") + ":");	//"Project Path"文本
    QLabel *labEditName = new QLabel(tr("Project Name") + ":"); //"Project Name"文本

    editPath->setValidator(regExpDir);  //输入框的正则表达式，下同
    editName->setValidator(regExpFile);

    infoWidget->setPointSize(10);

    QWidget *centralWidget = new QWidget;	//中心控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);	//底部控件

    connect(editPath, &QLineEdit::textChanged, [this]{ check(); });
    connect(editName, &QLineEdit::textChanged, [this]{ check(); });
    connect(btnBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(onAccept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    QGridLayout *layEdit = new QGridLayout;     //网格布局
    layEdit->addWidget(labEditPath, 0, 0);
    layEdit->addWidget(editPath, 0, 1);
    layEdit->addWidget(btnBrowse, 0, 2);
    layEdit->addWidget(labEditName, 1, 0);
    layEdit->addWidget(editName, 1, 1, 1, 2);

    QVBoxLayout *layCentral = new QVBoxLayout;	//中心布局
    layCentral->setMargin(24);
    layCentral->addWidget(labTitle);
    layCentral->addSpacing(10);
    layCentral->addLayout(layEdit);
    layCentral->addSpacing(5);
    layCentral->addWidget(infoWidget);
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
    check();
}

void NewProjDialog::check() {
    QString path = editPath->text();
    QString name = editName->text();
    QDir dir(path);
    btnOK->setEnabled(false);
    if(!QFileInfo(path).isAbsolute() || !dir.cd(path)) {
        infoWidget->setData(tr("Project path does not exists"), QColor(245, 120, 120));
        return;
    }
    if(name.isEmpty()) {
        infoWidget->setData(tr("Project name is empty"), QColor(245, 120, 120));
        return;
    }
    if(dir.exists(name)) {
        infoWidget->setData(tr("Project already exists"), QColor(245, 245, 120));
    } else infoWidget->clear();;
    btnOK->setEnabled(true);
}

void NewProjDialog::onBrowse() {
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Path"), config.value("Path/NewProj").toString());
    if(path.isEmpty())
        return;
    config.setValue("Path/NewProj", QFileInfo(path).path());
    editPath->setText(path);
}

void NewProjDialog::onAccept() {
    accept();
}

void NewProjDialog::onCancel() {
    reject();
}
