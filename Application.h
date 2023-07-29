#ifndef APPLICATION_H
#define APPLICATION_H

#include "MainWindow.h"
#include <QApplication>
#include <QNetworkAccessManager>

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

    static QNetworkAccessManager & netMgmr();
    static MainWindow & mainWindow();
    static QUrl clashApiUrl();

private:
    // QNetworkAccessManager must be initialized before MainWindow since it will be
    // accessed in MainWindow's constructor
    QNetworkAccessManager m_netMgr;
    MainWindow m_mainWnd;
};

#endif // APPLICATION_H
