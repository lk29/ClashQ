#include "ConnectionModel.h"
#include "ConnectionSortModel.h"
#include <QDateTime>

ConnectionSortModel::ConnectionSortModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
}

bool ConnectionSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == ConnectionModel::HeaderUpload ||
        left.column() == ConnectionModel::HeaderDownload ||
        left.column() == ConnectionModel::HeaderTime)
    {
        QVariant leftData(sourceModel()->data(left, Qt::UserRole));
        QVariant rightData(sourceModel()->data(right, Qt::UserRole));
        return leftData < rightData;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}
