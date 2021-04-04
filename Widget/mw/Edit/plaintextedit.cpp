#include "plaintextedit.h"

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    connect(this, &PlainTextEdit::blockCountChanged, this, &PlainTextEdit::updateLineNumberAreaWidth);
    connect(this, &PlainTextEdit::updateRequest, this, &PlainTextEdit::updateLineNumberArea);

    updateTabStopDistance();    //更新tab距离
    setLineWrapMode(LineWrapMode::NoWrap);  //不自动换行
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);     //总是显示横向滑动条
}

void PlainTextEdit::updateTabStopDistance() {
    int fontWidth = fontMetrics().horizontalAdvance('_');
    if(mPrevFontWidth != fontWidth) {
        setTabStopDistance(fontWidth * 4);
        mPrevFontWidth = fontWidth;
    }
}

void PlainTextEdit::wheelEvent(QWheelEvent *ev) {
    if(ev->modifiers() == Qt::ControlModifier) {    //如果同时按下了Ctrl键
        int pointSize = font().pointSize();
        int offset = ev->delta() / 120;
        int res = qMax(1, pointSize + offset);
        if(res != pointSize) {
            j::SetPointSize(this, res);
            emit pointSizeChanged(res);
        }
        return;     //return是为了不调用父类wheelEvent
    }
    QPlainTextEdit::wheelEvent(ev);
}

void PlainTextEdit::paintEvent(QPaintEvent *ev) {
    updateTabStopDistance();

    QPlainTextEdit::paintEvent(ev);
}

void PlainTextEdit::resizeEvent(QResizeEvent *ev) {
    QPlainTextEdit::resizeEvent(ev);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


//LineNumberArea相关
void PlainTextEdit::lineNumberAreaPaintEvent(QPaintEvent *ev) {
    QPainter p(lineNumberArea);
    p.fillRect(ev->rect(), QColor(240, 240, 240));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while(block.isValid() && top <= ev->rect().bottom()) {
        if(block.isVisible() && bottom >= ev->rect().top()) {
            p.setPen(blockNumber == textCursor().blockNumber() ? QColor(80, 80, 80) : QColor(160, 160, 160));
            p.drawText(lineNumberAreaLeftSpacing, top,
                       lineNumberArea->width() - lineNumberAreaTotalSpacing(), fontMetrics().height(),
                       Qt::AlignRight, QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
}

int PlainTextEdit::lineNumberAreaWidth() {
    int digits = 0;
    int count = blockCount();
    while(count) {
        count /= 10;
        digits++;
    }

    return lineNumberAreaTotalSpacing() + fontMetrics().horizontalAdvance('9') * digits;
}

void PlainTextEdit::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PlainTextEdit::updateLineNumberArea(const QRect &rect, int dy) {
    if(dy) {
        lineNumberArea->scroll(0, dy);
    } else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if(rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}



//-----------------------------------------//
LineNumberArea::LineNumberArea(PlainTextEdit *edit)
    : QWidget(edit), pEdit(edit) {}

QSize LineNumberArea::sizeHint() const {
    return QSize(pEdit->lineNumberAreaWidth(), 0);
}

void LineNumberArea::wheelEvent(QWheelEvent *ev) {
    pEdit->wheelEvent(ev);
}

void LineNumberArea::paintEvent(QPaintEvent *ev) {
    pEdit->lineNumberAreaPaintEvent(ev);
}
