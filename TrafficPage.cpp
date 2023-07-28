#include "Application.h"
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
    ui->chartView->setRenderHints(QPainter::Antialiasing);

    QChart *chart = ui->chartView->chart();
    chart->legend()->hide();
    chart->setBackgroundRoundness(0);
    chart->setMargins(QMargins(10, 10, 10, 10));
    chart->setContentsMargins(0, 0, 0, 0);

    QColor upColor(Qt::blue);
    QColor downColor(Qt::red);
    upColor.setAlpha(100);
    downColor.setAlpha(100);

    QAreaSeries *upSeries = new QAreaSeries(new QLineSeries());
    QAreaSeries *downSeries = new QAreaSeries(new QLineSeries());
    upSeries->setPen(QPen(Qt::blue));
    upSeries->setColor(upColor);
    downSeries->setPen(QPen(Qt::red));
    downSeries->setColor(downColor);

    chart->addSeries(upSeries);
    chart->addSeries(downSeries);
    chart->createDefaultAxes();

    QAbstractAxis *xAxis = chart->axisX();
    QPen pen(xAxis->gridLinePen());
    pen.setStyle(Qt::DashLine);
    xAxis->setRange(0, s_maxX);
    xAxis->setLabelsVisible(false);
    xAxis->setGridLinePen(pen);

    QAbstractAxis *yAxis = chart->axisY();
    yAxis->setGridLinePen(pen);

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
    QJsonValueRef upValue = jsonObj[QLatin1String("up")];
    QJsonValueRef downValue = jsonObj[QLatin1String("down")];

    if (upValue.isUndefined() || downValue.isUndefined()) {
        return;
    }

    QChart *chart = ui->chartView->chart();
    QList<QAbstractSeries *> series = chart->series();
    QLineSeries *upSeries = qobject_cast<QAreaSeries *>(series[0])->upperSeries();
    QLineSeries *downSeries = qobject_cast<QAreaSeries *>(series[1])->upperSeries();

    QVector<QPointF> upPoints = upSeries->pointsVector();
    QVector<QPointF> downPoints = downSeries->pointsVector();
    if (upPoints.size() > s_maxX) {
        upPoints.pop_front();
        downPoints.pop_front();
    }

    double maxY = 1;
    for (int i= 0; i < upPoints.size(); ++i) {
        QPointF &upPt = upPoints[i];
        QPointF &downPt = downPoints[i];
        upPt.rx() -= 1;
        downPt.rx() -= 1;

        double tempY = std::max(upPt.y(), downPt.y());
        if (tempY > maxY) {
            maxY = tempY;
        }
    }

    double upY = upValue.toDouble();
    double downY = downValue.toDouble();
    double tempY = std::max(upY, downY);
    if (tempY > maxY) {
        maxY = tempY;
    }
    chart->axisY()->setRange(0, maxY);
    upPoints.push_back(QPointF(s_maxX, upY));
    downPoints.push_back(QPointF(s_maxX, downY));

    upSeries->replace(upPoints);
    downSeries->replace(downPoints);
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
