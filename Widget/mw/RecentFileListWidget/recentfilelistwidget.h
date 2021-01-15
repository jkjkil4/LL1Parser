#ifndef RECENTFILELISTWIDGET_H
#define RECENTFILELISTWIDGET_H

#include <QListView>

#include "rfldelegate.h"
#include "rflmodel.h"

class RecentFileListWidget : public QListView
{
    Q_OBJECT
public:
    class Item
    {
    public:
        Item(const QModelIndex &index) : index(index) {}

        QString name() { return index.data(Qt::UserRole).toString(); }
        QString path() { return index.data(Qt::UserRole + 1).toString(); }
        int row() { return index.row(); }

    private:
        const QModelIndex &index;
    };

    explicit RecentFileListWidget(QWidget *parent = nullptr);

signals:
    void itemClicked(const Item &item);

private:
    RFLDelegate *pDelegate = new RFLDelegate(this);
    RFLModel *pModel = new RFLModel(this);
};

#endif // RECENTFILELISTWIDGET_H
