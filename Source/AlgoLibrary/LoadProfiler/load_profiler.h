/**
 * @file    load_profiler.h
 * @brief   Header file for load_profiler.c
 * @version	1.0.0
 * @date    05.03.2026
 * @author  Filip Radojevic & Haris Turkmanovic
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#ifndef LOAD_PROFILER_H
#define LOAD_PROFILER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Defines
 ******************************************************************************/


 /*******************************************************************************
 * Typedefs
 ******************************************************************************/

typedef struct {
    /* Config */
    float capacity_mAh;
    float capacity_As;
    float target_dSoC;            /* [%] */
    float profiler_period;        /* [s] */
    float max_sampling_time;      /* [s] */
    float low_current_threshold;  /* [A] */
    float delta_current_threshold;/* [A] */

    /* State */
    float   prev_current;
    float   prev_period;
    float   prev_deltat;
    float   soc;
    float   delta_t;
    uint8_t initialized;
} LoadProfiler;

typedef struct {
    float   period;   /* preporuceni Ts [s] */
    uint8_t updated;  /* 1 ako treba pokrenuti Kalman */
    float   soc;      /* akumulirani SoC od poslednjeg update-a */
    float   deltat;   /* proteklo vreme od poslednjeg update-a [s] */
    uint8_t wakeup;   /* 1 ako struja presla low threshold prema gore */
} LoadProfilerResult;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

 /* Setup */
void LP_SetAll(LoadProfiler *lp);
void LP_Init                    (LoadProfiler *lp);
void LP_SetCapacity             (LoadProfiler *lp, float capacity_mAh);
void LP_SetSoCThreshold         (LoadProfiler *lp, float dSoC_percent);
void LP_SetPeriodThreshold      (LoadProfiler *lp, float period_threshold);
void LP_SetExecutionPeriod      (LoadProfiler *lp, float profiler_period_s);
void LP_SetMaxSamplingTime      (LoadProfiler *lp, float max_time_s);
void LP_SetLowCurrentThreshold  (LoadProfiler *lp, float threshold_A);
void LP_SetDeltaCurrentThreshold(LoadProfiler *lp, float delta_threshold_A);

/* Execute */
LoadProfilerResult LP_Execute(LoadProfiler *lp, float I_A);

/*******************************************************************************
 * Variables
 ******************************************************************************/    

#ifdef __cplusplus
}
#endif

#endif /* LOAD_PROFILER_H */