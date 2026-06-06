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


typedef struct postSimulationAnalysys_t {
    double rmse            = 0.0;
    double mae             = 0.0;
    double bias            = 0.0;
    double stdDev          = 0.0;
    double sumSquaredError = 0.0;
    double sumAbsError     = 0.0;
    double sumError        = 0.0;
    double maxErr          = 0.0;
} postSimulationAnalysys_t;

/*******************************************************************************
 * AlgoTab - Algorithm Subsystem Block
 ******************************************************************************/
typedef struct algoTab_t {
    uint64_t                 sampleNum = 0;
    algoTimeTableDuration_e  algo;
    QString                  algoName;
    QString                  platformName;
    platform_current_mah_e   platformEnum    = PLATFORM_CURRENT_ESP;
    BatteryModel            *batteryModel    = nullptr;
    TimeTable               *timeTable       = nullptr;
    PlatformCurrent         *platformCurrent = nullptr;
    SimulatorContainer      *container       = nullptr;
    SimulatorSample_t        simulatorSample;
    postSimulationAnalysys_t postSimulationAnalysys;
    double avgBatteryCurr = 0.0;
    double avgSystemCurr  = 0.0;
} algoTab_t;

struct algoTableRow_t {
    QString algoText;
    QString platformText;
    QString modeText;
    QString periodText;
    QString batteryText;
    QVariant algoData;
    QVariant platformData;
    QVariant modeData;
    int      periodValue;
    QVariant batteryData;
};

/*******************************************************************************
 * SimulatorWnd
 ******************************************************************************/
class SimulatorWnd : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorWnd(QWidget *parent = nullptr);

private slots:

    // On buttons clicked that directly manipulate simulation
    void onPlayClicked();
    void onStopClicked();
    void onPauseClicked();
    void onSpeedUpClicked();

    // This are 3 buttons that opens a config windows
    void onConfigClicked();
    void onLogInfoClicked();
    void onSimuSettingsClicked();

    // Window for output results from config simulation settings
    void onOutputSimulationChecked();

    // Other functionality
    void onTimerTick();
    void onLedTimerTimeout();
    void redistributeDocks();
    void startOfflineSimulation();


    double getSimulationEndSoC()  { return endSoC;     }
    double getSimulationInitSoC() { return initialSoC; }

    void setLed(QLabel *led,
                const QString &color = "None",
                simulationState_e simulationState = SIMULATION_UNINIT);

    QTableWidget* createResultsTable(algoTab_t tab,
                                    double avgBat,
                                    double avgSys,
                                    lastSimulationStepsValues finalResults,
                                    float timeElapsed,
                                    int totalSamples,
                                    const QString &algoName,
                                    const QString &platformName);

signals:
    void sigPlay();
    void sigStop();
    void sigPause();
    void sigSpeedUp();
    void sigOpenOuputWindow();

private:
    void replotActiveTab();
    void clearAlgoTabs();

    // This is visibility list for all algo in simulation settings that choose if algo needs to be plot
    QList<bool> visibleAlgosInCompare;

    QList<bool> visibleOutputInSimulationSettings;

    // Saving tagle from config settings to know which algo's are previous choosed
    QList<algoTableRow_t> savedAlgoTableRows;

    QDialog *outputDialog = nullptr;

    // ── Toolbar ───────────────────────────────────────────────────────────────
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    QPushButton *loadBtn;
    QPushButton *speedUpBtn;
    QToolButton *logInfoBtn;
    QToolButton *configBtn;
    QToolButton *simulationSettingsBtn;

    // ── Shared plots (dock content, reused for all algos) ─────────────────────
    Plot *currentBatteryPlot;
    Plot *currentPlatformPlot;
    Plot *currentSystemPlot;
    Plot *voltagePlot;
    Plot *socPlot;
    Plot *socErrorDiffPlot = nullptr;

    // ── Dock widgets ──────────────────────────────────────────────────────────
    QDockWidget *currentBatteryDock;
    QDockWidget *currentSystemDock;
    QDockWidget *currentPlatformDock;
    QDockWidget *voltageDock;
    QDockWidget *socDock;
    QDockWidget *socAlgoDiffDock;
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


// ── Protypes of functions ─────────────────────────────────────────────────────────────────

/* This function is handler that is being called when we call either qDebug, qWarning, qCritical...
    We installed this handler with function qInstallMessageHandler in constructor of our class SimWind */
void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg);


/* This function only formats in QString totalSeconds in format :
 * hh:mm:ss
   We use this in tables for logging time of each algo etc... */
static QString formatSeconds(float totalSeconds);


/* Extract based on time table algo name and returns that string */
static QString algoTabName(algoTimeTableDuration_e algo);


// Gives hardcoded file paths for each algo
static QString algoFlagsPath(algoTimeTableDuration_e algo);

// Hardcoded output path based on time table of algo
static QString algoOutputPath(algoTimeTableDuration_e algo);

#endif // SIMULATORWND_H
