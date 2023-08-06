#include "Application.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Utils.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>
#include <Windows.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sizeAdjusted(false),
    m_sysShutdown(false),
    m_settings(getFilePath(PathType::IniFile), QSettings::IniFormat),
    m_actionGroup(nullptr)
{
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget(&m_trafficStats);
    ui->statusBar->addPermanentWidget(&m_connStats);
    ui->statusBar->setStyleSheet(QStringLiteral("QLabel { padding-left: 10px; padding-right: 10px; }"));

    Application::instance()->installNativeEventFilter(this);

    QStringList profiles = m_settings.value(QStringLiteral("profile")).toStringList();
    profiles.removeAll(QString());
    for (const QString &prof : qAsConst(profiles)) {
        QString text("Profile \"");
        text += prof;
        text += '\"';

        QAction *action = m_trayIconMenu.addAction(text);
        action->setData(prof);
        action->setCheckable(true);
        m_actionGroup.addAction(action);
    }
    connect(&m_actionGroup, &QActionGroup::triggered, this, &MainWindow::actionGroupTriggered);

    m_trayIconMenu.addSeparator();
    QAction *actionOpenCfg = m_trayIconMenu.addAction(QStringLiteral("Open Config"));
    QAction *actionOpenClashCfg = m_trayIconMenu.addAction(QStringLiteral("Open Clash Config"));
    QAction *actionOpenWorkDir = m_trayIconMenu.addAction(QStringLiteral("Open Working Directory"));
    connect(actionOpenCfg, &QAction::triggered, this, &MainWindow::openCfgTriggered);
    connect(actionOpenClashCfg, &QAction::triggered, this, &MainWindow::openClashCfgTriggered);
    connect(actionOpenWorkDir, &QAction::triggered, this, &MainWindow::openWorkDirTriggered);

    m_trayIconMenu.addSeparator();
    QAction *actionQuit = m_trayIconMenu.addAction(QStringLiteral("Quit"));
    connect(actionQuit, &QAction::triggered, this, &MainWindow::close);

    setTrayIcon(QIcon::Disabled);
    m_trayIcon.setContextMenu(&m_trayIconMenu);
    m_trayIcon.show();
    connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    m_clash.setWorkingDirectory(getFilePath(PathType::BaseDir));
    m_clash.setArguments(QStringList() << "-d" << ".");
    m_clash.setProgram(getFilePath(PathType::ClashExecutable));
    m_clash.closeReadChannel(QProcess::StandardError);
    m_clash.closeWriteChannel();

    connect(&m_clash, &QProcess::errorOccurred, this, &MainWindow::clashErrorOccurred);
    connect(&m_clash, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::clashFinished);
    connect(&m_clash, &QProcess::readyReadStandardOutput, this, &MainWindow::clashStdoutReady);
    connect(&m_clash, &QProcess::started, this, &MainWindow::clashStarted);

    if (profiles.isEmpty()) {
        ui->logPage->appendLog("no profile");
    } else {
        auto actions = m_actionGroup.actions();
        int profIdx = m_settings.value(QStringLiteral("recent"), 0).toInt();
        if (profIdx < 0 || profIdx >= actions.size()) {
            profIdx = 0;
        }
        actions.at(profIdx)->setChecked(true);
        fetchConfig(profiles.at(profIdx));
    }
}

MainWindow::~MainWindow()
{
    if (m_clash.state() == QProcess::Running) {
        m_clash.close();
    }
    delete ui;
}

void MainWindow::updateTrafficStats(double ulTraffic, double dlTraffic)
{
    QString text("UL: ");
    text += prettyBytes(ulTraffic);
    text += "    DL: ";
    text += prettyBytes(dlTraffic);

    m_trafficStats.setText(text);
}

void MainWindow::updateConnStats(double ulTotal, double dlTotal, int numOfConns)
{
    QString text("UL Total: ");
    text += prettyBytes(ulTotal);
    text += "    DL Total: ";
    text += prettyBytes(dlTotal);
    text += "    Conn: ";
    text += QString::number(numOfConns);

    m_connStats.setText(text);
}

bool MainWindow::nativeEventFilter(const QByteArray & /*eventType*/, void *message, long * /*result*/)
{
    if (static_cast<MSG *>(message)->message == WM_QUERYENDSESSION) {
        m_sysShutdown = true;
    }
    return false;
}

void MainWindow::showEvent(QShowEvent *event)
{
    emit becomeVisible();
    QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous() && !m_sysShutdown) {
        event->ignore();
        showMinimized();
    } else {
        event->accept();

        auto actions = m_actionGroup.actions();
        for (int i = 0; i < actions.size(); ++i) {
            if (actions.at(i)->isChecked()) {
                m_settings.setValue(QStringLiteral("recent"), i);
                break;
            }
        }
    }
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowStateChange:
        if (isMinimized() && !event->spontaneous()) {
            QTimer::singleShot(150, this, &MainWindow::hide);
        }
        break;
    case QEvent::ShowToParent:
        if (!m_sizeAdjusted) {
            m_sizeAdjusted = true;

            QSizeF newSize;
            QFontMetricsF fm = fontMetrics();
            newSize.setWidth(fm.averageCharWidth() * 250);
            newSize.setHeight(newSize.width() * 9 / 16);

            QSizeF oldSize = size();
            int dx = qRound((newSize.width() - oldSize.width())/2.0);
            int dy = qRound((newSize.height() - oldSize.height())/2.0);

            setGeometry(geometry().adjusted(-dx, -dy, dx, dy));
        }
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}

void MainWindow::fetchConfig(const QString &profile)
{
    QString tooltip("Profile: ");
    tooltip += profile;
    m_trayIcon.setToolTip(tooltip);

    QUrl url(m_settings.value(QStringLiteral("url")).toString());
    if (!url.isValid()) {
        ui->logPage->appendLog(url.errorString());
        return;
    }

    QString path('/');
    path += profile.toLower();
    snprintf(m_iv, sizeof(m_iv), "%08d", int(time(nullptr) % 100000000));

    url.setPath(path);
    url.setQuery(m_iv);

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);

    QNetworkRequest request(url);
    request.setSslConfiguration(sslConfig);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ClashQ");

    QNetworkReply *reply = Application::netMgmr().get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::fetchCfgReplyFinished);

    QString logText("fetching configuration for profile \"");
    logText += profile;
    logText += '\"';
    ui->logPage->appendLog(logText);
}

void MainWindow::fetchClashVer()
{
    QUrl url(Application::clashApiUrl());
    url.setPath(QStringLiteral("/version"));

    QNetworkRequest request(url);
    QNetworkReply *reply = Application::netMgmr().get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::getVerReplyFinished);
}

QByteArray MainWindow::decryptConfig(const QByteArray &ba)
{
    QByteArray result;

    QString key = m_settings.value(QStringLiteral("key")).toString();
    if (key.isEmpty()) {
        ui->logPage->appendLog("key is empty");
        return result;
    }

    const EVP_CIPHER *cipher = EVP_get_cipherbyname("des");
    BIO *memBio = BIO_new_mem_buf(ba.data(), ba.size());
    BIO *b64Bio = BIO_new(BIO_f_base64());
    BIO *cipherBio = BIO_new(BIO_f_cipher());
    BIO_set_cipher(cipherBio, cipher, (unsigned char *)key.toStdString().c_str(), (unsigned char *)m_iv, 0);

    BIO_push(b64Bio, memBio);
    BIO_push(cipherBio, b64Bio);

    int len;
    char buf[1024];
    while ((len = BIO_read(cipherBio, buf, sizeof(buf))) > 0) {
        result.append(buf, len);
    }

    if (!BIO_get_cipher_status(cipherBio)) {
        result.clear();
        ui->logPage->appendLog("failed to decrypt configuration");
    }

    BIO_free_all(cipherBio);
    return result;
}

void MainWindow::setTrayIcon(QIcon::Mode mode)
{
    QIcon icon(QStringLiteral(":/app.ico"));
    if (mode == QIcon::Disabled) {
        m_trayIcon.setIcon(icon.pixmap(16, 16, mode));
    } else {
        m_trayIcon.setIcon(icon);
    }
}

void MainWindow::fetchCfgReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        ui->logPage->appendLog(reply->errorString());
        return;
    }

    QFile file(getFilePath(PathType::ClashConfig));
    if (!file.open(QIODevice::WriteOnly)) {
        ui->logPage->appendLog(file.errorString());
        return;
    }

    QByteArray ba(decryptConfig(reply->readAll()));
    if (ba.isEmpty()) {
        return;
    }
    ui->logPage->appendLog("decrypt configuration successfully");

    file.write(ba);
    file.close();

    m_clash.start(QIODevice::ReadOnly);
}

void MainWindow::getVerReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString log("failed to get clash version (");
        log += reply->errorString();
        log += ')';
        ui->logPage->appendLog(log);
        QTimer::singleShot(1000, this, &MainWindow::fetchClashVer);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject jsonObj = jsonDoc.object();
    QJsonValue jsonVer = jsonObj[QLatin1String("version")];
    if (jsonVer.isString()) {
        QString tooltip = m_trayIcon.toolTip();
        tooltip += "\nClash: ";
        tooltip += jsonVer.toString();

        m_trayIcon.setToolTip(tooltip);
    }
}

void MainWindow::clashErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        ui->logPage->appendLog("failed to start clash subprocess");
        break;
    case QProcess::Crashed:
        ui->logPage->appendLog("clash subprocess crashed");
        break;
    case QProcess::Timedout:
        ui->logPage->appendLog("wait for clash subprocess time out");
        break;
    case QProcess::WriteError:
        ui->logPage->appendLog("failed to write to clash subprocess");
        break;
    case QProcess::ReadError:
        ui->logPage->appendLog("failed to read from clash subprocess");
        break;
    default:
        ui->logPage->appendLog("unknown error occurred in clash subprocess");
        break;
    }
}

void MainWindow::clashFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    setTrayIcon(QIcon::Disabled);
}

void MainWindow::clashStdoutReady()
{
    QProcess *subprocess = qobject_cast<QProcess *>(sender());
    ui->logPage->appendClashLog(subprocess->readAllStandardOutput().trimmed());
}

void MainWindow::clashStarted()
{
    setTrayIcon(QIcon::Normal);
    QTimer::singleShot(1000, this, &MainWindow::fetchClashVer);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) {
            if (isMinimized()) {
                showNormal();
            } else {
                hide();
            }
        } else {
            showNormal();
            activateWindow();
        }
    }
}

void MainWindow::actionGroupTriggered(QAction *action)
{
    if (m_clash.state() == QProcess::Running) {
        m_clash.close();
        m_clash.waitForFinished();
    }

    fetchConfig(action->data().toString().toLower());
}

void MainWindow::openCfgTriggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(getFilePath(PathType::IniFile)));
}

void MainWindow::openClashCfgTriggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(getFilePath(PathType::ClashConfig)));
}

void MainWindow::openWorkDirTriggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(getFilePath(PathType::BaseDir)));
}

QString MainWindow::getFilePath(PathType pt)
{
    QDir dir = QDir::home();
    dir.cd(QStringLiteral("clash"));

    switch (pt) {
    case PathType::BaseDir:
        return dir.absolutePath();
    case PathType::IniFile:
        return dir.absoluteFilePath(QStringLiteral("config.ini"));
    case PathType::ClashExecutable:
        return dir.absoluteFilePath(QStringLiteral("clash.exe"));
    case PathType::ClashConfig:
        return dir.absoluteFilePath(QStringLiteral("config.yaml"));
    default:
        return QString();
    }
}
