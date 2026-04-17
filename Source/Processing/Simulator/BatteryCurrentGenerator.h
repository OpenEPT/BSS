#ifndef BATTERYCURRENTGENERATOR_H
#define BATTERYCURRENTGENERATOR_H

#include <QObject>
#include <QVector>
#include "SimulatorInputProcessing.h"


class BatteryCurrentGenerator : public QObject
{
    Q_OBJECT

public:

    explicit BatteryCurrentGenerator(QObject *ptr = nullptr);

    bool  isLatchActive();

    float getTime();
    float getIndex();
    float getSystemCurrent(float currA);
    void  getBcgParams(float currA, current_flag_e currFlag);
    float getIBat() const;
    float getIplatform() const;
    float getIsys() const;

    void setTimeAlgoExe();

public slots:
    void onPlatformCurrentReady(float currentA);

signals:
    void requestPlatformCurrent();
    void batteryCurrentReady(float iSys, float iPlatform, float iBat, float Ts);

private:

    current_flag_e outputCurrFlag;
    bool latch;
    float iSys;
    float iPlatform;
    float iBat;
    TickTimer_t tickTimer;
    float algoTimeExe;
};

#endif // BATTERYCURRENTGENERATOR_H
