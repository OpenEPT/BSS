#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <QVector>
#include <QObject>

typedef enum algoTimeTableDuration_e{
    LP = 1,
    K0 = 10,
    K2 = 50,
    ADAPTIVE_LP = LP,
    ADAPTIVE_LP_K0 = LP + K0,
    ADAPTIVE_LP_K2 = LP + K2
} algoTimeTableDuration_e;

class TimeTable : public QObject{
    Q_OBJECT
public:
    explicit TimeTable(algoTimeTableDuration_e whitchAlgo, QObject *p = nullptr);

    algoTimeTableDuration_e getWhitchAlgo() const;

public slots:
    void onWitchAlgo(algoTimeTableDuration_e whitchAlgo);

signals:
    void durationAlgo(algoTimeTableDuration_e tAlgo);

private:
    algoTimeTableDuration_e whitchAlgo;
};

#endif // TIMETABLE_H
