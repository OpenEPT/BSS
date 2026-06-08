#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <QVector>
#include <QObject>
#include <QMetaType>


typedef enum algoPreProcessingTime_e{
    PREPROCESSING_SYSTEM_TIME = 0,
    PREPROCESSING_USER_TIME   = 0,
} algoPreProcessingTime_e;

typedef enum algoProcessingSystemTime_e{
    LP_SYSTEM_TIME          = 10,    // 10μs  → 0.001ms  (zanemarljivo)
    K0_SYSTEM_TIME          = 1500, // 1500μs → 1.5ms
    K2_SYSTEM_TIME          = 5000, // 5000μs → 5ms
    ADAPTIVE_LP_SYSTEM_TIME    = LP_SYSTEM_TIME,
    ADAPTIVE_LP_K0_SYSTEM_TIME = K0_SYSTEM_TIME,
    ADAPTIVE_LP_K2_SYSTEM_TIME = K2_SYSTEM_TIME,
} algoProcessingSystemTime_e;


typedef enum algoProcessingUserTime_e{
    PROCESSING_USER_TIME = 0,
} algoProcessingUserTime_e;

typedef enum algoPostProcessingTime_e{
    POSTPROCESSING_SYSTEM_TIME = 0,
    POSTPROCESSING_USER_TIME   = 0,
} algoPostProcessingTime_e;


typedef enum algoTimeTableDuration_e{
    LP          = LP_SYSTEM_TIME,           // 1μs
    K0          = K0_SYSTEM_TIME,           // 1500μs
    K2          = K2_SYSTEM_TIME,           // 5000μs
    ADAPTIVE_LP    = LP_SYSTEM_TIME,
    ADAPTIVE_LP_K0 = K0_SYSTEM_TIME,
    ADAPTIVE_LP_K2 = K2_SYSTEM_TIME
} algoTimeTableDuration_e;
Q_DECLARE_METATYPE(algoTimeTableDuration_e)

class TimeTable : public QObject{
    Q_OBJECT
public:
    explicit TimeTable(algoTimeTableDuration_e whitchAlgo, QObject *p = nullptr);

    int getPreProcessingSystemTime()  const;
    int getPreProcessingUserTime()    const;
    int getProcessingUserTime()       const;
    int getPostProcessingSystemTime() const;
    int getPostProcessingUserTime()   const;

    int getProcessingSystemTime(algoTimeTableDuration_e algo) const {
        switch(algo) {
        case LP:              return LP_SYSTEM_TIME;
        case K0:              return K0_SYSTEM_TIME;
        case K2:              return K2_SYSTEM_TIME;
        default:              return LP_SYSTEM_TIME;
        }
    }
    int getTotalDuration(algoTimeTableDuration_e algo) const;

    algoTimeTableDuration_e getWhitchAlgo() const;

public slots:
    void onWitchAlgo(algoTimeTableDuration_e algo);

signals:
    void durationAlgo(int tAlgo);

private:
    algoTimeTableDuration_e whitchAlgo;
    algoTimeTableDuration_e currentAlgo;
};

#endif // TIMETABLE_H
