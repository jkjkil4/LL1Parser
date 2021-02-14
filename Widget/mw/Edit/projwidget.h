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
#include "Class/parser.h"

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

    VAR_GET_FUNC(ProjName, mProjName, QString)
    VAR_GET_FUNC(ProjPath, mProjPath, QString)
    VAR_GET_FUNC(IsSaved, mIsSaved, bool)

signals:
    void stateChanged(bool mIsSaved);    //当保存情况改变时发出的信号

private slots:
    void onParse();

private:
    QString mProjName;   //项目名称
    QString mProjPath;   //项目路径

    bool mIsSaved = true;    //是否保存

    PlainTextEdit *mEdit = new PlainTextEdit;
    NoteWidget *mNoteWidget = new NoteWidget;
    PlainButton *mBtnParse = new PlainButton;
};
