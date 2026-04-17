#include "BatteryCurrentGenerator.h"


BatteryCurrentGenerator::BatteryCurrentGenerator(QObject *parent)
    : QObject{parent}
{
    iSys       = 0.0f;
    iPlatform  = 0.0f;
    iBat       = 0.0f;
    outputCurrFlag = N;
    latch      = false;
}

float BatteryCurrentGenerator::getSystemCurrent(float currA){
    iSys = currA;
    return iSys;
}

void BatteryCurrentGenerator::onPlatformCurrentReady(float currentA){
    iPlatform = currentA;
}


void BatteryCurrentGenerator::getBcgParams(float currA, current_flag_e currFlag){
    switch(currFlag){

        // Flag N means no Algo execution
        case N:{
            iSys = currA;
            iPlatform = 0;
            iBat = iSys;

            outputCurrFlag = currFlag;
            latch = false;

            break;
        }

        // Flag P means parrallel regime
        case P:{
            iSys = currA;

            emit requestPlatformCurrent(); // Get platform current

            iBat = iSys + iPlatform; // [A]

            //TODO: Flag and latch lopgic should be implemented
            outputCurrFlag = currFlag;

            break;
        }
        case S:{


            break;
        }

        default: break;
    }

    emit batteryCurrentReady(iSys, iPlatform, iBat, 0.01);
}

float BatteryCurrentGenerator::getIBat() const {return iBat;}

float BatteryCurrentGenerator::getIplatform() const {return iPlatform;}

float BatteryCurrentGenerator::getIsys() const {return iSys;}
