#pragma once

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "Widget/Other/colorwidget.h"
#include "header.h"

class CheckBoxVerifyDialog : public QDialog
{
    Q_OBJECT
public:
    enum Button { Cancel, Yes, No };

    CheckBoxVerifyDialog(const QString &text, const QString &cbbText, QWidget *parent = nullptr);

    bool checked() { return mCheckBox->isChecked(); }
    void setChecked(bool checked = true) { mCheckBox->setChecked(checked); }
    
    VAR_GET_FUNC(btnCancelText, mBtnCancel->text(), QString)
    void setBtnCancelText(const QString &text) { mBtnCancel->setText(text); }
    VAR_GET_FUNC(btnNoText, mBtnNo->text(), QString)
    void setBtnNoText(const QString &text) { mBtnNo->setText(text); }
    VAR_GET_FUNC(btnYesText, mBtnYes->text(), QString)
    void setBtnYesText(const QString &text) { mBtnYes->setText(text); }

private:
    QCheckBox *mCheckBox = new QCheckBox;
    QPushButton *mBtnCancel = new QPushButton(tr("Cancel"));
    QPushButton *mBtnNo = new QPushButton(tr("No"));
    QPushButton *mBtnYes = new QPushButton(tr("Yes"));
};