#include "Utils.h"

QString prettyBytes(double bytes)
{
    if (std::abs(bytes) < 1024) {
        return QString::asprintf("%.0f B", bytes);
    }
    bytes /= 1024;
    if (std::abs(bytes) < 1024) {
        return QString::asprintf("%.1f KB", bytes);
    }
    bytes /= 1024;
    if (std::abs(bytes) < 1024) {
        return QString::asprintf("%.1f MB", bytes);
    }
    bytes /= 1024;
    if (std::abs(bytes) < 1024) {
        return QString::asprintf("%.1f GB", bytes);
    }
    bytes /= 1024;
    return QString::asprintf("%.1f TB", bytes);
}

QString prettyDuration(double sec)
{
    if (std::abs(sec) < 2) {
        return QString::asprintf("%.0f second", sec);
    }
    if (std::abs(sec) < 60) {
        return QString::asprintf("%.0f seconds", sec);
    }
    sec /= 60;
    if (std::abs(sec) < 2) {
        return QString::asprintf("%.1f minute", sec);
    }
    if (std::abs(sec) < 60) {
        return QString::asprintf("%.1f minutes", sec);
    }
    sec /= 60;
    if (std::abs(sec) < 2) {
        return QString::asprintf("%.1f hour", sec);
    }
    return QString::asprintf("%.1f hours", sec);
}
