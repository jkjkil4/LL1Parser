#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>

#include <QIcon>
#include <QPainter>

#include <Lib/header.h>

class MainWindowView;

class SideBar : public QWidget
{
    Q_OBJECT
protected:
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
    void setItemHeight(int _itemHeight) { itemHeight = _itemHeight; updateMinHeight(); }

    VAR_FUNC(Margin, margin, int, , )
    VAR_FUNC(Spacing, spacing, int, , )
    VAR_FUNC(IconSize, iconSize, QSize, , )

signals:
    void clicked(const Data &data);

private:
    QList<Data> lDatas;

    int margin = 2;
    int spacing = 4;
    int itemHeight = 50;
    QSize iconSize = QSize(24, 24);
};

#endif // SIDEBAR_H
