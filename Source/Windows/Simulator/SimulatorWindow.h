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

typedef struct SimulatorCurrent_t
{
    double system;
    double battery;
    double platform;
} SimulatorCurrent_t;

typedef struct SimulatorVoltage_t
{
    double battery;
} SimulatorVoltage_t;

typedef struct SimulatorSoC_t
{
    double system;
    double battery;
} SimulatorSoC_t;


typedef struct SimulatorSample_t
{
    double time;

    SimulatorCurrent_t current;
    SimulatorVoltage_t voltage;
    SimulatorSoC_t soc;

    electricCharges_t electricCharge;
} SimulatorSample_t;

typedef struct SimulatorAlgoResult_t
{
    QString algoName;   // "K0", "K2", "Adaptive"
    int algoId;

    QVector<SimulatorSample_t> samples;
} SimulatorAlgoResult_t;

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
    QString flagPath;
    QString selectedPlatform;

    // Leds Labels
    QLabel *ledConfigLabel;
    QLabel *ledPlaySimulationLabel;

    // Status Labels
    QLabel *statusConfigLabel;
    QLabel *statusPlaySimulationLabel;

    // Progress Bar
    QProgressBar *progressBar;

    // Constructors
    SimulatorContainer *simulatorContainer;
    BatteryModel *batteryModel;
    PlatformCurrent *platformCurrent;
    TimeTable *timeTable;

    // Timer
    QTimer *ledTimer;
    QTimer *timer;
    QTimer *offlineTimer;

    simulationState_e currentSimState;
    SimulatorAlgoResult_t simulatorAlgoResults;

    bool ledBlinkState = false;
    bool isOfflineMode = false;
    int timerPeriodMs;
    int currentSample;
    double averageBatteryCurr = 0.0;
    double averageSystemCurr = 0.0;
    double initialSoC = 100.0;
    double endSoC = 0.0;
    algoTimeTableDuration_e algoSelected = LP;
};

#endif // SIMULATORWND_H
