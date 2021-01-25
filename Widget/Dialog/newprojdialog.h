#ifndef NEWPROJDIALOG_H
#define NEWPROJDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include <Lib/PlainButton/plainbutton.h>
#include "Class/colorwidget.h"
#include "header.h"

class NewProjDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewProjDialog(QWidget *parent = nullptr);

private:
    QLineEdit *editPath = new QLineEdit;
    QLineEdit *editName = new QLineEdit;

    PlainButton *btnBrowse = new PlainButton(tr("Browse") + "...");

    PlainButton *btnOK = new PlainButton(tr("OK"));
    PlainButton *btnCancel = new PlainButton(tr("Cancel"));
};

#endif // NEWPROJDIALOG_H
