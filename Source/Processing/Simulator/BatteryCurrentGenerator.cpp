#include "BatteryCurrentGenerator.h"


BatteryCurrentGenerator::BatteryCurrentGenerator(QObject *parent)
    : QObject{parent}
    , current{0.0, 0.0, 0.0}
{
    algoCycles = 0;
    cycleCounter = 0;
    latch = false;
    outputCurrFlag = N;
    latch      = false;
}

double BatteryCurrentGenerator::getSystemCurrent(double currA){
    current.iSys = currA;
    return current.iSys;
}

void BatteryCurrentGenerator::getBcgParams(double currA) {

    algoCurrentInfo_t algoCurrentInfo = algo->preProcessing();

    switch(algoCurrentInfo.flag) {

    // Flag N, No Algo execution - Platforma off
    case N: {
        current.iSys = currA;
        current.iPlatform = 0.0;
        current.iBat = current.iSys;
        outputCurrFlag = N;
        break;
    }

    // Flag P parallel regime - Algo works, Platform on
    case P: {
        current.iSys = currA;
        current.iPlatform = algoCurrentInfo.currentPlatform;
        current.iBat = current.iSys + current.iPlatform;
        outputCurrFlag = P;
        break;
    }

    case S: {
        current.iSys = currA;
        current.iPlatform = 0.0;
        current.iBat = current.iSys;
        outputCurrFlag = S;
        break;
    }

    default: break;
    }

    // BCG ready, signal for voltage estimator
    emit batteryCurrentReady(current.iSys, current.iPlatform, current.iBat, 0.01);
}

void BatteryCurrentGenerator::updateFlagFromAlgo(current_flag_e newFlag)
{
    // Update the internal output flag immediately
    this->outputCurrFlag = newFlag;

    // If the flag is N, ensure platform current is 0
    if (newFlag == N) {
        current.iPlatform = 0.0;
    }
}

double BatteryCurrentGenerator::getIBat() const {return current.iBat;}

double BatteryCurrentGenerator::getIplatform() const {return current.iPlatform;}

double BatteryCurrentGenerator::getIsys() const {return current.iSys;}
