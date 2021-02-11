#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include <Lib/PlainButton/plainbutton.h>
#include "Widget/Other/colorwidget.h"
#include "notewidget.h"
#include "plaintextedit.h"
#include "header.h"

class ProjWidget : public QWidget
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;

public:
    explicit ProjWidget(const QString &projPath, QWidget *parent = nullptr);

    bool load();    //读取
    bool save();    //保存

    void setSaved(bool _isSaved);

    void updateTr();

    VAR_GET_FUNC(ProjName, projName, QString)
    VAR_GET_FUNC(ProjPath, projPath, QString)
    VAR_GET_FUNC(IsSaved, isSaved, bool)

signals:
    void stateChanged(bool isSaved);    //当保存情况改变时发出的信号

private:
    QString projName;   //项目名称
    QString projPath;   //项目路径

    bool isSaved = true;    //是否保存

    PlainTextEdit *edit = new PlainTextEdit;
    NoteWidget *noteWidget = new NoteWidget;
    PlainButton *btnParse = new PlainButton;
};
