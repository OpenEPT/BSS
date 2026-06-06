/*
 * kalman0.c
 *
 *  Created on: Oct 7, 2025
 *      Author: elektronika
 *
 *  Scalar EKF (1x1) for the simple Rint model.
 */

#include "kalman0.h"
#include "../Operations/Poly/poly.h"          /* POLY_Eval_SoC / _Der */
#include "../Profiler/profiler.h"
#include "../Operations/Parameters/parameters.h"
#include <math.h>

/* Cached parameters from region search (we only use r0 in Rint) */
static parameters_t prvKALMAN0_PARAMETERS;

///* Rebuild discrete model (scalar):
// *   F  = 1
// *   B0 = -(Ts_s / Q_mAh)
// */
static void KALMAN0_RebuildDiscreteModel(Kalman0* k,
                                        const parameters_t* prm,
                                        double Ts_s,
                                        float_point_t Q_mAh)
{
   (void)prm; /* prm kept for symmetry/possible extensions; unused in Rint */

   /* F is constant 1 for Rint */
   k->F  = 1.0f;

   /* Input gain for SoC propagation with current in mA and capacity in mAh */
   // Filip: Modified
   k->B0 = (float)(-Ts_s / (Q_mAh * 3.6));

   /* Save context */
   k->Ts_s       = Ts_s;
   k->Q_mAh      = Q_mAh;
   k->model_valid = 1u;
}

/* Inicijalizacija – korisnik puni x, P, Rk spolja po potrebi (scalar varijanta) */
void KALMAN0_Init(Kalman0* k)
{
    PARAMETERS_Init();

    /* Defaults / zero */
    k->x = 0.0f;
    k->P = 0.0f;

    k->H  = 0.0f;
    k->Rk = 0.0f;

    k->F  = 1.0f;       /* constant for Rint */
    k->B0 = 0.0f;
    k->Qk = 0.0f;

    k->Ts_s        = 0.0;
    k->Q_mAh       = 0.0;
    k->model_valid = 0u;

    k->use_poly    = 1u;   /* default: use polynomial OCV/dVOC */

    /* --- Pull initial parameters from parameters_data (.kalman_parameters) --- */
    float_point_t qsoc = 0.0;
    (void)PARAMETERS_GetProcessVariancesK0(&qsoc); /* use qsoc only */

    float_point_t rmeas = 0.0;
    (void)PARAMETERS_GetMeasurementVariances(&rmeas);

    float_point_t x0 = 0.0;
    (void)PARAMETERS_GetInitialStateValuesK0(&x0); /* use x0 only */

    float_point_t p0 = 0.0;
    (void)PARAMETERS_GetInitialPValuesK0(&p0);     /* use p0 only */

    /* Apply scalars */
    k->x  = x0;
    k->P  = p0;
    k->Qk = qsoc;
    k->Rk = rmeas;

    /* Leave B0 unset until first predict (depends on Ts/Q). */
}

/*
 * Predict step (scalar Rint):
 *   x <- F*x + B0*I        (F = 1)
 *   P <- F*P*F + Qk  => P <- P + Qk
 *
 * Returns: 1 if region changed (params updated), 0 otherwise.
 */
uint8_t KALMAN0_Predict(Kalman0* k,
                        float_point_t I_meas_mA,
                        double Ts_s,
                        float_point_t Q_mAh,
                        int* perf)
{

    int perf_k0_pred_sreach, perf_k0_pred_model_cal, perf_k0_pred_eq, perf_k0_pred_total, dummy = 0;

    uint8_t regionChanged = 0u;

    /* 1) Region/parameter search (kept for consistency; we use r0 in update) */
    PROFILER_Reset();
    PROFILER_Start();

    const double soc_now = (double)k->x; /* SoC in [0..1] */
    regionChanged = PARAMETERS_GetReistanceK0(soc_now, &k->r0, &dummy);

    PROFILER_Stop();
    perf_k0_pred_sreach = PROFILER_GetValue();

    /* 2) Rebuild discrete model only if needed */
    PROFILER_Reset();
    PROFILER_Start();

   if (regionChanged == 1u ||
       k->model_valid == 0u ||
       (k->Ts_s != (double)Ts_s) ||
       (k->Q_mAh != (double)Q_mAh))
   {
       KALMAN0_RebuildDiscreteModel(k, &prvKALMAN0_PARAMETERS, (double)Ts_s, (double)Q_mAh);
   }

    PROFILER_Stop();
    perf_k0_pred_model_cal = PROFILER_GetValue();

    /* 3) Propagate state & covariance (scalar) */
    PROFILER_Reset();
    PROFILER_Start();


    /* x = F*x + B0*I; with F=1 */
    k->x = k->x + k->B0 * I_meas_mA;

    /* P = P + Qk */
    k->P = k->P + k->Qk;

    PROFILER_Stop();
    perf_k0_pred_eq = PROFILER_GetValue();

    perf_k0_pred_total = perf_k0_pred_sreach + perf_k0_pred_model_cal + perf_k0_pred_eq;
    if (perf) { *perf = perf_k0_pred_total; }

    return regionChanged;
}

/*
 * Update step (scalar Rint):
 *   v_pred = voc - I*r0
 *   y      = V_meas - v_pred
 *   H      = dVOC/dSoC
 *   S      = H*P*H + Rk
 *   K      = P*H / S
 *   x      = x + K*y
 *   P      = (1 - K*H) * P
 */
float_point_t KALMAN0_Update(Kalman0* k,
                             float_point_t I_meas_mA,
                             float_point_t V_meas)
{
    int perf_k0_update_poly = 0;
    int perf_k0_update_eq = 0;
    int perf_k0_update_total = 0;
    int dummy            = 0;

    const float_point_t soc = k->x;
    float_point_t voc       = 0.0f;
    float_point_t dvoc_dsoc = 0.0f;

    /* 1) VOC and derivative (polynomial path only in this Rint variant) */
    PROFILER_Reset();
    PROFILER_Start();

	voc        = POLY_Eval_SoC(soc, &dummy);
	dvoc_dsoc  = POLY_Eval_SoC_Der(soc, &dummy);

    PROFILER_Stop();
    perf_k0_update_poly = PROFILER_GetValue();

    /* 2) Scalar EKF update */
    PROFILER_Reset();
    PROFILER_Start();

    /* Measurement prediction */
    float_point_t v_pred = voc - I_meas_mA * (k->r0);

    /* Innovation */
    float_point_t y  = V_meas - v_pred;

    k->residual = y; /* Store for potential diagnostics */

    /* H = dVOC/dSoC */
    k->H = dvoc_dsoc;

    /* S = H*P*H + Rk (scalar) */
    float_point_t S = k->H * k->P * k->H + k->Rk;

    /* Guard against degenerate S (very small or zero) */
    float_point_t K = 0.0f;
    if (S != 0.0f) {
        K = (k->P * k->H) / S;
    } else {
        /* Fall back to no update if S is zero (should not happen in practice) */
        K = 0.0f;
    }

    /* State and covariance updates */
    k->x = k->x + K * y;
    k->P = (1.0f - K * k->H) * k->P;

    PROFILER_Stop();
    perf_k0_update_eq = PROFILER_GetValue();

    perf_k0_update_total = perf_k0_update_poly + perf_k0_update_eq;

    return k->x;
}
