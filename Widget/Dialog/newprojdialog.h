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

#include <Lib/PlainButton/plainbutton.h>
#include "Class/colorwidget.h"
#include "header.h"

/*
NewProjDialog
用来让用户新建项目
*/
class NewProjDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewProjDialog(QWidget *parent = nullptr);

private slots:
    void onBrowse();

    void onAccept();
    void onCancel();

private:
    QLineEdit *editPath = new QLineEdit;	//用于编辑路径
    QLineEdit *editName = new QLineEdit;	//用于编辑名称

    PlainButton *btnBrowse = new PlainButton(tr("Browse") + "...");	//用于浏览文件

    PlainButton *btnOK = new PlainButton(tr("OK"));         //"确认"按钮
    PlainButton *btnCancel = new PlainButton(tr("Cancel"));	//"取消"按钮
};

#endif // NEWPROJDIALOG_H
