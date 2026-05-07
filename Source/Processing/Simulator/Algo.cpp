#include "Algo.h"


Algo::Algo(algoTimeTableDuration_e whitchAlgo,
           algoConfig_t config,
           QObject *p)
    : QObject(p)
{
    this->algo = whitchAlgo;
    algoInit(config);
}

void Algo::algoInit(const algoConfig_t &config)
{
    algoConfig = config;
}

void Algo::onPlatformCurrentReady(double currentA){
    algoCurrentInfo.currentPlatform = currentA;
}

void Algo::setAlgoDynamics(const QVector<algoFlags_t>& flags) {
    this->algoDynamics = flags;
    currentDynIndex = 0;
    remainingSteps = 0;
}

void Algo::algoReset(float socReference)
{
    currentDynIndex    = 0;
    remainingSteps     = 0;
    ppSignalReady      = false;
    veReady            = false;
    fixedPeriodCounter = 0;
    algoSocOutput      = socReference;
}

/*********** Algo processing functions *******************/

algoCurrentInfo_t Algo::preProcessing(){
    algoCurrentInfo_t algoCurrentInfo = preProcessingSystem();
    preProcessingUser();

    return algoCurrentInfo;
}


void Algo::processing(double iBat){
    processingSystem(iBat);

    if (currentFlag == P){
        processingUser(iBat);
    }

    // Call Post Processing
    postProcessing();
}


void Algo::postProcessing(){
    postProcessingSystem();
    postProcessingUser();
}


/*********** Algo System processing functions *******************/

algoCurrentInfo_t Algo::preProcessingSystem()
{
    // Take every step current and previous flag
    if (algoConfig.periodMode == ALGO_PERIOD_FIXED) {
        previousFlag = currentFlag;  // ← sačuvaj pre promene

        fixedPeriodCounter++;

        if (fixedPeriodCounter >= algoConfig.fixedPeriod) {
            currentFlag        = P;
            fixedPeriodCounter = 0;
        } else {
            currentFlag = N;
        }
    } else {
        // Čita iz fajla - postojeći kod
        if (currentDynIndex < algoDynamics.size()) {
            if (remainingSteps > 0) {
                currentFlag = algoDynamics[currentDynIndex - 1].flag;
                remainingSteps--;
            } else {
                currentFlag = algoDynamics[currentDynIndex].flag;

                if (currentDynIndex == 0) {
                    previousFlag = N;
                } else {
                    previousFlag = algoDynamics[currentDynIndex - 1].flag;
                }

                remainingSteps = algoDynamics[currentDynIndex].exe - 1;

                if (currentDynIndex >= 2) {
                    lastRemainingSteps = algoDynamics[currentDynIndex - 2].exe;
                } else {
                    lastRemainingSteps = 0;
                }

                lastRemainingSteps = currentDynIndex++;
            }
        }
    }

    // If latched - always output P regardless of what vector says
    if (ppDone) {

        if ((currentFlag != P && previousFlag == N) || (currentFlag != P)) {
            algoPeriod++;
        }

        calculateExeTimeInProcessing = false;
        algoCurrentInfo.flag = P;

        return algoCurrentInfo; // BCG request flag
    }

    if ((currentFlag != P && previousFlag == P)) {

        algoPeriod = 0;
        lastFlag = previousFlag;

        algoCurrentInfo.flag = lastFlag;

        calculateExeTimeInProcessing = true;
        ppDone = true;

        return algoCurrentInfo;
    }

    // P flag: self-latch and signal processing
    if (currentFlag == P && !ppDone) {

        calculateExeTimeInProcessing = true;
        ppDone = true;
        ppSignalReady = true;

        algoState = ALGO_WAITING_VE;
        algoStates.preprocessing = ALGO_PREPROCESSING_ACTIVE;

        emit requestPlatformCurrent();
    }

    if(currentFlag == N){
        algoPeriod++;
    }
    lastFlag = currentFlag;
    algoCurrentInfo.flag = ppDone ? P : currentFlag;

    return algoCurrentInfo;
}

void Algo::processingSystem(double iBat)
{
    if (!ppSignalReady || !veReady) return;

    ppSignalReady = false;
    veReady       = false;
    algoStates.processing = ALGO_PROCESSING_RUNNING;


    if (calculateExeTimeInProcessing == true){
        // Get exe
        int exeCycles = 0;

        if(algoConfig.periodMode == ALGO_PERIOD_FROM_FILE){
            if (currentDynIndex > 0 && currentDynIndex <= algoDynamics.size()) {
                exeCycles = algoDynamics[currentDynIndex - 1].exe;
                algo = intToAlgoDuration(exeCycles);
            }
        }

        else{
            // Algo is already good value due to init algo
        }

        // Trigger TimeTable → onAlgoDuration → ppCycles
        emit getWitchAlgo(algo);
    }
}


void Algo::postProcessingSystem()
{
    ppCounter++;

    qBath += (algoDurationPerSample * algoCurrentInfo.currentPlatform) / 3.6f; // [mAh]

    if(nextCycleResetAlgoPeriod){
        nextCycleResetAlgoPeriod  = false;
        if (currentFlag == P){
            algoPeriod = 1;
        }
    }

    if (ppCounter >= ppCycles) {

        nextCycleResetAlgoPeriod = true;;
        ppDone                   = false;

        ppCounter                 = 0;
        ppCycles                  = 0;
        algoDurationPerSample     = 0.0f;

        emit preprocessingDone();
    }
}


/*********** Algo User processing functions *******************/
void Algo::preProcessingUser(){}

void Algo::processingUser(float iBat){

    // P flag - Calculate Coulomb Counter
    algoSocOutput -= (iBat * (static_cast<float>(algoPeriod) * 0.01f)) / (457.0f * 3.6f);

    if (algoSocOutput < 0.0f) algoSocOutput = 0.0f;
    if (algoSocOutput > 1.0f) algoSocOutput = 1.0f;

}
void Algo::postProcessingUser(){}


// Getters:
float Algo::getQBath(){
    return qBath;
}

/******************************* Slots *******************************/


void Algo::onAlgoDuration(int duration)
{
    float T_algo = static_cast<float>(duration) / 1000.0f; // ms → s
    float Ts     = 0.01f; // 10ms

    algoDurationPerSample = T_algo; // [s]

    if (Ts >= T_algo) {
        ppCycles = 0;
        return;
    }

    // Clamp value to the sample value
    if(algoDurationPerSample >= Ts){
        algoDurationPerSample = Ts;
    }

    ppCycles = static_cast<int>(T_algo / Ts);
}

void Algo::onVoltageEstimatorDone(double vBat, float iSys, float iPlatform, double iBat, float soc)
{
    Q_UNUSED(vBat) Q_UNUSED(iSys) Q_UNUSED(iPlatform) Q_UNUSED(soc)

    veReady = true;     // Set Vrdy
    processing(iBat);       // Call processing system
}

algoTimeTableDuration_e Algo::intToAlgoDuration(int index)
{
    switch(index) {
    case 1:  return LP;
    case 2:  return K0;
    case 3:  return K2;
    case 4:  return ADAPTIVE_LP;
    case 5:  return ADAPTIVE_LP_K0;
    case 6:  return ADAPTIVE_LP_K2;
    default: return LP;
    }
}


