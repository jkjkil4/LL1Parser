#include "rfldelegate.h"

RFLDelegate::RFLDelegate(QObject *parent) : ChequeredDelegate(parent) {}

void RFLDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    ChequeredDelegate::paint(p, option, index);

    p->save();

    QString name = index.data(Qt::UserRole).toString();
    QString path = index.data(Qt::UserRole + 1).toString();
    QRect rect = option.rect;
    rect.setX(rect.x() + margin);
    rect.setY(rect.y() + margin);
    rect.setWidth(rect.width() - margin);
    rect.setHeight(rect.height() - margin);

    j::SetPointSize(p, namePointSize);
    p->setPen(nameColor);
    p->drawText(rect, Qt::AlignLeft | Qt::AlignTop, name);
    j::SetPointSize(p, pathPointSize);
    p->setPen(pathColor);
    p->drawText(rect, Qt::AlignLeft | Qt::AlignBottom, path);

    p->restore();
}

QSize RFLDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = ChequeredDelegate::sizeHint(option, index);
    return QSize(size.width(),  height);
}
