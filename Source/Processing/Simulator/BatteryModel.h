#ifndef BATTERYMODEL_H
#define BATTERYMODEL_H

#include <QObject>
#include <QVector>

// Structs defined here - single source of truth
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

typedef struct ocvSocCurve_t {
    float soc;
    float ocv;
} ocvSocCurve_t;

// Forward declaration
class SimulatorInput;
class SocReference;

class BatteryModel : public QObject
{
    Q_OBJECT

public:
    explicit BatteryModel(QObject *parent = nullptr);

    void                         batteryModelInit(SimulatorInput *input);
    batteryParamaters_t          stepAndGetParams(float currA, float Ts);
    void                         reset(double SocReference);
    double                       getOcv()               const;
    float                        getSoC()               const;
    batteryParamaters_t          getCurrentParams()     const;
    QVector<batteryParamaters_t> getBMParameters()      const;
    QVector<double>              getBMPolynomCoefficients() const;

signals:
    void loadError(const QString &message);

private:
    SocReference                 *socReference;
    SimulatorInput               *simulatorInput;
    QVector<batteryParamaters_t>  lookupParams;
    QVector<double>               lookupOcvPoly;
    batteryParamaters_t           currentParams;

    batteryParamaters_t findParamsByDoD(float dodPct) const;
};

#endif // BATTERYMODEL_H
