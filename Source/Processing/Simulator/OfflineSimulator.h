#ifndef OFFLINESIMULATOR_H
#define OFFLINESIMULATOR_H

#include <QObject>
#include "Processing/Simulator/.h"

class OfflineSimulator : public QObject
{
    Q_OBJECT
public:
    explicit OfflineSimulator(SimulatorContainer* sim, QObject* parent = nullptr)
        : QObject(parent), simulator(sim) {}

public slots:
    void process() {
        while (!simulator->isFinished()) {
            simulator->proccesOffline();
        }
        emit finished();
    }

signals:
    void finished();

private:
    SimulatorContainer* simulator;
};

#endif // OFFLINESIMULATOR_H
