/**
 * @file    kalman_adaptive.h
 * @brief   Header file for kalman_adaptive.c
 * @version 1.0.0
 * @date    05.03.2026
 * @author  Filip Radojevic & Haris Turkmanovic
 */

#ifndef KALMAN_ADAPTIVE_H
#define KALMAN_ADAPTIVE_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "kalman.h"
#include "kalman0.h"
#include "../LoadProfiler/load_profiler.h"
#include "../Operations/Parameters/parameters.h"
#include "../Operations/Poly/poly.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

#define INPUT_FILE_PATH           "current_pattern.csv" /* Path to input CSV file with current data */
#define ADAPTIVE_MAX_SAMPLES      150000                /* Maximum number of samples to process (adjust as needed) */

/* Signal */
#define BASE_SAMPLING_TIME        0.01f                 /* [s] */

/* Battery */
#define BATTERY_CAPACITY_mAh      457.0f                /* [mAh] */
#define SOC_START_KALMAN          100.0f                /* [%] */
#define SOC_START_SIGNAL          100.0f                /* [%] */

/* Noise */
#define CURRENT_NOISE_VARIANCE    0.000002f             /* [A^2] */
#define VOLTAGE_NOISE_VARIANCE    0.000002f             /* [V^2] */

/* Platform */
#define PLATFORM_CURRENT_MA       10.0f                 /* [mA] */

/* Algorithm durations */
#define K0_ALGORITHM_DURATION_MS  1.5f                  /* [ms] */
#define K2_ALGORITHM_DURATION_MS  5.0f                  /* [ms] */
#define CC_ALGORITHM_DURATION_MS  0.01f                 /* [ms] */

/* K2 variances */
#define K2_Q_VF                   0.0001f               /* Variance of voltage error in K2 */
#define K2_Q_VS                   1e-5f                 /* Variance of SoC error in K2 */
#define K2_Q_VF_H                 1e-1f                 /* Variance of voltage error in K2-Hybrid */
#define K2_Q_VS_H                 1e-2f                 /* Variance of SoC error in K2-Hybrid */

/* SoC variances */
#define TARGET_SOC_VAR_K2_H       0.02f                 /* Target SoC variance for K2-Hybrid */
#define TARGET_SOC_VAR_K0_H       0.05f                 /* Target SoC variance for K0-Hybrid */

/* Execution periods */
#define K0_EXECUTION_PERIOD       0.01f                 /* [s] */
#define K2_EXECUTION_PERIOD       0.55f                 /* [s] */

/* LoadProfiler */
#define LP_TARGET_DSOC            0.5f                  /* [%] */
#define LP_PROFILER_PERIOD        0.03f                 /* [s] */
#define LP_MAX_SAMPLING_TIME      85.0f                 /* [s] */
#define LP_LOW_CURRENT_THR        0.02f                 /* [A] */
#define LP_DELTA_T_THR            0.5f                  /* [s] */

/* Hybrid logic */
#define MAX_SYSTEM_CURRENT        5.0f                  /* [A] */
#define K2_ENTRY_CURRENT_THR      0.2f                  /* [A] */
#define RES_EXIT_THRESHOLD        0.02f                 /* [A] */
#define RES_DURATION_LIMIT        200.0f                /* [s] */

/*******************************************************************************
 * Typedefs
 ******************************************************************************/

typedef struct {
    double voltage;
    double current;
    double time;
} KalmanSample;


#if CHOOSE_PLATFORM == 1

typedef struct {
    FILE *lp;
    FILE *results;
} SimOutputFiles;

#endif

typedef struct {
    Kalman        *k2_h;
    Kalman0       *k0_h;

    KalmanSample  *data;
    int            num_samples;

#if CHOOSE_PLATFORM == 1
    
    SimOutputFiles files;

#endif
} AdaptiveKalman;

/*******************************************************************************
 * Public API
 ******************************************************************************/

int KALMAN_ADAPTIVE_Run(AdaptiveKalman *ctx);

#ifdef __cplusplus
}
#endif


#endif /* KALMAN_ADAPTIVE_H */
