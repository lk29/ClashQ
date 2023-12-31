#include "Application.h"
#include "TrafficPage.h"
#include "TrafficSpeedTicker.h"
#include "ui_TrafficPage.h"
#include <plottables/plottable-graph.h>
#include <layoutelements/layoutelement-legend.h>
#include <QJsonDocument>
#include <QNetworkReply>

const double TrafficPage::s_maxX = 180;

TrafficPage::TrafficPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrafficPage),
    m_reqOngoing(false)
{
    ui->setupUi(this);
    ui->plot->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    ui->plot->legend->setVisible(true);
    ui->plot->xAxis->setRange(0, s_maxX);
    ui->plot->xAxis->setTickLabels(false);
    ui->plot->xAxis->setTicks(false);
    ui->plot->yAxis->setTicker(QSharedPointer<TrafficSpeedTicker>(new TrafficSpeedTicker()));

    QCPGraph *ulGraph = ui->plot->addGraph();
    QColor ulFillColor(Qt::blue);
    ulFillColor.setAlpha(60);
    QBrush ulBrush(ulFillColor);
    ulGraph->setName(QStringLiteral("Upload"));
    ulGraph->setPen(QPen(Qt::blue));
    ulGraph->setBrush(ulBrush);

    QCPGraph *dlGraph = ui->plot->addGraph();
    QColor dlFillColor(Qt::red);
    dlFillColor.setAlpha(60);
    QBrush dlBrush(dlFillColor);
    dlGraph->setName(QStringLiteral("Download"));
    dlGraph->setPen(QPen(Qt::red));
    dlGraph->setBrush(dlBrush);

    connect(&Application::mainWindow(), &MainWindow::clashApiReady, this, &TrafficPage::sendRequest);
}

TrafficPage::~TrafficPage()
{
    delete ui;
}

void TrafficPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    ui->plot->yAxis->rescale(true);
    ui->plot->replot();
}

void TrafficPage::sendRequest()
{
    if (!m_reqOngoing) {
        m_reqOngoing = true;

        QUrl url(Application::clashApiUrl());
        url.setPath(QStringLiteral("/traffic"));

        QNetworkRequest request(url);
        QNetworkReply *reply = Application::netMgmr().get(request);
        connect(reply, &QNetworkReply::readyRead, this, &TrafficPage::replyReadyRead);
        connect(reply, &QNetworkReply::finished, this, &TrafficPage::replyFinished);
    }
}

void TrafficPage::replyReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!Application::mainWindow().isVisible()) {
        QCPGraph *ulGraph = ui->plot->graph(0);
        QCPGraph *dlGraph = ui->plot->graph(1);
        ulGraph->data()->clear();
        dlGraph->data()->clear();
        reply->close();
        return;
    }

    QJsonDocument jsonDoc(QJsonDocument::fromJson(reply->readAll()));
    QJsonObject jsonObj(jsonDoc.object());
    QJsonValueRef ulValue = jsonObj[QLatin1String("up")];
    QJsonValueRef dlValue = jsonObj[QLatin1String("down")];
    if (ulValue.isUndefined() || dlValue.isUndefined()) {
        return;
    }

    double ulBytes = ulValue.toDouble();
    double dlBytes = dlValue.toDouble();
    Application::mainWindow().updateTrafficStats(ulBytes, dlBytes);

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

    ulNewData.push_back(QCPGraphData(s_maxX, ulBytes));
    dlNewData.push_back(QCPGraphData(s_maxX, dlBytes));

    ulData->set(ulNewData, true);
    dlData->set(dlNewData, true);

    if (isVisible()) {
        ui->plot->yAxis->rescale(true);
        ui->plot->replot();
    }
}

void TrafficPage::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    m_reqOngoing = false;
}
