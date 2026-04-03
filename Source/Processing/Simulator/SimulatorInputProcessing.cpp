#include "SimulatorInputProcessing.h"
#include <QDebug>

SimulatorInput::SimulatorInput(QObject *parent)
    : QObject{parent}
{
}

// Current CSV format:
//   Line 1: description
//   Line 2: dashes
//   Line 3: header (Current [mA], CurTime [ms])
//   Line 4+: data
bool SimulatorInput::loadCurrentCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open current file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.readLine();
    in.readLine();
    in.readLine();

    systemTime.clear();
    systemCurrent.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 2) continue;

        float current = parts[0].toFloat();
        float time    = parts[1].toFloat();

        systemCurrent.append(current);
        systemTime.append(time);
    }

    file.close();
    emit fileLoadedCurrCSV(systemTime.size());
    return true;
}

// OCV polynomial CSV format:
//   Line 1: header (Coefficient)
//   Line 2+: one coefficient per line
bool SimulatorInput::loadCoefcientsOcvPolyCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open OCV poly file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.readLine();

    systemCoeffcientOcvPoly.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        systemCoeffcientOcvPoly.append(line.toDouble());
    }

    file.close();
    emit fileLoadedCoefOcvPolyCSV();
    return true;
}

// Parameters CSV format:
//   Line 1: header
//   Line 2+: Region, DoD, RSlow, CSlow, RFast, CFast, RInternal, RBorder, RAverage
bool SimulatorInput::loadParametersCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open parameters file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.readLine();

    batteryParams.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",", Qt::KeepEmptyParts);
        if (parts.size() < 9) {
            qWarning() << "Invalid row:" << line;
            continue;
        }

        batteryParamaters_t p;
        p.region    = parts[0].toInt();
        p.dod       = parts[1].toFloat();
        p.rSlow     = parts[2].toFloat();
        p.cSlow     = parts[3].toFloat();
        p.rFast     = parts[4].toFloat();
        p.cFast     = parts[5].toFloat();
        p.rInternal = parts[6].toFloat();
        p.rBorder   = parts[7].toFloat();
        p.rAverage  = parts[8].toFloat();

        batteryParams.append(p);
    }

    file.close();
    emit fileLoadedParametersCSV();
    return true;
}

QVector<float> SimulatorInput::getTime() const
{
    return systemTime;
}

QVector<float> SimulatorInput::getCurrent() const
{
    return systemCurrent;
}

QVector<double> SimulatorInput::getCoefficientOcvPoly() const
{
    return systemCoeffcientOcvPoly;
}

QVector<batteryParamaters_t> SimulatorInput::getBatteryParams() const
{
    return batteryParams;
}
