#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *labLanguage = new QLabel(tr("Language") + ":");
    connect(mBtnOK, SIGNAL(clicked()), this, SLOT(onAccept()));
    connect(mBtnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    mBtnCancel->setFocusPolicy(Qt::NoFocus);

    QWidget *centralWidget = new QWidget;   //中心控件
    ColorWidget *bottomWidget = new ColorWidget(C_BOTTOM);	//底部控件


    //创建布局
    QHBoxLayout *layLanguage = new QHBoxLayout;		//选择语言
    layLanguage->addWidget(labLanguage);
    layLanguage->addWidget(mCbbLanguage, 1);

    QVBoxLayout *layCentral = new QVBoxLayout;		//中心布局
    layCentral->addLayout(layLanguage);
    layCentral->addStretch();
    centralWidget->setLayout(layCentral);

    QHBoxLayout *layBottom = new QHBoxLayout;		//底部按钮
    layBottom->addStretch();
    layBottom->addWidget(mBtnOK);
    layBottom->addWidget(mBtnCancel);
    bottomWidget->setLayout(layBottom);

    QVBoxLayout *layMain = new QVBoxLayout;         //主布局
    layMain->setMargin(0);
    layMain->addWidget(centralWidget, 1);
    layMain->addWidget(bottomWidget);
    setLayout(layMain);


    initCbbLanguage();		//设定语言选择框的内容
    setWindowTitle(tr("Settings"));     //设置窗口标题
}

void SettingsDialog::onAccept() {
    QString locale = mCbbLanguage->currentData().toString();
    if(locale != translator.locale())
        translator.setLocale(locale);
    accept();
}

void SettingsDialog::onCancel() {
    reject();
}

#define ADD_ITEM(text, data) mCbbLanguage->addItem(text, data)
void SettingsDialog::initCbbLanguage() {
    mCbbLanguage->clear();   //清空原有的选项

    //添加选项
    ADD_ITEM("<" + tr("Auto") + ">", "Auto");
    ADD_ITEM(tr("English") + " - " + "English", "");
    ADD_ITEM(tr("Chinese(Simplified)") + " - " + "中文(简体)", "zh_CN");

    //根据当前语言设定所选的选项
    QString locale = translator.locale();
    int count = mCbbLanguage->count();
    for(int i = 0; i < count; i++) {
        if(mCbbLanguage->itemData(i) == locale) {
            mCbbLanguage->setCurrentIndex(i);
            break;
        }
    }
}
#undef ADD_ITEM
