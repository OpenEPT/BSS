#ifndef BATTERYCURRENTGENERATOR_H
#define BATTERYCURRENTGENERATOR_H

#include <QObject>
#include <QVector>
#include "SimulatorInputProcessing.h"
#include "TimeTable.h"
#include "Algo.h"

typedef struct currentBCG_t{
    double iSys;
    double iPlatform;
    double iBat;
} currentBCG_t;

class BatteryCurrentGenerator : public QObject
{
    Q_OBJECT

public:

    explicit BatteryCurrentGenerator(QObject *ptr = nullptr);

    bool  isLatchActive();

    double getTime();
    float getIndex();
    double getSystemCurrent(double currA);
    void  getBcgParams(double currA);
    double getIBat() const;
    double getIplatform() const;
    double getIsys() const;
    current_flag_e getOutputFlag() const { return outputCurrFlag; }

    void setTimeAlgoExe();

public slots:
    void updateFlagFromAlgo(current_flag_e newFlag);
    void setAlgo(Algo *a) { algo = a; }

signals:
    void batteryCurrentReady(double iSys, double iPlatform, double iBat, double Ts);

private:
    Algo *algo;
    current_flag_e outputCurrFlag;
    TickTimer_t tickTimer;
    currentBCG_t current;
    bool latch;
    double algoTimeExe;
    int algoCycles;      // How many cycles algo execute
    int cycleCounter;    // Cycles current counter
};

#endif // BATTERYCURRENTGENERATOR_H
