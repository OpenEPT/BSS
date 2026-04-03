#include "SocReference.h"

SocReference::SocReference(QObject *parent)
    : QObject{parent}
{
    soC      = 1.0f;
    capacity = BATTERY_CAPACITY_mAh * 3.6f;
}

float SocReference::calculateRefSoC(float currA, float Ts)
{
    soC -= (currA * Ts) / capacity;

    if (soC < 0.0f) soC = 0.0f;
    if (soC > 1.0f) soC = 1.0f;

    return soC;
}

float SocReference::getSoC() const
{
    return soC;
}

float SocReference::getCapacity() const
{
    return capacity;
}
