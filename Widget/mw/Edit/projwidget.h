#ifndef PROJWIDGET_H
#define PROJWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <Lib/PlainButton/plainbutton.h>
#include "Widget/Other/colorwidget.h"
#include "header.h"

class ProjWidget : public QWidget
{
    Q_OBJECT
protected:
    void changeEvent(QEvent *ev) override;

public:
    explicit ProjWidget(const QString &projPath, QWidget *parent = nullptr);

    void updateTr();

    VAR_GET_FUNC(ProjPath, projPath, QString)

private:
    QString projPath;

    QPlainTextEdit *edit = new QPlainTextEdit;
    PlainButton *btnParse = new PlainButton;
};

#endif // PROJWIDGET_H
