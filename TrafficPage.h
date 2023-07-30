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
    void sendRequest();
    void replyReadyRead();
    void replyFinished();
    void mainWndVisible();
    void mainWndHidden();

private:
    Ui::TrafficPage *ui;
    bool m_keepReceiving;

    static const double s_maxX;
};

#endif // TRAFFICPAGE_H
