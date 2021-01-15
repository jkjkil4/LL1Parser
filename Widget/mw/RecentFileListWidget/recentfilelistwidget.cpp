#include "recentfilelistwidget.h"

RecentFileListWidget::RecentFileListWidget(QWidget *parent) : QListView(parent)
{
    setItemDelegate(pDelegate);
    setModel(pModel);

    connect(this, &RecentFileListWidget::clicked, [this](const QModelIndex &index){ emit itemClicked(index); });
}

