#ifndef SIMULATORWND_H
#define SIMULATORWND_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include <QToolButton>
#include <QDockWidget>
#include <QMenu>

#include "Windows/Plot/plot.h"
#include "Processing/Simulator/SimulatorContainer.h"

typedef enum simulationState_e{
    SIMULATION_UNINIT  = 0,
    SIMULATION_PLAYING = 1,
    SIMULATION_STOPPED = 2,
    SIMULATION_PAUSED  = 3
} simulationState_e;

class SimulatorWnd : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorWnd(QWidget *parent = nullptr);

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onPauseClicked();
    void onSpeedUpClicked();
    void onConfigClicked();
    void onLogInfoClicked();
    void onTimerTick();
    void onLedTimerTimeout();
    void redistributeDocks();
    void startOfflineSimulation();
    double getSimulationEndSoC(){return endSoC;}
    double getSimulationInitSoC(){return initialSoC;}
    void setLed(QLabel *led, const QString &color = "None", simulationState_e simulationState = SIMULATION_UNINIT);

signals:
    void sigPlay();
    void sigStop();
    void sigPause();
    void sigSpeedUp();

private:
    // Buttons
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    QPushButton *loadBtn;
    QPushButton *speedUpBtn;
    QToolButton *logInfoBtn;

    // Config button
    QToolButton *configBtn;
    QAction     *showCurrentBatteryAction;
    QAction     *showCurrentSystemAction;
    QAction     *showCurrentPlatformAction;
    QAction     *showVoltageAction;
    QAction     *showSocAction;

    // Plots
    Plot *currentBatteryPlot;
    Plot *currentPlatformPlot;
    Plot *currentSystemPlot;
    Plot *voltagePlot;
    Plot *socPlot;

    // Dock widgets
    QDockWidget *currentBatteryDock;
    QDockWidget *currentSystemDock;
    QDockWidget *currentPlatformDock;
    QDockWidget *voltageDock;
    QDockWidget *socDock;
    QDockWidget *logDock;


    // Log console
    QTextEdit *logConsole;

    // File paths
    QString currPath;
    QString polynomsPath;
    QString ocvSOCPath;
    QString parametersPath;
    QString noisePath;
    QString selectedPlatform;

    // Leds Labels
    QLabel *ledConfigLabel;
    QLabel *ledPlaySimulationLabel;

    // Status Labels
    QLabel *statusConfigLabel;
    QLabel *statusPlaySimulationLabel;

    // Progress Bar
    QProgressBar *progressBar;

    // Simulator container (single entry point)
    SimulatorContainer *simulatorContainer;

    // Timer
    QTimer *ledTimer;
    QTimer *timer;
    QTimer *offlineTimer;

    simulationState_e currentSimState;

    bool ledBlinkState = false;
    bool isOfflineMode = false;
    int timerPeriodMs;
    int currentSample;
    float averageBatteryCurr = 0.0f;
    float averageSystemCurr = 0.0f;
    double initialSoC = 100.0;
    double endSoC = 0.0;
};

#endif // SIMULATORWND_H
