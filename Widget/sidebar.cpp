#include "sidebar.h"


SideBar::SideBar(QWidget *parent) : QWidget(parent)
{
    j::LimitWidth(this, 70);
    setMouseTracking(true);
}

void SideBar::append(const QIcon &icon, QString name, const QString &text) {
    lDatas << Data{ icon, name, text };
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
    if(ev->button() == Qt::LeftButton) {  //如果左键按下
        bool paintIcon = lDatas.size() * itemHeight <= height();
        int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());
        int _checkedIndex = ev->y() / _itemHeight;  //新的checkedIndex
        if(_checkedIndex >= 0 && _checkedIndex < lDatas.size() && checkedIndex != _checkedIndex) {  //判断是否在一定范围内并且是否变化
            checkedIndex = _checkedIndex;
            emit clicked(lDatas[checkedIndex]);
            update();
        }
    }
}

void SideBar::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() == Qt::NoButton) {	//如果没有按下任何按键
        bool paintIcon = lDatas.size() * itemHeight <= height();
        int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size());
        int _mouseOverIndex = ev->y() / _itemHeight; //新的mouseOverIndex
        if(mouseOverIndex != _mouseOverIndex) { //判断是否变化
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

    p.fillRect(0, 0, width(), height(), backgroundColor); //填充背景
    p.setPen(textColor);    //设置画笔颜色(绘制文字)
    j::SetPointSize(&p, 8); //设置点大小

    int fmHeight = QFontMetrics(font()).height();   //字体高度

    bool paintIcon = lDatas.size() * itemHeight <= height();  //是否绘制图标
    int _itemHeight = (paintIcon ? itemHeight : height() / lDatas.size()); //调整后的itemHeight

    int iconX = (width() - iconSize.width()) / 2;  //图标的x
    int iconY = _itemHeight - fmHeight - spacing - iconSize.height(); //图标相对当前位置的y
    int index = 0;
    for(Data &data : lDatas) {  //遍历所有的内容
        QRect rect(0, index * _itemHeight, width(), _itemHeight); //全范围
        QRect clipRect(margin, margin + index * _itemHeight, width() - 2 * margin, _itemHeight - 2 * margin);  //裁剪范围

        //绘制背景
        p.setClipRect(rect);
        if(checkedIndex == index) {
            p.fillRect(rect.x() + 2, rect.y(), rect.width() - 2, rect.height(), checkedColor);
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

        //绘制左侧细条
        p.setClipRect(rect);
        if(checkedIndex == index) {
            p.fillRect(rect.x(), rect.y(), 2, rect.height(), checkedLeftColor);
        }

        index++;
    }
}
