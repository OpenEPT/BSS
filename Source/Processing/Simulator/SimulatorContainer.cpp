#include "Processing/Simulator/SimulatorContainer.h"
#include <QDebug>

SimulatorContainer::SimulatorContainer(QObject *parent)
    : QObject(parent)
    ,currentIndex(0),
    timeS(0.0f),
    electricCharge(0.0)
{
    input            = new SimulatorInput(this);
    batteryModel     = new BatteryModel(this);
    voltageEstimator = new VoltageEstimator(this);
    socReference     = new SocReference(this);
    platformCurr     = new PlatformCurrent(PLATFORM_CURRENT_ESP, this);
    batteryCurrGen   = new BatteryCurrentGenerator(this);
    timeTable        = new TimeTable(K2, this);
    algo             = new Algo(K2, this);
    noiseGenerator   = new NoiseGenerator(input);

    outputFile   = new QFile("/home/filip/Projects/Master/Output/output.csv");
    if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open output file:" << outputFile->errorString();
    } else {
        qDebug() << "Output file opened successfully";
        outputStream = new QTextStream(outputFile);
        *outputStream << "K,Flag[N,P,S],Ibat[A],Vbat[V],SoCIsys[%],SoCIbat[%],WhichAlgo[Type],AlgoDuration[ms],ElectricCharge[As]\n";
    }
    outputStream = new QTextStream(outputFile);
    *outputStream << "K,Flag[N,P,S],Ibat[A],Vbat[V],SoCIsys[%],SoCIbat[%],WhichAlgo[Type],AlgoDuration[ms],ElectricCharge[As]\n";

    connect(batteryCurrGen, &BatteryCurrentGenerator::requestPlatformCurrent,
            platformCurr, &PlatformCurrent::onPlatformCurrentRequested,
            Qt::DirectConnection);

    connect(platformCurr, &PlatformCurrent::platformCurrentReady,
            batteryCurrGen, &BatteryCurrentGenerator::onPlatformCurrentReady,
            Qt::DirectConnection);

    connect(batteryCurrGen, &BatteryCurrentGenerator::batteryCurrentReady,
            voltageEstimator, &VoltageEstimator::calculateTerminalVoltage,
            Qt::DirectConnection);

    connect(voltageEstimator, &VoltageEstimator::terminalVoltageCalculated,
            this, [=](double vBat, float iSys, float iPlatform, double iBat, float soc) {
                if (!offlineMode) {
                    emit dataSample(timeS, vBat, iSys, iPlatform, iBat, soc * 100.0f, lastSoCIsys);
                }
            }, Qt::DirectConnection);

    connect(algo, &Algo::algoTrigger,
            timeTable, &TimeTable::onWitchAlgo,
            Qt::DirectConnection);

    // connect(timeTable, &TimeTable::durationAlgo,
    //         platformCurr, &PlatformCurrent::platformCurrentReady,
    //         Qt::DirectConnection);
}

bool SimulatorContainer::loadFiles()
{
    if (paramsFilePath.isEmpty()  || ocvPolyFilePath.isEmpty() ||
        ocvSocFilePath.isEmpty()  || currentFilePath.isEmpty() ||
        noisePath.isEmpty())
    {
        emit loadError("Not all file paths are set");
        return false;
    }

    if (!input->loadParametersCSV(paramsFilePath)) {
        emit loadError("Failed to load parameters");
        return false;
    }
    if (!input->loadCoefcientsOcvPolyCSV(ocvPolyFilePath)) {
        emit loadError("Failed to load OCV poly");
        return false;
    }
    if (!input->loadOcvCSV(ocvSocFilePath)) {
        emit loadError("Failed to load OCV SoC curve");
        return false;
    }
    if (!input->loadCurrentCSV(currentFilePath)) {
        emit loadError("Failed to load current CSV");
        return false;
    }
    if(!input->loadNoiseCSV(noisePath)){
        emit loadError("Failed to load noise CSV");  // ← pogrešna poruka
        return false;
    }

    batteryModel->batteryModelInit(input);
    voltageEstimator->voltageEstimatorInit(batteryModel);
    currentIndex   = 0;
    electricCharge = 0.0;

    emit loadSuccess();
    return true;
}

QString algoToString(algoTimeTableDuration_e algo)
{
    switch(algo) {
    case LP:              return "LP";
    case K0:              return "K0";
    case K2:              return "K2";
    case ADAPTIVE_LP_K0:  return "ADAPTIVE_LP_K0";
    case ADAPTIVE_LP_K2:  return "ADAPTIVE_LP_K2";
    default:              return "UNKNOWN";
    }
}

QString flagToString(current_flag_e flag)
{
    switch(flag) {
    case N:              return "N";
    case P:              return "P";
    case S:              return "S";
    default:              return "UNKNOWN";
    }
}

void writeFile(QTextStream *outputStream, int k, current_flag_e flag, float iBat, double vBat,
               float socIsys, float soc, algoTimeTableDuration_e algoDuration, float elecrticCharge)
{
    *outputStream << k                          << ","
                  << flagToString(flag)         << ","
                  << iBat                       << ","
                  << vBat                       << ","
                  << socIsys                    << ","
                  << soc                        << ","
                  << algoToString(algoDuration) << ","  // ← string umesto int
                  << algoDuration               << ","  // ← trajanje u ms
                  << elecrticCharge             << "\n";
    outputStream->flush();
}

void SimulatorContainer::step()
{
    if (isFinished()) return;

    // Cache references (no copy!)
    const auto& currVec = input->getCurrent();
    const auto& timeVec = input->getTime();
    const auto& flagVec = input->getFlag();

    // Take Noise
    noiseParameters_e noise = noiseGenerator->getNoise(currentIndex);

    // Get current + noise
    float currA = currVec[currentIndex] / 1000.0f; // mA → A
    currA += noise.currentNoise;

    // Calculate Isys SoC reference
    float socIsysRef = socReference->calculateRefSoCIsys(currA, 0.01) * 100;
    lastSoCIsys   = socIsysRef;

    // Get time and flag
    timeS                  = timeVec[currentIndex] / 1000.0f; // ms → s
    current_flag_e flag    = flagVec[currentIndex];

    // System current goes to BCG modul
    float iSys = batteryCurrGen->getSystemCurrent(currA);
    batteryCurrGen->getBcgParams(iSys, flag);

    // If flag is P/S triggers time table
    algoTimeTableDuration_e algoDuration = timeTable->getWhitchAlgo();
    float durationMS = static_cast<float>(algoDuration);

    // Get platform current + electric charge
    float currMA = batteryCurrGen->getIplatform();
    float q      = (currMA / 1000.0f) * (durationMS / 1000.0f);

    // Take I, V, SoC
    float  iBat = batteryCurrGen->getIBat();
    double vBat = voltageEstimator->getVTerminal();
    float  soc  = batteryModel->getSoC() * 100.0f;

    if (flag == N) {
        writeFile(outputStream, currentIndex, flag, iBat, vBat, socIsysRef, soc, algoDuration, electricCharge);
    } else {
        electricCharge += q;
        algo->algoExe(K2);
        writeFile(outputStream, currentIndex, flag, iBat, vBat, socIsysRef, soc, algoDuration, electricCharge);
    }

    lastIBat      = iBat;
    lastVBat      = vBat;
    lastISys      = iSys;
    lastIPlatform = batteryCurrGen->getIplatform();
    lastSoC       = soc;
    lastSoCIsys   = socIsysRef;
    lastElectricCharge = electricCharge;

    currentIndex++;
}

void SimulatorContainer::reset(double SocReference)
{
    currentIndex = 0;
    voltageEstimator->voltageEstimatorInit(batteryModel);
    socReference->socReferenceSet(SocReference);
    batteryModel->reset(SocReference);
}

bool SimulatorContainer::isFinished() const
{
    return currentIndex >= input->getNumberOfSamples();
}
