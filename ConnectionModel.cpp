#include "Application.h"
#include "ConnectionModel.h"
#include "Utils.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>

ConnectionModel::ConnectionModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_keepSending(false)
{
    connect(&Application::mainWindow(), &MainWindow::becomeVisible, this, &ConnectionModel::mainWndVisible);
    connect(&Application::mainWindow(), &MainWindow::becomeHidden, this, &ConnectionModel::mainWndHidden);
}

QVariant ConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role == Qt::InitialSortOrderRole) {
        return Qt::DescendingOrder;
    }

    if (role == Qt::DisplayRole) {
        switch (section) {
        case HeaderHost:
            return "Host";
        case HeaderUpload:
            return "Upload";
        case HeaderDownload:
            return "Download";
        case HeaderChains:
            return "Chains";
        case HeaderRule:
            return "Rule";
        case HeaderTime:
            return "Time";
        case HeaderSource:
            return "Source";
        case HeaderDestIp:
            return "Destination IP";
        case HeaderType:
            return "Type";
        case HeaderProcess:
            return "Process";
        default:
            return QVariant();
        }
    }

    return QVariant();
}

int ConnectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_connInfos.size();
}

int ConnectionModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return HeaderCount;
}

QVariant ConnectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const ConnInfo &info = m_connInfos[index.row()];

    // Sort role
    if (role == Qt::UserRole) {
        switch (index.column()) {
        case HeaderUpload:
            return info.upload;
        case HeaderDownload:
            return info.download;
        case HeaderTime:
            return info.start.secsTo(QDateTime::currentDateTime());
        default:
            return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case HeaderHost:
            return info.host;
        case HeaderUpload:
            return prettyBytes(info.upload);
        case HeaderDownload:
            return prettyBytes(info.download);
        case HeaderChains:
            return info.chains;
        case HeaderRule:
            return info.rule;
        case HeaderTime:
            return prettyDuration(info.start.secsTo(QDateTime::currentDateTime()));
        case HeaderSource:
            return info.source;
        case HeaderDestIp:
            return info.destIp;
        case HeaderType:
            return info.type;
        case HeaderProcess:
            return info.process;
        default:
            return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case HeaderUpload:
        case HeaderDownload:
        case HeaderTime:
            return Qt::AlignRight;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

void ConnectionModel::sendRequest()
{
    QUrl url(Application::clashApiUrl());
    url.setPath(QStringLiteral("/connections"));

    QNetworkRequest request(url);
    QNetworkReply *reply = Application::netMgmr().get(request);
    connect(reply, &QNetworkReply::finished, this, &ConnectionModel::replyFinished);
}

void ConnectionModel::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (!m_keepSending) {
        return;
    }

    QTimer::singleShot(1000, this, &ConnectionModel::sendRequest);

    QJsonDocument jsonDoc(QJsonDocument::fromJson(reply->readAll()));
    QJsonObject jsonObj(jsonDoc.object());
    QJsonArray conns = jsonObj[QLatin1String("connections")].toArray();

    beginResetModel();
    m_connInfos.clear();
    for (int i = 0; i < conns.count(); ++i) {
        QJsonObject connObj = conns[i].toObject();
        QJsonObject metaObj = connObj[QLatin1String("metadata")].toObject();
        QJsonArray chains = connObj[QLatin1String("chains")].toArray();

        QStringList chainsStrs;
        for (int i = chains.count() - 1; i >= 0; --i) {
            chainsStrs.append(chains[i].toString());
        }

        ConnInfo info;
        info.host = metaObj[QLatin1String("host")].toString();
        info.upload = connObj[QLatin1String("upload")].toInt();
        info.download = connObj[QLatin1String("download")].toInt();
        info.chains = chainsStrs.join('/');
        info.rule = connObj[QLatin1String("rule")].toString();
        QString rulePayload(connObj[QLatin1String("rulePayload")].toString());
        if (!rulePayload.isEmpty()) {
            info.rule += '(';
            info.rule += rulePayload;
            info.rule += ')';
        }
        info.start = QDateTime::fromString(connObj[QLatin1String("start")].toString(), Qt::ISODateWithMs);
        info.source = metaObj[QLatin1String("sourceIP")].toString();
        info.source += ':';
        info.source += metaObj[QLatin1String("sourcePort")].toString();
        info.destIp = metaObj[QLatin1String("destinationIP")].toString();
        info.type = metaObj[QLatin1String("type")].toString();
        info.type += '(';
        info.type += metaObj[QLatin1String("network")].toString();
        info.type += ')';
        info.process = metaObj[QLatin1String("processPath")].toString();

        if (info.host.isEmpty()) {
            info.host = info.destIp;
        }
        info.host += ':';
        info.host += metaObj[QLatin1String("destinationPort")].toString();

        m_connInfos.append(std::move(info));
    }
    endResetModel();
}

void ConnectionModel::mainWndVisible()
{
    m_keepSending = true;
    sendRequest();
}

void ConnectionModel::mainWndHidden()
{
    m_keepSending = false;
}
