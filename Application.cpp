#include "Application.h"

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
}

QNetworkAccessManager & Application::netMgmr()
{
    return qobject_cast<Application *>(instance())->m_netMgr;
}

MainWindow & Application::mainWindow()
{
    return qobject_cast<Application *>(instance())->m_mainWnd;
}

QUrl Application::clashApiUrl()
{
    return QUrl(QStringLiteral("http://127.0.0.1:9090/"));
}
