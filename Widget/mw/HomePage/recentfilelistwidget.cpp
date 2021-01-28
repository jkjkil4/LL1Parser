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
        DTextModel::Data data{ QFileInfo(path).completeBaseName(), path };
        int index = pModel->indexOf(data);
        if(index != 0) {
            pModel->remove(index);
            pModel->insert(0, data);
            update();
        }
    });
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

