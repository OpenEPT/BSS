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
    iBat = 0;
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
void VoltageEstimator::calculateTerminalVoltage(float iSys, float iPlatform, float iBat, float Ts)
{
    if (!batteryModel) {
        qWarning() << "VoltageEstimator: batteryModel not initialized!";
        return;
    }

    batteryParamaters_t params = batteryModel->stepAndGetParams(iBat, Ts);

    double alphaSlow = std::exp(-Ts / (params.rSlow * params.cSlow));
    double alphaFast = std::exp(-Ts / (params.rFast * params.cFast));

    vSlow = (vSlow - iBat * params.rSlow) * alphaSlow + iBat * params.rSlow;
    vFast = (vFast - iBat * params.rFast) * alphaFast + iBat * params.rFast;

    double ocv = batteryModel->getOcv();

    vTerminal = ocv - iBat * params.rInternal - vSlow - vFast;

    emit terminalVoltageCalculated(vTerminal, iSys, iPlatform, iBat, batteryModel->getSoC());
}

double VoltageEstimator::getVTerminal() const { return vTerminal; }
double VoltageEstimator::getVSlow()     const { return vSlow; }
double VoltageEstimator::getVFast()     const { return vFast; }
