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
    explicit RecentFileListWidget(QWidget *parent = nullptr);

    void updateList();

signals:
    void itemClicked(const QString &filePath);

private:
    DTextDelegate *pDelegate = new DTextDelegate(this);
    DTextModel *pModel = new DTextModel(this);
};

typedef RecentFileListWidget RFLWidget;
