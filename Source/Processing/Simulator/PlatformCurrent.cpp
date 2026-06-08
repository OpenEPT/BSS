#include "PlatformCurrent.h"

#include <QDebug>

PlatformCurrent::PlatformCurrent(platform_current_mah_e platform, QObject *parent)
    : QObject(parent)
{
    currBcgRequest = false;
    currConsumptionRequest = false;
    platformCurr = platform;
}


void PlatformCurrent::onPlatformCurrentRequested(){
    float current = static_cast<float>(platformCurr);
    emit platformCurrentReady(current / 1000); // Converts to mAh
}

platform_current_mah_e PlatformCurrent::getPlatformCurrent() const {return platformCurr;}
