#include "Algo.h"
#include "QDebug"


Algo::Algo(algoTimeTableDuration_e whitchAlgo,
           algoConfig_t config,
           void *userInitParams,
           QObject *p)
    : QObject(p)
{
    this->algo = whitchAlgo;
    algoInit(config);
    algoInitUser(userInitParams);
}

void Algo::algoInit(const algoConfig_t &config)
{
    algoConfig = config;
}

void Algo::algoInitUser(void *initiParams)
{
    KALMAN0_Init(&k0);
    KALMAN_Init(&k2);
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


void Algo::processing(double vBat, double iBat){
    processingSystem(iBat);

    if (currentFlag == P){
        processingUser(vBat, iBat);
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
        previousFlag = currentFlag;

        fixedPeriodCounter++;

        if (fixedPeriodCounter >= algoConfig.fixedPeriod) {
            currentFlag        = P;
            fixedPeriodCounter = 0;
        } else {
            currentFlag = N;
        }
    } else {
        if (currentDynIndex < algoDynamics.size()) {
            previousFlag = currentFlag;

            currentFlag = algoDynamics[currentDynIndex].flag;

            int algoType = algoDynamics[currentDynIndex].exe;
            if (currentFlag == P && algoType > 0) {
                algo = intToAlgoDuration(algoType);
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

        algoCurrentInfo.time += 0.01f;
        return algoCurrentInfo; // BCG request flag
    }

    if ((currentFlag != P && previousFlag == P)) {

        algoPeriod = 0;
        lastFlag = previousFlag;

        // TODO: Check when algo duration is bigger then Ts
        //algoCurrentInfo.flag = lastFlag;
        algoCurrentInfo.flag = currentFlag;

        calculateExeTimeInProcessing = true;
        ppDone = true;


        algoCurrentInfo.time += 0.01f;
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


    algoCurrentInfo.time += 0.01f;

    return algoCurrentInfo;
}

void Algo::processingSystem(double iBat)
{
    if (!ppSignalReady || !veReady) return;

    ppSignalReady = false;
    veReady       = false;
    algoStates.processing = ALGO_PROCESSING_RUNNING;

    if (calculateExeTimeInProcessing == true){
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
        currentFlag              = N;

        ppCounter                 = 0;
        ppCycles                  = 0;
        algoDurationPerSample     = 0.0f;

        emit preprocessingDone();
    }

    currentDynIndex++;
}


/*********** Algo User processing functions *******************/
void Algo::preProcessingUser(){}

void Algo::processingUser(double vBat, float iBat)
{
    int perf1 = 0;

    float Ts   = algoPeriod * 0.01f;
    float QmAh = 457.0f;

    switch(currentAlgo)
    {
    case K0:{
        KALMAN0_Predict(&k0, iBat, Ts, QmAh, &perf1);
        algoSocOutput = KALMAN0_Update(&k0, iBat, vBat);
        break;
    }
    case K2:{
        KALMAN_Predict(&k2, iBat, Ts, QmAh, &perf1);
        algoSocOutput = KALMAN_Update(&k2, iBat, vBat);
        break;
    }
    case LP:{
        float dSoC = -(iBat * Ts) / (QmAh * 3.6f);
        algoSocOutput += dSoC;
        break;
    }
    default:
        break;
    }
}


void Algo::postProcessingUser(){}


// Getters:
double Algo::getQBath(){
    return qBath;
}

/******************************* Slots *******************************/


void Algo::onAlgoDuration(int duration)
{
    // duration [μs]
    float T_algo = static_cast<float>(duration) / 1000000.0f; // μs → s
    float Ts     = 0.01f; // 10[ms] -> 0.01s

    algoDurationPerSample = T_algo; // [s]

    if (T_algo <= Ts) {
        ppCycles = 1;
        return;
    }

    ppCycles = static_cast<int>(T_algo / Ts);
}

void Algo::onVoltageEstimatorDone(double vBat, float iSys, float iPlatform, float iBat, float soc)
{
    Q_UNUSED(iSys) Q_UNUSED(iPlatform) Q_UNUSED(soc)

    veReady = true;     // Set Vrdy
    processing(vBat, iBat);       // Call processing system
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

void Algo::setAlgo(algoTimeTableDuration_e algo)
{
    currentAlgo = algo;
}

