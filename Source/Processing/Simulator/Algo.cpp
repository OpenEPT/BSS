#include "Algo.h"


Algo::Algo(algoTimeTableDuration_e whitchAlgo, QObject *p)
    : QObject(p)
{
    this->algo = whitchAlgo;
}

void Algo::onPlatformCurrentReady(double currentA){
    algoCurrentInfo.currentPlatform = currentA;
}

void Algo::setAlgoDynamics(const QVector<algoFlags_t>& flags) {
    this->algoDynamics = flags;
    currentDynIndex = 0;
    remainingSteps = 0;
}

void Algo::algoReset(){
    currentDynIndex = 0;
    remainingSteps = 0;
    ppSignalReady = false;
    veReady       = false;
}

/*********** Algo processing functions *******************/

algoCurrentInfo_t Algo::preProcessing(){
    algoCurrentInfo_t algoCurrentInfo = preProcessingSystem();
    preProcessingUser();

    return algoCurrentInfo;
}


void Algo::processing(double iBat){
    processingSystem(iBat);
    processingUser(iBat);
}


void Algo::postProcessing(){
    postProcessingSystem();
    postProcessingUser();
}


/*********** Algo System processing functions *******************/

algoCurrentInfo_t Algo::preProcessingSystem()
{
    // Always increment position in vector
    current_flag_e currentFlag = N;

    if (currentDynIndex < algoDynamics.size()) {
        if (remainingSteps > 0) {
            currentFlag = algoDynamics[currentDynIndex - 1].flag;
            remainingSteps--;
        } else {
            currentFlag    = algoDynamics[currentDynIndex].flag;
            remainingSteps = algoDynamics[currentDynIndex].exe - 1;
            currentDynIndex++;
        }
    }

    // If latched - always output P regardless of what vector says
    if (latch) {
        algoPeriod++;
        calculateExeTimeInProcessing = false;
        latch         = true;
        ppSignalReady = true;
        algoCurrentInfo.flag = P;
        return algoCurrentInfo;
    }

    // P flag: self-latch and signal processing
    if (currentFlag == P && !latch) {
        calculateExeTimeInProcessing = true;
        latch         = true;
        ppSignalReady = true;
        algoState     = ALGO_WAITING_VE;
        algoStates.preprocessing = ALGO_PREPROCESSING_ACTIVE;
        emit requestPlatformCurrent();
    }

    if(currentFlag != P){
        algoPeriod++;
    }

    lastFlag             = currentFlag;
    algoCurrentInfo.flag = latch ? P : currentFlag;
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
        if (currentDynIndex > 0 && currentDynIndex <= algoDynamics.size()) {
            exeCycles = algoDynamics[currentDynIndex - 1].exe;
        }

        algo = intToAlgoDuration(exeCycles);

        // Trigger TimeTable → onAlgoDuration → ppCycles
        emit getWitchAlgo(algo);

        ppCounter                 = 0;
        algoState                 = ALGO_POSTPROCESSING;
        algoStates.processing     = ALGO_PROCESSING_IDLE;
        algoStates.postprocessing = ALGO_POSTPROCESSING_RUNNING;
    }
}


void Algo::postProcessingSystem()
{
    if (algoState != ALGO_POSTPROCESSING) return;

    ppCounter++;

    qBath += (algoDurationPerSample * algoCurrentInfo.currentPlatform) / 3.6f; // [mAh]

    if (ppCounter >= ppCycles) {
        algoPeriod = 1;
        algoDurationPerSample     = 0.0f;
        latch                     = false;
        ppCounter                 = 0;
        ppCycles                  = 0;
        algoState                 = ALGO_INACTIVE;
        algoStates.preprocessing  = ALGO_PREPROCESSING_INACTIVE;
        algoStates.postprocessing = ALGO_POSTPROCESSING_DONE;
        emit preprocessingDone();
    }
}


/*********** Algo User processing functions *******************/
void Algo::preProcessingUser(){}

void Algo::processingUser(float iBat){
    if (!latch) return;  // ← N flag,

    // P flag - Calculate Coulomb Counter
    algoSocOutput -= (iBat * (static_cast<float>(algoPeriod) * 0.01f)) / (457.0f * 3.6f);

    if (algoSocOutput < 0.0f) algoSocOutput = 0.0f;
    if (algoSocOutput > 1.0f) algoSocOutput = 1.0f;

    // Call Post Processing
    postProcessing();
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


