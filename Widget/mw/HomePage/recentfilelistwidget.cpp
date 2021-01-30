#include "recentfilelistwidget.h"

//#include <QDebug>

RecentFileListWidget::RecentFileListWidget(QWidget *parent) : QListView(parent)
{
    setItemDelegate(pDelegate);
    setModel(pModel);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    connect(this, &RecentFileListWidget::clicked, [this](const QModelIndex &index){
        emit itemClicked(Item{ index.row(), index.data(Qt::UserRole + 1).toString() });
    });
    connect(&rfManager, &RecentFileManager::appended, [this](const QString &path) {
        DTextModel::Data data{ QFileInfo(path).completeBaseName(), path };
        int index = pModel->indexOf(data);
        if(index != 0) {
            pModel->remove(index);
            pModel->insert(0, data);
            update();
        }
    });
}

RFLWidget::Item RecentFileListWidget::currentItem() {
    QModelIndex index = currentIndex();
    return Item{ index.row(), index.data(Qt::UserRole + 1).toString() };
}

void RecentFileListWidget::updateList() {
    bool ok;
    QStringList list = rfManager.loadAll(20, &ok, [](const QString& path) -> bool {
        QFileInfo info(path);
        return info.isFile() && info.completeSuffix() == SUFFIX;
    });
    pModel->clear();
    for(QString& path : list)
        pModel->append(DTextModel::Data{ QFileInfo(path).completeBaseName(), path });
    update();
}

void RecentFileListWidget::remove(int index) {
    if(index < 0 || index >= pModel->count())
        return;
    pModel->remove(index);
    blockSignals(true);
    for(int i = pModel->count() - 1; i >= 0; i--)
        rfManager.append(pModel->at(i).text2);
    blockSignals(false);
}

