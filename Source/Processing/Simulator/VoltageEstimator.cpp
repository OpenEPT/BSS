#include "VoltageEstimator.h"
#include <cmath>
#include <QDebug>

VoltageEstimator::VoltageEstimator(QObject *parent)
    : QObject{parent}
    , batteryModel(nullptr)
    , vSlow(0.0)
    , vFast(0.0)
    , vTerminal(0.0)
{
}

void VoltageEstimator::voltageEstimatorInit(BatteryModel *model)
{
    batteryModel = model;
    vSlow        = 0.0;
    vFast        = 0.0;
    vTerminal    = 0.0;
}

// Dual RC model:
//   alphaSlow = exp(-Ts / (rSlow * cSlow))
//   alphaFast = exp(-Ts / (rFast * cFast))
//   vSlow[k]  = (vSlow[k-1] - I*rSlow) * alphaSlow + I*rSlow
//   vFast[k]  = (vFast[k-1] - I*rFast) * alphaFast + I*rFast
//   vTerminal = OCV - I*rInternal - vSlow - vFast
double VoltageEstimator::calculateTerminalVoltage(float currA, float Ts)
{
    if (!batteryModel) {
        qWarning() << "VoltageEstimator: batteryModel not initialized!";
        return 0.0;
    }

    batteryParamaters_t params = batteryModel->stepAndGetParams(currA, Ts);

    double alphaSlow = std::exp(-Ts / (params.rSlow * params.cSlow));
    double alphaFast = std::exp(-Ts / (params.rFast * params.cFast));

    vSlow = (vSlow - currA * params.rSlow) * alphaSlow + currA * params.rSlow;
    vFast = (vFast - currA * params.rFast) * alphaFast + currA * params.rFast;

    double ocv = batteryModel->getOcv();

    vTerminal = ocv - currA * params.rInternal - vSlow - vFast;

    emit terminalVoltageCalculated(vTerminal, batteryModel->getSoC());

    return vTerminal;
}

double VoltageEstimator::getVTerminal() const { return vTerminal; }
double VoltageEstimator::getVSlow()     const { return vSlow; }
double VoltageEstimator::getVFast()     const { return vFast; }
