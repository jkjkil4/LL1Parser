#pragma once

#include <QListView>
#include <QScrollBar>

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
        int row;
        QString filePath;
    };

    explicit RecentFileListWidget(QWidget *parent = nullptr);

    Item currentItem();
    Item itemAt(int y);

    void loadList();
    void remove(int index);

signals:
    void itemClicked(const Item &item);

private slots:
    void onRFChanged();

private:
    DTextDelegate *pDelegate = new DTextDelegate(this);
    DTextModel *pModel = new DTextModel(this);
};

typedef RecentFileListWidget RFLWidget;
