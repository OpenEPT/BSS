#ifndef SOCREFERENCE_H
#define SOCREFERENCE_H

#include <QObject>

#define BATTERY_CAPACITY_mAh 457.0f

class SocReference : public QObject
{
    Q_OBJECT

public:
    explicit SocReference(QObject *parent = nullptr);

    void  socReferenceSet(double socReference);
    void  setSocIsys(double socIsysCurrent);
    double calculateRefSoC(double currA, double Ts);
    double calculateRefSoCIsys(double currA, double Ts);
    double getSoC()      const;
    double getSocIsys()  const;
    double getCapacity() const;

private:
    double soC;
    double socIsys;
    double capacity;
};

#endif // SOCREFERENCE_H
