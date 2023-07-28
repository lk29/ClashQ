#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QNetworkAccessManager>

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

    static QNetworkAccessManager & netMgmr();
    static QUrl clashApiUrl();

private:
    QNetworkAccessManager m_netMgr;
};

#endif // APPLICATION_H
