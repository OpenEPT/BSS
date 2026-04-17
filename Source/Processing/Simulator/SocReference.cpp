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

void SocReference::setSocIsys(float socIsysCurrent){
    socIsys = socIsysCurrent;
}


float SocReference::calculateRefSoC(float currA, float Ts)
{
    soC -= (currA * Ts) / capacity;

    if (soC < 0.0f) soC = 0.0f;
    if (soC > 1.0f) soC = 1.0f;

    return soC;
}


float SocReference::calculateRefSoCIsys(float currA, float Ts)
{
    socIsys -= (currA * Ts) / capacity;

    if (socIsys < 0.0f) socIsys = 0.0f;
    if (socIsys > 1.0f) socIsys = 1.0f;

    return socIsys;
}

float SocReference::getSoC() const
{
    return soC;
}

float SocReference::getSocIsys() const
{
    return socIsys;
}

float SocReference::getCapacity() const
{
    return capacity;
}
