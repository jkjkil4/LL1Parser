#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include "Widget/Other/colorwidget.h"
#include "header.h"

class DelayedVerifyDialog : public QDialog
{
public:
    DelayedVerifyDialog(const QString &text, const QString &btnText, QWidget *parent = nullptr);

    void delayedVerify(int delayMsec);

    QLabel *label() { return mLabText; }
    QPushButton *button() { return mBtnAccept; }

private:
    QLabel *mLabText = new QLabel;
    QPushButton *mBtnAccept = new QPushButton;

    QTimer *mTimer = new QTimer(this);
};