#include "Application.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Utils.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QNetworkReply>
#include <QTimer>
#include <Windows.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sizeAdjusted(false),
    m_hidden(true),
    m_sysShutdown(false),
    m_settings(iniFilePath(), QSettings::IniFormat),
    m_actionGroup(nullptr)
{
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget(&m_trafficStats);
    ui->statusBar->addPermanentWidget(&m_connStats);
    ui->statusBar->setStyleSheet(QStringLiteral("QLabel { padding-left: 4px; padding-right: 4px; }"));

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
    connect(actionOpenCfg, &QAction::triggered, this, &MainWindow::openCfgTriggered);
    QAction *actionOpenClashCfg = m_trayIconMenu.addAction(QStringLiteral("Open Clash Config"));
    connect(actionOpenClashCfg, &QAction::triggered, this, &MainWindow::openClashCfgTriggered);

    m_trayIconMenu.addSeparator();
    QAction *actionQuit = m_trayIconMenu.addAction(QStringLiteral("Quit"));
    connect(actionQuit, &QAction::triggered, this, &MainWindow::close);

    setTrayIcon(QIcon::Disabled);
    m_trayIcon.setContextMenu(&m_trayIconMenu);
    m_trayIcon.show();
    connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd(QStringLiteral("clash"));
    m_clash.setWorkingDirectory(dir.absolutePath());
    m_clash.setArguments(QStringList() << "-d" << "." << "-f" << "config.yaml");
    m_clash.setProgram(dir.absoluteFilePath("clash-windows-amd64-v3.exe"));

    connect(&m_clash, &QProcess::errorOccurred, this, &MainWindow::clashErrorOccurred);
    connect(&m_clash, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::clashFinished);
    connect(&m_clash, &QProcess::readyReadStandardError, this, &MainWindow::clashStderrReady);
    connect(&m_clash, &QProcess::readyReadStandardOutput, this, &MainWindow::clashStdoutReady);
    connect(&m_clash, &QProcess::started, this, &MainWindow::clashStarted);

    if (profiles.isEmpty()) {
        ui->logPage->appendHtmlLog("no profile");
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

void MainWindow::fetchConfig(const QString &profile)
{
    QString tooltip("Profile \"");
    tooltip += profile;
    tooltip += '\"';
    m_trayIcon.setToolTip(tooltip);

    QUrl url(m_settings.value(QStringLiteral("url")).toString());
    if (!url.isValid()) {
        ui->logPage->appendHtmlLog(url.errorString());
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
    connect(reply, &QNetworkReply::finished, this, &MainWindow::replyFinished);

    QString logText("fetching configuration for profile \"");
    logText += profile;
    logText += '\"';
    ui->logPage->appendHtmlLog(logText);
}

QByteArray MainWindow::decryptConfig(const QByteArray &ba)
{
    QByteArray result;

    QString key = m_settings.value(QStringLiteral("key")).toString();
    if (key.isEmpty()) {
        ui->logPage->appendHtmlLog("key is empty");
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
        ui->logPage->appendHtmlLog("failed to decrypt configuration");
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab) {
        int index = ui->tabWidget->currentIndex();
        if (++index >= ui->tabWidget->count()) {
            index = 0;
        }
        ui->tabWidget->setCurrentIndex(index);
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (m_hidden) {
        m_hidden = false;
        emit becomeVisible();
    }
    QMainWindow::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    if (!event->spontaneous() && !isVisible()) {
        m_hidden = true;
        emit becomeHidden();
    }
    QMainWindow::hideEvent(event);
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
                return;
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

QString MainWindow::iniFilePath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    return dir.absoluteFilePath(QStringLiteral("config.ini"));
}

void MainWindow::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        ui->logPage->appendHtmlLog(reply->errorString());
        return;
    }

    QDir dir(m_clash.workingDirectory());
    QFile file(dir.absoluteFilePath(QStringLiteral("config.yaml")));
    if (!file.open(QIODevice::WriteOnly)) {
        ui->logPage->appendHtmlLog(file.errorString());
        return;
    }

    QByteArray ba(decryptConfig(reply->readAll()));
    if (ba.isEmpty()) {
        return;
    }
    ui->logPage->appendHtmlLog("decrypt configuration successfully");

    file.write(ba);
    file.close();

    m_clash.start(QIODevice::ReadOnly);
}

void MainWindow::clashErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        ui->logPage->appendHtmlLog("failed to start clash subprocess");
        break;
    case QProcess::Crashed:
        ui->logPage->appendHtmlLog("clash subprocess crashed");
        break;
    case QProcess::Timedout:
        ui->logPage->appendHtmlLog("wait for clash subprocess time out");
        break;
    case QProcess::WriteError:
        ui->logPage->appendHtmlLog("failed to write to clash subprocess");
        break;
    case QProcess::ReadError:
        ui->logPage->appendHtmlLog("failed to read from clash subprocess");
        break;
    default:
        ui->logPage->appendHtmlLog("unknown error occurred in clash subprocess");
        break;
    }
}

void MainWindow::clashFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    setTrayIcon(QIcon::Disabled);
}

void MainWindow::clashStderrReady()
{
    QProcess *subprocess = qobject_cast<QProcess *>(sender());
    ui->logPage->appendTextLog(subprocess->readAll().trimmed());
}

void MainWindow::clashStdoutReady()
{
    QProcess *subprocess = qobject_cast<QProcess *>(sender());
    ui->logPage->appendTextLog(subprocess->readAll().trimmed());
}

void MainWindow::clashStarted()
{
    setTrayIcon(QIcon::Normal);
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
    QDir dir(QCoreApplication::applicationDirPath());
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absoluteFilePath("config.ini")));
}

void MainWindow::openClashCfgTriggered()
{
    QDir dir(m_clash.workingDirectory());
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absoluteFilePath("config.yaml")));
}
