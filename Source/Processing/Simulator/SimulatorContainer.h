#ifndef SIMULATORCONTAINER_H
#define SIMULATORCONTAINER_H

#include <QObject>
#include "Processing/Simulator/SimulatorInputProcessing.h"
#include "Processing/Simulator/BatteryModel.h"
#include "Processing/Simulator/VoltageEstimator.h"
#include "Processing/Simulator/SocReference.h"
#include "Processing/Simulator/PlatformCurrent.h"
#include "Processing/Simulator/BatteryCurrentGenerator.h"
#include "Processing/Simulator/TimeTable.h"
#include "Processing/Simulator/Algo.h"
#include "Processing/Simulator/NoiseGenerator.h"


typedef struct lastSimulationStepsValues{
    double  lastIBat                 = 0.0;
    double lastVBat                  = 0.0;
    double  lastISys                 = 0.0;
    double  lastIPlatform            = 0.0;
    double  lastSoC                  = 0.0;
    double  lastSoCIsys              = 0.0;
    double lastElectricChargeBattery = 0.0;
    double lastElectricChargeSystem  = 0.0;
    double lastElectricChargeAlgo    = 0.0;
} lastSimulationStepsValues;

typedef struct electricCharges_t{
    double electricChargeBattery;
    double electricChargeSystem;
    double electricChargeAlgo;
} electricCharges_t;

class SimulatorContainer : public QObject
{
    Q_OBJECT
public:
    explicit SimulatorContainer(BatteryModel    *sharedBatteryModel,
                                PlatformCurrent *sharedPlatformCurrent,
                                TimeTable       *sharedTimeTable,
                                QObject *parent = nullptr);

    bool loadFiles();
    void proccesOffline();
    void step();
    void reset(double SocReference);
    bool isFinished() const;

    void setCurrentPath (const QString &p) { currentFilePath  = p;  }
    void setParamsPath  (const QString &p) { paramsFilePath   = p;  }
    void setOcvPolyPath (const QString &p) { ocvPolyFilePath  = p;  }
    void setOcvSocPath  (const QString &p) { ocvSocFilePath   = p;  }
    void setNoisePath   (const QString &p) { noisePath        = p;  }
    void setFlagsPath   (const QString &p) { flagPath         = p;}
    void setOfflineMode (bool val)         { offlineMode      = val;}
    void setPlatformCurrent(platform_current_mah_e platformCurrent) {platformCurr->setPlatformCurrent(platformCurrent);}
    void setAlgoSelected(algoTimeTableDuration_e algoSelected){algoSelectContainer = algoSelected;}
    double  getTimeS()     const { return timeS; }
    double  getIBat()      const { return lastValues.lastIBat; }
    double  getVBat()      const { return lastValues.lastVBat; }
    double  getISys()      const { return lastValues.lastISys; }
    double  getIPlatform() const { return lastValues.lastIPlatform; }
    double  getSoC()       const { return lastValues.lastSoC; }
    double  getSoCIsys()       const { return lastValues.lastSoCIsys; }
    double  getElectricChargeBattery() const {return lastValues.lastElectricChargeBattery;}
    double  getElectricChargeSystem() const {return lastValues.lastElectricChargeSystem;}
    lastSimulationStepsValues getLastValuesOfSimulationStep() const {return lastValues;}
    int getNumberOfSamples() const { return inputProcessing->getNumberOfSamples(); }

signals:
    void dataSample(float timeS, double vBat, float iSys, float iPlatform, float iBat, float socPct, float socIsysPct);
    void loadError(const QString &msg);
    void loadSuccess();
private:
    // Owned by container:
    SimulatorInput          *inputProcessing;
    VoltageEstimator        *voltageEstimator;
    SocReference            *socReference;
    BatteryCurrentGenerator *batteryCurrGen;
    Algo                    *algo;
    NoiseGenerator          *noiseGenerator;

    // Shared - NOT owned by container:
    BatteryModel            *batteryModel;
    PlatformCurrent         *platformCurr;
    TimeTable               *timeTable;


    // OutputFiles
    QFile       *outputFile;
    QTextStream *outputStream;

    // FilePaths
    QString currentFilePath;
    QString paramsFilePath;
    QString ocvPolyFilePath;
    QString ocvSocFilePath;
    QString noisePath;
    QString flagPath;

    // Variables
    algoTimeTableDuration_e algoSelectContainer; // Whitch algo
    lastSimulationStepsValues lastValues;        // Values for every sample
    bool offlineMode = false;                    // container Mode analysys
    float timeS;                                 // Global time for simulation
    electricCharges_t electricCharges;           // TODO: ovo ce se premestiti u algo block
    int globalSampleIndex;                       // Global sample index for this containter
};
#endif // SIMULATORCONTAINER_H
