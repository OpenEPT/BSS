#include "simulatorprocessing.h"
#include "qdebug.h"
#include "qfileinfo.h"


SimulatorInput::SimulatorInput(QObject *parent)
    : QObject{parent}
{

}

bool SimulatorInput::loadCurrentCSV(QString filePath)
{
    // Init class QFile
    QFile file(filePath);

    // Open file
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return false;
    }

    // Init QTextStream
    QTextStream in(&file);

    // Read whole file
    while(!in.atEnd()) {
        QString line     = in.readLine();    // Read 1 line from file
        QStringList parts = line.split(","); // Extract ','

        float time    = parts[0].toFloat();
        float current = parts[1].toFloat();

        // Private arguments of class
        systemTime.append(time);
        systemCurrent.append(current);
    }

    // Emit signal, file is read
    emit fileLoaded(systemTime.size());

    return true;
}

QVector<float> SimulatorInput::getTime() const{
    return systemTime;
}


QVector<float> SimulatorInput::getCurrent() const{
    return systemCurrent;
}
