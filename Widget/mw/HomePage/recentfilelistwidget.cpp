#include "recentfilelistwidget.h"

//#include <QDebug>

RecentFileListWidget::RecentFileListWidget(QWidget *parent) : QListView(parent)
{
    setItemDelegate(pDelegate);
    setModel(pModel);
    setMouseTracking(true);

    connect(this, &RecentFileListWidget::clicked, [this](const QModelIndex &index){
        emit itemClicked(index.data(Qt::UserRole + 1).toString());
    });
    connect(&rfManager, &RecentFileManager::appended, [this](const QString &path) {
        pModel->append(QFileInfo(path).completeBaseName(), path);
    });
}

void RecentFileListWidget::updateList() {
    bool ok;
    QStringList list = rfManager.loadAll(20, &ok, [](const QString& path) -> bool { return QFileInfo(path).isFile(); });
    pModel->clear();
    for(QString& path : list)
        pModel->append(QFileInfo(path).completeBaseName(), path);
    update();
}

