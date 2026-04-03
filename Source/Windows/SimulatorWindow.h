#ifndef SIMULATORWND_H
#define SIMULATORWND_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "Windows/Plot/plot.h"

class SimulatorWnd : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorWnd(QWidget *parent = nullptr);

private slots:
    void onPlayClicked();   // reaguje kada klikneš Play
    void onStopClicked();   // reaguje kada klikneš Stop
    void onPauseClicked();  // reaguje kada klikneš Pause
    void onLoadClicked();   // reaguje kada klikneš Load file

signals:
    void sigPlay();   // obavesti MainWindow da je Play kliknut
    void sigStop();   // obavesti MainWindow da je Stop kliknut
    void sigPause();  // obavesti MainWindow da je Pause kliknut

private:
    // Dugmad
    QPushButton *mPlayBtn;
    QPushButton *mStopBtn;
    QPushButton *mPauseBtn;
    QPushButton *mLoadBtn;

    // Grafici
    Plot *mCurrentPlot;   // graf struje
    Plot *mVoltagePlot;   // graf napona
};

#endif
