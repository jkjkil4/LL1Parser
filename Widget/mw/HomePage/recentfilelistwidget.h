#pragma once

#include <QListView>

#include <Lib/Item/dtextdelegate.h>
#include <Lib/Item/dtextmodel.h>
#include <Lib/recentfilemanager.h>
#include "header.h"

class RecentFileListWidget : public QListView
{
    Q_OBJECT
public:
    struct Item
    {
        const int row;
        const QString &filePath;
    };

    explicit RecentFileListWidget(QWidget *parent = nullptr);

    Item currentItem();

    void updateList();
    void remove(int index);

signals:
    void itemClicked(const Item &item);

private:
    DTextDelegate *pDelegate = new DTextDelegate(this);
    DTextModel *pModel = new DTextModel(this);
};

typedef RecentFileListWidget RFLWidget;
