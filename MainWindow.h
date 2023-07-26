#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void fetchConfig(const QString &profile);
    QByteArray decryptConfig(const QByteArray &ba);

    void closeEvent(QCloseEvent *event) override;

    static QString iniFilePath();

private slots:
    void replyFinished();
    void clashErrorOccurred(QProcess::ProcessError error);
    void clashStderrReady();
    void clashStdoutReady();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void actionGroupTriggered(QAction *action);

private:
    Ui::MainWindow *ui;
    char m_iv[9];
    QSettings m_settings;
    QMenu m_trayIconMenu;
    QSystemTrayIcon m_trayIcon;
    QProcess m_clash;
    QNetworkAccessManager m_netMgr;
};
#endif // MAINWINDOW_H
