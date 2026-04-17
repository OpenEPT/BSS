#include "Algo.h"


Algo::Algo(algoTimeTableDuration_e whitchAlgo, QObject *p)
    : QObject(p)
{
    this->algo = whitchAlgo;
}


void Algo::algoExe(algoTimeTableDuration_e whitchAlgo){
    this->algo = whitchAlgo;
    emit algoTrigger(this->algo);
}
