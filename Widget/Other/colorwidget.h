#pragma once

#include <QWidget>
#include <QPainter>

#include <Lib/header.h>

class ColorWidget : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(0, 0, width(), height(), mColor);
    }

public:
    using QWidget::QWidget;
    ColorWidget(QColor color, QWidget *parent = nullptr) : QWidget(parent), mColor(color) {}

    VAR_FUNC_USER(Color, mColor, QColor, update(), , )

private:
    QColor mColor = QColor(250, 250, 250);
};
