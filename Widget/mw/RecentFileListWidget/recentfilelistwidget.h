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

        QString name() const { return index.data(Qt::UserRole).toString(); }
        QString path() const { return index.data(Qt::UserRole + 1).toString(); }
        int row() const { return index.row(); }

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

typedef RecentFileListWidget RFLWidget;

#endif // RECENTFILELISTWIDGET_H
