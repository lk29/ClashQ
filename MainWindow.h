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

private:
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool event(QEvent *event) override;

    void fetchConfig(const QString &profile);
    void fetchClashVer();
    QByteArray decryptConfig(const QByteArray &ba);
    void setTrayIcon(QIcon::Mode mode);

    void fetchCfgReplyFinished();
    void getVerReplyFinished();
    void clashErrorOccurred(QProcess::ProcessError error);
    void clashFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void clashStdoutReady();
    void clashStarted();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void actionGroupTriggered(QAction *action);
    void openCfgTriggered();
    void openClashCfgTriggered();

    enum class PathType {
        BaseDir,
        IniFile,
        ClashExecutable,
        ClashConfig,
    };

    static QString getFilePath(PathType pt);

private:
    Ui::MainWindow *ui;
    char m_iv[9];
    bool m_sizeAdjusted;
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
