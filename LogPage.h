#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>

namespace Ui {
class LogPage;
}

class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

    void appendHtmlLog(const char *text);
    void appendHtmlLog(const QString &text);
    void appendTextLog(const QString &text);

private:
    QString genLogHeader();

private:
    Ui::LogPage *ui;
};

#endif // LOGPAGE_H
