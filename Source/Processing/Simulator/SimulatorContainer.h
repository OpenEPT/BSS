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
    double lastIBat                  = 0.0;
    double lastVBat                  = 0.0;
    double lastISys                  = 0.0;
    double lastIPlatform             = 0.0;
    double lastSoC                   = 0.0;
    double lastSoCIsys               = 0.0;
    double lastElectricChargeBattery = 0.0;
    double lastElectricChargeSystem  = 0.0;
    double lastElectricChargeAlgo    = 0.0;

    // User Algo soc return
    double lastUserSocAlgoOutput     = 0.0;
} lastSimulationStepsValues;

typedef struct electricCharges_t{
    double electricChargeBattery = 0.0;
    double electricChargeSystem  = 0.0;
    double electricChargeAlgo    = 0.0;
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
    void step();
    void reset(double SocReference);
    bool isFinished() const;

    // Setters
    void setCurrentPath    (const QString &p) { currentFilePath = p;  }
    void setParamsPath     (const QString &p) { paramsFilePath  = p;  }
    void setOcvPolyPath    (const QString &p) { ocvPolyFilePath = p;  }
    void setOcvSocPath     (const QString &p) { ocvSocFilePath  = p;  }
    void setNoisePath      (const QString &p) { noisePath       = p;  }
    void setFlagsPath      (const QString &p) { flagPath        = p;  }
    void setOutputPath     (const QString &p) { outputFilePath  = p;  }
    void setOfflineMode    (bool val)         { offlineMode     = val;}
    void setPlatformCurrent(platform_current_mah_e pc) { platformCurr->setPlatformCurrent(pc); }
    void setAlgoSelected   (algoTimeTableDuration_e a) { algoSelectContainer = a; }

    // Getters
    double getTimeS()       const { return timeS; }
    double getIBat()        const { return lastValues.lastIBat; }
    double getVBat()        const { return lastValues.lastVBat; }
    double getISys()        const { return lastValues.lastISys; }
    double getIPlatform()   const { return lastValues.lastIPlatform; }
    double getSoC()         const { return lastValues.lastSoC; }
    double getSoCIsys()     const { return lastValues.lastSoCIsys; }
    double getElectricChargeBattery() const { return lastValues.lastElectricChargeBattery; }
    double getElectricChargeSystem()  const { return lastValues.lastElectricChargeSystem;  }
    double getAlgoSoc() const {return lastValues.lastUserSocAlgoOutput;}
    lastSimulationStepsValues getLastValuesOfSimulationStep() const { return lastValues; }
    int    getNumberOfSamples() const { return inputProcessing->getNumberOfSamples(); }

signals:
    void dataSample(float timeS, double vBat, float iSys,
                    float iPlatform, float iBat, float socPct, float socIsysPct);
    void loadError(const QString &msg);
    void loadSuccess();

private:
    // Owned
    SimulatorInput          *inputProcessing;
    VoltageEstimator        *voltageEstimator;
    SocReference            *socReference;
    BatteryCurrentGenerator *batteryCurrGen;
    Algo                    *algo;
    NoiseGenerator          *noiseGenerator;

    // Shared (not owned)
    BatteryModel            *batteryModel;
    PlatformCurrent         *platformCurr;
    TimeTable               *timeTable;

    // Output file
    QFile       *outputFile   = nullptr;
    QTextStream *outputStream = nullptr;
    QString      outputFilePath = "/home/filip/Projects/Master/Output/output.csv"; // default

    // File paths
    QString currentFilePath;
    QString paramsFilePath;
    QString ocvPolyFilePath;
    QString ocvSocFilePath;
    QString noisePath;
    QString flagPath;

    // State
    algoTimeTableDuration_e algoSelectContainer;
    lastSimulationStepsValues lastValues;
    electricCharges_t electricCharges;
    bool  offlineMode      = false;
    float timeS            = 0.0f;
    int   globalSampleIndex = 0;
};

#endif // SIMULATORCONTAINER_H
