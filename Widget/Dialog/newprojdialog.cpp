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

    //绑定信号与槽
    connect(editPath, SIGNAL(textChanged(const QString&)), this, SLOT(onCheck()));
    connect(editName, SIGNAL(textChanged(const QString&)), this, SLOT(onCheck()));
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
    onCheck();
}

void NewProjDialog::onCheck() {
    //得到路径和名称
    QString path = editPath->text();
    QString name = editName->text();

    QDir dir(path);
    btnOK->setEnabled(false);	//暂时设置按钮不可用
    if(!QFileInfo(path).isAbsolute() || !dir.cd(path)) {	//如果路径不存在
        infoWidget->setData(tr("The project path does not exists"), QColor(245, 120, 120));
        return;
    }
    if(name.isEmpty()) {	//如果名称为空
        infoWidget->setData(tr("The project name is empty"), QColor(245, 120, 120));
        return;
    }
    name += "." SUFFIX;		//名称附加上后缀名
    if(dir.exists(name)) {	//如果文件存在
        if(QFileInfo(dir.absoluteFilePath(name)).isDir()) {	//判断是否与文件夹重复
            infoWidget->setData(tr("The project name(%1) is duplicated with a folder").arg(name), QColor(245, 120, 120));
            return;
        }
        infoWidget->setData(tr("Project already exists"), QColor(245, 245, 120));	//显示警告，但是不视为错误
    } else infoWidget->clear();
    btnOK->setEnabled(true);	//设置按钮可用
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
    onCheck();
    if(!btnOK->isEnabled())
        return;
    accept();
}

void NewProjDialog::onCancel() {
    reject();
}
