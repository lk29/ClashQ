#ifndef CONNECTIONMODEL_H
#define CONNECTIONMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>

class ConnectionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum {
        HeaderHost,
        HeaderUpload,
        HeaderDownload,
        HeaderChains,
        HeaderRule,
        HeaderTime,
        HeaderSource,
        HeaderDestIp,
        HeaderType,
        HeaderProcess,
        HeaderCount
    };

    explicit ConnectionModel(QObject *parent = nullptr);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private slots:
    void sendRequest();
    void replyFinished();
    void mainWndVisible();
    void mainWndHidden();

private:
    struct ConnInfo {
        double upload;
        double download;
        QDateTime start;
        QString id;
        QString host;
        QString chains;
        QString rule;
        QString process;
        QString source;
        QString destIp;
        QString type;
    };

    bool m_mainWndVisible;
    QVector<ConnInfo> m_connInfos;
};

#endif // CONNECTIONMODEL_H
