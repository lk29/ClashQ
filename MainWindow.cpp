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
{
    ui->setupUi(this);
    ui->logEdit->setFont(QFont(QStringLiteral("Consolas"), 10));

    QActionGroup *actionGroup = new QActionGroup(this);
    connect(actionGroup, &QActionGroup::triggered, this, &MainWindow::actionGroupTriggered);

    QStringList profiles = m_settings.value(QStringLiteral("profile")).toStringList();
    profiles.removeAll(QString());
    for (const QString &prof : qAsConst(profiles)) {
        QString text("Profile \"");
        text += prof;
        text += '\"';

        QAction *action = m_trayIconMenu.addAction(text);
        action->setData(prof);
        action->setCheckable(true);
        actionGroup->addAction(action);
    }

    m_trayIconMenu.addSeparator();
    QAction *actionOpenCfg = m_trayIconMenu.addAction(QStringLiteral("Open Config"));
    connect(actionOpenCfg, &QAction::triggered, this, &MainWindow::openCfgTriggered);
    QAction *actionOpenClashCfg = m_trayIconMenu.addAction(QStringLiteral("Open Clash Config"));
    connect(actionOpenClashCfg, &QAction::triggered, this, &MainWindow::openClashCfgTriggered);

    m_trayIconMenu.addSeparator();
    QAction *actionQuit = m_trayIconMenu.addAction(QStringLiteral("Quit"));
    connect(actionQuit, &QAction::triggered, QCoreApplication::instance(), &QCoreApplication::quit);

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
    connect(&m_clash, &QProcess::readyReadStandardError, this, &MainWindow::clashStderrReady);
    connect(&m_clash, &QProcess::readyReadStandardOutput, this, &MainWindow::clashStdoutReady);
    connect(&m_clash, &QProcess::started, this, &MainWindow::clashStarted);

    if (profiles.isEmpty()) {
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: no profile"));
        show();
    } else {
        auto actions = actionGroup->actions();
        actions.first()->setChecked(true);

        fetchConfig(profiles.first());
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
        QString text("ClashQ: ");
        text += url.errorString();
        ui->logEdit->appendPlainText(text);
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

    ui->logEdit->appendPlainText(QStringLiteral("ClashQ: fetching configuration"));
}

QByteArray MainWindow::decryptConfig(const QByteArray &ba)
{
    QByteArray result;

    QString key = m_settings.value(QStringLiteral("key")).toString();
    if (key.isEmpty()) {
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: key is empty"));
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
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: failed to decrypt config"));
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
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
        QString text("ClashQ: ");
        text += reply->errorString();
        ui->logEdit->appendPlainText(text);
        return;
    }

    QDir dir(m_clash.workingDirectory());
    QFile file(dir.absoluteFilePath(QStringLiteral("config.yaml")));
    if (!file.open(QIODevice::WriteOnly)) {
        QString text("ClashQ: ");
        text += file.errorString();
        ui->logEdit->appendPlainText(text);
        return;
    }

    QByteArray ba(decryptConfig(reply->readAll()));
    if (ba.isEmpty()) {
        return;
    }
    ui->logEdit->appendPlainText(QStringLiteral("ClashQ: decrypt configuration successfully"));

    file.write(ba);
    file.close();

    m_clash.start(QIODevice::ReadOnly);
}

void MainWindow::clashErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: failed to start clash subprocess"));
        break;
    case QProcess::Crashed:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: clash subprocess crashed"));
        break;
    case QProcess::Timedout:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: wait for clash subprocess time out"));
        break;
    case QProcess::WriteError:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: failed to write to clash subprocess"));
        break;
    case QProcess::ReadError:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: failed to read from clash subprocess"));
        break;
    default:
        ui->logEdit->appendPlainText(QStringLiteral("ClashQ: unknown error occurred in clash subprocess"));
        break;
    }
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
        setTrayIcon(QIcon::Disabled);
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
