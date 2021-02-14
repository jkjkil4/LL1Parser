#pragma once

#include <QWidget>

#include <Lib/header.h>
#include <Lib/paint.h>

class InfoWidget : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *) override;

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    explicit InfoWidget(const QString &text, QColor col, QWidget *parent = nullptr);

    void setData(const QString &text, QColor col);
    void clear();
    void setPointSize(int pointSize);

private:
    void updateHeight();

    bool mPaint = false;
    QString mText;
    QColor mCol;
};

