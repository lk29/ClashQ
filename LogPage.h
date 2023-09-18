#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>
#include <QRegularExpression>

namespace Ui {
class LogPage;
}

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Unknown,
};

class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

    int avgCharWidth() const;

    void appendLog(LogLevel level, const char *text);
    void appendLog(LogLevel level, const QString &text);
    void appendClashLog(const QString &text);

private:
    QString genLogHeader(LogLevel level, const QString &time=QString());

private:
    Ui::LogPage *ui;
    QRegularExpression m_re;
};

#endif // LOGPAGE_H
