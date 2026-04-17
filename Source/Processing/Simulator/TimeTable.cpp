#include "TimeTable.h"


TimeTable::TimeTable(algoTimeTableDuration_e whitchAlgo, QObject *p)
    : QObject(p)
{
    this->whitchAlgo = whitchAlgo;
}


void TimeTable::onWitchAlgo(algoTimeTableDuration_e whitchAlgo){
    this->whitchAlgo = whitchAlgo;
    emit durationAlgo(this->whitchAlgo);
}


algoTimeTableDuration_e  TimeTable::getWhitchAlgo() const {return whitchAlgo;}
