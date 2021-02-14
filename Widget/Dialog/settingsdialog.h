#pragma once

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>

//#include <Lib/PlainButton/plainbutton.h>

#include "Widget/Other/colorwidget.h"
#include "Class/translator.h"
#include "header.h"

/*
SettingsWidget
用来让用户对相关选项进行设置
*/
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onAccept();
    void onCancel();

private:
    void initCbbLanguage();     //设置语言选择框内容

    QComboBox *mCbbLanguage = new QComboBox;     //语言选择框

//    PlainButton *btnOK = new PlainButton(tr("OK"));         //"确定"按钮
//    PlainButton *btnCancel = new PlainButton(tr("Cancel"));	//"取消"按钮
    QPushButton *mBtnOK = new QPushButton(tr("OK"));         //"确定"按钮
    QPushButton *mBtnCancel = new QPushButton(tr("Cancel"));	//"取消"按钮
};
