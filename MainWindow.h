#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractNativeEventFilter>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateTrafficStats(double ulTraffic, double dlTraffic);
    void updateConnStats(double ulTotal, double dlTotal, int numOfConns);

    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

signals:
    void becomeVisible();
    void becomeHidden();

private:
    void fetchConfig(const QString &profile);
    QByteArray decryptConfig(const QByteArray &ba);
    void setTrayIcon(QIcon::Mode mode);

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool event(QEvent *event) override;

    static QString iniFilePath();

private slots:
    void fetchCfgReplyFinished();
    void getVerReplyFinished();
    void clashErrorOccurred(QProcess::ProcessError error);
    void clashFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void clashStderrReady();
    void clashStdoutReady();
    void clashStarted();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void actionGroupTriggered(QAction *action);
    void openCfgTriggered();
    void openClashCfgTriggered();

private:
    Ui::MainWindow *ui;
    char m_iv[9];
    bool m_sizeAdjusted;
    bool m_hidden;
    bool m_sysShutdown;
    QSettings m_settings;
    QMenu m_trayIconMenu;
    QActionGroup m_actionGroup;
    QSystemTrayIcon m_trayIcon;
    QLabel m_trafficStats;
    QLabel m_connStats;
    QProcess m_clash;
};
#endif // MAINWINDOW_H
