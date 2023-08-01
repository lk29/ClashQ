#include "ConnectionModel.h"
#include "ConnectionSortModel.h"
#include <QDateTime>

ConnectionSortModel::ConnectionSortModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
}

bool ConnectionSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    switch (left.column()) {
    case ConnectionModel::HeaderUpload:
    case ConnectionModel::HeaderDownload:
    case ConnectionModel::HeaderTime: {
        QVariant leftData(sourceModel()->data(left, Qt::UserRole));
        QVariant rightData(sourceModel()->data(right, Qt::UserRole));
        return leftData < rightData;
    }
    default:
        return QSortFilterProxyModel::lessThan(left, right);
    }
}
