#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>

#include <QIcon>
#include <QPainter>
#include <QMouseEvent>

#include <Lib/header.h>

#include <QDebug>

class MainWindowView;

class SideBar : public QWidget
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;

public:
    struct Data
    {
        QIcon icon;
        QString text;
    };

    explicit SideBar(QWidget *parent = nullptr);

    void append(const QIcon &icon, const QString &text);
    void setText(int index, const QString &text);

    void updateMinHeight();

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
    void clicked(const Data &data);

private:
    QList<Data> lDatas;

    int margin = 2;
    int spacing = 4;
    int itemHeight = 50;
    QSize iconSize = QSize(24, 24);

    QColor backgroundColor = QColor(70, 70, 70);
    QColor mouseOverColor = QColor(100, 100, 100);
    QColor checkedColor = QColor(40, 40, 40);
    QColor checkedLeftColor = QColor(190, 190, 190);
    QColor textColor = Qt::lightGray;

    int checkedIndex = 0;
    int mouseOverIndex = -1;
};

#endif // SIDEBAR_H
