#include "fileswidget.h"

FilesWidget::FilesWidget(QWidget *parent) : QListWidget(parent)
{ 
    connect(this, &FilesWidget::itemDoubleClicked, [](QListWidgetItem *item) {
        QString path = QFileInfo(item->text()).path();
        if(QDir().exists(path))
            QDesktopServices::openUrl(QUrl("file:///" + path));
    });
}

FilesWidget::FilesWidget(const QStringList &list, QWidget *parent) 
    : FilesWidget(parent)
{
    for(const QString &filePath : list) {
        QListWidgetItem *item = new QListWidgetItem(filePath);
        addItem(item);
    }
}