#include "recentfilelistwidget.h"

//#include <QDebug>

RecentFileListWidget::RecentFileListWidget(QWidget *parent) : QListView(parent)
{
    setItemDelegate(mDelegate);
    setModel(mModel);
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
        int index = realY / mDelegate->height();
        if(index < mModel->count()) {
            return Item{ index, mModel->data(index) };
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
    if(index < 0 || index >= mModel->count())
        return;
    mModel->remove(index);
    blockSignals(true);
    for(int i = mModel->count() - 1; i >= 0; i--)
        rfManager.append(mModel->at(i).text2);
    blockSignals(false);
}

void RecentFileListWidget::onRFChanged() {
    mModel->clear();
    for(const QString& path : rfManager.list())
        mModel->append(DTextModel::Data{ QFileInfo(path).completeBaseName(), path });
    update();
}

