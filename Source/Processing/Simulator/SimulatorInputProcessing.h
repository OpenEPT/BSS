#ifndef SIMULATORINPUTPROCESSING_H
#define SIMULATORINPUTPROCESSING_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QFile>
#include <QTextStream>

typedef struct batteryParamaters_t {
    int   region;
    float dod;
    float rSlow;
    float cSlow;
    float rFast;
    float cFast;
    float rInternal;
    float rBorder;
    float rAverage;
} batteryParamaters_t;

class SimulatorInput : public QObject
{
    Q_OBJECT

public:
    explicit SimulatorInput(QObject *parent = nullptr);

    bool loadCurrentCSV          (const QString &filePath);
    bool loadCoefcientsOcvPolyCSV(const QString &filePath);
    bool loadParametersCSV       (const QString &filePath);

    QVector<float>               getTime()               const;
    QVector<float>               getCurrent()            const;
    QVector<double>              getCoefficientOcvPoly() const;
    QVector<batteryParamaters_t> getBatteryParams()      const;

signals:
    void fileLoadedCurrCSV(int numSamples);
    void fileLoadedCoefOcvPolyCSV();
    void fileLoadedParametersCSV();
    void loadError(const QString &message);

private:
    QVector<float>               systemTime;
    QVector<float>               systemCurrent;
    QVector<double>              systemCoeffcientOcvPoly;
    QVector<batteryParamaters_t> batteryParams;
};

#endif // SIMULATORINPUTPROCESSING_H
