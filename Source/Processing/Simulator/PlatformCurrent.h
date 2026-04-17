#ifndef PLATFORMCURRENT_H
#define PLATFORMCURRENT_H

#include "QVector"
#include <QObject>

#define WHITCH_PLATFORM 0; // Index of platform_current_mah_e

typedef enum platform_current_mah_e{
    PLATFORM_CURRENT_ESP = 80,
    PLATFORM_CURRENT_NXP = 100,
    PLATFORM_CURRENT_STM = 120,
    PLATFORM_CURRENT_NRF = 60
} platform_current_mah_e;

class PlatformCurrent: public QObject
{
    Q_OBJECT
public:
    explicit PlatformCurrent(platform_current_mah_e platform, QObject *p = nullptr);
    void setPlatformCurrent(platform_current_mah_e platformCurrent){platformCurr = platformCurrent;}

    platform_current_mah_e getPlatformCurrent() const;


public slots:
    void onPlatformCurrentRequested();

signals:
    void platformCurrentReady(float currentMA);

private:
    bool currConsumptionRequest;
    bool currBcgRequest;
    platform_current_mah_e platformCurr;
};

#endif // PLATFORMCURRENT_H
