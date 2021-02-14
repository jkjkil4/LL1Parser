#include "newprojdialog.h"
#include <QCoreApplication>

NewProjDialog::NewProjDialog(QWidget *parent) : QDialog(parent)
{
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    mEditPath->setText(config.value("Path/ProjDefault").toString());

    QLabel *labTitle = new QLabel(tr("New Project"));		//标题文本
    j::SetPointSize(labTitle, 18);
    j::SetBold(labTitle, true);

    QLabel *labEditPath = new QLabel(tr("Project Path") + ":");	//"Project Path"文本
    QLabel *labEditName = new QLabel(tr("Project Name") + ":"); //"Project Name"文本

    mEditPath->setValidator(mRegExpDir);  //输入框的正则表达式，下同
    mEditName->setValidator(mRegExpFile);

    mBtnBrowse->setFocusPolicy(Qt::NoFocus);
    mBtnCancel->setFocusPolicy(Qt::NoFocus);

    mInfoWidget->setPointSize(10);

    QWidget *centralWidget = new QWidget;	//中心控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);	//底部控件

    //绑定信号与槽
    connect(mEditPath, SIGNAL(textChanged(const QString&)), this, SLOT(onCheck()));
    connect(mEditName, SIGNAL(textChanged(const QString&)), this, SLOT(onCheck()));
    connect(mBtnBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));
    connect(mBtnOK, SIGNAL(clicked()), this, SLOT(onAccept()));
    connect(mBtnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    QGridLayout *layEdit = new QGridLayout;     //网格布局
    layEdit->addWidget(labEditPath, 0, 0);
    layEdit->addWidget(mEditPath, 0, 1);
    layEdit->addWidget(mBtnBrowse, 0, 2);
    layEdit->addWidget(labEditName, 1, 0);
    layEdit->addWidget(mEditName, 1, 1, 1, 2);

    QVBoxLayout *layCentral = new QVBoxLayout;	//中心布局
    layCentral->setMargin(24);
    layCentral->addWidget(labTitle);
    layCentral->addSpacing(10);
    layCentral->addLayout(layEdit);
    layCentral->addWidget(mCheckSetToDefault, 0, Qt::AlignLeft);
    layCentral->addSpacing(5);
    layCentral->addWidget(mInfoWidget);
    layCentral->addStretch();
    centralWidget->setLayout(layCentral);

    QHBoxLayout *layBottom = new QHBoxLayout;	//底部布局
    layBottom->addStretch();
    layBottom->addWidget(mBtnOK);
    layBottom->addWidget(mBtnCancel);
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
    QString path = mEditPath->text();
    QString name = mEditName->text();

    QDir dir(path);
    mBtnOK->setEnabled(false);	//暂时设置按钮不可用
    if(!QFileInfo(path).isAbsolute() || !dir.cd(path)) {	//如果路径不存在
        mInfoWidget->setData(tr("The project path does not exists"), QColor(245, 120, 120));
        return;
    }
    if(name.isEmpty()) {	//如果名称为空
        mInfoWidget->setData(tr("The project name is empty"), QColor(245, 120, 120));
        return;
    }
    name += "." SUFFIX;		//名称附加上后缀名
    if(dir.exists(name)) {	//如果文件存在
        if(QFileInfo(dir.absoluteFilePath(name)).isDir()) {	//判断是否与文件夹重复
            mInfoWidget->setData(tr("The project name(%1) is duplicated with a folder").arg(name), QColor(245, 120, 120));
            return;
        }
        mInfoWidget->setData(tr("Project already exists"), QColor(245, 245, 120));	//显示警告，但是不视为错误
    } else mInfoWidget->clear();
    mBtnOK->setEnabled(true);	//设置按钮可用
}

void NewProjDialog::onBrowse() {
    QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Path"), config.value("Path/NewProj").toString());
    if(path.isEmpty())
        return;
    config.setValue("Path/NewProj", path);
    mEditPath->setText(path);
}

void NewProjDialog::onAccept() {
    onCheck();
    if(!mBtnOK->isEnabled())
        return;
    if(mCheckSetToDefault->isChecked()) {
        QSettings config(APP_DIR + "/Config/config.ini", QSettings::IniFormat);
        config.setValue("Path/ProjDefault", mEditPath->text());
    }
    accept();
}

void NewProjDialog::onCancel() {
    reject();
}
