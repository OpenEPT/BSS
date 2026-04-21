#ifndef VOLTAGEESTIMATOR_H
#define VOLTAGEESTIMATOR_H

#include <QObject>
#include "BatteryModel.h"

#define SAMPLING_PERIOD_S  0.01

typedef struct voltagesEstimators_t {
    double vSlow;
    double vFast;
    double vTerminal;
} voltagesEstimators_t;

class VoltageEstimator : public QObject
{
    Q_OBJECT

public:
    explicit VoltageEstimator(QObject *parent = nullptr);

    void   voltageEstimatorInit(BatteryModel *model);

    double getVTerminal() const;
    double getVSlow()     const;
    double getVFast()     const;

public slots:
    void calculateTerminalVoltage(float iSys, float iPlatform, float iBat, float Ts = SAMPLING_PERIOD_S);

signals:
    void terminalVoltageCalculated(double vTerminal, float iSys, float iPlatform, double iBat, double soC);

private:

    // Inputs
    BatteryModel *batteryModel;
    float iBat;

    // Outputs
    voltagesEstimators_t voltage;
};

#endif // VOLTAGEESTIMATOR_H
