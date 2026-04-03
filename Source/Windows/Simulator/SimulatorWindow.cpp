#include "SimulatorWindow.h"

SimulatorWnd::SimulatorWnd(QWidget *parent)
    : QWidget(parent)
{
    // Init simulator modules
    simulatorInput   = new SimulatorInput(this);
    batteryModel     = new BatteryModel(this);
    voltageEstimator = new VoltageEstimator(this);
    timer            = new QTimer(this);
    currentIndex     = 0;

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top horizontal layout with buttons
    QHBoxLayout *topLayout = new QHBoxLayout();

    playBtn  = new QPushButton("Play");
    stopBtn  = new QPushButton("Stop");
    pauseBtn = new QPushButton("Pause");
    loadBtn  = new QPushButton("Load");

    topLayout->addWidget(playBtn);
    topLayout->addWidget(stopBtn);
    topLayout->addWidget(pauseBtn);
    topLayout->addWidget(loadBtn);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // Plots
    currentPlot = new Plot(500, 200, false, this);
    voltagePlot = new Plot(500, 200, false, this);

    currentPlot->setTitle("Battery Current");
    currentPlot->setYLabel("[A]");
    currentPlot->setXLabel("[s]");

    voltagePlot->setTitle("Battery Voltage");
    voltagePlot->setYLabel("[V]");
    voltagePlot->setXLabel("[s]");

    mainLayout->addWidget(currentPlot);
    mainLayout->addWidget(voltagePlot);

    // Connect buttons
    connect(playBtn,  SIGNAL(clicked()), this, SLOT(onPlayClicked()));
    connect(stopBtn,  SIGNAL(clicked()), this, SLOT(onStopClicked()));
    connect(pauseBtn, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
    connect(loadBtn,  SIGNAL(clicked()), this, SLOT(onLoadClicked()));

    // Initial button state
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);
}

void SimulatorWnd::onTimerTick()
{
    if (currentIndex >= simulatorInput->getTime().size()) {
        timer->stop();
        playBtn->setEnabled(true);
        stopBtn->setEnabled(false);
        pauseBtn->setEnabled(false);
        qDebug() << "Simulation finished";
        return;
    }

    float currA = simulatorInput->getCurrent()[currentIndex] / 1000.0f; // mA → A
    float time  = simulatorInput->getTime()[currentIndex]    / 1000.0f; // ms → s

    double vBat = voltageEstimator->calculateTerminalVoltage(currA);

    QVector<double> x  = {(double)time};
    QVector<double> yI = {(double)currA};
    QVector<double> yV = {vBat};

    currentPlot->appendData(yI, x);
    voltagePlot->appendData(yV, x);

    currentIndex++;
}

void SimulatorWnd::onPlayClicked()
{
    if (currentIndex >= simulatorInput->getTime().size())
        currentIndex = 0;

    playBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    pauseBtn->setEnabled(true);

    timer->start(10); // one sample every 10ms
    emit sigPlay();
}

void SimulatorWnd::onStopClicked()
{
    timer->stop();
    currentIndex = 0;

    currentPlot->clear();
    voltagePlot->clear();

    playBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    pauseBtn->setEnabled(false);

    emit sigStop();
}

void SimulatorWnd::onPauseClicked()
{
    if (timer->isActive()) {
        timer->stop();
        pauseBtn->setText("Resume");
    } else {
        timer->start(10);
        pauseBtn->setText("Pause");
    }

    emit sigPause();
}

void SimulatorWnd::onLoadClicked()
{
    QString paramsPath = "/home/filip/Projects/Master/Params/parameters.csv";
    QString ocvPath    = "/home/filip/Projects/Master/Params/ocv_poly_10.csv";

    QString currentPath = QFileDialog::getOpenFileName(
        this,
        "Open Current CSV",
        "/home/filip/Projects/Master/Params/",
        "CSV files (*.csv)"
        );

    if (currentPath.isEmpty()) {
        qDebug() << "Current file not selected";
        return;
    }

    // Load input files
    simulatorInput->loadParametersCSV(paramsPath);
    simulatorInput->loadCoefcientsOcvPolyCSV(ocvPath);
    simulatorInput->loadCurrentCSV(currentPath);

    qDebug() << "Loaded" << simulatorInput->getBatteryParams().size() << "param rows";
    qDebug() << "Loaded" << simulatorInput->getCoefficientOcvPoly().size() << "OCV coeffs";
    qDebug() << "Loaded" << simulatorInput->getCurrent().size() << "current samples";

    // Init modules
    batteryModel->batteryModelInit(simulatorInput);
    voltageEstimator->voltageEstimatorInit(batteryModel);

    qDebug() << "All modules initialized - ready to play";

    // Reset state
    currentIndex = 0;
    currentPlot->clear();
    voltagePlot->clear();

    playBtn->setEnabled(true);
}
