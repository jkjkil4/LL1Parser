#pragma once

#include <QWidget>
#include <QTimer>
#include <QPainter>

#include <Lib/header.h>

class NoteWidget : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *) override;

public:
    explicit NoteWidget(QWidget *parent = nullptr);

    VAR_FUNC_USER(Text, mText, QString, start(), const, &)
    VAR_FUNC_USER(Color, mColor, QRgb, update(), , )

private slots:
    void onTimer();

private:
    void start();

    QString mText;
    QRgb mColor = qRgb(0, 0, 255);
    int mAph = 0;

    QTimer *mTimer = new QTimer(this);
};

