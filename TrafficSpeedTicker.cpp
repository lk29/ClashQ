#include "TrafficSpeedTicker.h"
#include "Utils.h"

QString TrafficSpeedTicker::getTickLabel(double tick, const QLocale &/*locale*/, QChar /*formatChar*/, int /*precision*/)
{
    return prettyBytes(tick);
}
