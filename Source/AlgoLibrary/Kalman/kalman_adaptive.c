/**
 * @file    kalman_adaptive.c
 * @brief   Adaptive Kalman filter implementation (Hybrid LP-driven)
 * @version 1.0.0
 * @date    05.03.2026
 * @author  Filip Radojevic & Haris Turkmanovic
 */

#include "kalman_adaptive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*******************************************************************************
 * Privatni tipovi
 ******************************************************************************/

typedef struct {
    /* RC terminal voltage state (true model) */
    float_point_t v_slow_term;
    float_point_t v_fast_term;

    /* Hybrid state */
    int           selected_kalman;  /* 1 = K0, 2 = K2 */
    uint8_t       kalman_changed;
    float_point_t res_duration;
    float_point_t soc_h_est;

    /* LP output */
    float_point_t T2;
    float_point_t kep;
    uint8_t       updated2;

    /* Napon za Kalman update */
    float_point_t V_k_h;

    /* Energija */
    float_point_t energy_h;
    float_point_t energy_cc;

    /* Perf brojaci */
    int perf;
    int perf1;
} SimState;

/*******************************************************************************
 * Utility API
 ******************************************************************************/

static void KALMAN_ADAPTIVE_LoadNoise(int num_samples, float *noise_current, float *noise_voltage)
{
#if CHOOSE_PLATFORM == 1
    FILE *f = fopen("Input_Files/noise.csv", "r");
    if (!f) {
        printf("Nema noise.csv, koristim nule\n");
        memset(noise_current, 0, num_samples * sizeof(float));
        memset(noise_voltage, 0, num_samples * sizeof(float));
    } else {
        for (int i = 0; i < num_samples; i++)
            (void)fscanf(f, "%f,%f", &noise_current[i], &noise_voltage[i]);
        fclose(f);
    }
#else
    memset(noise_current, 0, num_samples * sizeof(float));
    memset(noise_voltage, 0, num_samples * sizeof(float));
#endif
}

static void KALMAN_ADAPTIVE_CalculateDoD(const KalmanSample *data, int num_samples,
                     float_point_t *DoD, float_point_t Q_C)
{
    float_point_t cumsum = 0.0f;
    for (int i = 0; i < num_samples; i++) {
        cumsum    += (float_point_t)data[i].current * BASE_SAMPLING_TIME;
        DoD[i]     = (SOC_START_SIGNAL / 100.0f) - (cumsum / Q_C * 3600.0f);
    }
}

static void KALMAN_ADAPTIVE_InitKalmans(AdaptiveKalman *ctx)
{
    float_point_t soc_var_k2_h = PARAMETERS_SocDeviationAndVariance(TARGET_SOC_VAR_K2_H);
    float_point_t soc_var_k0_h = PARAMETERS_SocDeviationAndVariance(TARGET_SOC_VAR_K0_H);

    KALMAN_Init(ctx->k2_h);
    ctx->k2_h->x.data[0][0]  = SOC_START_KALMAN / 100.0f;
    ctx->k2_h->x.data[1][0]  = 0.0f;
    ctx->k2_h->x.data[2][0]  = 0.0f;
    ctx->k2_h->Qk.data[0][0] = soc_var_k2_h;
    ctx->k2_h->Qk.data[1][1] = K2_Q_VF_H;
    ctx->k2_h->Qk.data[2][2] = K2_Q_VS_H;
    ctx->k2_h->Rk.data[0][0] = VOLTAGE_NOISE_VARIANCE;

    KALMAN0_Init(ctx->k0_h);
    ctx->k0_h->x  = SOC_START_KALMAN / 100.0f;
    ctx->k0_h->P  = 0.0f;
    ctx->k0_h->Qk = soc_var_k0_h;
    ctx->k0_h->Rk = VOLTAGE_NOISE_VARIANCE;
}

static void KALMAN_ADAPTIVE_LoadProfilerUpdate(SimState *s, LoadProfiler *lp,
                                float_point_t *I_k_term,
                                float_point_t E_INC_CC)
{
    if (*I_k_term * 1000.0f < PLATFORM_CURRENT_MA)
        *I_k_term += PLATFORM_CURRENT_MA / 1000.0f;

    /* LP Update*/
    LoadProfilerResult lp_res = LP_Execute(lp, *I_k_term);
    s->T2       = lp_res.period;
    s->updated2 = lp_res.updated;
    s->kep      = lp_res.deltat;

    /* K0 → K2 if current exceeds threshold */
    if (*I_k_term > K2_ENTRY_CURRENT_THR && s->selected_kalman == 1) {
        s->kalman_changed  = 1u;
        s->selected_kalman = 2;
        s->res_duration    = 0.0f;
    }

    /* Update Energy Consumption */
    s->energy_h  += E_INC_CC;
    s->energy_cc += E_INC_CC;
}

static void KALMAN_ADAPTIVE_UpdateTerminalVoltage(SimState *s, float_point_t I,
                                   const parameters_t *p)
{
    float_point_t a_s = exp(-BASE_SAMPLING_TIME / (p->rs * p->cs));
    float_point_t a_f = exp(-BASE_SAMPLING_TIME / (p->rf * p->cf));
    s->v_slow_term = (s->v_slow_term - I * p->rs) * a_s + I * p->rs;
    s->v_fast_term = (s->v_fast_term - I * p->rf) * a_f + I * p->rf;
}

static void KALMAN_ADAPTIVE_UpdateState(SimState *s, AdaptiveKalman *ctx, int i,
                         float_point_t I_k_term, float_point_t DoD,
                         float_point_t noise_v,
                         const float_point_t *soc_true_list,
                         float_point_t E_INC_K0, float_point_t E_INC_K2)
{
    /* Reinit Kalman if changed */
    if (s->kalman_changed) {
        if (s->selected_kalman == 1) {
            ctx->k0_h->x = s->soc_h_est;
            ctx->k0_h->P = 1e-13f;
        } else {
            ctx->k2_h->x.data[0][0] = s->soc_h_est;
            ctx->k2_h->x.data[1][0] = 0.0f;
            ctx->k2_h->x.data[2][0] = 0.0f;
            ctx->k2_h->P.data[0][0] = 1e-12f;
            ctx->k2_h->P.data[1][1] = 0.0f;
            ctx->k2_h->P.data[2][2] = 0.0f;
        }
        s->kalman_changed = 0u;
    }

    /* Calculate Terminal Voltage */
    parameters_t p_h;
    int perf_ocv = 0;
    PARAMETERS_GetReistanceCapacitanceAlways(DoD, &p_h);
    float_point_t ocv_h = PARAMETERS_GetOCV(DoD, &perf_ocv);
    s->V_k_h = ocv_h - (I_k_term * p_h.r0 + s->v_slow_term + s->v_fast_term) + noise_v;

    /* Predict + Update Kalmans */
    if (s->selected_kalman == 1) {
        KALMAN0_Predict(ctx->k0_h, I_k_term, s->T2, BATTERY_CAPACITY_mAh, &s->perf1);
        s->soc_h_est  = KALMAN0_Update(ctx->k0_h, I_k_term, s->V_k_h);
        s->energy_h  += E_INC_K0;
    } else {
        KALMAN_Predict(ctx->k2_h, I_k_term, s->T2, BATTERY_CAPACITY_mAh, &s->perf);
        s->soc_h_est  = KALMAN_Update(ctx->k2_h, I_k_term, s->V_k_h);
        s->energy_h  += E_INC_K2;
    }

#if CHOOSE_PLATFORM == 1
    fprintf(ctx->files.lp,
        "Step %d: I=%.4fA SoC_true=%.4f%% SoC_H=%.4f%% err=%.4f%% "
        "T2=%.2fs Kalman=%s res=%.4f E_H=%.6f\n",
        i,
        (double)I_k_term,
        (double)(soc_true_list[i] * 100.0f),
        (double)(s->soc_h_est * 100.0f),
        (double)((s->soc_h_est - soc_true_list[i]) * 100.0f),
        (double)s->T2,
        s->selected_kalman == 1 ? "K0" : "K2",
        (double)(s->selected_kalman == 1 ? ctx->k0_h->residual : ctx->k2_h->residual),
        (double)s->energy_h);
#else
    (void)soc_true_list; (void)i;
#endif

    /* K2 → K0 switching */
    if (s->selected_kalman == 2) {
        float_point_t res_h = ctx->k2_h->residual;
        if (fabs(res_h) <= RES_EXIT_THRESHOLD)
            s->res_duration += s->kep;
        else
            s->res_duration = 0.0f;

        if (s->res_duration >= RES_DURATION_LIMIT) {
            s->selected_kalman = 1;
            s->kalman_changed  = 1u;
            s->res_duration    = 0.0f;
        }
    }
}

static void KALMAN_ADAPTIVE_WriteResults(AdaptiveKalman *ctx, SimState *s, int i,
                           const float_point_t *soc_true_list,
                           const float_point_t *cc_list,
                           float_point_t I_k_term)
{
#if CHOOSE_PLATFORM == 1
    fprintf(ctx->files.results,
        "%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n",
        i,
        (double)soc_true_list[i+1],
        (double)cc_list[i+1],
        (double)s->soc_h_est,
        (double)s->V_k_h,
        (double)I_k_term,
        (double)(s->selected_kalman == 1 ? ctx->k0_h->residual : ctx->k2_h->residual));
#else
    (void)ctx; (void)s; (void)i;
    (void)soc_true_list; (void)cc_list; (void)I_k_term;
#endif
}

/*******************************************************************************
 * Public API
 ******************************************************************************/

int KALMAN_ADAPTIVE_Run(AdaptiveKalman *ctx)
{
    /* Check if profiler period is a multiple of base sampling time */
    if ((int)(LP_PROFILER_PERIOD * 1000) % (int)(BASE_SAMPLING_TIME * 1000) != 0) {
        printf("ERROR: Profiler period mora biti visekratnik BASE_SAMPLING_TIME!\n");
        return 1;
    }

    /* Initialize arrays */
    static float_point_t soc_true_list [ADAPTIVE_MAX_SAMPLES + 1];
    static float_point_t cc_list       [ADAPTIVE_MAX_SAMPLES + 1];
    static float_point_t DoD_calculated[ADAPTIVE_MAX_SAMPLES];
    static float         noise_current [ADAPTIVE_MAX_SAMPLES];
    static float         noise_voltage [ADAPTIVE_MAX_SAMPLES];

    /* Calculate constants */
    const float_point_t Q_C      = BATTERY_CAPACITY_mAh * 3.6f;
    const float_point_t E_INC_K2 = K2_ALGORITHM_DURATION_MS / 1000.0f * PLATFORM_CURRENT_MA / 3600.0f;
    const float_point_t E_INC_K0 = K0_ALGORITHM_DURATION_MS / 1000.0f * PLATFORM_CURRENT_MA / 3600.0f;
    const float_point_t E_INC_CC = CC_ALGORITHM_DURATION_MS / 1000.0f * PLATFORM_CURRENT_MA / 3600.0f;
    const int           N_profiler = (int)(LP_PROFILER_PERIOD / BASE_SAMPLING_TIME + 0.5f);

    /* Load Noise */
    KALMAN_ADAPTIVE_LoadNoise(ctx->num_samples, noise_current, noise_voltage);
    
    /* Calculate DoD from current data */
    KALMAN_ADAPTIVE_CalculateDoD(ctx->data, ctx->num_samples, DoD_calculated, Q_C);
    
    /* Initialize Kalmans */
    KALMAN_ADAPTIVE_InitKalmans(ctx);

    /* Initialize Load Profiler */
    LoadProfiler lp;
    LP_SetAll(&lp);

    /* Initialize SoC lists */
    soc_true_list[0] = DoD_calculated[0];
    cc_list[0]       = DoD_calculated[0];

    /* Initialize Simulation State */
    SimState s = {
        .selected_kalman = 2,
        .kalman_changed  = 0u,
        .res_duration    = 0.0f,
        .soc_h_est       = SOC_START_KALMAN / 100.0f,
        .T2              = LP_MAX_SAMPLING_TIME,
        .kep             = LP_MAX_SAMPLING_TIME,
        .updated2        = 0u,
    };

    /* Main Simulation Loop */
    for (int i = 0; i < ctx->num_samples; i++)
    {
        /* Load New Values */
        float_point_t I_k      = (float_point_t)ctx->data[i].current;
        float_point_t I_k_term = I_k + noise_current[i];
        float_point_t DoD      = soc_true_list[i];
        s.updated2 = 0u;

        /* Get RC Parameters */
        parameters_t p_term;
        PARAMETERS_GetReistanceCapacitanceAlways(DoD, &p_term);

        /* LP Update */
        if (i % N_profiler == 0)
            KALMAN_ADAPTIVE_LoadProfilerUpdate(&s, &lp, &I_k_term, E_INC_CC);

        /* Update Terminal Voltage */
        KALMAN_ADAPTIVE_UpdateTerminalVoltage(&s, I_k_term, &p_term);

        /* Calculate Adaptive Kalman */
        if (s.updated2) {
            KALMAN_ADAPTIVE_UpdateState(&s, ctx, i, I_k_term, DoD, noise_voltage[i],
                        soc_true_list, E_INC_K0, E_INC_K2);
        }

        /* SoC Progression */
        float_point_t delta_soc    = s.updated2 ? (I_k_term * s.T2 / Q_C) : 0.0f;
        soc_true_list[i+1]         = soc_true_list[i] - (BASE_SAMPLING_TIME * I_k_term) / Q_C;
        cc_list[i+1]               = cc_list[i] - delta_soc;

        /* Write Results */
        KALMAN_ADAPTIVE_WriteResults(ctx, &s, i, soc_true_list, cc_list, I_k_term);
    }

#if CHOOSE_PLATFORM == 1
    fclose(ctx->files.lp);
    fclose(ctx->files.results);
#endif

    printf("Adaptive gotovo.\n");
    return 0;
}