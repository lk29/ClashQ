#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>
#include <QRegularExpression>

namespace Ui {
class LogPage;
}

class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

    int avgCharWidth() const;

    void appendLog(const char *text);
    void appendLog(const QString &text);
    void appendClashLog(const QString &text);

private:
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Unknown,
    };

    QString genLogHeader(const QString &time=QString(), LogLevel level=LogLevel::Info);

private:
    Ui::LogPage *ui;
    QRegularExpression m_re;
};

#endif // LOGPAGE_H
