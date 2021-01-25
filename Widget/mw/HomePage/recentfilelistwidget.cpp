#include "recentfilelistwidget.h"

//#include <QDebug>

RecentFileListWidget::RecentFileListWidget(QWidget *parent) : QListView(parent)
{
    setItemDelegate(pDelegate);
    setModel(pModel);

    connect(this, &RecentFileListWidget::clicked, [this](const QModelIndex &index){
        emit itemClicked(index.data(Qt::UserRole + 1).toString());
    });
}

void RecentFileListWidget::setPath(const QString &path) {
    manager.setFilePath(path);
}

void RecentFileListWidget::updateList() {
    bool ok;
    QStringList list = manager.loadAll(20, &ok, [](const QString& path) -> bool { return QFileInfo(path).isFile(); });
    pModel->clear();
    for(QString& path : list)
        pModel->append(QFileInfo(path).completeBaseName(), path);
    update();
}

void RecentFileListWidget::append(const QString &path) {
    manager.append(path);
    pModel->insert(0, QFileInfo(path).completeBaseName(), path);
}

