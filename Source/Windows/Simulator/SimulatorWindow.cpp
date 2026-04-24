/**
 * @file    SimulatorWindow.cpp
 * @brief   Simulator window - dock layout preserved, multi-algo tab bar
 * @version 2.0.0
 * @date    2026-04-21
 * @author  Filip Radojevic
 */

#include "SimulatorWindow.h"
#include <QtConcurrent>

/*******************************************************************************
 * Global log console
 ******************************************************************************/
QTextEdit *g_logConsole = nullptr;

void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    if (!g_logConsole) return;
    QString prefix;
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG] "; break;
    case QtWarningMsg:  prefix = "[WARN]  "; break;
    case QtCriticalMsg: prefix = "[ERROR] "; break;
    default: break;
    }
    QMetaObject::invokeMethod(g_logConsole, "append",
                              Qt::QueuedConnection,
                              Q_ARG(QString, prefix + msg));
}

static QString formatSeconds(float totalSeconds)
{
    int h = (int)(totalSeconds / 3600);
    int m = (int)((totalSeconds - h * 3600) / 60);
    int s = (int)(totalSeconds - h * 3600 - m * 60);
    return QString("%1h %2m %3s")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

QTableWidget* SimulatorWnd::createResultsTable(
    double avgBat, double avgSys,
    lastSimulationStepsValues finalResults,
    float timeElapsed, int totalSamples,
    const QString &algoName)
{
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(2);
    table->horizontalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);

    // Lambda helpers
    auto addSeparator = [&](const QString &title) {
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem(" " + title);
        QFont f = item->font(); f.setBold(true); item->setFont(f);
        item->setBackground(QColor("#efefef"));
        table->setItem(row, 0, item);
        QTableWidgetItem *empty = new QTableWidgetItem("");
        empty->setBackground(QColor("#efefef"));
        table->setItem(row, 1, empty);
        table->setRowHeight(row, 20);
    };

    auto addRow = [&](const QString &label, const QString &value, bool bold = false) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(label));
        QTableWidgetItem *valItem = new QTableWidgetItem(value);
        valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (bold) {
            QFont f = valItem->font(); f.setBold(true); valItem->setFont(f);
        }
        table->setItem(row, 1, valItem);
        table->setRowHeight(row, 24);
    };

    // Calculations
    platform_current_mah_e pc = PLATFORM_CURRENT_ESP;
    if      (selectedPlatform == "NXP")   pc = PLATFORM_CURRENT_NXP;
    else if (selectedPlatform == "STM32") pc = PLATFORM_CURRENT_STM;
    else if (selectedPlatform == "NRF")   pc = PLATFORM_CURRENT_NRF;
    float current = static_cast<float>(pc);

    QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(timeElapsed * 1000));
    QString formattedTime = time.toString("hh'h' mm'm' ss's'");

    double deltaSOC    = (initialSoC * 100.0) - finalResults.lastSoC;
    double capacityMah = 0.0;
    if (deltaSOC > 0.0)
        capacityMah = (finalResults.lastElectricChargeBattery / deltaSOC) * 100.0;

    double avgSysMa = avgSys * 1000.0;
    double avgBatMa = avgBat * 1000.0;

    float maxOpTime = (avgSysMa > 0.0) ? (float)((capacityMah / avgSysMa) * 3600.0) : 0.0f;
    float sysOpTime = (avgBatMa > 0.0) ? (float)((capacityMah / avgBatMa) * 3600.0) : 0.0f;
    float algoCost  = maxOpTime - sysOpTime;

    // Fill table
    addSeparator("General");
    addRow("Platform",      selectedPlatform);
    if (!algoName.isEmpty()) addRow("Algorithm", algoName);
    addRow("Total Samples", QString::number(totalSamples));

    addSeparator("State of Charge");
    addRow("Initial SoC Battery", QString("%1 %").arg(initialSoC * 100.0, 0, 'f', 1));
    addRow("Final SoC Battery",   QString("%1 %").arg(finalResults.lastSoC, 0, 'f', 2));
    addRow("Initial SoC System",  QString("%1 %").arg(initialSoC * 100.0, 0, 'f', 1));
    addRow("Final SoC System",    QString("%1 %").arg(finalResults.lastSoCIsys, 0, 'f', 2));

    addSeparator("Voltage");
    addRow("Final Terminal Voltage", QString("%1 V").arg(finalResults.lastVBat, 0, 'f', 4));

    addSeparator("Current");
    addRow("Avg Battery Current", QString("%1 mA").arg(avgBatMa, 0, 'f', 4));
    addRow("Avg System Current",  QString("%1 mA").arg(avgSysMa, 0, 'f', 4));
    addRow("Platform Current",    QString("%1 mA").arg(current,  0, 'f', 4));

    addSeparator("Electric Charge");
    addRow("Battery Capacity", QString("%1 mAh").arg(capacityMah, 0, 'f', 2));
    addRow("Charge Battery",   QString("%1 mAh").arg(finalResults.lastElectricChargeBattery, 0, 'f', 4));
    addRow("Charge System",    QString("%1 mAh").arg(finalResults.lastElectricChargeSystem,  0, 'f', 4));
    addRow("Charge Algo",      QString("%1 mAh").arg(finalResults.lastElectricChargeAlgo,    0, 'f', 4));

    addSeparator("Time");
    addRow("Simulation Time",    formattedTime);
    addRow("Maximal Oper. Time", formatSeconds(maxOpTime));
    addRow("System Oper. Time",  formatSeconds(sysOpTime));
    addRow("Algo Time Cost",     formatSeconds(algoCost), true);

    return table;
}

/*******************************************************************************
 * Static helpers
 ******************************************************************************/
static QString algoTabName(algoTimeTableDuration_e algo)
{
    switch (algo) {
    case K0:             return "K0";
    case K2:             return "K2";
    case LP:             return "LP";
    case ADAPTIVE_LP_K2: return "Adaptive";
    default:             return "Unknown";
    }
}

static QString algoFlagsPath(algoTimeTableDuration_e algo)
{
    switch (algo) {
    case K0:  return "/home/filip/Projects/Master/Params/flags_k0.csv";
    case K2:  return "/home/filip/Projects/Master/Params/flags_k2.csv";
    case LP:  return "/home/filip/Projects/Master/Params/flags_lp.csv";
    default:  return "/home/filip/Projects/Master/Params/flags_adaptive.csv";
    }
}

static QString algoOutputPath(algoTimeTableDuration_e algo)
{
    return QString("/home/filip/Projects/Master/Output/output_%1.csv")
        .arg(algoTabName(algo));
}


/*******************************************************************************
 * Constructor
 ******************************************************************************/
SimulatorWnd::SimulatorWnd(QWidget *parent)
    : QMainWindow(parent)
    , currentSimState(SIMULATION_UNINIT)
    , timerPeriodMs(10)
    , currentSample(0)
{

    /* Constructors ************************************************************/
    batteryModel       = new BatteryModel(this);
    platformCurrent    = new PlatformCurrent(PLATFORM_CURRENT_ESP, this);
    timeTable          = new TimeTable(K2, this);
    simulatorContainer = new SimulatorContainer(batteryModel, platformCurrent, timeTable, this);


    /* Timers ******************************************************************/
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SimulatorWnd::onTimerTick);

    ledTimer = new QTimer(this);
    ledTimer->setInterval(500);
    ledTimer->start();
    connect(ledTimer, &QTimer::timeout, this, &SimulatorWnd::onLedTimerTimeout);


    /* ToolBar *****************************************************************/
    QToolBar *tabToolBar = new QToolBar(this);
    tabToolBar->setMovable(false);
    tabToolBar->setFloatable(false);
    tabToolBar->addWidget(tabBar);
    addToolBar(Qt::TopToolBarArea, tabToolBar);

    playBtn    = new QPushButton("Play");
    stopBtn    = new QPushButton("Stop");
    pauseBtn   = new QPushButton("Pause");
    speedUpBtn = new QPushButton("1x");
    configBtn  = new QToolButton(this);
    configBtn->setIcon(QIcon("/home/filip/Projects/Master/GUI/Documentation/img/conf.jpeg"));
    configBtn->setFixedSize(30, 30);
    logInfoBtn = new QToolButton(this);
    logInfoBtn->setIcon(QIcon("/home/filip/Projects/Master/GUI/Documentation/img/info_button.png"));
    logInfoBtn->setFixedSize(30, 30);


    playBtn->setEnabled(false);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true);
    logInfoBtn->setEnabled(false);

    tabToolBar->addWidget(playBtn);
    tabToolBar->addWidget(stopBtn);
    tabToolBar->addWidget(pauseBtn);
    tabToolBar->addWidget(speedUpBtn);
    tabToolBar->addWidget(configBtn);
    tabToolBar->addWidget(logInfoBtn);

    /* Tab bar *****************************************************************/
    tabBar = new QTabBar(this);
    tabBar->setExpanding(false);
    tabBar->setVisible(false);
    connect(tabBar, &QTabBar::currentChanged, this, &SimulatorWnd::onTabChanged);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(tabBar);
    centralLayout->addStretch();
    setCentralWidget(centralWidget);

    /* Plots *******************************************************************/
    currentBatteryPlot  = new Plot(400, 100, false, this);
    currentSystemPlot   = new Plot(400, 100, false, this);
    currentPlatformPlot = new Plot(400, 100, false, this);
    voltagePlot         = new Plot(400, 100, false, this);
    socPlot             = new Plot(400, 100, false, this, true);

    currentBatteryPlot->setTitle("Battery Current");
    currentBatteryPlot->setYLabel("[A]");
    currentBatteryPlot->setXLabel("[s]");

    currentSystemPlot->setTitle("System Current");
    currentSystemPlot->setYLabel("[A]");
    currentSystemPlot->setXLabel("[s]");

    currentPlatformPlot->setTitle("Platform Current");
    currentPlatformPlot->setYLabel("[A]");
    currentPlatformPlot->setXLabel("[s]");

    voltagePlot->setTitle("Battery Voltage");
    voltagePlot->setYLabel("[V]");
    voltagePlot->setXLabel("[s]");

    socPlot->setTitle("Battery SoC");
    socPlot->setYLabel("[%]");
    socPlot->setXLabel("[s]");
    socPlot->setGraphName(0, "SoC Battery");
    socPlot->addLineGraph(QColor(255, 100, 0), "SoC Isys");

    currentBatteryPlot->setMinimumHeight(100);
    currentSystemPlot->setMinimumHeight(100);
    currentPlatformPlot->setMinimumHeight(100);
    voltagePlot->setMinimumHeight(100);
    socPlot->setMinimumHeight(100);

    /* Log console *****************************************************************/
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setMaximumHeight(150);
    logConsole->setStyleSheet(
        "background-color: #1e1e1e; color: #d4d4d4; font-family: monospace;");
    g_logConsole = logConsole;
    qInstallMessageHandler(logHandler);

    /* Dock widgets ****************************************************************/
    currentBatteryDock = new QDockWidget("Battery Current", this);
    currentBatteryDock->setWidget(currentBatteryPlot);
    currentBatteryDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    currentBatteryDock->setFeatures(QDockWidget::DockWidgetMovable |
                                    QDockWidget::DockWidgetFloatable |
                                    QDockWidget::DockWidgetClosable);

    currentSystemDock = new QDockWidget("System Current", this);
    currentSystemDock->setWidget(currentSystemPlot);
    currentSystemDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    currentSystemDock->setFeatures(QDockWidget::DockWidgetMovable |
                                   QDockWidget::DockWidgetFloatable |
                                   QDockWidget::DockWidgetClosable);

    currentPlatformDock = new QDockWidget("Platform Current", this);
    currentPlatformDock->setWidget(currentPlatformPlot);
    currentPlatformDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    currentPlatformDock->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable |
                                     QDockWidget::DockWidgetClosable);

    voltageDock = new QDockWidget("Battery Voltage", this);
    voltageDock->setWidget(voltagePlot);
    voltageDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    voltageDock->setFeatures(QDockWidget::DockWidgetMovable |
                             QDockWidget::DockWidgetFloatable |
                             QDockWidget::DockWidgetClosable);

    socDock = new QDockWidget("SOC", this);
    socDock->setWidget(socPlot);
    socDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    socDock->setFeatures(QDockWidget::DockWidgetMovable |
                         QDockWidget::DockWidgetFloatable |
                         QDockWidget::DockWidgetClosable);

    logDock = new QDockWidget("Log Console", this);
    logDock->setWidget(logConsole);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    logDock->setFeatures(QDockWidget::DockWidgetMovable |
                         QDockWidget::DockWidgetFloatable |
                         QDockWidget::DockWidgetClosable);

    addDockWidget(Qt::BottomDockWidgetArea, logDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentBatteryDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentSystemDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentPlatformDock);
    addDockWidget(Qt::RightDockWidgetArea,  voltageDock);
    addDockWidget(Qt::RightDockWidgetArea,  socDock);

    splitDockWidget(currentBatteryDock,  currentSystemDock,   Qt::Vertical);
    splitDockWidget(currentSystemDock,   currentPlatformDock, Qt::Vertical);
    splitDockWidget(voltageDock,         socDock,             Qt::Vertical);

    resizeDocks({currentBatteryDock, currentSystemDock, currentPlatformDock,
                 voltageDock, socDock},
                {200, 200, 200, 200, 200}, Qt::Vertical);
    resizeDocks({currentBatteryDock, voltageDock}, {700, 700}, Qt::Horizontal);

    resize(1600, 1000);

    /* Status bar *****************************************************************/
    statusBar()->setStyleSheet("border-top: 1px solid palette(mid); padding: 2px;");

    ledConfigLabel         = new QLabel(this);
    ledPlaySimulationLabel = new QLabel(this);
    statusConfigLabel      = new QLabel(this);
    statusPlaySimulationLabel = new QLabel(this);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedWidth(200);
    progressBar->setFixedHeight(25);
    progressBar->setTextVisible(true);

    ledConfigLabel->setFixedSize(16, 16);
    ledPlaySimulationLabel->setFixedSize(16, 16);
    setLed(ledConfigLabel, "grey");
    setLed(ledPlaySimulationLabel, "grey");

    statusConfigLabel->setText("Not Configured");
    statusPlaySimulationLabel->setText("Not Initialized");

    statusBar()->addWidget(statusPlaySimulationLabel);
    statusBar()->addWidget(ledPlaySimulationLabel);
    statusBar()->addWidget(statusConfigLabel);
    statusBar()->addWidget(ledConfigLabel);
    statusBar()->addPermanentWidget(progressBar);

    /* Button connections *****************************************************************/
    connect(playBtn,    &QPushButton::clicked,  this, &SimulatorWnd::onPlayClicked);
    connect(stopBtn,    &QPushButton::clicked,  this, &SimulatorWnd::onStopClicked);
    connect(pauseBtn,   &QPushButton::clicked,  this, &SimulatorWnd::onPauseClicked);
    connect(speedUpBtn, &QPushButton::clicked,  this, &SimulatorWnd::onSpeedUpClicked);
    connect(configBtn,  &QToolButton::clicked,  this, &SimulatorWnd::onConfigClicked);
    connect(logInfoBtn, &QToolButton::clicked,  this, &SimulatorWnd::onLogInfoClicked);

    /* Online dataSample connection *******************************************************/
    connect(simulatorContainer, &SimulatorContainer::dataSample, this,
            [=](float timeS, double vBat, float iSys, float iPlatform,
                float iBat, float socPct, float socIsysPct)
            {
                if (socPct <= endSoC * 100.0f) {
                    timer->stop();
                    playBtn->setEnabled(false);
                    stopBtn->setEnabled(true);
                    pauseBtn->setEnabled(false);
                    speedUpBtn->setEnabled(false);
                    logInfoBtn->setEnabled(true);
                    qDebug() << "Simulation finished, reached end SoC";
                    return;
                }
                currentBatteryPlot->appendData ({(double)iBat},      {(double)timeS});
                currentSystemPlot->appendData  ({(double)iSys},      {(double)timeS});
                currentPlatformPlot->appendData({(double)iPlatform}, {(double)timeS});
                voltagePlot->appendData        ({vBat},              {(double)timeS});
                socPlot->appendData            ({(double)socPct},    {(double)timeS});
                socPlot->appendData2           ({(double)socIsysPct},{(double)timeS});
                currentSample++;
                int pct = (int)((float)currentSample /
                                 simulatorContainer->getNumberOfSamples() * 100.0f);
                progressBar->setValue(pct);
            }, Qt::QueuedConnection);

    connect(simulatorContainer, &SimulatorContainer::loadSuccess, this,
            [=]{ playBtn->setEnabled(true); });
    connect(simulatorContainer, &SimulatorContainer::loadError, this,
            [=](const QString &msg){ qDebug() << "Load error:" << msg; });
}

/*******************************************************************************
 * clearAlgoTabs
 ******************************************************************************/
void SimulatorWnd::clearAlgoTabs()
{
    // Remove all tabs from bar
    while (tabBar->count() > 0)
        tabBar->removeTab(0);

    // Delete containers (owned objects)
    for (auto &tab : algoTabs) {
        delete tab.container;
    }
    algoTabs.clear();
    tabBar->setVisible(false);
}

/*******************************************************************************
 * replotActiveTab - pushes data of active tab into the shared plots
 ******************************************************************************/
void SimulatorWnd::replotActiveTab()
{
    int idx = tabBar->currentIndex();
    if (idx < 0 || idx >= algoTabs.size()) return;

    const algoTab_t &tab = algoTabs[idx];

    currentBatteryPlot->setData (tab.simulatorSample.current.battery,  tab.simulatorSample.time);
    currentSystemPlot->setData  (tab.simulatorSample.current.system,   tab.simulatorSample.time);
    currentPlatformPlot->setData(tab.simulatorSample.current.platform, tab.simulatorSample.time);
    voltagePlot->setData        (tab.simulatorSample.voltage.battery,  tab.simulatorSample.time);
    socPlot->setData            (tab.simulatorSample.soc.battery,      tab.simulatorSample.time);
    socPlot->setSecondGraphData (tab.simulatorSample.soc.system,       tab.simulatorSample.time);

    currentBatteryPlot->replotAll();
    currentSystemPlot->replotAll();
    currentPlatformPlot->replotAll();
    voltagePlot->replotAll();
    socPlot->replotAll();
}

/*******************************************************************************
 * onTabChanged
 ******************************************************************************/
void SimulatorWnd::onTabChanged(int index)
{
    Q_UNUSED(index)
    replotActiveTab();
}

/*******************************************************************************
 * onTimerTick - online mode
 ******************************************************************************/
void SimulatorWnd::onTimerTick()
{
    simulatorContainer->step();
}

/*******************************************************************************
 * onConfigClicked
 ******************************************************************************/
void SimulatorWnd::onConfigClicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Config");
    dialog->resize(1100, 600);

    QVBoxLayout *mainLayout   = new QVBoxLayout(dialog);
    QHBoxLayout *groupsLayout = new QHBoxLayout();

    // ── Group boxes ───────────────────────────────────────────────────────────
    QGroupBox *graphicsGroup         = new QGroupBox("Choose Graphics");
    QGroupBox *inputFilesGroup       = new QGroupBox("Choose Files");
    inputFilesGroup->setFixedWidth(600);
    QGroupBox *algoGroup             = new QGroupBox("Choose Algo");
    QGroupBox *analysisGroupBox      = new QGroupBox("Choose Analysis");
    QGroupBox *choosePlatformGroupBox= new QGroupBox("Choose Platform");

    // ── Layouts ───────────────────────────────────────────────────────────────
    QVBoxLayout *leftGroup          = new QVBoxLayout();
    QVBoxLayout *rightFilesGroup    = new QVBoxLayout();
    QVBoxLayout *rightAlgoGroup     = new QVBoxLayout();
    QVBoxLayout *analysisLayout     = new QVBoxLayout();
    QVBoxLayout *choosePlatformLayout = new QVBoxLayout();

    // ── Analysis radio ────────────────────────────────────────────────────────
    QRadioButton *onlineAnalysys  = new QRadioButton("Online Analysis");
    QRadioButton *offlineAnalysys = new QRadioButton("Offline Analysis");
    QButtonGroup *analysisBtnGroup = new QButtonGroup(dialog);
    analysisBtnGroup->addButton(onlineAnalysys);
    analysisBtnGroup->addButton(offlineAnalysys);
    onlineAnalysys->setChecked(true);
    analysisLayout->addWidget(onlineAnalysys);
    analysisLayout->addWidget(offlineAnalysys);
    analysisGroupBox->setLayout(analysisLayout);

    // ── Algo checkboxes ───────────────────────────────────────────────────────
    QCheckBox *k0       = new QCheckBox("K0");
    QCheckBox *k2       = new QCheckBox("K2");
    QCheckBox *lp       = new QCheckBox("LP");
    QCheckBox *adaptive = new QCheckBox("Adaptive");
    k0->setChecked(true);

    rightAlgoGroup->addWidget(k0);
    rightAlgoGroup->addWidget(k2);
    rightAlgoGroup->addWidget(lp);
    rightAlgoGroup->addWidget(adaptive);
    algoGroup->setLayout(rightAlgoGroup);

    // ── Platform combo ────────────────────────────────────────────────────────
    QComboBox *platformComboBox = new QComboBox();
    platformComboBox->addItem("ESP32");
    platformComboBox->addItem("NXP");
    platformComboBox->addItem("STM32");
    platformComboBox->addItem("NRF");
    QMap<QString, platform_current_mah_e> platformMap = {
        {"ESP32", PLATFORM_CURRENT_ESP},
        {"NXP",   PLATFORM_CURRENT_NXP},
        {"STM32", PLATFORM_CURRENT_STM},
        {"NRF",   PLATFORM_CURRENT_NRF}
    };
    choosePlatformLayout->addWidget(platformComboBox);
    choosePlatformGroupBox->setLayout(choosePlatformLayout);

    // ── Graphics checkboxes ───────────────────────────────────────────────────
    QCheckBox *showCurrentBattery  = new QCheckBox("Show Current Battery");
    QCheckBox *showCurrentSystem   = new QCheckBox("Show Current System");
    QCheckBox *showCurrentPlatform = new QCheckBox("Show Current Platform");
    QCheckBox *showVoltage         = new QCheckBox("Show Voltage");
    QCheckBox *showSoc             = new QCheckBox("Show SoC");

    showCurrentBattery->setChecked(currentBatteryDock->isVisible());
    showCurrentSystem->setChecked(currentSystemDock->isVisible());
    showCurrentPlatform->setChecked(currentPlatformDock->isVisible());
    showVoltage->setChecked(voltageDock->isVisible());
    showSoc->setChecked(socDock->isVisible());

    leftGroup->addWidget(showCurrentBattery);
    leftGroup->addWidget(showCurrentSystem);
    leftGroup->addWidget(showCurrentPlatform);
    leftGroup->addWidget(showVoltage);
    leftGroup->addWidget(showSoc);
    graphicsGroup->setLayout(leftGroup);

    // ── File path rows ────────────────────────────────────────────────────────
    QPushButton *loadCurrentBtn     = new QPushButton("Current");
    QPushButton *loadPolyBtn        = new QPushButton("Poly");
    QPushButton *loadOcvSocCurveBtn = new QPushButton("OCV");
    QPushButton *loadParamsBtn      = new QPushButton("Params");
    QPushButton *loadNoisesBtn      = new QPushButton("Noise");
    QPushButton *loadFlagsBtn       = new QPushButton("Flags");

    QLineEdit *currentPath  = new QLineEdit(currPath);       currentPath->setReadOnly(true);
    QLineEdit *polyPath     = new QLineEdit(polynomsPath);   polyPath->setReadOnly(true);
    QLineEdit *ocvSocPath   = new QLineEdit(ocvSOCPath);     ocvSocPath->setReadOnly(true);
    QLineEdit *paramsPath   = new QLineEdit(parametersPath); paramsPath->setReadOnly(true);
    QLineEdit *noisePath    = new QLineEdit(this->noisePath);noisePath->setReadOnly(true);
    QLineEdit *flagsPath    = new QLineEdit(this->flagPath); flagsPath->setReadOnly(true);

    auto createRow = [&](QPushButton *btn, QLineEdit *edit) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setContentsMargins(4, 2, 4, 2);
        row->addWidget(btn);
        row->addWidget(edit);
        rightFilesGroup->addLayout(row);
    };
    rightFilesGroup->setSpacing(6);
    rightFilesGroup->setContentsMargins(8, 8, 8, 8);
    createRow(loadCurrentBtn,     currentPath); rightFilesGroup->addStretch();
    createRow(loadPolyBtn,        polyPath);    rightFilesGroup->addStretch();
    createRow(loadOcvSocCurveBtn, ocvSocPath);  rightFilesGroup->addStretch();
    createRow(loadParamsBtn,      paramsPath);  rightFilesGroup->addStretch();
    createRow(loadNoisesBtn,      noisePath);   rightFilesGroup->addStretch();
    createRow(loadFlagsBtn,       flagsPath);   rightFilesGroup->addStretch();
    inputFilesGroup->setLayout(rightFilesGroup);

    // Set default paths
    currentPath->setText("/home/filip/Projects/Master/Params/current_pattern_najnoviji.csv");
    polyPath->setText   ("/home/filip/Projects/Master/Params/ocv_poly_10.csv");
    ocvSocPath->setText ("/home/filip/Projects/Master/Params/ocv.csv");
    paramsPath->setText ("/home/filip/Projects/Master/Params/parameters.csv");
    noisePath->setText  ("/home/filip/Projects/Master/Params/noise.csv");

    // ── SoC group ─────────────────────────────────────────────────────────────
    QGroupBox   *socGroup  = new QGroupBox("SoC Settings");
    QVBoxLayout *socLayout = new QVBoxLayout();
    socLayout->setSpacing(6);
    socLayout->setContentsMargins(8, 8, 8, 8);

    QDoubleSpinBox *initSocSpinBox = new QDoubleSpinBox();
    initSocSpinBox->setRange(0.0, 100.0);
    initSocSpinBox->setValue(initialSoC * 100.0);
    initSocSpinBox->setSuffix(" %");
    initSocSpinBox->setDecimals(1);
    initSocSpinBox->setFixedWidth(80);

    QDoubleSpinBox *endSocSpinBox = new QDoubleSpinBox();
    endSocSpinBox->setRange(0.0, 100.0);
    endSocSpinBox->setValue(endSoC * 100.0);
    endSocSpinBox->setSuffix(" %");
    endSocSpinBox->setDecimals(1);
    endSocSpinBox->setFixedWidth(80);

    QHBoxLayout *initSocRow = new QHBoxLayout();
    initSocRow->addWidget(new QLabel("Initial SoC:"));
    initSocRow->addWidget(initSocSpinBox);
    QHBoxLayout *endSocRow = new QHBoxLayout();
    endSocRow->addWidget(new QLabel("End SoC:"));
    endSocRow->addWidget(endSocSpinBox);
    socLayout->addLayout(initSocRow);
    socLayout->addLayout(endSocRow);
    socLayout->addStretch();
    socGroup->setLayout(socLayout);

    // ── Main layout ───────────────────────────────────────────────────────────
    groupsLayout->addWidget(graphicsGroup,          1);
    groupsLayout->addWidget(inputFilesGroup,        2);
    groupsLayout->addWidget(algoGroup,              1);
    groupsLayout->addWidget(choosePlatformGroupBox, 1);
    groupsLayout->addWidget(analysisGroupBox,       1);
    groupsLayout->addWidget(socGroup,               1);

    mainLayout->addLayout(groupsLayout);
    mainLayout->addStretch();

    QDialogButtonBox *buttonsOkCancel =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonsOkCancel, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonsOkCancel, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonsOkCancel);
    mainLayout->addLayout(buttonLayout);

    /* Execute *****************************************************************/
    if (dialog->exec() != QDialog::Accepted) return;

    double newInitSoC = initSocSpinBox->value() / 100.0;
    double newEndSoC  = endSocSpinBox->value()  / 100.0;

    if (newInitSoC <= newEndSoC) {
        QMessageBox::warning(this, "Invalid SoC Settings",
                             "Initial SoC must be greater than End SoC.");
        return;
    }

    initialSoC = newInitSoC;
    endSoC     = newEndSoC;

    // Collect selected algos
    QList<algoTimeTableDuration_e> selectedAlgos;
    if (k0->isChecked())       selectedAlgos << K0;
    if (k2->isChecked())       selectedAlgos << K2;
    if (lp->isChecked())       selectedAlgos << LP;
    if (adaptive->isChecked()) selectedAlgos << ADAPTIVE_LP_K2;

    if (selectedAlgos.isEmpty()) {
        QMessageBox::warning(this, "No Algo", "Select at least one algorithm.");
        return;
    }

    isOfflineMode    = offlineAnalysys->isChecked();
    selectedPlatform = platformComboBox->currentText();
    platform_current_mah_e platform =
        platformMap.value(selectedPlatform, PLATFORM_CURRENT_ESP);

    currPath        = currentPath->text();
    polynomsPath    = polyPath->text();
    ocvSOCPath      = ocvSocPath->text();
    parametersPath  = paramsPath->text();
    this->noisePath = noisePath->text();

    // Dock visibility
    currentBatteryDock->setVisible(showCurrentBattery->isChecked());
    currentSystemDock->setVisible(showCurrentSystem->isChecked());
    currentPlatformDock->setVisible(showCurrentPlatform->isChecked());
    voltageDock->setVisible(showVoltage->isChecked());
    socDock->setVisible(showSoc->isChecked());
    redistributeDocks();

    // Clear plots
    currentBatteryPlot->clear();
    currentSystemPlot->clear();
    currentPlatformPlot->clear();
    voltagePlot->clear();
    socPlot->clear();

    if (!isOfflineMode) {
        // ── Online mode: single algo only ─────────────────────────────────────
        clearAlgoTabs();

        algoTimeTableDuration_e algo = selectedAlgos.first();

        // Reconfigure flags path
        flagsPath->setText(algoFlagsPath(algo));
        this->flagPath = algoFlagsPath(algo);

        simulatorContainer->setAlgoSelected(algo);
        simulatorContainer->setPlatformCurrent(platform);
        simulatorContainer->setCurrentPath(currPath);
        simulatorContainer->setOcvPolyPath(polynomsPath);
        simulatorContainer->setOcvSocPath(ocvSOCPath);
        simulatorContainer->setParamsPath(parametersPath);
        simulatorContainer->setNoisePath(this->noisePath);
        simulatorContainer->setFlagsPath(this->flagPath);
        simulatorContainer->setOfflineMode(false);
        simulatorContainer->loadFiles();
        simulatorContainer->reset(initialSoC);

        speedUpBtn->setEnabled(false);
        speedUpBtn->setText("1x");
        timerPeriodMs = 10;

    } else {
        /* Offline mode: one container per algo ***************************************/
        clearAlgoTabs();

        for (algoTimeTableDuration_e algo : selectedAlgos) {
            algoTab_t tab;
            tab.algo     = algo;
            tab.algoName = algoTabName(algo);

            tab.container = new SimulatorContainer(
                batteryModel, platformCurrent, timeTable, this);

            tab.container->setCurrentPath(currPath);
            tab.container->setOcvPolyPath(polynomsPath);
            tab.container->setOcvSocPath(ocvSOCPath);
            tab.container->setParamsPath(parametersPath);
            tab.container->setNoisePath(this->noisePath);
            tab.container->setFlagsPath(algoFlagsPath(algo));
            tab.container->setOutputPath(algoOutputPath(algo));
            tab.container->setAlgoSelected(algo);
            tab.container->setOfflineMode(true);
            tab.container->setPlatformCurrent(platform);

            connect(tab.container, &SimulatorContainer::loadError, this,
                    [=](const QString &msg){ qDebug() << "Load error:" << msg; });

            algoTabs.append(tab);
            tabBar->addTab(tab.algoName);
        }

        // Load files for all
        for (auto &tab : algoTabs) {
            if (!tab.container->loadFiles()) return;
            tab.container->reset(initialSoC);
        }

        tabBar->setVisible(true);
        tabBar->setCurrentIndex(0);

        speedUpBtn->setEnabled(false);
        speedUpBtn->setText("Offline");
    }

    playBtn->setEnabled(true);
    setLed(ledConfigLabel, "green");
    statusConfigLabel->setText("Configured");
}

/*******************************************************************************
 * onLogInfoClicked
 ******************************************************************************/
void SimulatorWnd::onLogInfoClicked()
{
    // Created dialog window
    QDialog *dialogWindow = new QDialog(this);
    dialogWindow->setWindowTitle("Simulator Log");
    dialogWindow->resize(600, 800);

    // Create vertical layout for table
    QVBoxLayout *layout = new QVBoxLayout(dialogWindow);


    if(isOfflineMode && !algoTabs.isEmpty()){

        // Create QTab
        QTabWidget *tabWidget = new QTabWidget(dialogWindow);

        // Offline analisys
        for (int i = 0; i < algoTabs.size(); i++) {
            const algoTab_t &tab = algoTabs[i];

            QTableWidget *table = createResultsTable(
                tab.avgBatteryCurr,
                tab.avgSystemCurr,
                tab.container->getLastValuesOfSimulationStep(),
                tab.container->getTimeS(),
                tab.container->getNumberOfSamples(),
                tab.algoName
                );

            tabWidget->addTab(table, tab.algoName);
        }
        tabWidget->setCurrentIndex(tabBar->currentIndex());

        layout->addWidget(tabWidget);
    }
    else {
            // Online analisys one tab
            QTableWidget *table = createResultsTable(
                averageBatteryCurr,
                averageSystemCurr,
                simulatorContainer->getLastValuesOfSimulationStep(),
                simulatorContainer->getTimeS(),
                simulatorContainer->getNumberOfSamples(),
                ""
                );
            layout->addWidget(table);
    }

    // Create exit button
    QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btn, &QDialogButtonBox::rejected, dialogWindow, &QDialog::reject);

    // Add btn do the window
    layout->addWidget(btn);
    dialogWindow->exec();
}

/*******************************************************************************
 * onPlayClicked
 ******************************************************************************/

void SimulatorWnd::onPlayClicked()
{
    if (isOfflineMode) {
        qDebug() << "Run Offline Simulation";
        startOfflineSimulation();
    } else {
        qDebug() << "Run Online Simulation";
        currentSimState = SIMULATION_PLAYING;
        playBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        pauseBtn->setEnabled(true);
        speedUpBtn->setEnabled(true);
        configBtn->setEnabled(false);
        setLed(ledPlaySimulationLabel, "green");
        statusPlaySimulationLabel->setText("Running");
        timer->start(timerPeriodMs);
        emit sigPlay();
    }
}

/*******************************************************************************
 * onStopClicked
 ******************************************************************************/
void SimulatorWnd::onStopClicked()
{
    currentSimState = SIMULATION_STOPPED;
    qDebug() << "Stop Simulation";

    timer->stop();

    if (isOfflineMode) {
        for (auto &tab : algoTabs)
            tab.container->reset(initialSoC);
    } else {
        simulatorContainer->reset(initialSoC);
    }

    currentBatteryPlot->clear();
    currentSystemPlot->clear();
    currentPlatformPlot->clear();
    voltagePlot->clear();
    socPlot->clear();

    if (!isOfflineMode) {
        speedUpBtn->setText("1x");
        timerPeriodMs = 10;
    }

    playBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true);
    logInfoBtn->setEnabled(false);

    setLed(ledPlaySimulationLabel, "red");
    statusPlaySimulationLabel->setText("Stopped");
    progressBar->setValue(0);
    currentSample = 0;

    emit sigStop();
}

/*******************************************************************************
 * onPauseClicked
 ******************************************************************************/
void SimulatorWnd::onPauseClicked()
{
    if (timer->isActive()) {
        currentSimState = SIMULATION_PAUSED;
        timer->stop();
        pauseBtn->setText("Resume");
        setLed(ledPlaySimulationLabel, "yellow");
        statusPlaySimulationLabel->setText("Paused");
    } else {
        currentSimState = SIMULATION_PLAYING;
        timer->start(timerPeriodMs);
        pauseBtn->setText("Pause");
        setLed(ledPlaySimulationLabel, "green");
        statusPlaySimulationLabel->setText("Running");
    }
    emit sigPause();
}

/*******************************************************************************
 * onSpeedUpClicked
 ******************************************************************************/
void SimulatorWnd::onSpeedUpClicked()
{
    if (isOfflineMode) return;
    if      (timerPeriodMs == 10) { speedUpBtn->setText("2x"); timerPeriodMs = 5;  }
    else if (timerPeriodMs == 5)  { speedUpBtn->setText("5x"); timerPeriodMs = 2;  }
    else                          { speedUpBtn->setText("1x"); timerPeriodMs = 10; }
    if (timer->isActive()) { timer->stop(); timer->start(timerPeriodMs); }
    emit sigSpeedUp();
}

/*******************************************************************************
 * startOfflineSimulation - each algo in its own thread, parallel
 ******************************************************************************/
void SimulatorWnd::startOfflineSimulation()
{
    playBtn->setEnabled(false);
    configBtn->setEnabled(false);
    setLed(ledPlaySimulationLabel, "green");
    statusPlaySimulationLabel->setText("Offline Processing");

    int tabCount = algoTabs.size();
    QSharedPointer<QAtomicInt> finishedCount(new QAtomicInt(0));

    for (int i = 0; i < tabCount; i++) {

        algoTabs[i].container->reset(initialSoC);
        algoTabs[i].avgBatteryCurr = 0.0;
        algoTabs[i].avgSystemCurr  = 0.0;
        algoTabs[i].simulatorSample.time.clear();
        algoTabs[i].simulatorSample.voltage.battery.clear();
        algoTabs[i].simulatorSample.current.battery.clear();
        algoTabs[i].simulatorSample.current.system.clear();
        algoTabs[i].simulatorSample.current.platform.clear();
        algoTabs[i].simulatorSample.soc.battery.clear();
        algoTabs[i].simulatorSample.soc.system.clear();

        QtConcurrent::run([this, i, tabCount, finishedCount]()
          {
              SimulatorContainer *container = algoTabs[i].container;
              double endSocPct = endSoC * 100.0;

              QVector<double> vTime, vBat, vIbat, vISys, vIPlatform, vSoc, vSocIsys;
              double avgBat = 0.0, avgSys = 0.0;
              int localSample = 0;

              while (!container->isFinished()) {
                  container->step();

                  double soc = container->getSoC();
                  vTime.push_back(container->getTimeS());
                  vBat.push_back(container->getVBat());
                  vIbat.push_back(container->getIBat());
                  vISys.push_back(container->getISys());
                  vIPlatform.push_back(container->getIPlatform());
                  vSoc.push_back(soc);
                  vSocIsys.push_back(container->getSoCIsys());

                  avgBat += container->getIBat();
                  avgSys += container->getISys();
                  localSample++;

                  if (soc <= endSocPct) break;

                  if (i == 0 && localSample % 1000 == 0) {
                      int pct = (localSample * 100) / container->getNumberOfSamples();
                      QMetaObject::invokeMethod(this, [=]() {
                          progressBar->setValue(pct);
                      });
                  }
              }

              // Calculate average Currents for info window
              if (localSample > 0) { avgBat /= localSample; avgSys /= localSample; }

              // Back to main thread: store data and replot if active tab
              QMetaObject::invokeMethod(this,
                                        [this, i, avgBat, avgSys,
                                         vTime, vBat, vIbat, vISys, vIPlatform, vSoc, vSocIsys,
                                         tabCount, finishedCount]()
                {
                    algoTabs[i].avgBatteryCurr                   = avgBat;
                    algoTabs[i].avgSystemCurr                    = avgSys;
                    algoTabs[i].simulatorSample.time             = vTime;
                    algoTabs[i].simulatorSample.voltage.battery  = vBat;
                    algoTabs[i].simulatorSample.current.battery  = vIbat;
                    algoTabs[i].simulatorSample.current.system   = vISys;
                    algoTabs[i].simulatorSample.current.platform = vIPlatform;
                    algoTabs[i].simulatorSample.soc.battery      = vSoc;
                    algoTabs[i].simulatorSample.soc.system       = vSocIsys;

                    // If this tab is currently visible, replot immediately
                    if (tabBar->currentIndex() == i)
                        replotActiveTab();

                    qDebug() << "Tab" << algoTabs[i].algoName << "finished";

                    int done = finishedCount->fetchAndAddOrdered(1) + 1;
                    if (done == tabCount) {
                        progressBar->setValue(100);
                        setLed(ledPlaySimulationLabel, "green");
                        statusPlaySimulationLabel->setText("Offline Done");
                        playBtn->setEnabled(false);
                        stopBtn->setEnabled(true);
                        configBtn->setEnabled(true);
                        logInfoBtn->setEnabled(true);
                        pauseBtn->setEnabled(false);
                        speedUpBtn->setEnabled(false);
                        qDebug() << "All simulations finished";
                    }
                });
          });
    }
}

/*******************************************************************************
 * setLed
 ******************************************************************************/
void SimulatorWnd::setLed(QLabel *led, const QString &color,
                          simulationState_e simulationState)
{
    QString ledColor = color;
    if (color == "None") {
        switch (simulationState) {
        case SIMULATION_UNINIT:  ledColor = "grey";   break;
        case SIMULATION_PLAYING: ledColor = "green";  break;
        case SIMULATION_STOPPED: ledColor = "red";    break;
        case SIMULATION_PAUSED:  ledColor = "yellow"; break;
        }
    }
    QString light, dark;
    if      (ledColor == "green")  { light = "#90ff90"; dark = "#006600"; }
    else if (ledColor == "red")    { light = "#ff9090"; dark = "#660000"; }
    else if (ledColor == "yellow") { light = "#ffff90"; dark = "#666600"; }
    else                           { light = "#cccccc"; dark = "#444444"; }

    led->setStyleSheet(QString(
                           "background-color: qradialgradient("
                           "cx:0.3,cy:0.3,radius:0.7,fx:0.3,fy:0.3,"
                           "stop:0 white,stop:0.3 %1,stop:1 %2);"
                           "border-radius:8px;border:1px solid #333;").arg(light).arg(dark));
}

/*******************************************************************************
 * onLedTimerTimeout
 ******************************************************************************/
void SimulatorWnd::onLedTimerTimeout()
{
    ledBlinkState = !ledBlinkState;
    if (ledBlinkState) setLed(ledPlaySimulationLabel, "None", currentSimState);
    else               setLed(ledPlaySimulationLabel, "grey");
}

/*******************************************************************************
 * redistributeDocks
 ******************************************************************************/
void SimulatorWnd::redistributeDocks()
{
    QList<QDockWidget*> visible;
    if (currentBatteryDock->isVisible())  visible << currentBatteryDock;
    if (currentSystemDock->isVisible())   visible << currentSystemDock;
    if (currentPlatformDock->isVisible()) visible << currentPlatformDock;
    if (voltageDock->isVisible())         visible << voltageDock;
    if (socDock->isVisible())             visible << socDock;
    if (visible.isEmpty()) return;

    int totalHeight = this->height() - 50;
    int each        = totalHeight / visible.size();
    QList<int> sizes;
    for (int i = 0; i < visible.size(); i++) sizes << each;
    resizeDocks(visible, sizes, Qt::Vertical);
}
