#ifndef ALGO_H
#define ALGO_H
#include <QVector>
#include <QObject>
#include "TimeTable.h"
#include "SimulatorInputProcessing.h"

typedef struct algoCurrentInfo_t {
    current_flag_e flag;
    float currentPlatform;
} algoCurrentInfo_t;

typedef enum {
    ALGO_PREPROCESSING_INACTIVE   = 0,
    ALGO_PREPROCESSING_TRANSITION = 1,
    ALGO_PREPROCESSING_ACTIVE     = 2,
} algoPreprocessingState_e;

typedef enum {
    ALGO_PROCESSING_IDLE    = 0,
    ALGO_PROCESSING_RUNNING = 1,
} algoProcessingState_e;

typedef enum {
    ALGO_POSTPROCESSING_IDLE    = 0,
    ALGO_POSTPROCESSING_RUNNING = 1,
    ALGO_POSTPROCESSING_DONE    = 2,
} algoPostprocessingState_e;

typedef struct {
    algoPreprocessingState_e  preprocessing;
    algoProcessingState_e     processing;
    algoPostprocessingState_e postprocessing;
} algoStates_t;

// Internal algo state machine state
typedef enum {
    ALGO_INACTIVE,
    ALGO_WAITING_VE,
    ALGO_PROCESSING,
    ALGO_POSTPROCESSING
} algoInternalState_e;

typedef enum {
    ALGO_PERIOD_FROM_FILE   = 0,  // Read period from file N/P flag
    ALGO_PERIOD_FIXED       = 1,  // Fixed period configured by user
} algoPeriodMode_e;

typedef struct algoConfig_t {
    algoPeriodMode_e periodMode = ALGO_PERIOD_FROM_FILE;
    int              fixedPeriod = 100;     // We can configure period if it's fixed
    void            *userData   = nullptr;  // For custom logic
} algoConfig_t;

class Algo : public QObject {
    Q_OBJECT
public:
    // default read from file with config
    explicit Algo(algoTimeTableDuration_e whitchAlgo, algoConfig_t config, QObject *p = nullptr);

    void algoInit(const algoConfig_t &config);
    void              setAlgoDynamics(const QVector<algoFlags_t>& flags);
    void algoReset(float socReference = 1.0f);

    algoCurrentInfo_t preProcessing();
    void              processing(double iBat);
    void              postProcessing();

    algoCurrentInfo_t preProcessingSystem();
    void              processingSystem(double iBat);
    void              postProcessingSystem();

    void              preProcessingUser();
    void              processingUser(float iBat);
    void              postProcessingUser();

    float             getQBath();
    static int algoNameToIndex(algoTimeTableDuration_e algo);


    // User getters
    float getAlgoSoCOutput() const {return algoSocOutput;}

public slots:
    void onPlatformCurrentReady(double currentA);
    void onVoltageEstimatorDone(double vBat, float iSys, float iPlatform, double iBat, float soc);
    void onAlgoDuration(int duration);
    algoTimeTableDuration_e intToAlgoDuration(int index);

signals:
    void getWitchAlgo(algoTimeTableDuration_e whitchAlgo);
    void flagStatusChanged(current_flag_e activeFlag);
    void requestPlatformCurrent();
    void preprocessingDone();

private:
    algoConfig_t algoConfig;
    algoTimeTableDuration_e   algo;
    QVector<algoFlags_t>      algoDynamics;
    algoStates_t              algoStates;
    algoInternalState_e       algoState    = ALGO_INACTIVE;
    algoCurrentInfo_t         algoCurrentInfo;

    current_flag_e  lastFlag                     = N;
    current_flag_e  currentFlag                  = N;
    current_flag_e  previousFlag                 = N;

    bool            calculateExeTimeInProcessing = false;
    bool            ppSignalReady                = false;
    bool            veReady                      = false;
    bool            ppDone                       = false;
    bool            nextCycleResetAlgoPeriod     = false;

    int             fixedPeriodCounter           = 0;
    int             currentDynIndex              = 0;
    int             remainingSteps               = 0;
    int             lastRemainingSteps           = 0;
    int             ppCounter                    = 0;
    int             ppCycles                     = 0;
    int             algoPeriod                   = 1;

    float           algoDurationPerSample        = 0.0f;
    float           qBath                        = 0.0f;


    // User Algo
    double           algoSocOutput               = 1.0;
};

#endif // ALGO_H
