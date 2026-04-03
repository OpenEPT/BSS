#ifndef VOLTAGEESTIMATOR_H
#define VOLTAGEESTIMATOR_H

#include <QObject>
#include "BatteryModel.h"

#define SAMPLING_PERIOD_S  0.01

class VoltageEstimator : public QObject
{
    Q_OBJECT

public:
    explicit VoltageEstimator(QObject *parent = nullptr);

    void   voltageEstimatorInit(BatteryModel *model);
    double calculateTerminalVoltage(float currA, float Ts = SAMPLING_PERIOD_S);

    double getVTerminal() const;
    double getVSlow()     const;
    double getVFast()     const;

signals:
    void terminalVoltageCalculated(double vTerminal, double soC);

private:
    BatteryModel *batteryModel;

    double vSlow;
    double vFast;
    double vTerminal;
};

#endif // VOLTAGEESTIMATOR_H
