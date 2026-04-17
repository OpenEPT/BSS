#ifndef SIMULATORINPUTPROCESSING_H
#define SIMULATORINPUTPROCESSING_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "BatteryModel.h"
#include "NoiseGenerator.h"


typedef enum current_flag_e{
    N = 0,
    P = 1,
    S = 2
} current_flag_e;

typedef struct {
    float time;
    int   index;
} TickTimer_t;


class SimulatorInput : public QObject
{
    Q_OBJECT

public:
    explicit SimulatorInput(QObject *parent = nullptr);

    bool loadCurrentCSV          (const QString &filePath);
    bool loadCoefcientsOcvPolyCSV(const QString &filePath);
    bool loadParametersCSV       (const QString &filePath);
    bool loadOcvCSV              (const QString &filePath);
    bool loadNoiseCSV            (const QString &filePath);

    const QVector<float>&          getTime()    const;
    const QVector<float>&          getCurrent() const;
    QVector<double>              getCoefficientOcvPoly() const;
    QVector<batteryParamaters_t> getBatteryParams()      const;
    QVector<ocvSocCurve_t>       getOcvSocCurve()        const;
    const QVector<current_flag_e>& getFlag()    const;
    noiseParameters_e            getNoise(int index) const;
    int                          getNumberOfSamples()    const;

signals:
    void fileLoadedCurrCSV(int numSamples);
    void fileLoadedCoefOcvPolyCSV();
    void fileLoadedParametersCSV();
    void fileLoadedOcvCSV();
    void loadError(const QString &message);

private:
    QVector<float>               systemTime;
    QVector<float>               systemCurrent;
    QVector<current_flag_e>      systemCurrFlags;
    QVector<double>              systemCoeffcientOcvPoly;
    QVector<ocvSocCurve_t>       systemOcvSocCurve;
    QVector<batteryParamaters_t> batteryParams;
    QVector<noiseParameters_e>   systemNoise;
    int                          numberOfCurrentSamples;

    friend class NoiseGenerator;
};

#endif // SIMULATORINPUTPROCESSING_H
