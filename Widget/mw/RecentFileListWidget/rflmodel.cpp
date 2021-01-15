#include "rflmodel.h"


RFLModel::RFLModel(QObject *parent) : QAbstractListModel(parent)
{
    vDatas << new RFLData{"a", "aa"} << new RFLData{"b", "ccbcc"} << new RFLData{"114", "514"};
}

RFLModel::~RFLModel() {
    for(RFLData *data : vDatas)
        delete data;
}

int RFLModel::rowCount(const QModelIndex &parent) const {
    if (parent.row() == -1)
        return vDatas.size();
    return 0;
}

QVariant RFLModel::data(const QModelIndex &index, int role) const {
    if(role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if(role >= Qt::UserRole) {
        RFLData *pData = vDatas.at(index.row());
        switch(role) {
        case Qt::UserRole:
            return pData->name;
        case Qt::UserRole + 1:
            return pData->path;
        }
    }
    return QVariant();
}

bool RFLModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(role >= Qt::UserRole) {
        RFLData *pData = vDatas.at(index.row());
        switch(role) {
        case Qt::UserRole:
            pData->name = value.toString();
            break;
        case Qt::UserRole + 1:
            pData->path = value.toString();
            break;
        default:
            return false;
        }
        return true;
    }
    return false;
}
