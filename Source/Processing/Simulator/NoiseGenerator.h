#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

typedef struct {
    float currentNoise;
    float voltageNoise;
} noiseParameters_e;

class SimulatorInput;

class NoiseGenerator {
public:
    explicit NoiseGenerator(SimulatorInput *simulatorInput);
    noiseParameters_e getNoise(int currentIdx);
private:
    SimulatorInput    *simulatorInput;
    noiseParameters_e  noise;
};

#endif // NOISEGENERATOR_H
