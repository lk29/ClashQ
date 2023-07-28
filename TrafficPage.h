#ifndef TRAFFICPAGE_H
#define TRAFFICPAGE_H

#include <QWidget>

namespace Ui {
class TrafficPage;
}

class TrafficPage : public QWidget
{
    Q_OBJECT

public:
    explicit TrafficPage(QWidget *parent = nullptr);
    ~TrafficPage();

private slots:
    void replyReadyRead();
    void replyFinished();

    void sendRequest();

private:
    Ui::TrafficPage *ui;

    static const double s_maxX;
};

#endif // TRAFFICPAGE_H
