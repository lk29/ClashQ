#include "LogPage.h"
#include "ui_LogPage.h"
#include <QDateTime>

LogPage::LogPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogPage)
{
    ui->setupUi(this);
    ui->logEdit->setFont(QFont(QStringLiteral("Consolas"), 10));
}

LogPage::~LogPage()
{
    delete ui;
}

void LogPage::appendHtmlLog(const char *text)
{
    QString logText(genLogHeader());
    logText += text;

    ui->logEdit->appendHtml(logText);
}

void LogPage::appendHtmlLog(const QString &text)
{
    QString logText(genLogHeader());
    logText += text;

    ui->logEdit->appendHtml(logText);
}

void LogPage::appendTextLog(const QString &text)
{
    ui->logEdit->appendPlainText(text);
}

QString LogPage::genLogHeader()
{
    QDateTime now(QDateTime::currentDateTime());
    // Make the formatted string have offset from UTC info.
    // Qt's bug?
    // https://bugreports.qt.io/browse/QTBUG-26161
    now.setOffsetFromUtc(now.offsetFromUtc());

    QString header("<font color='#5c00e6'>");
    header += now.toString(Qt::ISODate);
    header += "</font>: ";

    return header;
}
