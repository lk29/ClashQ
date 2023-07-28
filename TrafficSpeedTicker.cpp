#include "TrafficSpeedTicker.h"

QString TrafficSpeedTicker::getTickLabel(double tick, const QLocale &/*locale*/, QChar /*formatChar*/, int /*precision*/)
{
    if (tick > 0) {
        if (tick < 1024) {
            return QString::asprintf("%.1f B/s", tick);
        }
        if (tick < 1024 * 1024) {
            return QString::asprintf("%.1f KB/s", tick / 1024);
        }
        if (tick < 1024 * 1024 * 1024) {
            return QString::asprintf("%.1f MB/s", tick / 1024 / 1024);
        }
    }
    return QString::asprintf("%.1f B/s", tick);
}
