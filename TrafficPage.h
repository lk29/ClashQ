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

private:
    virtual void showEvent(QShowEvent *event) override;

    void sendRequest();
    void replyReadyRead();
    void replyFinished();

private:
    Ui::TrafficPage *ui;
    bool m_reqOngoing;

    static const double s_maxX;
};

#endif // TRAFFICPAGE_H
