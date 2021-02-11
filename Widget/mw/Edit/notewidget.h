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

    VAR_FUNC_USER(Text, text, QString, start(), const, &)
    VAR_FUNC_USER(Color, color, QRgb, update(), , )

private slots:
    void onTimer();

private:
    void start();

    QString text;
    QRgb color = qRgb(0, 0, 255);
    int aph = 0;

    QTimer *timer = new QTimer(this);
};

