#include "TimeTable.h"

TimeTable::TimeTable(algoTimeTableDuration_e whitchAlgo, QObject *p)
    : QObject(p)
    , whitchAlgo(whitchAlgo)
    , currentAlgo(whitchAlgo)
{
}

void TimeTable::onWitchAlgo(algoTimeTableDuration_e algo)
{
    whitchAlgo  = algo;
    currentAlgo = algo;
    int time = getTotalDuration(algo);
    emit durationAlgo(time);
}


int TimeTable::getPreProcessingSystemTime()  const {
    return PREPROCESSING_SYSTEM_TIME;
}

int TimeTable::getPreProcessingUserTime()    const {
    return PREPROCESSING_USER_TIME;
}

int TimeTable::getProcessingUserTime()       const {
    return PROCESSING_USER_TIME;
}

int TimeTable::getPostProcessingSystemTime() const {
    return POSTPROCESSING_SYSTEM_TIME;
}

int TimeTable::getPostProcessingUserTime()   const {
    return POSTPROCESSING_USER_TIME;
}


int TimeTable::getTotalDuration(algoTimeTableDuration_e algo) const {
    return getPreProcessingSystemTime()
           + getPreProcessingUserTime()
           + getProcessingSystemTime(algo)
           + getProcessingUserTime()
           + getPostProcessingSystemTime()
           + getPostProcessingUserTime();
}

algoTimeTableDuration_e TimeTable::getWhitchAlgo() const
{
    return currentAlgo;
}
