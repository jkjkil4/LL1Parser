#ifndef RFLDELEGATE_H
#define RFLDELEGATE_H

#include <Lib/Item/chequereddelegate.h>
#include <Lib/paint.h>

class RFLDelegate : public ChequeredDelegate
{
public:
    RFLDelegate(QObject *parent = nullptr);

    void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    VAR_FUNC(NamePointSize, namePointSize, int, , )
    VAR_FUNC(PathPointSize, pathPointSize, int, , )
    VAR_FUNC(NameColor, nameColor, QColor, , )
    VAR_FUNC(PathColor, pathColor, QColor, , )
    VAR_FUNC(Height, height, int, , )
    VAR_FUNC(Margin, margin, int, , )

private:
    int namePointSize = 10;
    int pathPointSize = 8;

    QColor nameColor = QColor(70, 40, 70);
    QColor pathColor = Qt::darkGray;

    int height = 40;
    int margin = 3;
};

#endif // RFLDELEGATE_H
