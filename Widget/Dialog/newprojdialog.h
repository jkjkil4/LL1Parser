#ifndef NEWPROJDIALOG_H
#define NEWPROJDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSettings>
#include <QRegExpValidator>

#include <Lib/PlainButton/plainbutton.h>
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

    QString projPath() { return editPath->text(); }
    QString projName() { return editName->text(); }

private slots:
    void onBrowse();

    void onAccept();
    void onCancel();

private:
    void check();

    QRegExpValidator *regExpDir = new QRegExpValidator(QRegExp("[^*?\"<>|]*"), this);
    QRegExpValidator *regExpFile = new QRegExpValidator(QRegExp("[A-Za-z0-9_-]*"), this);

    QLineEdit *editPath = new QLineEdit;	//用于编辑路径
    QLineEdit *editName = new QLineEdit;	//用于编辑名称

    PlainButton *btnBrowse = new PlainButton(tr("Browse") + "...");	//用于浏览文件

    InfoWidget *infoWidget = new InfoWidget;

    PlainButton *btnOK = new PlainButton(tr("OK"));         //"确认"按钮
    PlainButton *btnCancel = new PlainButton(tr("Cancel"));	//"取消"按钮
};

#endif // NEWPROJDIALOG_H
