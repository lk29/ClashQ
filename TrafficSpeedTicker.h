#ifndef TRAFFICSPEEDTICKER_H
#define TRAFFICSPEEDTICKER_H

#include "QCustomPlot/src/axis/axisticker.h"

class TrafficSpeedTicker : public QCPAxisTicker
{
private:
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
};

#endif // TRAFFICSPEEDTICKER_H
