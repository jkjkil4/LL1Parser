#pragma once

#include <QPlainTextEdit>
#include <QTextBlock>

#include <Lib/paint.h>

class LineNumberArea;

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
protected:
    void wheelEvent(QWheelEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;

public:
    friend class LineNumberArea;

    explicit PlainTextEdit(QWidget *parent = nullptr);

    void updateTabStopDistance();   //用于更新tab的距离

signals:
    void pointSizeChanged(int pointSize);   //当字体的点大小发生改变时发出的信号

private:
    int mPrevFontWidth = 0;  //之前的字体宽度，用于判断是否自动更新tab距离

    //LineNumberArea相关
public:
    void lineNumberAreaPaintEvent(QPaintEvent *ev);
    int lineNumberAreaWidth();

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int ry);

private:
    LineNumberArea *lineNumberArea;
    int lineNumberAreaLeftSpacing = 15;
    int lineNumberAreaRightSpacing = 10;
    int lineNumberAreaTotalSpacing() { return lineNumberAreaLeftSpacing + lineNumberAreaRightSpacing; }
};


//------------------------------------------------------//
class LineNumberArea : public QWidget   //参考了Qt的例子:Code Editor Example
{
public:
    LineNumberArea(PlainTextEdit *edit);

    QSize sizeHint() const override;

protected:
    void wheelEvent(QWheelEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;

private:
    PlainTextEdit *pEdit;
};
