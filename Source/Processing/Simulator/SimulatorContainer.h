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


class SimulatorContainer : public QObject
{
    Q_OBJECT
public:
    explicit SimulatorContainer(QObject *parent = nullptr);
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
    void setOfflineMode (bool val)         { offlineMode      = val;}
    void setPlatformCurrent(platform_current_mah_e platformCurrent) {platformCurr->setPlatformCurrent(platformCurrent);}
    float  getTimeS()     const { return timeS; }
    float  getIBat()      const { return lastIBat; }
    double getVBat()      const { return lastVBat; }
    float  getISys()      const { return lastISys; }
    float  getIPlatform() const { return lastIPlatform; }
    float  getSoC()       const { return lastSoC; }
    float  getSoCIsys()       const { return lastSoCIsys; }
    float  getElectricCharge() const {return lastElectricCharge;}
    int getNumberOfSamples() const { return input->getNumberOfSamples(); }

signals:
    void dataSample(float timeS, double vBat, float iSys, float iPlatform, float iBat, float socPct, float socIsysPct);
    void loadError(const QString &msg);
    void loadSuccess();
private:
    SimulatorInput          *input;
    BatteryModel            *batteryModel;
    VoltageEstimator        *voltageEstimator;
    SocReference            *socReference;
    PlatformCurrent         *platformCurr;
    BatteryCurrentGenerator *batteryCurrGen;
    TimeTable               *timeTable;
    Algo                    *algo;
    NoiseGenerator          *noiseGenerator;
    int currentIndex;
    float timeS;
    double electricCharge;

    // Last values after every step()
    float  lastIBat          = 0.0f;
    double lastVBat          = 0.0;
    float  lastISys          = 0.0f;
    float  lastIPlatform     = 0.0f;
    float  lastSoC           = 0.0f;
    float  lastSoCIsys       = 0.0f;
    float lastElectricCharge = 0.0f;

    QFile       *outputFile;
    QTextStream *outputStream;
    QString currentFilePath;
    QString paramsFilePath;
    QString ocvPolyFilePath;
    QString ocvSocFilePath;
    QString noisePath;
    bool offlineMode = false;
};
#endif // SIMULATORCONTAINER_H
