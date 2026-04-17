#ifndef ALGO_H
#define ALGO_H

#include <QVector>
#include <QObject>
#include "TimeTable.h"


class Algo : public QObject{
    Q_OBJECT
public:
    explicit Algo(algoTimeTableDuration_e whitchAlgo, QObject *p = nullptr);
    void algoExe(algoTimeTableDuration_e whitchAlgo);

signals:
    void algoTrigger(algoTimeTableDuration_e whitchAlgo);

private:
    algoTimeTableDuration_e algo;
};

#endif // ALGO_H
