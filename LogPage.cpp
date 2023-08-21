#include "LogPage.h"
#include "ui_LogPage.h"
#include <QDateTime>

LogPage::LogPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogPage),
    m_re(QStringLiteral(R"--(time="(.+?)" level=(.+?) msg="(.+)")--"))
{
    ui->setupUi(this);
    ui->logEdit->setFont(QFont(QStringLiteral("Consolas"), 10));
}

LogPage::~LogPage()
{
    delete ui;
}

int LogPage::avgCharWidth() const
{
    return ui->logEdit->fontMetrics().averageCharWidth();
}

void LogPage::appendLog(const char *text)
{
    QString html(genLogHeader());
    html += text;

    ui->logEdit->appendHtml(html);
}

void LogPage::appendLog(const QString &text)
{
    QString html(genLogHeader());
    html += text;

    ui->logEdit->appendHtml(html);
}

void LogPage::appendClashLog(const QString &text)
{
    enum {
        CapTime = 1,
        CapLevel,
        CapMsg,
    };

    QRegularExpressionMatchIterator iter = m_re.globalMatch(text);
    if (!iter.hasNext()) {
        ui->logEdit->appendPlainText(text);
        return;
    }
    do {
        QRegularExpressionMatch match = iter.next();

        LogLevel level;
        QStringRef levelStr = match.capturedRef(CapLevel);
        if (levelStr == "debug") {
            level = LogLevel::Debug;
        } else if (levelStr == "info") {
            level = LogLevel::Info;
        } else if (levelStr == "warning") {
            level = LogLevel::Warning;
        } else if (levelStr == "error") {
            level = LogLevel::Error;
        } else {
            level = LogLevel::Unknown;
        }

        QString html = genLogHeader(match.captured(CapTime), level);
        html += match.capturedRef(CapMsg);

        ui->logEdit->appendHtml(html);
    } while (iter.hasNext());
}

QString LogPage::genLogHeader(const QString &time, LogLevel level)
{
    QString result;

    QDateTime dateTime;
    if (time.isEmpty()) {
        result += "<font color='#5c00e6'>";
        dateTime = QDateTime::currentDateTime();
    } else {
        dateTime = QDateTime::fromString(time, Qt::ISODate);
    }
    result += dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (time.isEmpty()) {
        result += "</font>";
    }

    switch (level) {
    case LogLevel::Debug:
        result += " DEBUG: ";
        break;
    case LogLevel::Info:
        result += " &nbsp;INFO: ";
        break;
    case LogLevel::Warning:
        result += " &nbsp;<font color='#ffcc00'>WARN</font>: ";
        break;
    case LogLevel::Error:
        result += " <font color='#e63900'>ERROR</font>: ";
        break;
    default:
        result += " <font color='#0000e6'>UNKNO</font>: ";
        break;
    }

    return result;
}
