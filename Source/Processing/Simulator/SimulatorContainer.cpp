#include "Processing/Simulator/SimulatorContainer.h"
#include <QDebug>

SimulatorContainer::SimulatorContainer(
    BatteryModel    *sharedBatteryModel,
    PlatformCurrent *sharedPlatformCurrent,
    TimeTable       *sharedTimeTable,
    QObject *parent)
    : QObject(parent)
    , batteryModel(sharedBatteryModel)
    , platformCurr(sharedPlatformCurrent)
    , timeTable(sharedTimeTable)
    , timeS(0.0f)
    , electricCharges{0.0, 0.0, 0.0}
    , globalSampleIndex(0)
{
    inputProcessing  = new SimulatorInput(this);
    voltageEstimator = new VoltageEstimator(this);
    socReference     = new SocReference(this);
    batteryCurrGen   = new BatteryCurrentGenerator(this);
    algo             = new Algo(K2, this);
    noiseGenerator   = new NoiseGenerator(inputProcessing);

    // Output file is opened in loadFiles() after outputFilePath is set
    outputFile   = nullptr;
    outputStream = nullptr;

    connect(algo, &Algo::requestPlatformCurrent,
            platformCurr, &PlatformCurrent::onPlatformCurrentRequested,
            Qt::DirectConnection);

    connect(platformCurr, &PlatformCurrent::platformCurrentReady,
            algo, &Algo::onPlatformCurrentReady,
            Qt::DirectConnection);

    connect(batteryCurrGen, &BatteryCurrentGenerator::batteryCurrentReady,
            voltageEstimator, &VoltageEstimator::calculateTerminalVoltage,
            Qt::DirectConnection);

    connect(voltageEstimator, &VoltageEstimator::terminalVoltageCalculated,
            this, [=](double vBat, float iSys, float iPlatform, double iBat, float soc) {
                if (!offlineMode) {
                    emit dataSample(timeS, vBat, iSys, iPlatform, iBat,
                                    soc * 100.0f, lastValues.lastSoCIsys);
                }
            }, Qt::DirectConnection);

    connect(algo, &Algo::getWitchAlgo,
            timeTable, &TimeTable::onWitchAlgo,
            Qt::DirectConnection);

    connect(timeTable, &TimeTable::durationAlgo,
            algo, &Algo::onAlgoDuration,
            Qt::DirectConnection);

    connect(algo, &Algo::flagStatusChanged, batteryCurrGen,
            &BatteryCurrentGenerator::updateFlagFromAlgo,
            Qt::DirectConnection);

    connect(voltageEstimator, &VoltageEstimator::terminalVoltageCalculated,
            algo, &Algo::onVoltageEstimatorDone,
            Qt::DirectConnection);
}

/*******************************************************************************
 * Helpers
 ******************************************************************************/
static QString algoToString(algoTimeTableDuration_e algo)
{
    switch(algo) {
    case LP:             return "LP";
    case K0:             return "K0";
    case K2:             return "K2";
    case ADAPTIVE_LP_K0: return "ADAPTIVE_LP_K0";
    case ADAPTIVE_LP_K2: return "ADAPTIVE_LP_K2";
    default:             return "UNKNOWN";
    }
}

static QString flagToString(current_flag_e flag)
{
    switch(flag) {
    case N:  return "N";
    case P:  return "P";
    case S:  return "S";
    default: return "UNKNOWN";
    }
}

static void writeFileLine(QTextStream *stream, int k,
                          current_flag_e flag, float iBat, double vBat,
                          float iPlat, float socIsys, float soc,
                          algoTimeTableDuration_e algoDuration,
                          double qBat, double qSys, double qAlgo)
{
    if (!stream) return;
    *stream << k                           << ","
            << flagToString(flag)          << ","
            << iBat                        << ","
            << vBat                        << ","
            << iPlat                       << ","
            << socIsys                     << ","
            << soc                         << ","
            << algoToString(algoDuration)  << ","
            << algoDuration                << ","
            << qBat                        << ","
            << qSys                        << ","
            << qAlgo                       << "\n";
    stream->flush();
}

/*******************************************************************************
 * loadFiles
 ******************************************************************************/
bool SimulatorContainer::loadFiles()
{
    if (paramsFilePath.isEmpty()  || ocvPolyFilePath.isEmpty() ||
        ocvSocFilePath.isEmpty()  || currentFilePath.isEmpty() ||
        noisePath.isEmpty()       || flagPath.isEmpty())
    {
        emit loadError("Not all file paths are set");
        return false;
    }

    // Open output file (path may have been set via setOutputPath)
    if (outputFile) {
        outputFile->close();
        delete outputStream;
        delete outputFile;
    }
    outputFile   = new QFile(outputFilePath);
    outputStream = nullptr;
    if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open output file:" << outputFile->errorString();
    } else {
        outputStream = new QTextStream(outputFile);
        *outputStream << "K,Flag[N,P,S],Ibat[A],Vbat[V],IPlat[A],SoCIsys[%],"
                         "SoCIbat[%],WhichAlgo[Type],AlgoDuration[ms],"
                         "ElectricChargeBattery[mAh],ElectricChargeSystem[mAh],"
                         "ElectricChargeAlgo[mAh]\n";
    }

    if (!inputProcessing->loadParametersCSV(paramsFilePath)) {
        emit loadError("Failed to load parameters"); return false;
    }
    if (!inputProcessing->loadCoefcientsOcvPolyCSV(ocvPolyFilePath)) {
        emit loadError("Failed to load OCV poly"); return false;
    }
    if (!inputProcessing->loadOcvCSV(ocvSocFilePath)) {
        emit loadError("Failed to load OCV SoC curve"); return false;
    }
    if (!inputProcessing->loadCurrentCSV(currentFilePath)) {
        emit loadError("Failed to load current CSV"); return false;
    }
    if (!inputProcessing->loadNoiseCSV(noisePath)) {
        emit loadError("Failed to load noise CSV"); return false;
    }
    if (!inputProcessing->loadAlgoFlagsCSV(flagPath)) {
        emit loadError("Failed to load flag CSV"); return false;
    }

    algo->setAlgoDynamics(inputProcessing->getAlgoFlags());
    batteryModel->batteryModelInit(inputProcessing);
    batteryCurrGen->setAlgo(algo);
    voltageEstimator->voltageEstimatorInit(batteryModel);

    globalSampleIndex = 0;
    electricCharges   = {0.0, 0.0, 0.0};

    emit loadSuccess();
    return true;
}

/*******************************************************************************
 * step
 ******************************************************************************/
void SimulatorContainer::step()
{
    if (isFinished()) return;

    // Take current and time for every step from file
    const auto &currVec = inputProcessing->getCurrent();
    const auto &timeVec = inputProcessing->getTime();

    // Take noise per step
    noiseParameters_e noise = noiseGenerator->getNoise(globalSampleIndex);

    double iSysA = 0.0;
    if (globalSampleIndex < currVec.size())
        iSysA = currVec[globalSampleIndex] / 1000.0;

    // Superimpose the noise and current
    //iSysA += noise.currentNoise;

    // Take time in seconds
    if (globalSampleIndex < timeVec.size())
        timeS = timeVec[globalSampleIndex] / 1000.0f;

    double socIsysRef = socReference->calculateRefSoCIsys(iSysA, 0.01) * 100.0;

    // Take system current
    double iSys = batteryCurrGen->getSystemCurrent(iSysA);

    // Entering system current in BCG and run all simulators steps...
    batteryCurrGen->getBcgParams(iSys);

    // Take step parameters
    double iBat      = batteryCurrGen->getIBat();
    double vBat      = voltageEstimator->getVTerminal();
    double soc       = batteryModel->getSoC() * 100.0;
    double iPlatform = batteryCurrGen->getIplatform();

    // Take flag to write in a file
    current_flag_e          outputFlag   = batteryCurrGen->getOutputFlag();

    // Take algo duration
    algoTimeTableDuration_e algoDuration = timeTable->getWhitchAlgo();

    // Calculate time interval between 2 samples
    double dt = 0.0;
    if (globalSampleIndex > 0 && globalSampleIndex < timeVec.size())
        dt = (timeVec[globalSampleIndex] - timeVec[globalSampleIndex - 1]) / 1000.0;

    // Calculate cumulative electric charge
    electricCharges.electricChargeBattery += (iBat  * dt) / 3.6;
    electricCharges.electricChargeSystem  += (iSysA * dt) / 3.6;
    electricCharges.electricChargeAlgo     = algo->getQBath();

    // Write every sample in file
    writeFileLine(outputStream,
                  globalSampleIndex, outputFlag,
                  iBat, vBat, iPlatform,
                  socIsysRef, soc, algoDuration,
                  algo->getQBath(),
                  electricCharges.electricChargeSystem,
                  electricCharges.electricChargeAlgo);

    // Save last values
    lastValues.lastIBat                  = iBat;
    lastValues.lastVBat                  = vBat;
    lastValues.lastISys                  = iSys;
    lastValues.lastIPlatform             = iPlatform;
    lastValues.lastSoC                   = soc;
    lastValues.lastSoCIsys               = socIsysRef;
    lastValues.lastElectricChargeBattery = electricCharges.electricChargeBattery;
    lastValues.lastElectricChargeSystem  = electricCharges.electricChargeSystem;
    lastValues.lastElectricChargeAlgo    = electricCharges.electricChargeAlgo;
    lastValues.lastUserSocAlgoOutput     = algo->getAlgoSoCOutput();

    globalSampleIndex++;
}

/*******************************************************************************
 * reset
 ******************************************************************************/
void SimulatorContainer::reset(double SocReference)
{
    globalSampleIndex = 0;
    electricCharges   = {0.0, 0.0, 0.0};
    voltageEstimator->voltageEstimatorInit(batteryModel);
    socReference->socReferenceSet(SocReference);
    socReference->setSocIsys(SocReference);
    batteryModel->reset(SocReference);
    algo->algoReset();
}

/*******************************************************************************
 * isFinished
 ******************************************************************************/
bool SimulatorContainer::isFinished() const
{
    return globalSampleIndex >= inputProcessing->getNumberOfSamples();
}
