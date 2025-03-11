#include "CameraModel.h"
#include "CameraInfo.h"
#include "CameraInfoManagement.h"
#include <QtDebug>

CameraModel::CameraModel(QObject *parent) : QAbstractTableModel(parent) {
    m_horHeadLabels.insert(ColumnChannel, "Channel");
    m_horHeadLabels.insert(ColumnIP, "IP");
    m_horHeadLabels.insert(ColumnProtocol, "Protocol");
    m_horHeadLabels.insert(ColumnCodec, "Codec");
    m_horHeadLabels.insert(ColumnState, "State");
    m_horHeadLabels.insert(ColumnRTSP, "RTSP");
    m_horHeadLabels.insert(ColumnDelete, "Delete");
}

void CameraModel::resetModel() {
    beginResetModel();

    m_listData.clear();
    for (int i = 0; i < 64; ++i) {
        auto *cameraInfo = CameraInfo::fromChannel(i);
        if (cameraInfo->isValid()) {
            m_listData.append(cameraInfo);
        }
    }

    endResetModel();
}

void CameraModel::updateModel(int channel) {
    int row = -1;
    for (int i = 0; i < m_listData.size(); ++i) {
        auto *info = m_listData.at(i);
        if (info->channel() == channel) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        return;
    }
    emit dataChanged(createIndex(row, ColumnChannel), createIndex(row, ColumnState));
}

QVariant CameraModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            return m_horHeadLabels.value(section);
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
        default:
            break;
        }
    }

    return QVariant();
}

int CameraModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return m_listData.count();
}

int CameraModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return m_horHeadLabels.count();
}

bool CameraModel::insertRows(int row, int count, const QModelIndex &parent) {
    beginInsertRows(parent, row, row + count - 1);

    endInsertRows();

    return true;
}

bool CameraModel::removeRows(int row, int count, const QModelIndex &parent) {
    beginRemoveRows(parent, row, row + count - 1);

    endRemoveRows();

    return true;
}

QVariant CameraModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    CameraInfo *info = m_listData.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ColumnChannel:
            return info->channelString();
        case ColumnIP:
            return info->ip();
        case ColumnProtocol:
            return info->protocolString();
        case ColumnCodec:
            return info->codecString();
        case ColumnState:
            return info->stateString();
        case ColumnRTSP:
            return info->rtspUrl();
        case ColumnDelete:
            return "Delete";
        default:
            break;
        }
        break;
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    default:
        break;
    }

    return QVariant();
}

QVariant CameraModel::itemText(int row, int column) const {
    return data(createIndex(row, column), Qt::DisplayRole);
}
