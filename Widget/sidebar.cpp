#include "sidebar.h"

#include <QDebug>

SideBar::SideBar(QWidget *parent) : QWidget(parent) {}

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
    update();
}

void SideBar::updateMinHeight() {
    setMinimumHeight(QFontMetrics(font()).height() * lDatas.size());
}

void SideBar::paintEvent(QPaintEvent *) {
    QPainter p(this);

    int fmHeight = QFontMetrics(font()).height();

    bool paintIcon = lDatas.size() * itemHeight <= height();
    int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());

    int iconX = (width() - iconSize.width()) / 2;
    int iconY = _itemHeight - fmHeight - spacing - iconSize.height();
    int index = 0;
    for(Data &data : lDatas) {
        QRect rect(margin, margin + index * _itemHeight, width() - 2 * margin, _itemHeight - 2 * margin);
        p.setClipRect(rect);
        if(paintIcon) {
            p.drawText(rect, Qt::AlignCenter | Qt::AlignBottom, data.text);
            data.icon.paint(&p, QRect(QPoint(iconX, iconY + index * _itemHeight), iconSize));
        } else {
            p.drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, data.text);
        }
        index++;
    }
}
