#pragma once

// #include <QWidget>
// #include <QPlainTextEdit>
// #include <QLabel>
// #include <QSplitter>

// #include <QMouseEvent>
// #include <QHBoxLayout>
// #include <QVBoxLayout>
// #include <QTimer>
// #include <QTime>
//#include <QProcess>
// #include <QDesktopServices>
// #include <QFileIconProvider>

// #include <Lib/PlainButton/plainbutton.h>
// #include "Widget/Other/colorwidget.h"
// #include "notewidget.h"
#include "plaintextedit.h"
// #include "outputwidget.h"
#include "header.h"
//#include "Class/Parser/parser.h"
// #include "Class/Parser/parser.h"
// #include "Class/ProjListWidgetItem/projlistwidgetitem.h"
#include "Class/texthighlighter.h"

class ProjWidget : public PlainTextEdit
{
    Q_OBJECT
protected:
    void focusInEvent(QFocusEvent *ev) override;
    // void changeEvent(QEvent *ev) override;

public:
    explicit ProjWidget(const CanonicalFilePath &projPath, QWidget *parent = nullptr);

    bool load();    //读取
    bool save();    //保存

    void setSaved(bool _isSaved);

    // void updateTr();

    VAR_GET_FUNC(projName, mProjName, QString)
    VAR_GET_FUNC(projPath, mProjPath, CanonicalFilePath)
    VAR_GET_FUNC(isSaved, mSaved, bool)

    // VAR_GET_FUNC(editWidget, mEdit, PlainTextEdit*)

signals:
    void focused();
    void stateChanged(bool mSaved);    //当保存情况改变时发出的信号
    // void processItemDbClick(ProjListWidgetItem *item);
    // void beforeParse(bool &cancel);    //若将cancel设置为true，则取消分析

private slots:
    // void onListWidgetDoubleClicked(QListWidgetItem *item);

    // void onParse();

private:
    QString mProjName;   //项目名称
    CanonicalFilePath mProjPath;   //项目路径

    bool mSaved = true;    //是否保存

    // PlainTextEdit *mEdit = new PlainTextEdit;
    TextHighlighter *mHighlighter = new TextHighlighter(document());

    // NoteWidget *mNoteWidget = new NoteWidget;
    // PlainButton *mBtnParse = new PlainButton;
    // OutputWidget *mOutputWidget = new OutputWidget;

    // QSplitter *mSplitter = new QSplitter(Qt::Vertical);
};
