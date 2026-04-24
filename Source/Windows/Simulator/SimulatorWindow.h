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
#include <QTabBar>
#include <QMenu>
#include <QVector>

#include "Windows/Plot/plot.h"
#include "Processing/Simulator/SimulatorContainer.h"

typedef enum simulationState_e{
    SIMULATION_UNINIT  = 0,
    SIMULATION_PLAYING = 1,
    SIMULATION_STOPPED = 2,
    SIMULATION_PAUSED  = 3
} simulationState_e;

typedef struct SimulatorCurrent_t {
    QVector<double> system;
    QVector<double> battery;
    QVector<double> platform;
} SimulatorCurrent_t;

typedef struct SimulatorVoltage_t {
    QVector<double> battery;
} SimulatorVoltage_t;

typedef struct SimulatorSoC_t {
    QVector<double> system;
    QVector<double> battery;
    QVector<double> algoSoc;
} SimulatorSoC_t;


typedef struct SimulatorSample_t {
    QVector<double>    time;
    SimulatorCurrent_t current;
    SimulatorVoltage_t voltage;
    SimulatorSoC_t     soc;
    electricCharges_t  electricCharge;
} SimulatorSample_t;

/*******************************************************************************
 * AlgoTab - Algorithm Subsystem Block
 ******************************************************************************/
typedef struct algoTab_t {
    algoTimeTableDuration_e algo;
    QString                 algoName;

    // Unique per tab:
    SimulatorContainer *container = nullptr;
    BatteryModel *batteryModel = nullptr;

    // Collected data vectors
    SimulatorSample_t simulatorSample;
    double avgBatteryCurr = 0.0;
    double avgSystemCurr  = 0.0;
} algoTab_t;

/*******************************************************************************
 * SimulatorWnd
 ******************************************************************************/
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
    void onTabChanged(int index);
    void redistributeDocks();
    void startOfflineSimulation();
    void replotCompareTab();
    double getSimulationEndSoC()  { return endSoC;     }
    double getSimulationInitSoC() { return initialSoC; }
    void setLed(QLabel *led,
                const QString &color = "None",
                simulationState_e simulationState = SIMULATION_UNINIT);

    QTableWidget* createResultsTable(double avgBat,
                                    double avgSys,
                                    lastSimulationStepsValues finalResults,
                                    float timeElapsed,
                                    int totalSamples,
                                    const QString &algoName);

signals:
    void sigPlay();
    void sigStop();
    void sigPause();
    void sigSpeedUp();

private:
    void clearAlgoTabs();
    void replotActiveTab();

    // ── Toolbar ───────────────────────────────────────────────────────────────
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    QPushButton *loadBtn;
    QPushButton *speedUpBtn;
    QToolButton *logInfoBtn;
    QToolButton *configBtn;

    // ── Tab bar (algo switcher) ────────────────────────────────────────────────
    QTabBar *tabBar;

    // ── Shared plots (dock content, reused for all algos) ─────────────────────
    Plot *currentBatteryPlot;
    Plot *currentPlatformPlot;
    Plot *currentSystemPlot;
    Plot *voltagePlot;
    Plot *socPlot;

    // ── Dock widgets ──────────────────────────────────────────────────────────
    QDockWidget *currentBatteryDock;
    QDockWidget *currentSystemDock;
    QDockWidget *currentPlatformDock;
    QDockWidget *voltageDock;
    QDockWidget *socDock;
    QDockWidget *logDock;
    QDockWidget *tabDock;

    // ── Log console ───────────────────────────────────────────────────────────
    QTextEdit *logConsole;

    // ── File paths ────────────────────────────────────────────────────────────
    QString currPath;
    QString polynomsPath;
    QString ocvSOCPath;
    QString parametersPath;
    QString noisePath;
    QString flagPath;
    QString selectedPlatform;

    // ── Status bar ────────────────────────────────────────────────────────────
    QLabel       *ledConfigLabel;
    QLabel       *ledPlaySimulationLabel;
    QLabel       *statusConfigLabel;
    QLabel       *statusPlaySimulationLabel;
    QProgressBar *progressBar;

    // ── Multi-algo data ───────────────────────────────────────────────────────
    QList<algoTab_t> algoTabs;

    // ── Online mode (single container, kept for compatibility) ────────────────
    BatteryModel       *batteryModel;
    PlatformCurrent    *platformCurrent;
    TimeTable          *timeTable;
    SimulatorContainer *simulatorContainer;

    // ── Timers ────────────────────────────────────────────────────────────────
    QTimer *ledTimer;
    QTimer *timer;
    QTimer *offlineTimer;

    // ── State ─────────────────────────────────────────────────────────────────
    simulationState_e currentSimState;
    bool   ledBlinkState   = false;
    bool   isOfflineMode   = false;
    int    timerPeriodMs;
    int    currentSample;
    double averageBatteryCurr = 0.0;
    double averageSystemCurr  = 0.0;
    double initialSoC = 1.0;
    double endSoC     = 0.0;
    algoTimeTableDuration_e algoSelected = LP;
};

#endif // SIMULATORWND_H
