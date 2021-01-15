#ifndef RFLMODEL_H
#define RFLMODEL_H

#include <QAbstractListModel>

#include <Lib/header.h>
#include <Lib/RecentFileManager/recentfilemanager.h>

//#include <QDebug>

class RFLModel : public QAbstractListModel
{
public:
    explicit RFLModel(QObject *parent = nullptr);
    ~RFLModel() override;

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void updateList();
    void append(const QString& path);

private:
    RecentFileManager rfManager;

    struct RFLData
    {
        QString name;
        QString path;
    };
    QVector<RFLData*> vDatas;
};

#endif // RFLMODEL_H
