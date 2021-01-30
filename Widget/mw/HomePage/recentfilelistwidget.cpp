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
    connect(&rfManager, SIGNAL(changed()), this, SLOT(onRFChanged()));
}

RFLWidget::Item RecentFileListWidget::currentItem() {
    QModelIndex index = currentIndex();
    return Item{ index.row(), index.data(Qt::UserRole + 1).toString() };
}

RFLWidget::Item RecentFileListWidget::itemAt(int y) {
    int realY = y + verticalScrollBar()->value();
    if(realY >= 0) {
        int index = realY / pDelegate->getHeight();
        if(index < pModel->count()) {
            return Item{ index, pModel->data(index) };
        }
    }
    return Item{ -1, "" };
}

void RecentFileListWidget::loadList() {
    rfManager.load(20, nullptr, [](const QString& path) -> bool {
        QFileInfo info(path);
        return info.isFile() && info.completeSuffix() == SUFFIX;
    });

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

void RecentFileListWidget::onRFChanged() {
    pModel->clear();
    for(const QString& path : rfManager.list())
        pModel->append(DTextModel::Data{ QFileInfo(path).completeBaseName(), path });
    update();
}

