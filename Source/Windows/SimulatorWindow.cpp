#include "SimulatorWindow.h"
#include <QFileDialog>

SimulatorWnd::SimulatorWnd(QWidget *parent)
    : QWidget(parent)
{
    // 1. Glavni vertikalni layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 2. Gornji horizontalni layout sa dugmadima
    // Gornji red sa dugmadima
    QHBoxLayout *topLayout = new QHBoxLayout();

    // Napravi dugmad
    mPlayBtn  = new QPushButton("Play");
    mStopBtn  = new QPushButton("Stop");
    mPauseBtn = new QPushButton("Pause");
    mLoadBtn  = new QPushButton("Load");

    // Dodaj dugmad u horizontalni layout
    topLayout->addWidget(mPlayBtn);
    topLayout->addWidget(mStopBtn);
    topLayout->addWidget(mPauseBtn);
    topLayout->addWidget(mLoadBtn);
    topLayout->addStretch(); // gurni dugmad ulevo

    // Dodaj topLayout u mainLayout
    mainLayout->addLayout(topLayout);

    // 3. Napravi dugmad i dodaj ih u topLayout
    // Napravi plotove
    mCurrentPlot = new Plot(500, 200, false, this);
    mVoltagePlot  = new Plot(500, 200, false, this);

    mCurrentPlot->setTitle("Battery Current");
    mCurrentPlot->setYLabel("[A]");
    mCurrentPlot->setXLabel("[s]");

    mVoltagePlot->setTitle("Battery Voltage");
    mVoltagePlot->setYLabel("[V]");
    mVoltagePlot->setXLabel("[s]");

    // Dodaj plotove u mainLayout
    mainLayout->addWidget(mCurrentPlot);
    mainLayout->addWidget(mVoltagePlot);

    connect(mPlayBtn,  SIGNAL(clicked()), this, SLOT(onPlayClicked()));
    connect(mStopBtn,  SIGNAL(clicked()), this, SLOT(onStopClicked()));
    connect(mPauseBtn, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
    connect(mLoadBtn,  SIGNAL(clicked()), this, SLOT(onLoadClicked()));
}

void SimulatorWnd::onPlayClicked()
{
    mPlayBtn->setEnabled(false);
    mStopBtn->setEnabled(true);
    mPauseBtn->setEnabled(true);
    emit sigPlay();
}

void SimulatorWnd::onStopClicked()
{
    mPlayBtn->setEnabled(true);
    mStopBtn->setEnabled(false);
    mPauseBtn->setEnabled(false);
    emit sigStop();
}

void SimulatorWnd::onPauseClicked()
{
    emit sigPause();
}

void SimulatorWnd::onLoadClicked()
{
    // Otvori file dialog
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Current CSV",
        "",
        "CSV files (*.csv)"
        );
    if(!filePath.isEmpty()) {
        // emit signal sa putanjom
    }
}
