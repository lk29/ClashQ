#ifndef CONNECTIONSORTMODEL_H
#define CONNECTIONSORTMODEL_H

#include <QSortFilterProxyModel>

class ConnectionSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ConnectionSortModel(QObject *parent = nullptr);

private:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // CONNECTIONSORTMODEL_H
