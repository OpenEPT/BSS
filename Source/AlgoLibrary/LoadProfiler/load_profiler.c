/**
 * @file    load_profiler.c
 * @brief   Implementation of the Load Profiler module
 * @version	1.0.0
 * @date    22.02.2025
 * @author  Filip Radojevic & Haris Turkmanovic
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "load_profiler.h"
#include "../Kalman/kalman_adaptive.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/


/*******************************************************************************
 * Typedefs
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/    


/*******************************************************************************
 * Code
 ******************************************************************************/

static float lp_fabs(float x) { return x < 0.0f ? -x : x; }

static float _calculate_period(LoadProfiler *lp, float I_A)
{
    if (I_A == 0.0f)
        return lp->max_sampling_time;

    if (lp_fabs(I_A) < lp->low_current_threshold)
        return lp->max_sampling_time;

    float dSoC_fraction = lp->target_dSoC / 100.0f;
    float T = (dSoC_fraction * lp->capacity_As) / lp_fabs(I_A);

    if (T > lp->max_sampling_time)
        return lp->max_sampling_time;

    return T;
}

void LP_SetAll(LoadProfiler *lp)
{
    LP_Init(lp);
    LP_SetCapacity             (lp, BATTERY_CAPACITY_mAh);
    LP_SetSoCThreshold         (lp, LP_TARGET_DSOC);
    LP_SetPeriodThreshold      (lp, LP_DELTA_T_THR);
    LP_SetExecutionPeriod      (lp, LP_PROFILER_PERIOD);
    LP_SetMaxSamplingTime      (lp, LP_MAX_SAMPLING_TIME);
    LP_SetLowCurrentThreshold  (lp, LP_LOW_CURRENT_THR);
    LP_SetDeltaCurrentThreshold(lp, LP_LOW_CURRENT_THR);
}


void LP_Init(LoadProfiler *lp)
{
    lp->capacity_mAh            = 0.0f;
    lp->capacity_As             = 0.0f;
    lp->target_dSoC             = 0.0f;
    lp->profiler_period         = 0.0f;
    lp->max_sampling_time       = 0.0f;
    lp->low_current_threshold   = 0.0f;
    lp->delta_current_threshold = 0.0f;
    lp->prev_current            = 0.0f;
    lp->prev_period             = 0.0f;
    lp->prev_deltat             = 0.0f;
    lp->soc                     = 0.0f;
    lp->delta_t                 = 0.0f;
    lp->initialized             = 0u;
}

void LP_SetCapacity(LoadProfiler *lp, float capacity_mAh)
{
    lp->capacity_mAh = capacity_mAh;
    lp->capacity_As  = capacity_mAh * 3.6f;
}

void LP_SetSoCThreshold(LoadProfiler *lp, float dSoC_percent)
{
    lp->target_dSoC = dSoC_percent;
}

void LP_SetPeriodThreshold(LoadProfiler *lp, float period_threshold)
{
    lp->delta_t = period_threshold;
}

void LP_SetExecutionPeriod(LoadProfiler *lp, float profiler_period_s)
{
    lp->profiler_period = profiler_period_s;
}

void LP_SetMaxSamplingTime(LoadProfiler *lp, float max_time_s)
{
    lp->max_sampling_time = max_time_s;
    lp->prev_period       = max_time_s;
}

void LP_SetLowCurrentThreshold(LoadProfiler *lp, float threshold_A)
{
    lp->low_current_threshold = threshold_A;
}

void LP_SetDeltaCurrentThreshold(LoadProfiler *lp, float delta_threshold_A)
{
    lp->delta_current_threshold = delta_threshold_A;
}

LoadProfilerResult LP_Execute(LoadProfiler *lp, float I_A)
{
    LoadProfilerResult result;
    result.updated = 0u;
    result.wakeup  = 0u;
    result.soc     = lp->soc;
    result.deltat  = lp->prev_deltat;
    result.period  = lp->prev_period;

    float period = lp->prev_period;
    float deltat = lp->prev_deltat;

    /* Wakeup: struja presla low threshold prema gore */
    if (lp->initialized &&
        lp->prev_current < lp->low_current_threshold &&
        I_A > lp->low_current_threshold)
    {
        result.wakeup = 1u;
    }

    /* Prvi poziv - ekvivalent Python prev_current == None */
    if (!lp->initialized)
    {
        period          = 0.0f;
        result.updated  = 1u;
        lp->initialized = 1u;
    }
    else
    {
        lp->delta_t += lp->profiler_period;
        lp->soc     += lp->profiler_period * I_A / lp->capacity_As;

        /* --- Maksimalno vreme isteklo --- */
        if (lp->delta_t >= lp->max_sampling_time)
        {
            if (lp->prev_current < lp->low_current_threshold &&
                I_A > lp->low_current_threshold)
            {
                result.updated = 0u;
            }
            else
            {
                result.updated = 1u;
                lp->soc = 0.0f;
                deltat  = lp->delta_t;
            }
            lp->delta_t = 0.0f;
        }

        /* --- SoC prag predjen --- */
        if (lp->soc >= lp->target_dSoC / 100.0f)
        {
            deltat         = lp->delta_t;
            lp->soc        = 0.0f;
            lp->delta_t    = 0.0f;
            result.updated = 1u;
        }

        /* --- Kraj aktivnog rezima (struja pala ispod threshold) --- */
        if (lp->soc > 0.001f &&
            lp->prev_current > lp->low_current_threshold &&
            I_A < lp->low_current_threshold)
        {
            deltat         = lp->delta_t;
            lp->soc        = 0.0f;
            lp->delta_t    = 0.0f;
            result.updated = 1u;
        }
    }

    if (result.updated)
        period = _calculate_period(lp, I_A);

    lp->prev_current = I_A;
    lp->prev_period  = period;
    lp->prev_deltat  = deltat;

    result.period = period;
    result.deltat = deltat;
    result.soc    = lp->soc;

    return result;
}