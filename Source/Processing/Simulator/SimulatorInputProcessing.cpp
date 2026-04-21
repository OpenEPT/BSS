#include "SimulatorInputProcessing.h"
#include <QDebug>

SimulatorInput::SimulatorInput(QObject *parent)
    : QObject{parent}
{
}

current_flag_e parseFlag(const QString &str)
{
    if (str.trimmed() == "P") return P;
    if (str.trimmed() == "S") return S;
    return N;  // default
}


noiseParameters_e SimulatorInput::getNoise(int index) const
{
    if (index < 0 || index >= systemNoise.size()) {
        noiseParameters_e zero{};
        zero.currentNoise = 0.0f;
        return zero;
    }
    return systemNoise[index];
}

bool SimulatorInput::loadNoiseCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open current file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 2) continue;

        noiseParameters_e noiseEntry;
        noiseEntry.currentNoise = parts[0].toFloat();
        noiseEntry.voltageNoise = parts[1].toFloat();
        systemNoise.append(noiseEntry);
    }


    file.close();

    qDebug() << "File " << filePath << "loaded successfully";

    emit fileLoadedCurrCSV(systemTime.size());
    return true;
}

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

    int i = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 1) continue;

        float current = parts[0].toFloat();
        float time    = parts[1].toFloat();

        systemCurrent.append(current);
        systemTime.append(time);

        i++;
    }

    numberOfCurrentSamples = i;

    file.close();

    qDebug() << "File " << filePath << "loaded successfully";

    emit fileLoadedCurrCSV(systemTime.size());
    return true;
}

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

    qDebug() << "File " << filePath << "loaded successfully";

    emit fileLoadedCoefOcvPolyCSV();
    return true;
}

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

    qDebug() << "File " << filePath << "loaded successfully";

    emit fileLoadedParametersCSV();
    return true;
}



bool SimulatorInput::loadAlgoFlagsCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.readLine(); // Preskoči header (Flag,Exe)

    algoFlags.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 2) continue; // Sigurnosna provera

        algoFlags_t p; // Koristi ime strukture koje smo definisali

        // 1. Konverzija String -> Enum (Flag)
        QString flagStr = parts[0].trimmed().toUpper();
        if (flagStr == "P") {
            p.flag = P;
        } else {
            p.flag = N;
        }

        // 2. Konverzija String -> Int (Exe)
        p.exe = parts[1].toInt();

        algoFlags.append(p);
    }

    file.close();
    qDebug() << "Loaded" << algoFlags.size() << "flags from" << filePath;
    emit fileLoadedOcvCSV();
    return true;
}

bool SimulatorInput::loadOcvCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open OCV SoC curve file:" << filePath;
        emit loadError("Cannot open: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.readLine();

    systemOcvSocCurve.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",", Qt::KeepEmptyParts);

        ocvSocCurve_t p;
        p.soc = parts[0].toFloat();
        p.ocv = parts[1].toFloat();

        systemOcvSocCurve.append(p);
    }

    file.close();

    qDebug() << "File " << filePath << "loaded successfully";
    emit fileLoadedOcvCSV();
    return true;
}

const QVector<float>& SimulatorInput::getTime()    const { return systemTime; }
const QVector<float>& SimulatorInput::getCurrent() const { return systemCurrent; }

QVector<double> SimulatorInput::getCoefficientOcvPoly() const
{
    return systemCoeffcientOcvPoly;
}

QVector<batteryParamaters_t> SimulatorInput::getBatteryParams() const
{
    return batteryParams;
}

QVector<ocvSocCurve_t> SimulatorInput::getOcvSocCurve() const
{
    return systemOcvSocCurve;
}

int SimulatorInput::getNumberOfSamples() const{ return numberOfCurrentSamples;}
