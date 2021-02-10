#pragma once

#include <QWidget>

#include <QIcon>
#include <QPainter>
#include <QMouseEvent>

#include <Lib/header.h>
#include <Lib/paint.h>

//#include <QDebug>

class MainWindowView;

/*
SideBar
主窗口左侧的菜单栏
*/
class SideBar : public QWidget
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *ev) override;     //用于相应鼠标点击
    void mouseMoveEvent(QMouseEvent *ev) override;      //用于更新mouseOverIndex
    void leaveEvent(QEvent *) override;                 //当鼠标离开控件时，将mouseOverIndex重置为-1
    void paintEvent(QPaintEvent *) override;            //绘制控件

public:
    struct Data  //单个数据
    {
        QIcon icon;
        MainWindowView *view;
        QString text;
    };

    explicit SideBar(QWidget *parent = nullptr);

    void append(const QIcon &icon, MainWindowView *view, const QString &text);    //追加内容
    void setText(int index, const QString &text);       //设置指定index处的文本

    MainWindowView* current() { return lDatas[checkedIndex].view; }
    void setCurrent(MainWindowView *view);

    void updateMinHeight();     //更新控件最小高度

    VAR_GET_FUNC(ItemHeight, itemHeight, int)
    void setItemHeight(int _itemHeight) { itemHeight = _itemHeight; updateMinHeight(); update(); }

    VAR_FUNC_USER(Margin, margin, int, update(), , )
    VAR_FUNC_USER(Spacing, spacing, int, update(), , )
    VAR_FUNC_USER(IconSize, iconSize, QSize, update(), , )
    VAR_FUNC_USER(BackgroundColor, backgroundColor, QColor, update(), , )
    VAR_FUNC_USER(MouseOverColor, mouseOverColor, QColor, update(), , )
    VAR_FUNC_USER(CheckedColor, checkedColor, QColor, update(), , )
    VAR_FUNC_USER(CheckedLeftColor, checkedLeftColor, QColor, update(), , )
    VAR_FUNC_USER(TextColor, textColor, QColor, update(), , )
    VAR_GET_FUNC(CheckedIndex, checkedIndex, int)

signals:
    void actived(const Data &data);     //当鼠标按下时发出的信号

private:
    QList<Data> lDatas;		//所有内容

    int margin = 2;         //边界空隙
    int spacing = 4;        //文字与图标的空隙
    int itemHeight = 50;    //单个内容的高度
    QSize iconSize = QSize(24, 24);  	//图标的大小

    QColor backgroundColor = QColor(70, 70, 70);        //背景颜色
    QColor mouseOverColor = QColor(100, 100, 100);      //当鼠标悬浮时的背景颜色
    QColor checkedColor = QColor(40, 40, 40);           //选中时的背景颜色
    QColor checkedLeftColor = QColor(190, 190, 190);	//选中时的左侧细条的颜色
    QColor textColor = Qt::lightGray;	//文字颜色

    int checkedIndex = 0;       //选中的index
    int mouseOverIndex = -1;	//鼠标悬浮的index
};
