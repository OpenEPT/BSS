#ifndef BATTERYMODEL_H
#define BATTERYMODEL_H

#include <QObject>
#include <QVector>
#include "SimulatorInputProcessing.h"
#include "SocReference.h"

class BatteryModel : public QObject
{
    Q_OBJECT

public:
    explicit BatteryModel(QObject *parent = nullptr);

    void                         batteryModelInit(SimulatorInput *input);
    batteryParamaters_t          stepAndGetParams(float currA, float Ts);
    double                       getOcv()               const;
    float                        getSoC()               const;
    batteryParamaters_t          getCurrentParams()     const;
    QVector<batteryParamaters_t> getBMParameters()      const;
    QVector<double>              getBMPolynomCoefficients() const;

signals:
    void loadError(const QString &message);

private:
    SocReference                 *socReference;
    QVector<batteryParamaters_t>  lookupParams;
    QVector<double>               lookupOcvPoly;
    batteryParamaters_t           currentParams;

    batteryParamaters_t findParamsByDoD(float dodPct) const;
};

#endif // BATTERYMODEL_H
