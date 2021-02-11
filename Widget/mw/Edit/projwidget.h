#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <Lib/PlainButton/plainbutton.h>
#include "Widget/Other/colorwidget.h"
#include "plaintextedit.h"
#include "header.h"

class ProjWidget : public QWidget
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;

public:
    explicit ProjWidget(const QString &projPath, QWidget *parent = nullptr);

    bool load();
    bool save();

    void setSaved(bool _isSaved);

    void updateTr();

    VAR_GET_FUNC(ProjName, projName, QString)
    VAR_GET_FUNC(ProjPath, projPath, QString)
    VAR_GET_FUNC(IsSaved, isSaved, bool)

signals:
    void stateChanged(bool isSaved);

private:
    QString projName;
    QString projPath;

    bool isSaved = true;

    PlainTextEdit *edit = new PlainTextEdit;
    PlainButton *btnParse = new PlainButton;
};
