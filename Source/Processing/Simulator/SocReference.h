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
    void  setSocIsys(float socIsysCurrent);
    float calculateRefSoC(float currA, float Ts);
    float calculateRefSoCIsys(float currA, float Ts);
    float getSoC()      const;
    float getSocIsys()  const;
    float getCapacity() const;

private:
    float soC;
    float socIsys;
    float capacity;
};

#endif // SOCREFERENCE_H
