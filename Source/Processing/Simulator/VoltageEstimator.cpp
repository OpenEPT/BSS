#include "VoltageEstimator.h"
#include <cmath>
#include <QDebug>

VoltageEstimator::VoltageEstimator(QObject *parent)
    : QObject{parent}
    , batteryModel(nullptr)
    , voltage{0.0, 0.0, 0.0}
{
    iBat = 0;
}

void VoltageEstimator::voltageEstimatorInit(BatteryModel *model)
{
    batteryModel = model;
    voltage.vSlow        = 0.0;
    voltage.vFast        = 0.0;
    voltage.vTerminal    = 0.0;
}

void VoltageEstimator::calculateTerminalVoltage(float iSys, float iPlatform, float iBat, float Ts)
{
    if (!batteryModel) {
        qWarning() << "VoltageEstimator: batteryModel not initialized!";
        return;
    }

    batteryParamaters_t params = batteryModel->stepAndGetParams(iBat, Ts);

    double alphaSlow = std::exp(-Ts / (params.rSlow * params.cSlow));
    double alphaFast = std::exp(-Ts / (params.rFast * params.cFast));

    voltage.vSlow = (voltage.vSlow - iBat * params.rSlow) * alphaSlow + iBat * params.rSlow;
    voltage.vFast = (voltage.vFast - iBat * params.rFast) * alphaFast + iBat * params.rFast;

    double ocv = batteryModel->getOcv();

    voltage.vTerminal = ocv - iBat * params.rInternal - voltage.vSlow - voltage.vFast;

    emit terminalVoltageCalculated(voltage.vTerminal, iSys, iPlatform, iBat, batteryModel->getSoC());
}

double VoltageEstimator::getVTerminal() const { return voltage.vTerminal; }
double VoltageEstimator::getVSlow()     const { return voltage.vSlow; }
double VoltageEstimator::getVFast()     const { return voltage.vFast; }
