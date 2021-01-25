#ifndef NEWPROJDIALOG_H
#define NEWPROJDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <Lib/PlainButton/plainbutton.h>
#include "Class/colorwidget.h"
#include "header.h"

class NewProjDialog : public QDialog
{
public:
    explicit NewProjDialog(QWidget *parent = nullptr);

private:
    PlainButton *btnOK = new PlainButton(tr("OK"));
    PlainButton *btnCancel = new PlainButton(tr("Cancel"));

};

#endif // NEWPROJDIALOG_H
