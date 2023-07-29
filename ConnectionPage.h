#ifndef CONNECTIONPAGE_H
#define CONNECTIONPAGE_H

#include <QWidget>

namespace Ui {
class ConnectionPage;
}

class ConnectionPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPage(QWidget *parent = nullptr);
    ~ConnectionPage();

private:
    Ui::ConnectionPage *ui;
};

#endif // CONNECTIONPAGE_H
