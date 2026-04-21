#include "SocReference.h"

SocReference::SocReference(QObject *parent)
    : QObject{parent}
{
    soC      = 1.0f;
    socIsys  = 1.0f;
    capacity = BATTERY_CAPACITY_mAh * 3.6f;
}

void SocReference::socReferenceSet(double socReference){
    soC = socReference;
}

void SocReference::setSocIsys(double socIsysCurrent){
    socIsys = socIsysCurrent;
}


double SocReference::calculateRefSoC(double currA, double Ts)
{
    soC -= (currA * Ts) / capacity;

    if (soC < 0.0f) soC = 0.0f;
    if (soC > 1.0f) soC = 1.0f;

    return soC;
}


double SocReference::calculateRefSoCIsys(double currA, double Ts)
{
    socIsys -= (currA * Ts) / capacity;

    if (socIsys < 0.0f) socIsys = 0.0f;
    if (socIsys > 1.0f) socIsys = 1.0f;

    return socIsys;
}

double SocReference::getSoC() const
{
    return soC;
}

double SocReference::getSocIsys() const
{
    return socIsys;
}

double SocReference::getCapacity() const
{
    return capacity;
}
