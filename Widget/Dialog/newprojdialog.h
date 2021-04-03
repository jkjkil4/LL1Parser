#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QCheckBox>
#include <QPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSettings>
#include <QRegExpValidator>

//#include <Lib/PlainButton/plainbutton.h>
#include "Widget/Other/colorwidget.h"
#include "header.h"
#include "Widget/Other/infowidget.h"

/*
NewProjDialog
用来让用户新建项目
*/
class NewProjDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewProjDialog(QWidget *parent = nullptr);

    QString projPath() const { return mEditPath->text(); }
    QString projName() const { return mEditName->text(); }

private slots:
    void onBrowse();	//浏览文件
    void onCheck();	//检查路径和文件名

    void onAccept();
    void onCancel();

private:
    //输入框的正则表达式
    QRegExpValidator *mRegExpDir = new QRegExpValidator(QRegExp("[^*?\"<>|]*"), this);
    QRegExpValidator *mRegExpFile = new QRegExpValidator(QRegExp(/*"[A-Za-z0-9_-]*"*/"[^/\\\\*?\"<>|]*"), this);

    QLineEdit *mEditPath = new QLineEdit;	//用于编辑路径
    QLineEdit *mEditName = new QLineEdit;	//用于编辑名称

    //PlainButton *btnBrowse = new PlainButton(tr("Browse") + "...");	//用于浏览文件
    QPushButton *mBtnBrowse = new QPushButton(tr("Browse") + "...");	//用于浏览文件

    QCheckBox *mCheckSetToDefault = new QCheckBox(tr("Use as default project path"));    //用于选择是否作为默认路径

    InfoWidget *mInfoWidget = new InfoWidget;	//用于显示提示信息

    //PlainButton *btnOK = new PlainButton(tr("OK"));         //"确认"按钮
    //PlainButton *btnCancel = new PlainButton(tr("Cancel"));	//"取消"按钮
    QPushButton *mBtnOK = new QPushButton(tr("OK"));         //"确认"按钮
    QPushButton *mBtnCancel = new QPushButton(tr("Cancel"));	//"取消"按钮
};

