#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopServices>
#include <QDir>
#include <QNetworkReply>
#include <openssl/bio.h>
#include <openssl/evp.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_sizeAdjusted(false)
    , m_settings(iniFilePath(), QSettings::IniFormat)
    , m_actionGroup(nullptr)
{
    ui->setupUi(this);
    ui->logEdit->setFont(QFont(QStringLiteral("Consolas"), 10));

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
        appendLog("no profile");
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

void MainWindow::fetchConfig(const QString &profile)
{
    QString tooltip("Profile \"");
    tooltip += profile;
    tooltip += '\"';
    m_trayIcon.setToolTip(tooltip);

    QUrl url(m_settings.value(QStringLiteral("url")).toString());
    if (!url.isValid()) {
        appendLog(url.errorString());
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

    QNetworkReply *reply = m_netMgr.get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::replyFinished);

    QString logText("fetching configuration for profile \"");
    logText += profile;
    logText += '\"';
    appendLog(logText);
}

QByteArray MainWindow::decryptConfig(const QByteArray &ba)
{
    QByteArray result;

    QString key = m_settings.value(QStringLiteral("key")).toString();
    if (key.isEmpty()) {
        appendLog("key is empty");
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
        appendLog("failed to decrypt configuration");
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

template<typename T>
void MainWindow::appendLog(T text)
{
    QDateTime now(QDateTime::currentDateTime());
    // Make the formatted string have offset from UTC info.
    // Qt's bug?
    // https://bugreports.qt.io/browse/QTBUG-26161
    now.setOffsetFromUtc(now.offsetFromUtc());

    QString logText("<font color='#5c00e6'>");
    logText += now.toString(Qt::ISODate);
    logText += "</font>: ";
    logText += text;

    ui->logEdit->appendHtml(logText);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) {
        event->ignore();
        hide();
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
    if (event->type() == QEvent::ShowToParent && !m_sizeAdjusted) {
        m_sizeAdjusted = true;

        QSizeF newSize;
        QFontMetricsF fm = fontMetrics();
        newSize.setWidth(fm.averageCharWidth() * 230);
        newSize.setHeight(newSize.width() * 9 / 16);

        QSizeF oldSize = size();
        int dx = qRound((newSize.width() - oldSize.width())/2.0);
        int dy = qRound((newSize.height() - oldSize.height())/2.0);

        setGeometry(geometry().adjusted(-dx, -dy, dx, dy));
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
        appendLog(reply->errorString());
        return;
    }

    QDir dir(m_clash.workingDirectory());
    QFile file(dir.absoluteFilePath(QStringLiteral("config.yaml")));
    if (!file.open(QIODevice::WriteOnly)) {
        appendLog(file.errorString());
        return;
    }

    QByteArray ba(decryptConfig(reply->readAll()));
    if (ba.isEmpty()) {
        return;
    }
    appendLog("decrypt configuration successfully");

    file.write(ba);
    file.close();

    m_clash.start(QIODevice::ReadOnly);
}

void MainWindow::clashErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        appendLog("failed to start clash subprocess");
        break;
    case QProcess::Crashed:
        appendLog("clash subprocess crashed");
        break;
    case QProcess::Timedout:
        appendLog("wait for clash subprocess time out");
        break;
    case QProcess::WriteError:
        appendLog("failed to write to clash subprocess");
        break;
    case QProcess::ReadError:
        appendLog("failed to read from clash subprocess");
        break;
    default:
        appendLog("unknown error occurred in clash subprocess");
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
    ui->logEdit->appendPlainText(subprocess->readAll().trimmed());
}

void MainWindow::clashStdoutReady()
{
    QProcess *subprocess = qobject_cast<QProcess *>(sender());
    ui->logEdit->appendPlainText(subprocess->readAll().trimmed());
}

void MainWindow::clashStarted()
{
    setTrayIcon(QIcon::Normal);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) {
            hide();
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
