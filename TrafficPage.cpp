#include "Application.h"
#include "QCustomPlot/src/plottables/plottable-graph.h"
#include "TrafficPage.h"
#include "ui_TrafficPage.h"
#include <QJsonDocument>
#include <QNetworkReply>

const double TrafficPage::s_maxX = 180;

TrafficPage::TrafficPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrafficPage)
{
    ui->setupUi(this);
    ui->plot->xAxis->setRange(0, s_maxX);
    ui->plot->xAxis->setTickLabels(false);
    ui->plot->xAxis->setTicks(false);

    QCPGraph *ulGraph = ui->plot->addGraph();
    QColor ulFillColor(Qt::blue);
    ulFillColor.setAlpha(100);
    QBrush ulBrush(ulFillColor);
    ulGraph->setPen(QPen(Qt::blue));
    ulGraph->setBrush(ulBrush);

    QCPGraph *dlGraph = ui->plot->addGraph();
    QColor dlFillColor(Qt::red);
    dlFillColor.setAlpha(100);
    QBrush dlBrush(dlFillColor);
    dlGraph->setPen(QPen(Qt::red));
    dlGraph->setBrush(dlBrush);

    sendRequest();
}

TrafficPage::~TrafficPage()
{
    delete ui;
}

void TrafficPage::replyReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    QJsonDocument jsonDoc(QJsonDocument::fromJson(reply->readAll()));
    QJsonObject jsonObj(jsonDoc.object());
    QJsonValueRef ulValue = jsonObj[QLatin1String("up")];
    QJsonValueRef dlValue = jsonObj[QLatin1String("down")];

    if (ulValue.isUndefined() || dlValue.isUndefined()) {
        return;
    }

    QCPGraph *ulGraph = ui->plot->graph(0);
    QCPGraph *dlGraph = ui->plot->graph(1);
    auto ulData = ulGraph->data();
    auto dlData = dlGraph->data();

    QVector<QCPGraphData> ulNewData, dlNewData;
    ulNewData.reserve(ulData->size() + 1);
    dlNewData.reserve(ulData->size() + 1);

    for (int i = ulData->size() > s_maxX ? 1 : 0; i < ulData->size(); ++i) {
        auto ulIter = ulData->at(i);
        auto dlIter = dlData->at(i);
        ulNewData.push_back(QCPGraphData(ulIter->key - 1, ulIter->value));
        dlNewData.push_back(QCPGraphData(dlIter->key - 1, dlIter->value));
    }

    ulNewData.push_back(QCPGraphData(s_maxX, ulValue.toDouble()));
    dlNewData.push_back(QCPGraphData(s_maxX, dlValue.toDouble()));

    ulData->set(ulNewData, true);
    dlData->set(dlNewData, true);
    ui->plot->yAxis->rescale(true);
    ui->plot->replot();
}

void TrafficPage::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    QTimer::singleShot(3000, this, &TrafficPage::sendRequest);
}

void TrafficPage::sendRequest()
{
    QUrl url(Application::clashApiUrl());
    url.setPath(QStringLiteral("/traffic"));

    QNetworkRequest request(url);
    QNetworkReply *reply = Application::netMgmr().get(request);
    connect(reply, &QNetworkReply::readyRead, this, &TrafficPage::replyReadyRead);
    connect(reply, &QNetworkReply::finished, this, &TrafficPage::replyFinished);
}
