#ifndef RECENTFILELISTWIDGET_H
#define RECENTFILELISTWIDGET_H

#include <QListView>

#include <Lib/Item/dtextdelegate.h>
#include <Lib/Item/dtextmodel.h>
#include <Lib/recentfilemanager.h>

class RecentFileListWidget : public QListView
{
    Q_OBJECT
public:
    explicit RecentFileListWidget(QWidget *parent = nullptr);

    void setPath(const QString& path);

    void updateList();
    void append(const QString& path);

signals:
    void itemClicked(const QString &path);

private:
    DTextDelegate *pDelegate = new DTextDelegate(this);
    DTextModel *pModel = new DTextModel(this);

    RecentFileManager manager;
};

typedef RecentFileListWidget RFLWidget;

#endif // RECENTFILELISTWIDGET_H
