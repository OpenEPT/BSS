#include "NoiseGenerator.h"
#include "SimulatorInputProcessing.h"


NoiseGenerator::NoiseGenerator(SimulatorInput *simulatorInput)
    : simulatorInput(simulatorInput) {
    noise.currentNoise = 0.0f;
    noise.voltageNoise = 0.0f;
}

noiseParameters_e NoiseGenerator::getNoise(int currentIdx) {
    noise = simulatorInput->getNoise(currentIdx);
    return noise;
}
