/*
 * parameters.c
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */
#include "parameters.h"
#include "parameters_config.h"
#include "../../Profiler/profiler.h"

static uint32_t prvPARAMETERS_LAST_REGION;
static uint32_t prvPARAMETERS_FIRST_SREACH;
// Filip: Modified
extern  const float_point_t PARAMETERS_BATTERY[7][PARAMETERS_BATTERY_REGION_NUMBER_DOD];
// extern  const float_point_t PARAMETERS_BATTERY[7][PARAMETERS_BATTERY_REGION_NUMBER];
extern  const float_point_t PARAMETERS0_BATTERY[3][PARAMETERS_BATTERY_REGION_NUMBER];
extern  const float_point_t PARAMETERS_KALMAN_INIT_SOC_VARIANCE;
extern  const float_point_t PARAMETERS_KALMAN0_INIT_SOC_VARIANCE;
extern  const float_point_t PARAMETERS_KALMAN_INIT_VF_VARIANCE;
extern  const float_point_t PARAMETERS_KALMAN_INIT_VS_VARIANCE;
extern  const float_point_t PARAMETERS_KALMAN_INIT_MEASUREMENT_VARIANCE;
extern  const float_point_t PARAMETERS_KALMAN_INIT_X[3];
extern  const float_point_t PARAMETERS_KALMAN_INIT_P[3];
extern  const float_point_t PARAMETERS_KALMAN0_INIT_X0;
extern  const float_point_t PARAMETERS_KALMAN0_INIT_P0;

static inline float_point_t fpt_abs(float_point_t x) { return (x < 0) ? -x : x; }


static uint32_t prvPARAMETERS_GetRegionBasedOnSoC(float_point_t soc)
{
    /* clamp SoC to [0,1] without using math.h */
    if (soc < 0.0) soc = 0.0;
    else if (soc > 1.0) soc = 1.0;

    /* DoD row */
    // Filip: Modified

    float_point_t current_dod_pct = soc * 100.0; // convert to percentage
    const float_point_t *dod_table = PARAMETERS_BATTERY[1];
    const uint32_t N = PARAMETERS_BATTERY_REGION_NUMBER_DOD;

    for (uint32_t i = 0; i < N - 1; i++)
    {
        if (current_dod_pct < dod_table[i] && current_dod_pct >= dod_table[i + 1])
        {
            return i;
        }
    }

    return 0;
}

uint8_t PARAMETERS_Init()
{
	prvPARAMETERS_LAST_REGION = 0;
	prvPARAMETERS_FIRST_SREACH = 1;

    return 0;
}



/* Unified definition: function always exists, behavior depends on config */
float_point_t PARAMETERS_GetOCV(float_point_t soc, int* perf)
{

// Koristi lookup tabelu!!!
// #if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 1
#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 0

    extern  const float_point_t PARAMETERS_OCV_INFO[3][PARAMETERS_CONFIG_OCV_SIZE];
    const float_point_t *xs = PARAMETERS_OCV_INFO[0];
    const float_point_t *ys = PARAMETERS_OCV_INFO[1];
    const int N = PARAMETERS_CONFIG_OCV_SIZE;
#if PARAMETERS_USE_PERF == 1
    PROFILER_Reset();
    PROFILER_Start();
#endif

    if (soc <= xs[0])   { PROFILER_Stop(); *perf = PROFILER_GetValue(); return ys[0]; }
    if (soc >= xs[N-1]) { PROFILER_Stop(); *perf = PROFILER_GetValue(); return ys[N-1]; }

    int lo = 0, hi = N - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (xs[mid] < soc) lo = mid + 1;
        else               hi = mid;
    }

    int idx_right = lo;
    int idx_left  = (idx_right > 0) ? (idx_right - 1) : idx_right;

    float_point_t dleft  = soc - xs[idx_left];
	float_point_t dright = xs[idx_right] - soc;
	/* Either use our abs… */
	int idx = (fpt_abs(dright) < fpt_abs(dleft)) ? idx_right : idx_left;
	/* …or compare squared distances to avoid even the branch above:
	   // int idx = (dright*dright < dleft*dleft) ? idx_right : idx_left;
	*/

#if PARAMETERS_USE_PERF == 1
    PROFILER_Stop();
    *perf = PROFILER_GetValue();
#else
    *perf = 0;
#endif
    return ys[idx];

#else
    /* Lookup table disabled — empty implementation */
    (void)soc;
    *perf = 0;
    return 0.0;
#endif
}
float_point_t PARAMETERS_GetOCVDer(float_point_t soc, int* perf)
{
#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 1

    extern const float_point_t PARAMETERS_OCV_INFO[3][PARAMETERS_CONFIG_OCV_SIZE];
    const float_point_t *xs = PARAMETERS_OCV_INFO[0];
    const float_point_t *ys_der = PARAMETERS_OCV_INFO[2];
    const int N = PARAMETERS_CONFIG_OCV_SIZE;

#if PARAMETERS_USE_PERF == 1
    PROFILER_Reset();
    PROFILER_Start();
#endif

    if (soc <= xs[0])   { PROFILER_Stop(); *perf = PROFILER_GetValue(); return ys_der[0]; }
    if (soc >= xs[N-1]) { PROFILER_Stop(); *perf = PROFILER_GetValue(); return ys_der[N-1]; }

    int lo = 0, hi = N - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (xs[mid] < soc) lo = mid + 1;
        else               hi = mid;
    }

    int idx_right = lo;
    int idx_left  = (idx_right > 0) ? (idx_right - 1) : idx_right;

    /* Compare distances without fabs() */
    float_point_t dleft  = soc - xs[idx_left];
    float_point_t dright = xs[idx_right] - soc;
    /* Either use our abs… */
    int idx = (fpt_abs(dright) < fpt_abs(dleft)) ? idx_right : idx_left;
    /* …or compare squared distances to avoid even the branch above:
       // int idx = (dright*dright < dleft*dleft) ? idx_right : idx_left;
    */


#if PARAMETERS_USE_PERF == 1
    PROFILER_Stop();
    *perf = PROFILER_GetValue();
#else
    *perf = 0;
#endif
    return ys_der[idx];

#else
    /* Lookup table disabled — empty implementation */
    (void)soc;
    *perf = 0;
    return 0.0;
#endif
}

void PARAMETERS_GetReistanceCapacitanceAlways(float_point_t soc, parameters_t* parameter)
{
    int regionNumber = prvPARAMETERS_GetRegionBasedOnSoC(soc);
    parameter->r0 = PARAMETERS_BATTERY[R_AVERAGE_ROW][regionNumber];
    parameter->rf = PARAMETERS_BATTERY[R_FAST_ROW][regionNumber];
    parameter->rs = PARAMETERS_BATTERY[R_SLOW_ROW][regionNumber];
    parameter->cf = PARAMETERS_BATTERY[C_FAST_ROW][regionNumber];
    parameter->cs = PARAMETERS_BATTERY[C_SLOW_ROW][regionNumber];
}

uint8_t PARAMETERS_GetReistanceCapacitance(float_point_t soc, parameters_t* parameter, int* perf)
{
	int regionNumber = 0;
	*perf = 0;
	uint8_t regionChanged = 0;
#if PARAMETERS_USE_PERF == 1
    PROFILER_Reset();
    PROFILER_Start();
#endif
	regionNumber = prvPARAMETERS_GetRegionBasedOnSoC(soc);
#if PARAMETERS_USE_PERF == 1
    PROFILER_Stop();
    *perf = PROFILER_GetValue();
#else
    *perf = 0;
#endif
    if ((prvPARAMETERS_FIRST_SREACH == 1) || (prvPARAMETERS_LAST_REGION != regionNumber))
	{
    	regionChanged = 1;
        // Filip: Modified
    	parameter->r0 = PARAMETERS_BATTERY[R_AVERAGE_ROW][regionNumber];
    	parameter->rf = PARAMETERS_BATTERY[R_FAST_ROW][regionNumber];
    	parameter->rs = PARAMETERS_BATTERY[R_SLOW_ROW][regionNumber];
    	parameter->cf = PARAMETERS_BATTERY[C_FAST_ROW][regionNumber];
    	parameter->cs = PARAMETERS_BATTERY[C_SLOW_ROW][regionNumber];
    	
        prvPARAMETERS_FIRST_SREACH = 0;
        prvPARAMETERS_LAST_REGION = regionNumber;

	}
    return regionChanged;
}
/* Return process variances (Q) */
uint8_t PARAMETERS_GetProcessVariances(float_point_t* qsoc, float_point_t* qfast, float_point_t* qslow)
{
    if (!qsoc || !qfast || !qslow) return 0;

    *qsoc  = PARAMETERS_KALMAN_INIT_SOC_VARIANCE;
    *qfast = PARAMETERS_KALMAN_INIT_VF_VARIANCE;
    *qslow = PARAMETERS_KALMAN_INIT_VS_VARIANCE;

    return 1;  /* success */
}

/* Return measurement variance (R) */
uint8_t PARAMETERS_GetMeasurementVariances(float_point_t* r)
{
    if (!r) return 0;

    *r = PARAMETERS_KALMAN_INIT_MEASUREMENT_VARIANCE;

    return 1;  /* success */
}

/* Return initial state values (x0) */
uint8_t PARAMETERS_GetInitialStateValues(float_point_t* x0, float_point_t* x1, float_point_t* x2)
{
    if (!x0 || !x1 || !x2) return 0;

    *x0 = PARAMETERS_KALMAN_INIT_X[0];
    *x1 = PARAMETERS_KALMAN_INIT_X[1];
    *x2 = PARAMETERS_KALMAN_INIT_X[2];

    return 1;  /* success */
}

/* Return initial covariance values (P0 diagonal) */
uint8_t PARAMETERS_GetInitialPValues(float_point_t* p0, float_point_t* p1, float_point_t* p2)
{
    if (!p0 || !p1 || !p2) return 0;

    *p0 = PARAMETERS_KALMAN_INIT_P[0];
    *p1 = PARAMETERS_KALMAN_INIT_P[1];
    *p2 = PARAMETERS_KALMAN_INIT_P[2];

    return 1;  /* success */
}
uint8_t PARAMETERS_GetProcessVariancesK0(float_point_t* qsoc)
{
    if (!qsoc) return 0;
    *qsoc = PARAMETERS_KALMAN0_INIT_SOC_VARIANCE;   /* from parameters_data.c */
    return 1;  /* success */
}

uint8_t PARAMETERS_GetInitialStateValuesK0(float_point_t* x0)
{
    if (!x0) return 0;
    *x0 = PARAMETERS_KALMAN0_INIT_X0;               /* from parameters_data.c */
    return 1; /* success */
}

uint8_t PARAMETERS_GetInitialPValuesK0(float_point_t* p0)
{
    if (!p0) return 0;
    *p0 = PARAMETERS_KALMAN0_INIT_P0;               /* from parameters_data.c */
    return 1; /* success */
}
uint8_t PARAMETERS_GetReistanceK0(float_point_t soc, float_point_t* resistance, int* perf)
{
    (void)resistance; /* cannot store back; param passed by value */
    int regionNumber = 0;
    uint8_t regionChanged = 0;

    if (perf) *perf = 0;

#if PARAMETERS_USE_PERF == 1
    PROFILER_Reset();
    PROFILER_Start();
#endif

    regionNumber = prvPARAMETERS_GetRegionBasedOnSoC(soc);

#if PARAMETERS_USE_PERF == 1
    PROFILER_Stop();
    if (perf) *perf = PROFILER_GetValue();
#endif

    if ((prvPARAMETERS_FIRST_SREACH == 1) || (prvPARAMETERS_LAST_REGION != regionNumber))
    {
        regionChanged = 1u;
        prvPARAMETERS_FIRST_SREACH = 0u;

        prvPARAMETERS_LAST_REGION  = regionNumber;
        *resistance = PARAMETERS_BATTERY[2][regionNumber] + 
                      PARAMETERS_BATTERY[4][regionNumber] + 
                      PARAMETERS_BATTERY[6][regionNumber];
    }

    return regionChanged;
}

float_point_t PARAMETERS_ComputeVarianceU(float_point_t R, float_point_t C, float_point_t Ts, float_point_t I)
{
    float_point_t tau = R * C;
    float_point_t alpha = expf(-Ts / tau);
    float_point_t b = R * (1.0f - alpha);
    float_point_t var_u = (b * I) * (b * I);
    return var_u;
}

float_point_t PARAMETERS_SocDeviationAndVariance(float_point_t target_soc)
{
    float_point_t sigma = (target_soc / 100.0f) / 3.0f;
    return sigma * sigma;
}
