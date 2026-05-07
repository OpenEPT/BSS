/**
 * @file    SimulatorWindow.cpp
 * @brief   Simulator window - dock layout preserved, multi-algo tab bar
 * @version 2.0.0
 * @date    2026-04-21
 * @author  Filip Radojevic
 */

#include "SimulatorWindow.h"
#include <QtConcurrent>

QTextEdit *g_logConsole = nullptr; // Global pointer for logging handler

/*******************************************************************************
 * Constructor
 ******************************************************************************/
SimulatorWnd::SimulatorWnd(QWidget *parent)
    : QMainWindow(parent)
    , currentSimState(SIMULATION_UNINIT)
    , timerPeriodMs(10)
    , currentSample(0)
{
    // ── Constructors ─────────────────────────────────────────────────────────────────
    batteryModel       = new BatteryModel(this);
    platformCurrent    = new PlatformCurrent(PLATFORM_CURRENT_ESP, this);
    timeTable          = new TimeTable(K2, this);
    simulatorContainer = new SimulatorContainer(batteryModel, platformCurrent, timeTable, this);

    // ── Timers ─────────────────────────────────────────────────────────────────
    timer = new QTimer(this); // Online simu

    ledTimer = new QTimer(this); // Led timer (status bar diode)
    ledTimer->setInterval(500);
    ledTimer->start();

    // ── Tool Bar ─────────────────────────────────────────────────────────────────
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);

    // Create push buttons
    playBtn    = new QPushButton("Play");
    stopBtn    = new QPushButton("Stop");
    pauseBtn   = new QPushButton("Pause");
    speedUpBtn = new QPushButton("1x");

    // Create tool buttons
    configBtn  = new QToolButton(this);
    configBtn->setIcon(QIcon("/home/filip/Projects/Master/GUI/Documentation/img/conf.jpeg"));
    configBtn->setFixedSize(30, 30);

    logInfoBtn = new QToolButton(this);
    logInfoBtn->setIcon(QIcon("/home/filip/Projects/Master/GUI/Documentation/img/info_button.png"));
    logInfoBtn->setFixedSize(30, 30);

    simulationSettingsBtn  = new QToolButton(this);
    simulationSettingsBtn->setIcon(QIcon("/home/filip/Projects/Master/GUI/Documentation/img/simulation_config_icon.jpeg"));
    simulationSettingsBtn->setFixedSize(30, 30);

    // Enabling buttons
    playBtn->setEnabled(false);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true);   // True
    logInfoBtn->setEnabled(false);
    simulationSettingsBtn->setEnabled(false);

    // Add buttons in Tab Tool Bar
    toolBar->addWidget(playBtn);
    toolBar->addWidget(stopBtn);
    toolBar->addWidget(pauseBtn);
    toolBar->addWidget(speedUpBtn);
    toolBar->addWidget(configBtn);
    toolBar->addWidget(simulationSettingsBtn);
    toolBar->addWidget(logInfoBtn);

    // Set tool bar on the top of the window area
    addToolBar(Qt::TopToolBarArea, toolBar);

    // ── Plots ─────────────────────────────────────────────────────────────────
    currentBatteryPlot  = new Plot(400, 100, false, this);
    currentSystemPlot   = new Plot(400, 100, false, this);
    currentPlatformPlot = new Plot(400, 100, false, this);
    voltagePlot         = new Plot(400, 100, false, this);
    socPlot             = new Plot(400, 100, false, this);

    // Current Battery plot
    currentBatteryPlot->setTitle("Battery Current");
    currentBatteryPlot->setYLabel("[A]");
    currentBatteryPlot->setXLabel("[s]");

    // Current System plot
    currentSystemPlot->setTitle("System Current");
    currentSystemPlot->setYLabel("[A]");
    currentSystemPlot->setXLabel("[s]");

    // Current Platform plot
    currentPlatformPlot->setTitle("Platform Current");
    currentPlatformPlot->setYLabel("[A]");
    currentPlatformPlot->setXLabel("[s]");

    // Voltage Terminal plot
    voltagePlot->setTitle("Battery Voltage");
    voltagePlot->setYLabel("[V]");
    voltagePlot->setXLabel("[s]");

    // SoC plot
    socPlot->setTitle("Battery SoC");
    socPlot->setYLabel("[%]");
    socPlot->setXLabel("[s]");

    // Set height of plots
    currentBatteryPlot->setMinimumHeight(150);
    currentSystemPlot->setMinimumHeight(150);
    currentPlatformPlot->setMinimumHeight(150);
    voltagePlot->setMinimumHeight(150);
    socPlot->setMinimumHeight(150);

    // ── Log console ─────────────────────────────────────────────────────────────────
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setMaximumHeight(150);
    logConsole->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: monospace;");
    g_logConsole = logConsole;
    qInstallMessageHandler(logHandler);

    // ── Dock  ─────────────────────────────────────────────────────────────────

    // Current battery dock
    currentBatteryDock = new QDockWidget("Battery Current", this);
    currentBatteryDock->setWidget(currentBatteryPlot);

    // Current system dock
    currentSystemDock = new QDockWidget("System Current", this);
    currentSystemDock->setWidget(currentSystemPlot);

    // Current platform dock
    currentPlatformDock = new QDockWidget("Platform Current", this);
    currentPlatformDock->setWidget(currentPlatformPlot);

    // Voltage terminal dock
    voltageDock = new QDockWidget("Battery Voltage", this);
    voltageDock->setWidget(voltagePlot);

    // SoC dock
    socDock = new QDockWidget("SoC", this);
    socDock->setWidget(socPlot);

    // Logging console dock
    logDock = new QDockWidget("Log Console", this);
    logDock->setWidget(logConsole);

    // Add widgets in docks
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentBatteryDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentSystemDock);
    addDockWidget(Qt::LeftDockWidgetArea,   currentPlatformDock);
    addDockWidget(Qt::RightDockWidgetArea,  voltageDock);
    addDockWidget(Qt::RightDockWidgetArea,  socDock);

    // Split docks area
    splitDockWidget(currentBatteryDock,  currentSystemDock,   Qt::Vertical);
    splitDockWidget(currentSystemDock,   currentPlatformDock, Qt::Vertical);
    splitDockWidget(voltageDock,         socDock,             Qt::Vertical);

    // Resize dock area
    resize(1600, 1000);

    // ── Status Bar ─────────────────────────────────────────────────────────────────
    statusBar()->setStyleSheet("border-top: 1px solid palette(mid); padding: 2px;");

    // Labels for each element in status bar
    ledConfigLabel            = new QLabel(this);
    ledPlaySimulationLabel    = new QLabel(this);
    statusConfigLabel         = new QLabel(this);
    statusPlaySimulationLabel = new QLabel(this);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedWidth(400);
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

    // ── Connections ─────────────────────────────────────────────────────────────────

    // Button connections
    connect(playBtn,    &QPushButton::clicked, this, &SimulatorWnd::onPlayClicked);
    connect(stopBtn,    &QPushButton::clicked, this, &SimulatorWnd::onStopClicked);
    connect(pauseBtn,   &QPushButton::clicked, this, &SimulatorWnd::onPauseClicked);
    connect(speedUpBtn, &QPushButton::clicked, this, &SimulatorWnd::onSpeedUpClicked);
    connect(configBtn,  &QToolButton::clicked, this, &SimulatorWnd::onConfigClicked);
    connect(logInfoBtn, &QToolButton::clicked, this, &SimulatorWnd::onLogInfoClicked);
    connect(simulationSettingsBtn, &QToolButton::clicked, this, &SimulatorWnd::onSimuSettingsClicked);


    // Online dataSample connection
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


    // Load signals
    connect(simulatorContainer, &SimulatorContainer::loadSuccess, this,
            [=]{
                playBtn->setEnabled(true);
            });

    connect(simulatorContainer, &SimulatorContainer::loadError, this,
            [=](const QString &msg){
                qDebug() << "Load error:" << msg;
            });

    // Timer connections
    connect(timer, &QTimer::timeout, this, &SimulatorWnd::onTimerTick);
    connect(ledTimer, &QTimer::timeout, this, &SimulatorWnd::onLedTimerTimeout);

    // If output simulation is checked in simulation settings window
    connect(this, &SimulatorWnd::sigOpenOuputWindow,
            this, &SimulatorWnd::onOutputSimulationChecked);
}

void SimulatorWnd::replotActiveTab()
{
    static const QList<QColor> colors = {
        QColor(40,  110, 255), QColor(255, 100,   0), QColor(0,   180,   0),
        QColor(180,   0, 180), QColor(255,   0,   0), QColor(0,   200, 200),
        QColor(255, 200,   0), QColor(0,   100, 100), QColor(255,   0, 150),
        QColor(100, 100,   0), QColor(0,    50, 200), QColor(200, 100,  50),
        QColor(100, 200,   0), QColor(200,   0,  50), QColor(0,   150, 255),
        QColor(150,   0, 255), QColor(255, 150,   0), QColor(0,   200, 100),
        QColor(200, 200,   0), QColor(100,   0, 200),
    };

    // Clear current plots
    socPlot->clearAllGraphs();
    voltagePlot->clearAllGraphs();
    currentBatteryPlot->clearAllGraphs();
    currentPlatformPlot->clearAllGraphs();
    currentSystemPlot->clearAllGraphs();

    // Enable legends for all graphs that need it
    currentPlatformPlot->enableLegend(true);
    socPlot->enableLegend(true);
    voltagePlot->enableLegend(true);
    currentBatteryPlot->enableLegend(true);

    // SocIsys GRAPH(0)
    socPlot->setGraphLineStyle(0, Qt::DotLine, QColor(0, 0, 0), "SoC Isys");
    socPlot->setGraphLineWidth(0, 2);
    socPlot->setGraphData(0, algoTabs[0].simulatorSample.soc.system,
                          algoTabs[0].simulatorSample.time);

    // Plot System current, it's the same for all algo
    currentSystemPlot->setGraphData(0, algoTabs[0].simulatorSample.current.system,
                          algoTabs[0].simulatorSample.time);

    int graphIdx = 1;
    int plotIdx = 0;

    // Plot all other graphs
    for (int i = 0; i < algoTabs.size(); i++) {

        // Check if algo is selected in simulation settings
        bool show = (i < visibleAlgosInCompare.size()) ? visibleAlgosInCompare[i] : true;

        // If it's not selected skip algo, doesn't need to be plotted
        if (!show) continue;


        // take algo tab
        const algoTab_t &tab = algoTabs[i];
        QColor colorAlgo   = colors[i % colors.size()];
        QColor colorSoCBat = colorAlgo.lighter(140);

        // ── SoC ───────────────────────────────────────────────────────────────────────────────────

        // First soc plot is referent soc bat
        socPlot->addLineGraphWithStyle(colorSoCBat, tab.algoName + " SoCBat", Qt::DashLine, 2);
        socPlot->setGraphData(graphIdx++, tab.simulatorSample.soc.battery, tab.simulatorSample.time);

        // Other plot is real algo soc
        socPlot->addLineGraph(colorAlgo, tab.algoName + " AlgoSoC");
        socPlot->setGraphLineWidth(graphIdx, 2);
        socPlot->setGraphData(graphIdx++, tab.simulatorSample.soc.algoSoc, tab.simulatorSample.time);

        // ── Voltage, Battery & Platform Current ───────────────────────────────────────────────────
        if (plotIdx == 0) {
            // Voltage plot
            voltagePlot->setGraphName(0, tab.algoName);
            voltagePlot->setGraphLineWidth(0, 2);
            voltagePlot->setData(tab.simulatorSample.voltage.battery, tab.simulatorSample.time);

            // Current Battery plot
            currentBatteryPlot->setGraphName(0, tab.algoName);
            currentBatteryPlot->setGraphLineWidth(0, 2);
            currentBatteryPlot->setData(tab.simulatorSample.current.battery, tab.simulatorSample.time);

            // Current platform plot
            currentPlatformPlot->setGraphName(0, tab.algoName);
            currentPlatformPlot->setGraphLineWidth(0, 2);
            currentPlatformPlot->setData(tab.simulatorSample.current.platform, tab.simulatorSample.time);

        } else {
            // Voltage plot
            voltagePlot->addLineGraph(colorAlgo, tab.algoName);
            voltagePlot->setGraphLineWidth(plotIdx, 2);
            voltagePlot->setGraphData(plotIdx, tab.simulatorSample.voltage.battery, tab.simulatorSample.time);

            // Current Battery plot
            currentBatteryPlot->addLineGraph(colorAlgo, tab.algoName);
            currentBatteryPlot->setGraphLineWidth(plotIdx, 2);
            currentBatteryPlot->setGraphData(plotIdx, tab.simulatorSample.current.battery, tab.simulatorSample.time);

            // Current platform plot
            currentPlatformPlot->addLineGraph(colorAlgo, tab.algoName);
            currentPlatformPlot->setGraphLineWidth(plotIdx, 2);
            currentPlatformPlot->setGraphData(plotIdx, tab.simulatorSample.current.platform, tab.simulatorSample.time);
        }
        plotIdx++;
    }

    // Replot all graphs
    socPlot->replotAll();
    voltagePlot->replotAll();
    currentBatteryPlot->replotAll();
    currentPlatformPlot->replotAll();
}

/*******************************************************************************
 * onTimerTick - online mode
 ******************************************************************************/
void SimulatorWnd::onTimerTick()
{
    simulatorContainer->step();
}

/*******************************************************************************
 * clearAlgoTabs - deletes all algo containers and clears the list
 ******************************************************************************/
void SimulatorWnd::clearAlgoTabs()
{
    // Delete all allocated objects for each algo tab
    for (auto &tab : algoTabs) {
        delete tab.container;
        delete tab.batteryModel;
        delete tab.timeTable;
        delete tab.platformCurrent;
    }

    // Clear the list
    algoTabs.clear();
}

/*******************************************************************************
 * onConfigClicked
 ******************************************************************************/
void SimulatorWnd::onConfigClicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Config");
    dialog->resize(1000, 1000);

    QVBoxLayout *mainLayout   = new QVBoxLayout(dialog);

    // ── Group boxes ───────────────────────────────────────────────────────────
    QGroupBox *graphicsGroup         = new QGroupBox("Choose Graphics");
    QGroupBox *inputFilesGroup       = new QGroupBox("Choose Files");
    inputFilesGroup->setFixedWidth(600);
    QGroupBox *analysisGroupBox      = new QGroupBox("Choose Analysis");
    QGroupBox *choosePlatformGroupBox= new QGroupBox("Choose Platform");

    // ── Layouts ───────────────────────────────────────────────────────────────
    QVBoxLayout *leftGroup          = new QVBoxLayout();
    QVBoxLayout *rightFilesGroup    = new QVBoxLayout();
    QVBoxLayout *analysisLayout     = new QVBoxLayout();
    QVBoxLayout *choosePlatformLayout = new QVBoxLayout();

    // ── Analysis radio ────────────────────────────────────────────────────────
    QRadioButton *onlineAnalysys  = new QRadioButton("Online Analysis");
    QRadioButton *offlineAnalysys = new QRadioButton("Offline Analysis");
    QButtonGroup *analysisBtnGroup = new QButtonGroup(dialog);
    analysisBtnGroup->addButton(onlineAnalysys);
    analysisBtnGroup->addButton(offlineAnalysys);
    offlineAnalysys->setChecked(true);
    analysisLayout->addWidget(onlineAnalysys);
    analysisLayout->addWidget(offlineAnalysys);
    analysisGroupBox->setLayout(analysisLayout);

    // ── Algo Configuration group ──────────────────────────────────────────────
    QGroupBox   *algoConfigGroup  = new QGroupBox("Algo Configuration");
    QHBoxLayout *algoConfigLayout = new QHBoxLayout();  // ← horizontalni: levo kontrole, desno tabela
    algoConfigLayout->setSpacing(8);
    algoConfigLayout->setContentsMargins(8, 8, 8, 8);

    // ── Levo: kontrole u grid layoutu ────────────────────────────────────────
    QComboBox *algoCombo     = new QComboBox();
    QComboBox *platformCombo = new QComboBox();
    QComboBox *modeCombo     = new QComboBox();
    QSpinBox  *periodSpin    = new QSpinBox();
    QComboBox *battCombo     = new QComboBox();
    QPushButton *addBtn      = new QPushButton("+ Add");

    algoCombo->addItem("K0",       QVariant::fromValue((int)K0));
    algoCombo->addItem("K2",       QVariant::fromValue((int)K2));
    algoCombo->addItem("LP",       QVariant::fromValue((int)LP));
    algoCombo->addItem("Adaptive", QVariant::fromValue((int)ADAPTIVE_LP_K2));

    platformCombo->addItem("ESP32", QVariant::fromValue((int)PLATFORM_CURRENT_ESP));
    platformCombo->addItem("NXP",   QVariant::fromValue((int)PLATFORM_CURRENT_NXP));
    platformCombo->addItem("STM32", QVariant::fromValue((int)PLATFORM_CURRENT_STM));
    platformCombo->addItem("NRF",   QVariant::fromValue((int)PLATFORM_CURRENT_NRF));

    modeCombo->addItem("From File", QVariant::fromValue((int)ALGO_PERIOD_FROM_FILE));
    modeCombo->addItem("Fixed",     QVariant::fromValue((int)ALGO_PERIOD_FIXED));

    periodSpin->setRange(1, 100000);
    periodSpin->setValue(100);
    periodSpin->setEnabled(false);

    battCombo->addItem("457 mAh",  457);
    battCombo->addItem("1000 mAh", 1000);
    battCombo->addItem("2000 mAh", 2000);

    addBtn->setFixedWidth(80);

    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int idx) { periodSpin->setEnabled(idx == 1); });

    // Grid: label u col 0, widget u col 1
    QGridLayout *controlsGrid = new QGridLayout();
    controlsGrid->setSpacing(6);
    controlsGrid->addWidget(new QLabel("Algo:"),            0, 0, Qt::AlignRight);
    controlsGrid->addWidget(algoCombo,                      0, 1);
    controlsGrid->addWidget(new QLabel("Platform:"),        1, 0, Qt::AlignRight);
    controlsGrid->addWidget(platformCombo,                  1, 1);
    controlsGrid->addWidget(new QLabel("Mode:"),            2, 0, Qt::AlignRight);
    controlsGrid->addWidget(modeCombo,                      2, 1);
    controlsGrid->addWidget(new QLabel("Period [samples]:"),3, 0, Qt::AlignRight);
    controlsGrid->addWidget(periodSpin,                     3, 1);
    controlsGrid->addWidget(new QLabel("Battery:"),         4, 0, Qt::AlignRight);
    controlsGrid->addWidget(battCombo,                      4, 1);
    controlsGrid->addWidget(addBtn,                         5, 1, Qt::AlignLeft);
    controlsGrid->setColumnStretch(1, 1);
    controlsGrid->setRowStretch(6, 1);  // ← potisni kontrole gore

    QWidget *controlsWidget = new QWidget();
    controlsWidget->setLayout(controlsGrid);
    controlsWidget->setFixedWidth(280);  // ← fiksna širina leve strane

    // ── Desno: tabela + remove dugme ─────────────────────────────────────────
    QVBoxLayout *tableLayout = new QVBoxLayout();

    QTableWidget *algoTable = new QTableWidget(0, 5);
    algoTable->setHorizontalHeaderLabels({"Algo", "Platform", "Mode", "Period [samples]", "Battery"});
    algoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    algoTable->verticalHeader()->setVisible(true);
    algoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    algoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    algoTable->setAlternatingRowColors(true);
    algoTable->setShowGrid(true);
    algoTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    algoTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // ── Restore saved table rows ──────────────────────────────────────────────
    for (const algoTableRow_t &row : savedAlgoTableRows) {
        int r = algoTable->rowCount();
        algoTable->insertRow(r);
        algoTable->setRowHeight(r, 28);
        algoTable->setItem(r, 0, new QTableWidgetItem(row.algoText));
        algoTable->setItem(r, 1, new QTableWidgetItem(row.platformText));
        algoTable->setItem(r, 2, new QTableWidgetItem(row.modeText));
        algoTable->setItem(r, 3, new QTableWidgetItem(row.periodText));
        algoTable->setItem(r, 4, new QTableWidgetItem(row.batteryText));
        algoTable->item(r, 0)->setData(Qt::UserRole, row.algoData);
        algoTable->item(r, 1)->setData(Qt::UserRole, row.platformData);
        algoTable->item(r, 2)->setData(Qt::UserRole, row.modeData);
        algoTable->item(r, 3)->setData(Qt::UserRole, row.periodValue);
        algoTable->item(r, 4)->setData(Qt::UserRole, row.batteryData);
        for (int c = 0; c < 5; c++)
            algoTable->item(r, c)->setTextAlignment(Qt::AlignCenter);
    }

    QPushButton *removeBtn = new QPushButton("Remove Selected");
    removeBtn->setFixedHeight(28);

    tableLayout->addWidget(algoTable, 1);
    tableLayout->addWidget(removeBtn);

    // ── Add logika ────────────────────────────────────────────────────────────
    connect(addBtn, &QPushButton::clicked, [=]() {
        int row = algoTable->rowCount();
        algoTable->insertRow(row);
        algoTable->setRowHeight(row, 28);

        algoTable->setItem(row, 0, new QTableWidgetItem(algoCombo->currentText()));
        algoTable->setItem(row, 1, new QTableWidgetItem(platformCombo->currentText()));
        algoTable->setItem(row, 2, new QTableWidgetItem(modeCombo->currentText()));
        algoTable->setItem(row, 3, new QTableWidgetItem(
                                       modeCombo->currentIndex() == 1 ? QString::number(periodSpin->value()) : "-"));
        algoTable->setItem(row, 4, new QTableWidgetItem(battCombo->currentText()));

        algoTable->item(row, 0)->setData(Qt::UserRole, algoCombo->currentData());
        algoTable->item(row, 1)->setData(Qt::UserRole, platformCombo->currentData());
        algoTable->item(row, 2)->setData(Qt::UserRole, modeCombo->currentData());
        algoTable->item(row, 3)->setData(Qt::UserRole, periodSpin->value());
        algoTable->item(row, 4)->setData(Qt::UserRole, battCombo->currentData());

        for (int c = 0; c < 5; c++)
            algoTable->item(row, c)->setTextAlignment(Qt::AlignCenter);
    });

    // ── Remove logika ─────────────────────────────────────────────────────────
    connect(removeBtn, &QPushButton::clicked, [=]() {
        QList<int> rows;
        for (auto *item : algoTable->selectedItems())
            if (!rows.contains(item->row()))
                rows.append(item->row());
        std::sort(rows.rbegin(), rows.rend());
        for (int r : rows)
            algoTable->removeRow(r);
    });

    // ── Složi: levo kontrole, desno tabela ───────────────────────────────────
    algoConfigLayout->addWidget(controlsWidget);          // ← levo, fiksno
    algoConfigLayout->addLayout(tableLayout, 1);          // ← desno, razvuci
    algoConfigGroup->setLayout(algoConfigLayout);

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
    // ── Gornji red: Graphics + Files + Analysis + SoC ─────────────────────────
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(graphicsGroup,    1);
    topLayout->addWidget(inputFilesGroup,  3);
    topLayout->addWidget(analysisGroupBox, 1);
    topLayout->addWidget(socGroup,         1);

    // ── Donji red: Algo Configuration - pun width ─────────────────────────────
    // algoConfigGroup već ima scroll tabelu i stretch

    // ── Main layout ───────────────────────────────────────────────────────────
    mainLayout->addLayout(topLayout); // Upper layout on main layout
    mainLayout->addWidget(algoConfigGroup, 1); // lower layout
    mainLayout->addStretch(0);

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

    // ── Validate SoC settings ─────────────────────────────────────────────────
    double newInitSoC = initSocSpinBox->value() / 100.0;
    double newEndSoC  = endSocSpinBox->value()  / 100.0;

    if (newInitSoC <= newEndSoC) {
        QMessageBox::warning(this, "Invalid SoC Settings",
                             "Initial SoC must be greater than End SoC.");
        return;
    }

    initialSoC = newInitSoC;
    endSoC     = newEndSoC;

    // ── Read algo configuration from table ────────────────────────────────────
    // Each row in algoTable represents one algo to simulate
    QList<algoTimeTableDuration_e> selectedAlgos;
    QList<platform_current_mah_e>  selectedPlatforms;
    QList<algoConfig_t>            selectedConfigs;

    for (int r = 0; r < algoTable->rowCount(); r++) {
        // Column 0: Algo type (K0, K2, LP, Adaptive)
        selectedAlgos.append(
            static_cast<algoTimeTableDuration_e>(
                algoTable->item(r, 0)->data(Qt::UserRole).toInt()));

        // Column 1: Platform (ESP32, NXP, STM32, NRF)
        selectedPlatforms.append(
            static_cast<platform_current_mah_e>(
                algoTable->item(r, 1)->data(Qt::UserRole).toInt()));

        // Column 2-3: Period mode (From File or Fixed) + period value
        algoConfig_t cfg;
        cfg.periodMode  = static_cast<algoPeriodMode_e>(
            algoTable->item(r, 2)->data(Qt::UserRole).toInt());
        cfg.fixedPeriod = algoTable->item(r, 3)->data(Qt::UserRole).toInt();
        selectedConfigs.append(cfg);
    }

    if (selectedAlgos.isEmpty()) {
        QMessageBox::warning(this, "No Algo", "Add at least one algorithm.");
        return;
    }

    savedAlgoTableRows.clear();
    for (int r = 0; r < algoTable->rowCount(); r++) {
        algoTableRow_t row;
        row.algoText     = algoTable->item(r, 0)->text();
        row.platformText = algoTable->item(r, 1)->text();
        row.modeText     = algoTable->item(r, 2)->text();
        row.periodText   = algoTable->item(r, 3)->text();
        row.batteryText  = algoTable->item(r, 4)->text();
        row.algoData     = algoTable->item(r, 0)->data(Qt::UserRole);
        row.platformData = algoTable->item(r, 1)->data(Qt::UserRole);
        row.modeData     = algoTable->item(r, 2)->data(Qt::UserRole);
        row.periodValue  = algoTable->item(r, 3)->data(Qt::UserRole).toInt();
        row.batteryData  = algoTable->item(r, 4)->data(Qt::UserRole);
        savedAlgoTableRows.append(row);
    }

    // ── Clear previous algo tabs before creating new ones ─────────────────────
    clearAlgoTabs();

    // ── Read general settings ─────────────────────────────────────────────────
    isOfflineMode    = offlineAnalysys->isChecked();
    selectedPlatform = platformCombo->currentText();  // used for display only

    // ── Read file paths ───────────────────────────────────────────────────────
    currPath        = currentPath->text();
    polynomsPath    = polyPath->text();
    ocvSOCPath      = ocvSocPath->text();
    parametersPath  = paramsPath->text();
    this->noisePath = noisePath->text();
    this->flagPath  = flagsPath->text();

    // ── Apply dock visibility ─────────────────────────────────────────────────
    currentBatteryDock->setVisible(showCurrentBattery->isChecked());
    currentSystemDock->setVisible(showCurrentSystem->isChecked());
    currentPlatformDock->setVisible(showCurrentPlatform->isChecked());
    voltageDock->setVisible(showVoltage->isChecked());
    socDock->setVisible(showSoc->isChecked());
    redistributeDocks();

    // ── Clear all plots ───────────────────────────────────────────────────────
    currentBatteryPlot->clear();
    currentSystemPlot->clear();
    currentPlatformPlot->clear();
    voltagePlot->clear();
    socPlot->clear();

    // ── Online mode ───────────────────────────────────────────────────────────
    if (!isOfflineMode) {

        // Online mode uses only the first algo in the table
        algoTimeTableDuration_e algo     = selectedAlgos.first();
        platform_current_mah_e  platform = selectedPlatforms.first();

        // Auto-select flags file based on algo type
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

        // ── Offline mode ──────────────────────────────────────────────────────────
    } else {

        // Create one SimulatorContainer per algo row in the table
        for (int r = 0; r < selectedAlgos.size(); r++) {
            algoTimeTableDuration_e algo     = selectedAlgos[r];
            platform_current_mah_e  platform = selectedPlatforms[r];
            // algoConfig_t         cfg      = selectedConfigs[r];  // TODO: pass to container

            algoTab_t tab;
            tab.algo         = algo;
            tab.algoName     = algoTabName(algo);
            tab.platformName = algoTable->item(r, 1)->text();  // ← iz tabele, per-row!
            tab.platformEnum = platform;

            // Each tab gets its own BatteryModel and TimeTable to avoid race conditions
            BatteryModel    *tabBatteryModel    = new BatteryModel(this);
            TimeTable       *tabTimeTable       = new TimeTable(algo, this);
            PlatformCurrent *tabPlatformCurrent = new PlatformCurrent(platform, this);

            tab.batteryModel    = tabBatteryModel;
            tab.timeTable       = tabTimeTable;
            tab.platformCurrent = tabPlatformCurrent;

            tab.container = new SimulatorContainer(
                tabBatteryModel, tabPlatformCurrent, tabTimeTable, this);

            // Set all file paths for this container
            tab.container->setCurrentPath(currPath);
            tab.container->setOcvPolyPath(polynomsPath);
            tab.container->setOcvSocPath(ocvSOCPath);
            tab.container->setParamsPath(parametersPath);
            tab.container->setNoisePath(this->noisePath);
            tab.container->setFlagsPath(algoFlagsPath(algo));  // flags per algo type
            tab.container->setOutputPath(algoOutputPath(algo));
            tab.container->setAlgoSelected(algo);
            tab.container->setOfflineMode(true);
            tab.container->setPlatformCurrent(platform);

            connect(tab.container, &SimulatorContainer::loadError, this,
                    [=](const QString &msg){ qDebug() << "Load error:" << msg; });

            algoTabs.append(tab);
        }

        // Load files for all containers
        for (auto &tab : algoTabs) {
            if (!tab.container->loadFiles()) return;
            tab.container->reset(initialSoC);
        }

        speedUpBtn->setEnabled(false);
        speedUpBtn->setText("Offline");
    }

    // ── Final UI state ────────────────────────────────────────────────────────
    playBtn->setEnabled(true);
    setLed(ledConfigLabel, "green");
    statusConfigLabel->setText("Configured");
}


/*******************************************************************************
 * onSimulationSettingsClicked
 ******************************************************************************/
void SimulatorWnd::onSimuSettingsClicked()
{
    // Check if there is a algo
    if (algoTabs.isEmpty()) {
        // Error for not chosing simulation params
        QMessageBox::information(this, "No Simulation", "Run a simulation first.");
        return;
    }

    // Creates qDialog for new window
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Simulation Display Settings");
    dialog->resize(400, 350);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Algorithm  group
    QGroupBox *algoGroup = new QGroupBox("Show Algorithms Plots");
    QVBoxLayout *algoLayout = new QVBoxLayout();

    // Simulation output group
    QGroupBox *outputGroup = new QGroupBox("Show Outputs Plots");
    QVBoxLayout *outputLayout = new QVBoxLayout();

    // List of checkboxes
    QList<QCheckBox*> compareCheckboxes;

    // CheckBox list for algo
    for (int i = 0; i < algoTabs.size(); i++) {
        // New checkbox
        QCheckBox *cb = new QCheckBox(algoTabs[i].algoName + " " + "[" + algoTabs[i].platformName + "]");

        // This is restoring previous state
        bool checked = (i < visibleAlgosInCompare.size()) ? visibleAlgosInCompare[i] : true;

        // Active cb set checked if it was previously setted
        cb->setChecked(checked);

        // Add to VBox
        algoLayout->addWidget(cb);

        // Save current state of checkbox for next time when they open window
        compareCheckboxes.append(cb);
    }

    // List of checkboxes
    QList<QCheckBox*> outputCheckboxes;

    for(int i = 0; i < 1; i++){
        QCheckBox *checkBoxOutput = new QCheckBox("Output SoC Difference");

        // Tihis is restoring previous state
        bool checkedOutput = (i < visibleOutputInSimulationSettings.size()) ? visibleOutputInSimulationSettings[i] : true;

        checkBoxOutput->setChecked(checkedOutput);

        // Add to output layout
        outputLayout->addWidget(checkBoxOutput);

        // Save current state of checkbox for next time when they open this window
        outputCheckboxes.append(checkBoxOutput);
    }

    // Add to visibility group
    algoGroup->setLayout(algoLayout);

    // Add to output group
    outputGroup->setLayout(outputLayout);

    // Add visibility group to the main layout
    mainLayout->addWidget(algoGroup);
    mainLayout->addWidget(outputGroup);

    // Strech it
    mainLayout->addStretch();

    // Buttons for accept or reject chosen articles
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // Add them in horizontal layout
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(buttons);

    // Add them in main layout
    mainLayout->addLayout(btnLayout);

    // Depends on clicked button
    if (dialog->exec() != QDialog::Accepted)
        return;

    // Save this state
    visibleAlgosInCompare.clear();
    for (int i = 0; i < compareCheckboxes.size(); i++)
        visibleAlgosInCompare.append(compareCheckboxes[i]->isChecked());

    visibleOutputInSimulationSettings.clear();
    for (int i = 0; i < outputCheckboxes.size(); i++)
        visibleOutputInSimulationSettings.append(outputCheckboxes[i]->isChecked());

    // Replot active tab with configured algo that we checked
    replotActiveTab();

    if (!outputCheckboxes.isEmpty() && outputCheckboxes[0]->isChecked()) {
        emit sigOpenOuputWindow();
    }
}


void SimulatorWnd::onOutputSimulationChecked(){
    QDialog *dialog = new QDialog(this);
    dialog->resize(800, 800);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    static const QList<QColor> colors = {
        QColor(40,  110, 255), QColor(255, 100,   0), QColor(0,   180,   0),
        QColor(180,   0, 180), QColor(255,   0,   0), QColor(0,   200, 200),
        QColor(255, 200,   0), QColor(0,   100, 100), QColor(255,   0, 150),
        QColor(100, 100,   0), QColor(0,    50, 200), QColor(200, 100,  50),
        QColor(100, 200,   0), QColor(200,   0,  50), QColor(0,   150, 255),
        QColor(150,   0, 255), QColor(255, 150,   0), QColor(0,   200, 100),
        QColor(200, 200,   0), QColor(100,   0, 200),
    };

    // Create plots for this window
    socErrorDiffPlot    = new Plot(400, 100, false, dialog);

    // SoC algo diff plot
    socErrorDiffPlot->setTitle("SoC Error Difference");
    socErrorDiffPlot->setYLabel("[%]");
    socErrorDiffPlot->setXLabel("[s]");

    layout->addWidget(socErrorDiffPlot);

    socErrorDiffPlot->clearAllGraphs();


    socErrorDiffPlot->enableLegend(true);

    // This info is for plot on soc error difference
    QVector<double> socError = {0};

    int plotIdx = 0;

    // Plot all other graphs
    for (int i = 0; i < algoTabs.size(); i++) {

        // Check if algo is selected in simulation settings
        bool show = (i < visibleAlgosInCompare.size()) ? visibleAlgosInCompare[i] : true;

        // If it's not selected skip algo, doesn't need to be plotted
        if (!show) continue;


        // take algo tab
        const algoTab_t &tab = algoTabs[i];
        QColor colorAlgo   = colors[i % colors.size()];
        QColor colorSoCBat = colorAlgo.lighter(140);

        // Reload the value
        socError = {0};

        for(int j = 0; j < tab.simulatorSample.soc.algoSoc.size(); j++){
            socError.append(tab.simulatorSample.soc.algoSoc[j] - tab.simulatorSample.soc.battery[j]);
        }


        // ── Voltage, Battery & Platform Current ───────────────────────────────────────────────────
        if (plotIdx == 0) {
            // Soc Error Difference
            socErrorDiffPlot->setGraphName(0, tab.algoName);
            socErrorDiffPlot->setGraphLineWidth(0, 2);
            socErrorDiffPlot->setData(socError, tab.simulatorSample.time);

        } else {
            // Soc Error Difference
            socErrorDiffPlot->addLineGraph(colorAlgo, tab.algoName);
            socErrorDiffPlot->setGraphLineWidth(plotIdx, 2);
            socErrorDiffPlot->setGraphData(plotIdx, socError, tab.simulatorSample.time);
        }
        plotIdx++;
    }

    // Replot all graphs
    socErrorDiffPlot->replotAll();

    dialog->show();
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

    // For offline analysys
    if(isOfflineMode && !algoTabs.isEmpty()){

        // Create QTab
        QTabWidget *tabWidget = new QTabWidget(dialogWindow);

        // Offline analisys
        for (int i = 0; i < algoTabs.size(); i++) {
            const algoTab_t &tab = algoTabs[i];

            // Fulfill the table
            QTableWidget *table = createResultsTable(
                tab.avgBatteryCurr,
                tab.avgSystemCurr,
                tab.container->getLastValuesOfSimulationStep(),
                tab.container->getTimeS(),
                tab.container->getNumberOfSamples(),
                tab.algoName,
                tab.platformName);

            // That table push into current tab
            tabWidget->addTab(table, tab.algoName + " " +"[" + tab.platformName + "]");
        }

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
                "",
                selectedPlatform
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
    // Run offline simulation
    if (isOfflineMode) {
        qDebug() << "Run Offline Simulation";
        startOfflineSimulation();
    }
    else {
        qDebug() << "Run Online Simulation";
        currentSimState = SIMULATION_PLAYING;

        // Enable buttons
        playBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        pauseBtn->setEnabled(true);
        speedUpBtn->setEnabled(true);
        configBtn->setEnabled(false);

        // Update status bar
        setLed(ledPlaySimulationLabel, "green");
        statusPlaySimulationLabel->setText("Running");

        // Start timer for online simulation
        timer->start(timerPeriodMs);

        // Emit signal that we clicked play btn
        emit sigPlay();
    }
}

/*******************************************************************************
 * onStopClicked
 ******************************************************************************/
void SimulatorWnd::onStopClicked()
{
    // Stop simulation
    currentSimState = SIMULATION_STOPPED;
    qDebug() << "Stop Simulation";

    // Stop the timer
    timer->stop();

    // Reset all tabs containers to the init values
    if (isOfflineMode) {
        for (auto &tab : algoTabs)
            tab.container->reset(initialSoC);
    }
    else {
        simulatorContainer->reset(initialSoC);
    }

    // Clear all graphs
    currentBatteryPlot->clearAllGraphs();
    currentSystemPlot->clearAllGraphs();
    currentPlatformPlot->clearAllGraphs();
    voltagePlot->clearAllGraphs();
    socPlot->clearAllGraphs();
    socErrorDiffPlot->clearAllGraphs();

    // Disable legends for all graphs
    currentPlatformPlot->enableLegend(false);
    socPlot->enableLegend(false);
    voltagePlot->enableLegend(false);
    currentBatteryPlot->enableLegend(false);
    socErrorDiffPlot->enableLegend(false);

    // Restore for online mode btn and timer period
    if (!isOfflineMode) {
        speedUpBtn->setText("1x");
        timerPeriodMs = 10;
    }

    // Buttons enable
    playBtn->setEnabled(true);   // Enable play to redo simu
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true); // Enable config
    logInfoBtn->setEnabled(false);
    simulationSettingsBtn->setEnabled(false);

    // Status bar update
    setLed(ledPlaySimulationLabel, "red");
    statusPlaySimulationLabel->setText("Stopped");
    progressBar->setValue(0);
    currentSample = 0;

    // Emit stop signal
    emit sigStop();
}

/*******************************************************************************
 * onPauseClicked
 ******************************************************************************/
void SimulatorWnd::onPauseClicked()
{
    // If timer is active stop it
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
    // Offline skip
    if (isOfflineMode) return;

    // Online analysis change period of timer, speed it up
    if      (timerPeriodMs == 10) { speedUpBtn->setText("2x"); timerPeriodMs = 5;  }
    else if (timerPeriodMs == 5)  { speedUpBtn->setText("5x"); timerPeriodMs = 2;  }
    else                          { speedUpBtn->setText("1x"); timerPeriodMs = 10; }
    if (timer->isActive()) { timer->stop(); timer->start(timerPeriodMs); }

    // Emit signal
    emit sigSpeedUp();
}

/*******************************************************************************
 * startOfflineSimulation - each algo in its own thread, parallel
 ******************************************************************************/
void SimulatorWnd::startOfflineSimulation()
{
    // Disable buttons
    playBtn->setEnabled(false);
    configBtn->setEnabled(false);

    // Update status bar
    setLed(ledPlaySimulationLabel, "green");
    statusPlaySimulationLabel->setText("Offline Processing");

    // Take tab count number
    int tabCount = algoTabs.size();
    QSharedPointer<QAtomicInt> finishedCount(new QAtomicInt(0));

    // Go through every tab and run the simulation for every algo type
    for (int i = 0; i < tabCount; i++) {

        // Reset current container of each algo
        algoTabs[i].container->reset(initialSoC);

        // Set all values to the init state
        algoTabs[i].avgBatteryCurr = 0.0;
        algoTabs[i].avgSystemCurr  = 0.0;
        algoTabs[i].simulatorSample.time.clear();
        algoTabs[i].simulatorSample.voltage.battery.clear();
        algoTabs[i].simulatorSample.current.battery.clear();
        algoTabs[i].simulatorSample.current.system.clear();
        algoTabs[i].simulatorSample.current.platform.clear();
        algoTabs[i].simulatorSample.soc.battery.clear();
        algoTabs[i].simulatorSample.soc.system.clear();
        algoTabs[i].simulatorSample.soc.algoSoc.clear();

        // Run the algo on each thread
        QtConcurrent::run([this, i, tabCount, finishedCount]()
          {
              // Take the current pointer for each algo container
              SimulatorContainer *container = algoTabs[i].container;

              // Normalize soc value
              double endSocPct = endSoC * 100.0;

              // Vectors for saving all samples
              QVector<double> vTime, vBat, vIbat, vISys, vIPlatform, vSoc, vSocIsys, vAlgoSoc;

              // Initial average current values
              double avgCurrBat = 0.0;
              double avgCurrSys = 0.0;

              // Current local sample
              int localSample = 0;

              // Infinity loop for each algo
              while (!container->isFinished()) {

                  // Do a step
                  container->step();

                  // Take every sample for each algo
                  double soc = container->getSoC();
                  vTime.push_back(container->getTimeS());
                  vBat.push_back(container->getVBat());
                  vIbat.push_back(container->getIBat());
                  vISys.push_back(container->getISys());
                  vIPlatform.push_back(container->getIPlatform());
                  vSoc.push_back(soc);
                  vSocIsys.push_back(container->getSoCIsys());
                  vAlgoSoc.push_back(container->getAlgoSoc() * 100.0);

                  // Accumulate value for average current
                  avgCurrBat += container->getIBat();
                  avgCurrSys += container->getISys();

                  // Increment local sample
                  localSample++;

                  // If condition from config are reached break simulation for that algo
                  if (soc <= endSocPct) break;

                  // Update progress bar
                  if (i == 0 && localSample % 1000 == 0) {
                      int pct = (localSample * 100) / container->getNumberOfSamples();
                      QMetaObject::invokeMethod(this, [=]() {
                          progressBar->setValue(pct);
                      });
                  }
              }

              // Calculate average Currents for info window
              if (localSample > 0) {
                  avgCurrBat /= localSample;
                  avgCurrSys /= localSample;
              }

              // Back to main thread: store data and replot if active tab
              QMetaObject::invokeMethod(this,
                                        [this, i, avgCurrBat, avgCurrSys,
                                         vTime, vBat, vIbat, vISys, vIPlatform, vSoc, vSocIsys, vAlgoSoc,
                                         tabCount, finishedCount]()
                {
                    // Fill the structs with each algo value
                    algoTabs[i].avgBatteryCurr                   = avgCurrBat;
                    algoTabs[i].avgSystemCurr                    = avgCurrSys;
                    algoTabs[i].simulatorSample.time             = vTime;
                    algoTabs[i].simulatorSample.voltage.battery  = vBat;
                    algoTabs[i].simulatorSample.current.battery  = vIbat;
                    algoTabs[i].simulatorSample.current.system   = vISys;
                    algoTabs[i].simulatorSample.current.platform = vIPlatform;
                    algoTabs[i].simulatorSample.soc.battery      = vSoc;
                    algoTabs[i].simulatorSample.soc.system       = vSocIsys;
                    algoTabs[i].simulatorSample.soc.algoSoc      = vAlgoSoc;

                    // Print in log tab that algo is finished
                    qDebug() << "Tab" << algoTabs[i].algoName << "finished";

                    int done = finishedCount->fetchAndAddOrdered(1) + 1;

                    // Check if every algo finished the simulation
                    if (done == tabCount) {
                        visibleAlgosInCompare.clear();
                        for (int i = 0; i < algoTabs.size(); i++)
                            visibleAlgosInCompare.append(true);

                        // Update status bar
                        progressBar->setValue(100);
                        setLed(ledPlaySimulationLabel, "green");
                        statusPlaySimulationLabel->setText("Offline Done");

                        // Buttons enable
                        playBtn->setEnabled(false);
                        stopBtn->setEnabled(true);
                        configBtn->setEnabled(true);
                        logInfoBtn->setEnabled(true);
                        simulationSettingsBtn->setEnabled(true);
                        pauseBtn->setEnabled(false);
                        speedUpBtn->setEnabled(false);

                        // Print log console
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

/*******************************************************************************
 * Helpers for Simulation Window file
 ******************************************************************************/

void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    // If it's called and empty return
    if (!g_logConsole) return;

    // Add prefix for each message
    QString prefix;

    // Select type of message
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG] "; break;
    case QtWarningMsg:  prefix = "[WARN]  "; break;
    case QtCriticalMsg: prefix = "[ERROR] "; break;
    default: break;
    }

    // Print in console
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
    const QString &algoName,
    const QString &platformName)
{
    // Creating table dynamicly
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

    // Adding separators for each main thing in the table
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

    // Add a row in a table
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

    // With platform name extracts real value of platform current
    platform_current_mah_e pc = PLATFORM_CURRENT_ESP;
    if      (platformName == "NXP")   pc = PLATFORM_CURRENT_NXP;
    else if (platformName == "STM32") pc = PLATFORM_CURRENT_STM;
    else if (platformName == "NRF")   pc = PLATFORM_CURRENT_NRF;
    float platformCurrent = static_cast<float>(pc);

    /* This is how much time did pass due whole simulation depends on sampling period
     and number of samples every simulation */
    QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(timeElapsed * 1000));
    QString formattedTime = time.toString("hh'h' mm'm' ss's'");

    /* Calculates system soc based on config settings of initial soc and from
     * end of simulation system soc */
    double deltaSOC    = (initialSoC * 100.0) - finalResults.lastSoC;
    double capacityMah = 0.0;

    // Calculates capacity of battery based on delta soc
    if (deltaSOC > 0.0)
        capacityMah = (finalResults.lastElectricChargeBattery / deltaSOC) * 100.0;

    double avgSysCurrMa = avgSys * 1000.0; // [A] -> [mA]
    double avgBatCurrMa = avgBat * 1000.0; // [A] -> [mA]

    // Calculates operational times based on average current and capacity of batteries
    float maxOpTime = (avgSysCurrMa > 0.0) ? (float)((capacityMah / avgSysCurrMa) * 3600.0) : 0.0f;
    float sysOpTime = (avgBatCurrMa > 0.0) ? (float)((capacityMah / avgBatCurrMa) * 3600.0) : 0.0f;
    float algoCost  = maxOpTime - sysOpTime;

    // Create table with variables that we calculate
    addSeparator("General");
    addRow("Platform",      platformName);
    if (!algoName.isEmpty()) addRow("Algorithm", algoName);
    addRow("Total Samples", QString::number(totalSamples));

    // Soc things
    addSeparator("State of Charge");
    addRow("Initial SoC Battery", QString("%1 %").arg(initialSoC * 100.0, 0, 'f', 1));
    addRow("Final SoC Battery",   QString("%1 %").arg(finalResults.lastSoC, 0, 'f', 2));
    addRow("Initial SoC System",  QString("%1 %").arg(initialSoC * 100.0, 0, 'f', 1));
    addRow("Final SoC System",    QString("%1 %").arg(finalResults.lastSoCIsys, 0, 'f', 2));

    // Voltage things
    addSeparator("Voltage");
    addRow("Final Terminal Voltage", QString("%1 V").arg(finalResults.lastVBat, 0, 'f', 4));

    // Current things
    addSeparator("Current");
    addRow("Avg Battery Current", QString("%1 mA").arg(avgBatCurrMa, 0, 'f', 4));
    addRow("Avg System Current",  QString("%1 mA").arg(avgSysCurrMa, 0, 'f', 4));
    addRow("Platform Current",    QString("%1 mA").arg(platformCurrent,  0, 'f', 4));

    // Electric charges things
    addSeparator("Electric Charge");
    addRow("Battery Capacity", QString("%1 mAh").arg(capacityMah, 0, 'f', 2));
    addRow("Charge Battery",   QString("%1 mAh").arg(finalResults.lastElectricChargeBattery, 0, 'f', 4));
    addRow("Charge System",    QString("%1 mAh").arg(finalResults.lastElectricChargeSystem,  0, 'f', 4));
    addRow("Charge Algo",      QString("%1 mAh").arg(finalResults.lastElectricChargeAlgo,    0, 'f', 4));

    // Time simulations things
    addSeparator("Time");
    addRow("Simulation Time",    formattedTime);
    addRow("Maximal Oper. Time", formatSeconds(maxOpTime));
    addRow("System Oper. Time",  formatSeconds(sysOpTime));
    addRow("Algo Time Cost",     formatSeconds(algoCost), true);

    return table;
}

static QString algoTabName(algoTimeTableDuration_e algo)
{
    // Based on time table gives exact name of algo
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
    // Gives hardcoded file paths for each algo
    switch (algo) {
    case K0:  return "/home/filip/Projects/Master/Params/flags_k0.csv";
    case K2:  return "/home/filip/Projects/Master/Params/flags_k2.csv";
    case LP:  return "/home/filip/Projects/Master/Params/flags_lp.csv";
    default:  return "/home/filip/Projects/Master/Params/flags_adaptive.csv";
    }
}

static QString algoOutputPath(algoTimeTableDuration_e algo)
{
    // Hardcoded output path based on time table of algo
    return QString("/home/filip/Projects/Master/Output/output_%1.csv")
        .arg(algoTabName(algo));
}
