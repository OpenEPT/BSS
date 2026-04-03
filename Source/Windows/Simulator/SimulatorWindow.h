#ifndef SIMULATORWND_H
#define SIMULATORWND_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QFileDialog>
#include <QDebug>

#include "Windows/Plot/plot.h"
#include "Processing/Simulator/SimulatorInputProcessing.h"
#include "Processing/Simulator/BatteryModel.h"
#include "Processing/Simulator/VoltageEstimator.h"

class SimulatorWnd : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorWnd(QWidget *parent = nullptr);

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onPauseClicked();
    void onLoadClicked();
    void onTimerTick();

signals:
    void sigPlay();
    void sigStop();
    void sigPause();

private:
    // Buttons
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    QPushButton *loadBtn;

    // Plots
    Plot *currentPlot;
    Plot *voltagePlot;

    // Simulator modules
    SimulatorInput   *simulatorInput;
    BatteryModel     *batteryModel;
    VoltageEstimator *voltageEstimator;

    // Simulation state
    QTimer *timer;
    int     currentIndex;
};

#endif // SIMULATORWND_H
