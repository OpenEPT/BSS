#include "BatteryModel.h"
#include <cmath>
#include <QDebug>

BatteryModel::BatteryModel(QObject *parent)
    : QObject{parent}
{
    socReference = new SocReference(this);
    memset(&currentParams, 0, sizeof(batteryParamaters_t));
}

void BatteryModel::batteryModelInit(SimulatorInput *input)
{
    lookupParams  = input->getBatteryParams();
    lookupOcvPoly = input->getCoefficientOcvPoly();

    qDebug() << "BatteryModel: loaded" << lookupParams.size()  << "param rows";
    qDebug() << "BatteryModel: loaded" << lookupOcvPoly.size() << "OCV poly coeffs";
}

batteryParamaters_t BatteryModel::stepAndGetParams(float currA, float Ts)
{
    socReference->calculateRefSoC(currA, Ts);

    float soC    = socReference->getSoC();
    float dodPct = (1.0f - soC) * 100.0f;

    currentParams = findParamsByDoD(dodPct);
    return currentParams;
}

batteryParamaters_t BatteryModel::findParamsByDoD(float dodPct) const
{
    if (lookupParams.isEmpty())
        qDebug() << "Lookup params is empty";
        return batteryParamaters_t{};

    for (int i = 0; i < lookupParams.size() - 1; i++) {
        if (dodPct >= lookupParams[i].dod &&
            dodPct <  lookupParams[i+1].dod) {
            return lookupParams[i];
        }
    }

    return lookupParams.last();
}

// OCV polynomial evaluation: OCV = sum(coeff[i] * SoC^i)
double BatteryModel::getOcv() const
{
    double soC    = static_cast<double>(socReference->getSoC());
    double ocv    = 0.0;
    double socPow = 1.0;

    for (int i = 0; i < lookupOcvPoly.size(); i++) {
        ocv    += lookupOcvPoly[i] * socPow;
        socPow *= soC;
    }

    return ocv;
}

float BatteryModel::getSoC() const
{
    return socReference->getSoC();
}

batteryParamaters_t BatteryModel::getCurrentParams() const
{
    return currentParams;
}

QVector<batteryParamaters_t> BatteryModel::getBMParameters() const
{
    return lookupParams;
}

QVector<double> BatteryModel::getBMPolynomCoefficients() const
{
    return lookupOcvPoly;
}
