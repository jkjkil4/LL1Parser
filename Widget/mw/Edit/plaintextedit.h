#pragma once

#include <QPlainTextEdit>

#include <Lib/paint.h>

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
protected:
    void wheelEvent(QWheelEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;

public:
    explicit PlainTextEdit(QWidget *parent = nullptr);

    void updateTabStopDistance();   //用于更新tab的距离

signals:
    void pointSizeChanged(int pointSize);   //当字体的点大小发生改变时发出的信号

private:
    int prevFontWidth = 0;  //之前的字体宽度，用于判断是否自动更新tab距离
};

