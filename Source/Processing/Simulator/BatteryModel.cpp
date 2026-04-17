#include "BatteryModel.h"
#include "SimulatorInputProcessing.h"
#include "SocReference.h"
#include <cmath>
#include <QDebug>

BatteryModel::BatteryModel(QObject *parent)
    : QObject{parent}
{
    socReference = new SocReference(this);
    memset(&currentParams, 0, sizeof(batteryParamaters_t));
}

void BatteryModel::reset(double SocReference)
{
    socReference->socReferenceSet(SocReference);
    memset(&currentParams, 0, sizeof(batteryParamaters_t));
}

void BatteryModel::batteryModelInit(SimulatorInput *input)
{
    simulatorInput = input;

    lookupParams  = input->getBatteryParams();
    lookupOcvPoly = input->getCoefficientOcvPoly();
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
    if (lookupParams.isEmpty()) {
        qDebug() << "Lookup params is empty";
        return batteryParamaters_t{};
    }

    for (int i = 0; i < lookupParams.size() - 1; i++) {
        if (dodPct >= lookupParams[i].dod &&
            dodPct <  lookupParams[i+1].dod) {
            return lookupParams[i];
        }
    }

    return lookupParams.last();
}

double BatteryModel::getOcv() const
{
    double soC = static_cast<double>(socReference->getSoC());
    int N = lookupOcvPoly.size();

    double acc = lookupOcvPoly[0];
    for (int i = 1; i < N; i++) {
        acc = acc * soC + lookupOcvPoly[i];
    }
    return acc;
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
