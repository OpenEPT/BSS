/**
 * @file    SimulatorWindow.cpp
 * @brief   Simulator window implementation - UI and simulation control
 * @version 1.0.0
 * @date    2026-04-16
 * @author  Filip Radojevic
 */

#include "SimulatorWindow.h"
#include <QtConcurrent>

/*******************************************************************************
 * Global log console pointer - used by Qt message handler
 ******************************************************************************/

QTextEdit *g_logConsole = nullptr;

/*******************************************************************************
 * Qt message handler - redirects qDebug/qWarning/qCritical to log console
 ******************************************************************************/

void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    if (!g_logConsole) return;

    QString prefix;
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG] "; break;
    case QtWarningMsg:  prefix = "[WARN]  "; break;
    case QtCriticalMsg: prefix = "[ERROR] "; break;
    default:            break;
    }

    QMetaObject::invokeMethod(g_logConsole, "append",
                              Qt::QueuedConnection,
                              Q_ARG(QString, prefix + msg));
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
    // Simulator container constructor
    simulatorContainer = new SimulatorContainer(this);


    // Timer for online analysys of simulator
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SimulatorWnd::onTimerTick);


    // Led Timer for blinking
    ledTimer = new QTimer(this);
    ledTimer->setInterval(500);
    ledTimer->start();
    connect(ledTimer, &QTimer::timeout, this, &SimulatorWnd::onLedTimerTimeout);


    // Toolbar
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);


    // Buttons
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




    // Initial button state
    playBtn->setEnabled(false);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true);
    logInfoBtn->setEnabled(false);


    // Add buttons to toolBar
    toolBar->addWidget(playBtn);
    toolBar->addWidget(stopBtn);
    toolBar->addWidget(pauseBtn);
    toolBar->addWidget(speedUpBtn);
    toolBar->addWidget(configBtn);
    toolBar->addWidget(logInfoBtn);


    // Plots
    currentBatteryPlot   = new Plot(400, 100, false, this);
    currentSystemPlot    = new Plot(400, 100, false, this);
    currentPlatformPlot  = new Plot(400, 100, false, this);
    voltagePlot          = new Plot(400, 100, false, this);
    socPlot              = new Plot(400, 100, false, this, true);

    // Current Battery Plot
    currentBatteryPlot->setTitle("Battery Current");
    currentBatteryPlot->setYLabel("[A]");
    currentBatteryPlot->setXLabel("[s]");


    // Current System Plot
    currentSystemPlot->setTitle("System Current");
    currentSystemPlot->setYLabel("[A]");
    currentSystemPlot->setXLabel("[s]");


    // Current Platform Plot
    currentPlatformPlot->setTitle("Platform Current");
    currentPlatformPlot->setYLabel("[A]");
    currentPlatformPlot->setXLabel("[s]");


    // Terminal Voltage Plot
    voltagePlot->setTitle("Battery Voltage");
    voltagePlot->setYLabel("[V]");
    voltagePlot->setXLabel("[s]");

    // Soc PLot
    socPlot->setTitle("Battery SoC");
    socPlot->setYLabel("[%]");
    socPlot->setXLabel("[s]");
    socPlot->setGraphName(0, "SoC Battery");
    socPlot->addLineGraph(QColor(255, 100, 0), "SoC Isys");

    // Plot sizes
    currentBatteryPlot->setMinimumHeight(100);
    currentSystemPlot->setMinimumHeight(100);
    currentPlatformPlot->setMinimumHeight(100);
    voltagePlot->setMinimumHeight(100);
    socPlot->setMinimumHeight(100);

    // Log console
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setMaximumHeight(150);
    logConsole->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: monospace;");
    g_logConsole = logConsole;
    qInstallMessageHandler(logHandler);


    // Dock widgets
    currentBatteryDock = new QDockWidget("Battery Current", this);
    currentBatteryDock->setWidget(currentBatteryPlot);
    currentBatteryDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    currentSystemDock = new QDockWidget("System Current", this);
    currentSystemDock->setWidget(currentSystemPlot);
    currentSystemDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    currentPlatformDock = new QDockWidget("Platform Current", this);
    currentPlatformDock->setWidget(currentPlatformPlot);
    currentPlatformDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    voltageDock = new QDockWidget("Battery Voltage", this);
    voltageDock->setWidget(voltagePlot);
    voltageDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    socDock = new QDockWidget("SOC", this);
    socDock->setWidget(socPlot);
    socDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    logDock = new QDockWidget("Log Console", this);
    logDock->setWidget(logConsole);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);


    addDockWidget(Qt::LeftDockWidgetArea,  currentBatteryDock);
    addDockWidget(Qt::LeftDockWidgetArea,  currentSystemDock);
    addDockWidget(Qt::LeftDockWidgetArea,  currentPlatformDock);
    addDockWidget(Qt::RightDockWidgetArea, voltageDock);
    addDockWidget(Qt::RightDockWidgetArea, socDock);

    splitDockWidget(currentBatteryDock,  currentSystemDock,   Qt::Vertical);
    splitDockWidget(currentSystemDock,   currentPlatformDock, Qt::Vertical);
    splitDockWidget(voltageDock,         socDock,             Qt::Vertical);

    resizeDocks(
        {currentBatteryDock, currentSystemDock, currentPlatformDock, voltageDock, socDock},
        {200, 200, 200, 200, 200},
        Qt::Vertical
        );
    resizeDocks(
        {currentBatteryDock, voltageDock},
        {700, 700},
        Qt::Horizontal
        );

    resize(1600, 1000);


    // Status Bar
    statusBar()->setStyleSheet(
        "border-top: 1px solid palette(mid);"
        "padding: 2px;"
        );

    // Leds
    ledConfigLabel    = new QLabel(this);
    ledPlaySimulationLabel = new QLabel(this);

    // Labels bonded to leds
    statusConfigLabel    = new QLabel(this);
    statusPlaySimulationLabel = new QLabel(this);

    // Progress Bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedWidth(200);
    progressBar->setFixedHeight(25);
    progressBar->setTextVisible(true);

    // Init leds
    ledConfigLabel->setFixedSize(16, 16);
    ledPlaySimulationLabel->setFixedSize(16,16);
    setLed(ledConfigLabel, "grey");
    setLed(ledPlaySimulationLabel, "grey");

    // Init labels
    statusConfigLabel->setText("Not Configured");
    statusPlaySimulationLabel->setText("Not Initialized");

    statusBar()->addWidget(statusPlaySimulationLabel);
    statusBar()->addWidget(ledPlaySimulationLabel);
    statusBar()->addWidget(statusConfigLabel);
    statusBar()->addWidget(ledConfigLabel);
    statusBar()->addPermanentWidget(progressBar);


    // Button connects
    connect(playBtn,    &QPushButton::clicked,  this, &SimulatorWnd::onPlayClicked);
    connect(stopBtn,    &QPushButton::clicked,  this, &SimulatorWnd::onStopClicked);
    connect(pauseBtn,   &QPushButton::clicked,  this, &SimulatorWnd::onPauseClicked);
    connect(speedUpBtn, &QPushButton::clicked,  this, &SimulatorWnd::onSpeedUpClicked);
    connect(configBtn,  &QToolButton::clicked,  this, &SimulatorWnd::onConfigClicked);
    connect(logInfoBtn, &QToolButton::clicked,  this, &SimulatorWnd::onLogInfoClicked);

    connect(simulatorContainer, &SimulatorContainer::dataSample, this,
            [=](float timeS, double vBat, float iSys, float iPlatform, float iBat, float socPct, float socIsysPct) {
                if (socPct <= endSoC) {
                    timer->stop();
                    playBtn->setEnabled(false);
                    stopBtn->setEnabled(true);
                    pauseBtn->setEnabled(false);
                    speedUpBtn->setEnabled(false);
                    qDebug() << "Simulation finished, reached end SoC";
                    return;
                }else{
                    currentBatteryPlot->appendData  ({(double)iBat},      {(double)timeS});
                    currentSystemPlot->appendData   ({(double)iSys},      {(double)timeS});
                    currentPlatformPlot->appendData ({(double)iPlatform}, {(double)timeS});
                    voltagePlot->appendData         ({vBat},              {(double)timeS});
                    socPlot->appendData             ({(double)socPct*100.0},    {(double)timeS});
                    socPlot->appendData2({(double)socIsysPct},  {(double)timeS});  // graph(1) - SoC Isys
                    currentSample++;
                    int pct = (int)((float)currentSample / simulatorContainer->getNumberOfSamples() * 1000.0f);
                    progressBar->setValue(pct);
                }
            }, Qt::QueuedConnection);

    connect(simulatorContainer, &SimulatorContainer::loadSuccess, this,
            [=]{ playBtn->setEnabled(true); });

    connect(simulatorContainer, &SimulatorContainer::loadError, this,
            [=](const QString &msg){ qDebug() << "Load error:" << msg; });

}

/*******************************************************************************
 * Slot - Simulation timer tick - advances simulation by one step
 ******************************************************************************/

void SimulatorWnd::onTimerTick()
{
    // Next step
    simulatorContainer->step();
}

/*******************************************************************************
 * Slot - Config button clicked - opens configuration dialog
 ******************************************************************************/

void SimulatorWnd::onConfigClicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Config");
    dialog->resize(1100, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    QHBoxLayout *groupsLayout = new QHBoxLayout();

    // ================= GROUP BOXES =================
    QGroupBox *graphicsGroup = new QGroupBox("Choose Graphics");
    QGroupBox *inputFilesGroup = new QGroupBox("Choose Files");
    inputFilesGroup->setFixedWidth(600);

    QGroupBox *algoGroup = new QGroupBox("Choose Algo");
    QGroupBox *analysisGroupBox = new QGroupBox("Choose Analysis");
    QGroupBox *choosePlatformGroupBox = new QGroupBox("Choose Platform");

    // ================= LAYOUTS =================
    QVBoxLayout *leftGroup = new QVBoxLayout();
    QVBoxLayout *rightFilesGroup = new QVBoxLayout();
    QVBoxLayout *rightAlgoGroup = new QVBoxLayout();
    QVBoxLayout *analysisLayout = new QVBoxLayout();
    QVBoxLayout *choosePlatformLayout = new QVBoxLayout();

    // ================= RADIO BUTTONS =================
    QRadioButton *onlineAnalysys = new QRadioButton("Online Analysis");
    QRadioButton *offlineAnalysys = new QRadioButton("Offline Analysis");

    // grupisanje (samo jedan aktivan)
    QButtonGroup *analysisBtnGroup = new QButtonGroup(dialog);
    analysisBtnGroup->addButton(onlineAnalysys);
    analysisBtnGroup->addButton(offlineAnalysys);

    onlineAnalysys->setChecked(true);

    analysisLayout->addWidget(onlineAnalysys);
    analysisLayout->addWidget(offlineAnalysys);
    analysisGroupBox->setLayout(analysisLayout);

    // ================= COMBO BOX =================
    QComboBox *algoComboBox = new QComboBox();
    algoComboBox->addItem("K0 slow");
    algoComboBox->addItem("K0 fast");
    algoComboBox->addItem("K2 slow");
    algoComboBox->addItem("K2 fast");
    algoComboBox->addItem("Adaptive slow");
    algoComboBox->addItem("Adaptive fast");

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

    rightAlgoGroup->addWidget(algoComboBox);
    algoGroup->setLayout(rightAlgoGroup);

    // ================= CHECKBOXES =================
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

    // ================= FILE BUTTONS =================
    QPushButton *loadCurrentBtn = new QPushButton("Current");
    QLineEdit *currentPath = new QLineEdit(currPath);
    currentPath->setReadOnly(true);

    QPushButton *loadPolyBtn = new QPushButton("Poly");
    QLineEdit *polyPath = new QLineEdit(polynomsPath);
    polyPath->setReadOnly(true);

    QPushButton *loadOcvSocCurveBtn = new QPushButton("OCV");
    QLineEdit *ocvSocPath = new QLineEdit(ocvSOCPath);
    ocvSocPath->setReadOnly(true);

    QPushButton *loadParamsBtn = new QPushButton("Params");
    QLineEdit *paramsPath = new QLineEdit(parametersPath);
    paramsPath->setReadOnly(true);

    QPushButton *loadNoisesBtn = new QPushButton("Noise");
    QLineEdit *noisePath = new QLineEdit(this->noisePath);
    noisePath->setReadOnly(true);

    auto createRow = [&](QPushButton *btn, QLineEdit *edit) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setContentsMargins(4, 2, 4, 2);  // levo, gore, desno, dole
        row->addWidget(btn);
        row->addWidget(edit);
        rightFilesGroup->addLayout(row);
    };

    rightFilesGroup->setSpacing(6);
    rightFilesGroup->setContentsMargins(8, 8, 8, 8);

    createRow(loadCurrentBtn, currentPath);
    rightFilesGroup->addStretch();
    createRow(loadPolyBtn, polyPath);
    rightFilesGroup->addStretch();
    createRow(loadOcvSocCurveBtn, ocvSocPath);
    rightFilesGroup->addStretch();
    createRow(loadParamsBtn, paramsPath);
    rightFilesGroup->addStretch();
    createRow(loadNoisesBtn, noisePath);
    rightFilesGroup->addStretch();

    inputFilesGroup->setLayout(rightFilesGroup);

    // ================= SOC GROUP =================
    QGroupBox *socGroup = new QGroupBox("SoC Settings");
    QVBoxLayout *socLayout = new QVBoxLayout();
    socLayout->setSpacing(6);
    socLayout->setContentsMargins(8, 8, 8, 8);

    // Initial SoC
    QHBoxLayout *initSocRow = new QHBoxLayout();
    QLabel *initSocLabel = new QLabel("Initial SoC:");
    QDoubleSpinBox *initSocSpinBox = new QDoubleSpinBox();
    initSocSpinBox->setRange(0.0, 100.0);
    initSocSpinBox->setValue(initialSoC);
    initSocSpinBox->setSuffix(" %");
    initSocSpinBox->setDecimals(1);
    initSocSpinBox->setFixedWidth(80);
    initSocRow->addWidget(initSocLabel);
    initSocRow->addWidget(initSocSpinBox);

    // End SoC
    QHBoxLayout *endSocRow = new QHBoxLayout();
    QLabel *endSocLabel = new QLabel("End SoC:");
    QDoubleSpinBox *endSocSpinBox = new QDoubleSpinBox();
    endSocSpinBox->setRange(0.0, 100.0);
    endSocSpinBox->setValue(endSoC);
    endSocSpinBox->setSuffix(" %");
    endSocSpinBox->setDecimals(1);
    endSocSpinBox->setFixedWidth(80);
    endSocRow->addWidget(endSocLabel);
    endSocRow->addWidget(endSocSpinBox);

    socLayout->addLayout(initSocRow);
    socLayout->addLayout(endSocRow);
    socLayout->addStretch();
    socGroup->setLayout(socLayout);

    // ================= BUTTONS =================
    QDialogButtonBox *buttonsOkCancel =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonsOkCancel);

    // ================= MAIN LAYOUT =================
    groupsLayout->addWidget(graphicsGroup, 1);
    groupsLayout->addWidget(inputFilesGroup, 2);
    groupsLayout->addWidget(algoGroup, 1);
    groupsLayout->addWidget(choosePlatformGroupBox, 1);
    groupsLayout->addWidget(analysisGroupBox, 1);
    groupsLayout->addWidget(socGroup,        1);

    mainLayout->addLayout(groupsLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // ================= CONNECTIONS =================
    connect(buttonsOkCancel, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonsOkCancel, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // auto connectFileBtn = [&](QPushButton *btn, QLineEdit *edit, const QString &title) {
    //     connect(btn, &QPushButton::clicked, this, [=]() {
    //         QString file = QFileDialog::getOpenFileName(this, title, "", "CSV Files (*.csv)");
    //         if (!file.isEmpty()) edit->setText(file);
    //     });
    // };

    // connectFileBtn(loadCurrentBtn, currentPath, "Select Current File");
    // connectFileBtn(loadPolyBtn, polyPath, "Select Poly File");
    // connectFileBtn(loadOcvSocCurveBtn, ocvSocPath, "Select OCV File");
    // connectFileBtn(loadParamsBtn, paramsPath, "Select Params File");
    // connectFileBtn(loadNoisesBtn, noisePath, "Select Noise File");

    currentPath->setText("/home/filip/Projects/Master/Params/current_output_haris_big.csv");
    polyPath->setText("/home/filip/Projects/Master/Params/ocv_poly_10.csv");
    ocvSocPath->setText("/home/filip/Projects/Master/Params/ocv.csv");
    paramsPath->setText("/home/filip/Projects/Master/Params/parameters.csv");
    noisePath->setText("/home/filip/Projects/Master/Params/noise.csv");


    // ================= EXEC =================
    if (dialog->exec() == QDialog::Accepted) {

        initialSoC = initSocSpinBox->value() / 100.0;
        endSoC     = endSocSpinBox->value() / 100.0;

        if(initialSoC <= endSoC){
            QMessageBox::warning(dialog, "Invalid SoC Settings",
                                 QString("Initial SoC must be greater than End Soc!")
                                     .arg(initSocSpinBox->value())
                                     .arg(endSocSpinBox->value()));
            return;
        }

        selectedPlatform = platformComboBox->currentText();
        platform_current_mah_e platform = platformMap.value(selectedPlatform, PLATFORM_CURRENT_ESP);
        simulatorContainer->setPlatformCurrent(platform);


        currentBatteryDock->setVisible(showCurrentBattery->isChecked());
        currentSystemDock->setVisible(showCurrentSystem->isChecked());
        currentPlatformDock->setVisible(showCurrentPlatform->isChecked());
        voltageDock->setVisible(showVoltage->isChecked());
        socDock->setVisible(showSoc->isChecked());

        redistributeDocks();

        currPath       = currentPath->text();
        polynomsPath   = polyPath->text();
        ocvSOCPath     = ocvSocPath->text();
        parametersPath = paramsPath->text();
        this->noisePath= noisePath->text();

        simulatorContainer->setCurrentPath(currPath);
        simulatorContainer->setOcvPolyPath(polynomsPath);
        simulatorContainer->setOcvSocPath(ocvSOCPath);
        simulatorContainer->setParamsPath(parametersPath);
        simulatorContainer->setNoisePath(this->noisePath);

        if (onlineAnalysys->isChecked()) {
            isOfflineMode = false;
            speedUpBtn->setEnabled(true);
            speedUpBtn->setText("1x");
            timerPeriodMs = 10;
        } else {
            isOfflineMode = true;
            speedUpBtn->setEnabled(false);
            speedUpBtn->setText("Offline");
        }
        simulatorContainer->loadFiles();
        simulatorContainer->reset(initialSoC/100.0);

        setLed(ledConfigLabel, "green");
        statusConfigLabel->setText("Configured");
    }
}


/*******************************************************************************
 * Slot - Log Info clicked - shows post simulations results
 ******************************************************************************/
void SimulatorWnd::onLogInfoClicked()
{
    QDialog *dialogWindow = new QDialog(this);
    dialogWindow->setWindowTitle("Simulation Log");
    dialogWindow->resize(400, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);


    QTextEdit *resultsText = new QTextEdit(dialogWindow);
    resultsText->setReadOnly(true);
    resultsText->setFixedSize(400, 400);

    platform_current_mah_e platformCurrent = PLATFORM_CURRENT_ESP;

    if (selectedPlatform == "ESP32")
        platformCurrent = PLATFORM_CURRENT_ESP;
    else if (selectedPlatform == "NXP")
        platformCurrent = PLATFORM_CURRENT_NXP;
    else if (selectedPlatform == "STM32")
        platformCurrent = PLATFORM_CURRENT_STM;
    else if (selectedPlatform == "NRF")
        platformCurrent = PLATFORM_CURRENT_NRF;

    float current = static_cast<float>(platformCurrent);


    resultsText->setFont(QFont("Courier New", 10));  // monospace font za poravnanje

    resultsText->append("=== Simulation Results ===\n");
    resultsText->append(QString("%1 %2")        .arg("Platform:",               -25).arg(selectedPlatform));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2")        .arg("Total samples:",          -25).arg(simulatorContainer->getNumberOfSamples()));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 %")      .arg("Initial SoC Battery:",    -25).arg(initialSoC * 100.0, 0, 'f', 1));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 %")      .arg("Final SoC Battery:",      -25).arg(simulatorContainer->getSoC(), 0, 'f', 2));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 %")      .arg("Initial SoC System:",     -25).arg(initialSoC * 100.0, 0, 'f', 1));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 %")      .arg("Final SoC System:",      -25).arg(simulatorContainer->getSoCIsys(), 0, 'f', 2));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 V")      .arg("Final Terminal Voltage:", -25).arg(simulatorContainer->getVBat(), 0, 'f', 4));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 A")      .arg("Average Battery Current:",-25).arg(averageBatteryCurr, 0, 'f', 4));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 A")      .arg("Average System Current:", -25).arg(averageSystemCurr, 0, 'f', 4));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 mA")      .arg("Platform Current:",       -25).arg(current, 0, 'f', 4));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 As")     .arg("Electric Charge:",        -25).arg(simulatorContainer->getElectricCharge(), 0, 'f', 4));
    resultsText->append(" ");
    resultsText->append(QString("%1 %2 s")      .arg("Simulation Time:",        -25).arg(simulatorContainer->getTimeS(), 0, 'f', 1));
    resultsText->append(" ");

    layout->addWidget(resultsText);

    QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btn, &QDialogButtonBox::rejected, dialogWindow, &QDialog::reject);
    layout->addWidget(btn);

    dialogWindow->exec();
}

/*******************************************************************************
 * Slot - Play button clicked - starts online or offline simulation
 ******************************************************************************/
void SimulatorWnd::onPlayClicked()
{
    if(isOfflineMode){
        qDebug() << "Run Offline Simulation";
    }
    else{
        qDebug() << "Run Online Simulation";
    }
    if (isOfflineMode) {
        startOfflineSimulation();
    }
    else {
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
 * Slot - Stop button clicked - stops simulation and resets state
 ******************************************************************************/
void SimulatorWnd::onStopClicked()
{

    currentSimState = SIMULATION_STOPPED;
    qDebug() << "Stop Simulation";

    // Stop the timer
    timer->stop();

    // Reset data set
    simulatorContainer->reset(initialSoC/100.0);


    // Clear plots
    currentBatteryPlot->clear();
    currentSystemPlot->clear();
    currentPlatformPlot->clear();
    voltagePlot->clear();
    socPlot->clear();

    // Reset speed btn and value
    if(!isOfflineMode){
        speedUpBtn->setText("1x");
        timerPeriodMs = 10;
    }

    // Reset btn states
    playBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
    speedUpBtn->setEnabled(false);
    configBtn->setEnabled(true);

    // Reset leds and labels
    setLed(ledPlaySimulationLabel, "red");
    statusPlaySimulationLabel->setText("Stopped");

    // Reset progress bar
    progressBar->setValue(0);
    currentSample = 0;

    // Emit signal
    emit sigStop();
}

/*******************************************************************************
 * Slot - Pause/Resume button clicked - toggles timer on/off
 ******************************************************************************/
void SimulatorWnd::onPauseClicked()
{
    if (timer->isActive()) {
        currentSimState = SIMULATION_PAUSED;
        qDebug() << "Pause Simulation";
        timer->stop();
        pauseBtn->setText("Resume");
        setLed(ledPlaySimulationLabel, "yellow");
        statusPlaySimulationLabel->setText("Paused");
    } else {
        currentSimState = SIMULATION_PLAYING;
        qDebug() << "Resume Simulation";
        timer->start(timerPeriodMs);
        pauseBtn->setText("Pause");
        setLed(ledPlaySimulationLabel, "green");
        statusPlaySimulationLabel->setText("Running");
    }

    emit sigPause();
}

/*******************************************************************************
 * Slot - Speed up button clicked - cycles through 1x / 2x / 5x
 ******************************************************************************/

void SimulatorWnd::onSpeedUpClicked()
{
    if (isOfflineMode) return;

    if (timerPeriodMs == 10) {
        speedUpBtn->setText("2x");
        timerPeriodMs = 5;
    } else if (timerPeriodMs == 5) {
        speedUpBtn->setText("5x");
        timerPeriodMs = 2;
    } else {
        speedUpBtn->setText("1x");
        timerPeriodMs = 10;
    }

    if (timer->isActive()) {
        timer->stop();
        timer->start(timerPeriodMs);
    }

    emit sigSpeedUp();
}

/*******************************************************************************
 * Helper - Set LED indicator color
 ******************************************************************************/
void SimulatorWnd::setLed(QLabel *led, const QString &color, simulationState_e simulationState) {
    QString ledColor = color;
    if (color == "None") {
        switch (simulationState) {
        case SIMULATION_UNINIT:   ledColor = "grey";   break;
        case SIMULATION_PLAYING:  ledColor = "green";  break;
        case SIMULATION_STOPPED:  ledColor = "red";    break;
        case SIMULATION_PAUSED:   ledColor = "yellow"; break;
        }
    }

    QString light, dark;
    if      (ledColor == "green")  { light = "#90ff90"; dark = "#006600"; }
    else if (ledColor == "red")    { light = "#ff9090"; dark = "#660000"; }
    else if (ledColor == "yellow") { light = "#ffff90"; dark = "#666600"; }
    else                           { light = "#cccccc"; dark = "#444444"; } // grey

    led->setStyleSheet(QString(
                           "background-color: qradialgradient("
                           "cx: 0.3, cy: 0.3, radius: 0.7,"
                           "fx: 0.3, fy: 0.3,"
                           "stop: 0 white,"
                           "stop: 0.3 %1,"
                           "stop: 1 %2);"
                           "border-radius: 8px;"
                           "border: 1px solid #333;"
                           ).arg(light).arg(dark));
}

/*******************************************************************************
 * Helper - onLedTimerTimeout changing colors when blinks
 ******************************************************************************/
void SimulatorWnd::onLedTimerTimeout()
{
    ledBlinkState = !ledBlinkState;

    if (ledBlinkState) {
        setLed(ledPlaySimulationLabel, "None", currentSimState);
    } else {
        setLed(ledPlaySimulationLabel, "grey");
    }
}

/*******************************************************************************
 * Helper - Redistribute visible dock sizes evenly
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

    // Qt5 fix:
    QList<int> sizes;
    for (int i = 0; i < visible.size(); i++)
        sizes << each;

    resizeDocks(visible, sizes, Qt::Vertical);
}


/*******************************************************************************
 * Offline simulation - runs all steps in worker thread, plots result when done
 ******************************************************************************/
void SimulatorWnd::startOfflineSimulation()
{
    // Run worker thread
    QtConcurrent::run([this]() {

        // Do UI things in main thread
        QMetaObject::invokeMethod(this, [=]() {
            playBtn->setEnabled(false);
            configBtn->setEnabled(false);
            setLed(ledPlaySimulationLabel, "green");
            statusPlaySimulationLabel->setText("Offline Processing");
        }, Qt::BlockingQueuedConnection);

        // Setting offline mode for Simulator Container
        simulatorContainer->setOfflineMode(true);

        // Get total samples
        int total = simulatorContainer->getNumberOfSamples();

        // Local vectors in Worker Thread
        QVector<double> vTime, vBat, vIBat, vISys, vIPlatform, vSoc, vSocIsys;
        vTime.reserve(total);
        vBat.reserve(total);
        vIBat.reserve(total);
        vISys.reserve(total);
        vIPlatform.reserve(total);
        vSoc.reserve(total);
        vSocIsys.reserve(total);

        // Local samples counter
        int localSample = 0;
        simulatorContainer->reset(initialSoC);

        // Loop
        while (!simulatorContainer->isFinished()) {

            // One step in simulator
            simulatorContainer->step();

            // Append all results
            vTime.append(simulatorContainer->getTimeS());
            vBat.append(simulatorContainer->getVBat());
            vIBat.append(simulatorContainer->getIBat());
            vISys.append(simulatorContainer->getISys());
            vIPlatform.append(simulatorContainer->getIPlatform());
            vSoc.append(simulatorContainer->getSoC());
            vSocIsys.append(simulatorContainer->getSoCIsys());

            double endSoc = getSimulationEndSoC() * 100.0;

            if(vSoc[localSample] <= endSoc){
                for(int i = 0; i < localSample; i++){
                    averageBatteryCurr += vIBat[localSample];
                    averageSystemCurr  += vISys[localSample];
                }
                averageBatteryCurr /= localSample;
                averageSystemCurr /= localSample;
                break;
            }

            // Increment sample
            localSample++;

            // Update progress bar
            if (localSample % 1000 == 0) {
                int pct = (localSample * 100) / total;
                QMetaObject::invokeMethod(this, [=]() {
                    progressBar->setValue(pct);
                });
            }
        }

        // Main thread invoke
        QMetaObject::invokeMethod(this, [=]() {

            // Setting data to plot
            currentBatteryPlot->setData(vIBat,       vTime);
            currentSystemPlot->setData(vISys,        vTime);
            currentPlatformPlot->setData(vIPlatform, vTime);
            voltagePlot->setData(vBat,               vTime);
            socPlot->setData(vSoc,                   vTime);
            socPlot->setSecondGraphData(vSocIsys,    vTime);

            // Replot it all
            currentBatteryPlot->replotAll();
            currentSystemPlot->replotAll();
            currentPlatformPlot->replotAll();
            voltagePlot->replotAll();
            socPlot->replotAll();

            // Set leds and texts in status bar
            setLed(ledPlaySimulationLabel, "green");
            statusPlaySimulationLabel->setText("Offline Done");

            // Config Tab bar
            playBtn->setEnabled(false);
            configBtn->setEnabled(true);
            stopBtn->setEnabled(true);
            logInfoBtn->setEnabled(true);

            simulatorContainer->reset(initialSoC);

            qDebug() << "Offline simulation finished";
        });
    });
}
