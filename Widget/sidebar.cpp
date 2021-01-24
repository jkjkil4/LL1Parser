#include "sidebar.h"


SideBar::SideBar(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void SideBar::append(const QIcon &icon, const QString &text) {
    lDatas << Data{ icon, text };
    updateMinHeight();
    update();
}

void SideBar::setText(int index, const QString &text) {
    if(index < 0 || index >= lDatas.size())
        return;
    lDatas[index].text = text;
    updateMinHeight();
}

void SideBar::updateMinHeight() {
    setMinimumHeight(QFontMetrics(font()).height() * lDatas.size());
}


void SideBar::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton) {
        bool paintIcon = lDatas.size() * itemHeight <= height();
        int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());
        int _checkedIndex = ev->y() / _itemHeight;
        if(_checkedIndex >= 0 && _checkedIndex < lDatas.size() && checkedIndex != _checkedIndex) {
            checkedIndex = _checkedIndex;
            emit clicked(lDatas[checkedIndex]);
            update();
        }
    }
}

void SideBar::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() == Qt::NoButton) {
        bool paintIcon = lDatas.size() * itemHeight <= height();
        int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());
        int _mouseOverIndex = ev->y() / _itemHeight;
        if(mouseOverIndex != _mouseOverIndex) {
            mouseOverIndex = _mouseOverIndex;
            update();
        }
    }
}

void SideBar::leaveEvent(QEvent *) {
    mouseOverIndex = -1;
    update();
}

void SideBar::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.fillRect(0, 0, width(), height(), backgroundColor);
    p.setPen(textColor);

    int fmHeight = QFontMetrics(font()).height();

    bool paintIcon = lDatas.size() * itemHeight <= height();
    int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());

    int iconX = (width() - iconSize.width()) / 2;
    int iconY = _itemHeight - fmHeight - spacing - iconSize.height();
    int index = 0;
    for(Data &data : lDatas) {
        QRect rect(0, index * _itemHeight, width(), _itemHeight);
        QRect clipRect(margin, margin + index * _itemHeight, width() - 2 * margin, _itemHeight - 2 * margin);

        //绘制背景
        p.setClipRect(rect);
        if(checkedIndex == index) {
            p.fillRect(rect.x() + 2, rect.y(), rect.width() - 2, rect.height(), checkedColor);
            p.fillRect(rect.x(), rect.y(), 2, rect.height(), checkedLeftColor);
        } else if(mouseOverIndex == index) {
            p.fillRect(rect, mouseOverColor);
        }

        //绘制图标和文字
        p.setClipRect(clipRect);
        if(paintIcon) {
            p.drawText(clipRect, Qt::AlignCenter | Qt::AlignBottom, data.text);
            data.icon.paint(&p, QRect(QPoint(iconX, iconY + index * _itemHeight), iconSize));
        } else {
            p.drawText(clipRect, Qt::AlignCenter | Qt::AlignVCenter, data.text);
        }
        index++;
    }
}
