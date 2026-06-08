/*
 * kalman_v2.c
 *
 *  Created on: Oct 8, 2025
 *      Author: elektronika
 */
#include "kalman.h"
#include "../Operations/Poly/poly.h"      /* trenutno NOP */
#include "../Profiler/profiler.h"
#include "../Operations/Parameters/parameters.h"

#define MAT_ADDR_3x3(Mptr)  ((uintptr_t)&((Mptr)->data[0][0]))
#define MAT_ADDR_3x1(Mptr)  ((uintptr_t)&((Mptr)->data[0][0]))  /* first column contiguous */
#define MAT_ADDR_1x3(Mptr)  ((uintptr_t)&((Mptr)->data[0][0]))

static parameters_t prvKALMAN_PARAMETERS;
/* ================= Configuration ================ */
/* If your build system doesn't set this, adjust:   */
/* 0 => float_point_t is 32-bit float               */
/* 1 => float_point_t is 64-bit double              */
#ifndef FLOAT_POINT_IS_DOUBLE
#define FLOAT_POINT_IS_DOUBLE 0
#endif

/* ================= Bit-cast helpers ============= */
#if FLOAT_POINT_IS_DOUBLE
static inline uint64_t fe_bits_from_fp(float_point_t f) {
    union { float_point_t f; uint64_t u; } v; v.f = f; return v.u;
}
static inline float_point_t fe_fp_from_bits(uint64_t u) {
    union { float_point_t f; uint64_t u; } v; v.u = u; return v.f;
}
#else
static inline uint32_t fe_bits_from_fp(float_point_t f) {
    union { float_point_t f; uint32_t u; } v; v.f = f; return v.u;
}
static inline float_point_t fe_fp_from_bits(uint32_t u) {
    union { float_point_t f; uint32_t u; } v; v.u = u; return v.f;
}
#endif

/* ================= Constants (no <math.h>) ====== */
#if FLOAT_POINT_IS_DOUBLE
/* double limits for exp() */
#define FP_EXP_MAX  709.0
#define FP_EXP_MIN -745.0
/* +Inf bit pattern (double) */
static inline float_point_t fe_pos_inf(void) { return fe_fp_from_bits(0x7FF0000000000000ULL); }
/* IEEE-754 double layout */
enum { FE_SIGN_SHIFT = 63, FE_EXP_SHIFT = 52 };
enum { FE_EXP_MASK  = 0x7FF };
enum { FE_MAN_MASK64 = 0x000FFFFFFFFFFFFFULL };
enum { FE_BIAS = 1023 };
#else
/* float limits for exp() */
#define FP_EXP_MAX  88.0f
#define FP_EXP_MIN -80.0f
/* +Inf bit pattern (float) */
static inline float_point_t fe_pos_inf(void) { return fe_fp_from_bits(0x7F800000u); }
/* IEEE-754 float layout */
enum { FE_SIGN_SHIFT = 31, FE_EXP_SHIFT = 23 };
enum { FE_EXP_MASK  = 0xFF };
enum { FE_MAN_MASK32 = 0x007FFFFFu };
enum { FE_BIAS = 127 };
#endif

/* Round-to-nearest integer (ties to +inf for >=0 path). No <math.h>. */
static inline int fe_round_to_int(float_point_t y) {
    return (y >= (float_point_t)0.0) ? (int)(y + (float_point_t)0.5)
                                     : (int)(y - (float_point_t)0.5);
}

/* Multiply by 2^k via exponent-bit adjust (no ldexp). */
static inline float_point_t fe_ldexp2k_fp(float_point_t a, int k) {
#if FLOAT_POINT_IS_DOUBLE
    uint64_t u = fe_bits_from_fp(a);
    uint64_t sign = u & (1ULL << FE_SIGN_SHIFT);
    uint64_t exp  = (u >> FE_EXP_SHIFT) & FE_EXP_MASK;
    uint64_t man  = u & FE_MAN_MASK64;

    if ((u << 1) == 0) return (float_point_t)0.0;   /* ±0 */
    if (exp == FE_EXP_MASK)  return a;              /* Inf/NaN */

    if (exp == 0) {
        /* subnormal: normalize mantissa */
        while ((man & (1ULL << FE_EXP_SHIFT)) == 0) {
            man <<= 1;
            k -= 1;
            if (man == 0) return (float_point_t)0.0;
        }
        man &= FE_MAN_MASK64;
        exp = 1;
    }

    int e = (int)exp - FE_BIAS + k;
    if (e >= (FE_EXP_MASK - FE_BIAS)) return fe_pos_inf();
    if (e <= -FE_BIAS) return (float_point_t)0.0;

    uint64_t newExp = (uint64_t)(e + FE_BIAS);
    return fe_fp_from_bits(sign | (newExp << FE_EXP_SHIFT) | man);
#else
    uint32_t u = fe_bits_from_fp(a);
    uint32_t sign = u & (1u << FE_SIGN_SHIFT);
    uint32_t exp  = (u >> FE_EXP_SHIFT) & FE_EXP_MASK;
    uint32_t man  = u & FE_MAN_MASK32;

    if ((u << 1) == 0) return (float_point_t)0.0;   /* ±0 */
    if (exp == FE_EXP_MASK)  return a;              /* Inf/NaN */

    if (exp == 0) {
        /* subnormal: normalize mantissa */
        while ((man & (1u << FE_EXP_SHIFT)) == 0) {
            man <<= 1;
            k -= 1;
            if (man == 0) return (float_point_t)0.0;
        }
        man &= FE_MAN_MASK32;
        exp = 1;
    }

    int e = (int)exp - FE_BIAS + k;
    if (e >= (FE_EXP_MASK - FE_BIAS)) return fe_pos_inf();
    if (e <= -FE_BIAS) return (float_point_t)0.0;

    uint32_t newExp = (uint32_t)(e + FE_BIAS);
    return fe_fp_from_bits(sign | (newExp << FE_EXP_SHIFT) | man);
#endif
}

/* ================= fast exp() (no <math.h>) =============== */
/* 5th-order polynomial on reduced range r in [-ln2/2, ln2/2].
   Good for filters/controls. If you need tighter error, switch to minimax coeffs. */
static inline float_point_t fast_exp_fp_nomath(float_point_t x)
{
    /* clamp to avoid extreme overflow/underflow and keep k reasonable */
    if (x > (float_point_t)FP_EXP_MAX)  return fe_pos_inf();
    if (x < (float_point_t)FP_EXP_MIN)  return (float_point_t)0.0;

    /* constants (as literals) */
#if FLOAT_POINT_IS_DOUBLE
    const float_point_t LN2     = (float_point_t)0.693147180559945309417232121458176568;  /* ln(2) */
    const float_point_t INV_LN2 = (float_point_t)1.44269504088896340735992468100189214;   /* 1/ln(2) */
    /* For double, 5th order is modest accuracy; increase order if you need tighter bounds. */
    const float_point_t c1 = (float_point_t)1.0;
    const float_point_t c2 = (float_point_t)1.0;
    const float_point_t c3 = (float_point_t)0.5;                   /* 1/2!  */
    const float_point_t c4 = (float_point_t)0.16666666666666666;   /* 1/3!  */
    const float_point_t c5 = (float_point_t)0.04166666666666666;   /* 1/4!  */
    const float_point_t c6 = (float_point_t)0.00833333333333333;   /* 1/5!  */
#else
    const float_point_t LN2     = (float_point_t)0.6931471805599453f;
    const float_point_t INV_LN2 = (float_point_t)1.4426950408889634f;
    /* Slightly rounded factorial inverses to be friendly to single-precision */
    const float_point_t c1 = (float_point_t)1.0f;
    const float_point_t c2 = (float_point_t)1.0f;
    const float_point_t c3 = (float_point_t)0.5f;             /* 1/2!  */
    const float_point_t c4 = (float_point_t)0.1666666716f;    /* ~1/3! */
    const float_point_t c5 = (float_point_t)0.0416666679f;    /* ~1/4! */
    const float_point_t c6 = (float_point_t)0.0083333332f;    /* ~1/5! */
#endif

    /* range reduction: x = k*ln2 + r  with small r */
    float_point_t y = x * INV_LN2;
    int k = fe_round_to_int(y);
    float_point_t r = x - (float_point_t)k * LN2;

    /* Estrin scheme for the polynomial: exp(r) ≈ 1 + r + r^2/2 + r^3/6 + r^4/24 + r^5/120 */
    float_point_t r2 = r * r;
    float_point_t t1 = c6 * r + c5;          /* r*c6 + c5 */
    float_point_t t2 = c4 * r + c3;          /* r*c4 + c3 */
    float_point_t t3 = (t1 * r2) + t2;       /* r^3..r^5 terms */
    float_point_t poly = (t3 * r2) + (c2 * r + c1);

    /* scale back: exp(x) = 2^k * exp(r) */
    return fe_ldexp2k_fp(poly, k);
}
static void KALMAN_RebuildDiscreteModel(Kalman* k,
                                        const parameters_t* prm,
										float_point_t Ts_s,
										float_point_t Q_mAh)
{
    /* 1) Alphas */
    float_point_t a_f = fast_exp_fp_nomath(-Ts_s / (prm->rf * prm->cf));
    float_point_t a_s = fast_exp_fp_nomath(-Ts_s / (prm->rs * prm->cs));

    /* 2) B terms (I in mA) */
    float_point_t B0 = -Ts_s / Q_mAh;
    float_point_t B1 = prm->rf * (1.0 - a_f);
    float_point_t B2 = prm->rs * (1.0 - a_s);

    /* 3) F (3x3) */
    k->F.data[0][0] = 1.0f;     k->F.data[0][1] = 0.0f;        k->F.data[0][2] = 0.0f;
    k->F.data[1][0] = 0.0f;     k->F.data[1][1] = a_f;  k->F.data[1][2] = 0.0f;
    k->F.data[2][0] = 0.0f;     k->F.data[2][1] = 0.0f;        k->F.data[2][2] = a_s;

    /* 4) Ft = F^T via addr API */
    MATRIX_transpose_3x3(MAT_ADDR_3x3(&k->F), MAT_ADDR_3x3(&k->Ft));

    /* 5) B (3x1) */
    k->B.data[0][0] = B0;
    k->B.data[1][0] = B1;
    k->B.data[2][0] = B2;

    /* 7) Save context */
    k->Ts_s        = Ts_s;
    k->Q_mAh       = Q_mAh;
    k->model_valid = 1;
}
/* Jednostavna pomoćna: postavi 3x3 identitet u M */

static void mat_eye_3x3(matrix_t* M) {
    M->rows = 3; M->cols = 3;
    M->data[0][0] = 1.0; M->data[0][1] = 0.0; M->data[0][2] = 0.0;
    M->data[1][0] = 0.0; M->data[1][1] = 1.0; M->data[1][2] = 0.0;
    M->data[2][0] = 0.0; M->data[2][1] = 0.0; M->data[2][2] = 1.0;
}

/* Inicijalizacija – korisnik puni x, P, Rk spolja po potrebi */
void KALMAN_Init(Kalman* k)
{
	PARAMETERS_Init();
    /* Core dims */
    k->x.rows = 3;  k->x.cols = 1;
    k->P.rows = 3;  k->P.cols = 3;
    k->H.rows = 1;  k->H.cols = 3;
    k->Rk.rows = 1; k->Rk.cols = 1;

    /* Persisted model matrices */
    k->F.rows = 3;   k->F.cols = 3;
    k->Ft.rows = 3;  k->Ft.cols = 3;
    k->B.rows = 3;   k->B.cols = 1;
    k->Qk.rows = 3;  k->Qk.cols = 3;

    /* Scratch buffers */
    k->FP.rows = 3;     k->FP.cols = 3;
    k->FP_Ft.rows = 3;  k->FP_Ft.cols = 3;
    k->Bu.rows = 3;     k->Bu.cols = 1;

    /* Defaults */
    k->Ts_s = 0.0;
    k->Q_mAh = 0.0;
    k->model_valid = 0;

    /* Zero all matrices we maintain */
    MATRIX_Zero(&k->F);
    MATRIX_Zero(&k->Ft);
    MATRIX_Zero(&k->B);
    MATRIX_Zero(&k->Qk);
    MATRIX_Zero(&k->FP);
    MATRIX_Zero(&k->FP_Ft);
    MATRIX_Zero(&k->Bu);
    MATRIX_Zero(&k->P);
    MATRIX_Zero(&k->x);
    MATRIX_Zero(&k->H);
    MATRIX_Zero(&k->Rk);

    /* --- Pull initial parameters from parameters_data (.kalman_parameters) --- */
    float_point_t qsoc = 0.0, qfast = 0.0, qslow = 0.0;
    (void)PARAMETERS_GetProcessVariances(&qsoc, &qfast, &qslow);

    float_point_t rmeas = 0.0;
    (void)PARAMETERS_GetMeasurementVariances(&rmeas);

    float_point_t x0 = 0.0, x1 = 0.0, x2 = 0.0;
    (void)PARAMETERS_GetInitialStateValues(&x0, &x1, &x2);

    float_point_t p0 = 0.0, p1 = 0.0, p2 = 0.0;
    (void)PARAMETERS_GetInitialPValues(&p0, &p1, &p2);

    /* --- Apply to matrices --- */
    /* State */
    k->x.data[0][0] = (float_point_t)x0;
    k->x.data[1][0] = (float_point_t)x1;
    k->x.data[2][0] = (float_point_t)x2;

    /* Covariance P (diagonal) */
    k->P.data[0][0] = (float_point_t)p0;  k->P.data[0][1] = 0.0;                 k->P.data[0][2] = 0.0;
    k->P.data[1][0] = 0.0;                 k->P.data[1][1] = (float_point_t)p1;  k->P.data[1][2] = 0.0;
    k->P.data[2][0] = 0.0;                 k->P.data[2][1] = 0.0;                 k->P.data[2][2] = (float_point_t)p2;

    /* Process noise Qk (diagonal) */
    k->Qk.data[0][0] = (float_point_t)qsoc;  k->Qk.data[0][1] = 0.0;                   k->Qk.data[0][2] = 0.0;
    k->Qk.data[1][0] = 0.0;                   k->Qk.data[1][1] = (float_point_t)qfast; k->Qk.data[1][2] = 0.0;
    k->Qk.data[2][0] = 0.0;                   k->Qk.data[2][1] = 0.0;                   k->Qk.data[2][2] = (float_point_t)qslow;

    /* Measurement noise Rk (1x1) */
    k->Rk.data[0][0] = (float_point_t)rmeas;

    /* H can be set on each update; leave zero here or set a default if desired */
    /* k->H = [dvoc/dsoc, -1, -1] is filled in KALMAN_Update per current SoC */
}
uint8_t KALMAN_Predict(Kalman* k,
                       float_point_t I_meas_mA,
                       float_point_t Ts_s,
                       float_point_t Q_mAh,
                       int* perf)
{
    int perf_k2_pred_sreach, perf_k2_pred_model_cal, perf_k2_pred_eq, perf_k2_pred_total, dummy = 0;
    uint8_t regionChanged = 0;

    PROFILER_Reset();
    PROFILER_Start();

    float_point_t soc_now = (float_point_t)k->x.data[0][0];   /* SoC in [0..1] */
    regionChanged = PARAMETERS_GetReistanceCapacitance(soc_now, &prvKALMAN_PARAMETERS, &dummy);

    PROFILER_Stop();
    perf_k2_pred_sreach = PROFILER_GetValue();

    PROFILER_Reset();
    PROFILER_Start();

    /* Rebuild discrete model only if needed */
    if (regionChanged == 1 ||
        k->model_valid == 0 ||
        (k->Ts_s != Ts_s) ||
        (k->Q_mAh != Q_mAh))
    {
        KALMAN_RebuildDiscreteModel(k, &prvKALMAN_PARAMETERS, Ts_s, Q_mAh);
    }

    PROFILER_Stop();
    perf_k2_pred_model_cal = PROFILER_GetValue();

    PROFILER_Reset();
    PROFILER_Start();

    /* x = F*x + B*I */
    matrix_t Fx, x_pred;
    Fx.rows = 3; Fx.cols = 1;
    x_pred.rows = 3; x_pred.cols = 1;

    MATRIX_multiply_3x3_3x1(MAT_ADDR_3x3(&k->F), MAT_ADDR_3x1(&k->x), MAT_ADDR_3x1(&Fx));      /* Fx = F*x */
    MATRIX_scalar_multiply_3x1(MAT_ADDR_3x1(&k->B), I_meas_mA, MAT_ADDR_3x1(&k->Bu));          /* Bu = B*I */
    MATRIX_add_3x1(MAT_ADDR_3x1(&Fx), MAT_ADDR_3x1(&k->Bu), MAT_ADDR_3x1(&x_pred));            /* x' = Fx + Bu */
    k->x = x_pred;

    /* P = F*P*F^T + Qk */
    MATRIX_multiply_3x3(MAT_ADDR_3x3(&k->F), MAT_ADDR_3x3(&k->P), MAT_ADDR_3x3(&k->FP));       /* FP = F*P */
    MATRIX_multiply_3x3(MAT_ADDR_3x3(&k->FP), MAT_ADDR_3x3(&k->Ft), MAT_ADDR_3x3(&k->FP_Ft));  /* FP_Ft = FP*Ft */
    MATRIX_add_3x3(MAT_ADDR_3x3(&k->FP_Ft), MAT_ADDR_3x3(&k->Qk), MAT_ADDR_3x3(&k->P));        /* P = FP_Ft + Qk */

    PROFILER_Stop();
    perf_k2_pred_eq = PROFILER_GetValue();

    perf_k2_pred_total = perf_k2_pred_sreach + perf_k2_pred_model_cal + perf_k2_pred_eq;
    *perf = perf_k2_pred_total;
    return regionChanged;
}

float_point_t KALMAN_Update(Kalman* k, float_point_t I_meas_mA, float_point_t V_meas)
{
    int perf_k2_update_eq = 0;
    int perf_k2_update_poly = 0;
    int perf_k2_update_total = 0;
    int dummy = 0;

    float_point_t soc = k->x.data[0][0];
    float_point_t voc = 0.0f;
    float_point_t dvoc_dsoc = 0.0f;

    PROFILER_Reset();
    PROFILER_Start();

    voc        = POLY_Eval_SoC(soc, &dummy);
    dvoc_dsoc  = POLY_Eval_SoC_Der(soc, &dummy);

    PROFILER_Stop();
    perf_k2_update_poly = PROFILER_GetValue();

    PROFILER_Reset();
    PROFILER_Start();

    /* 2) v_pred = voc - x[1] - x[2] - I*R0 */
    float_point_t v_pred = voc - k->x.data[1][0] - k->x.data[2][0] - I_meas_mA * prvKALMAN_PARAMETERS.r0;

    /* 3) y = V_meas - v_pred */
    float_point_t y = V_meas - v_pred;

    /* 4) H = [dvoc_dsoc, -1, -1] (1x3) */
    k->H.rows = 1; k->H.cols = 3;
    k->H.data[0][0] = dvoc_dsoc;
    k->H.data[0][1] = -1.0f;
    k->H.data[0][2] = -1.0f;

    /* 5) Hp = P * H^T  => (3x3)*(3x1) = 3x1 */
    matrix_t HT, Hp;
    HT.rows = 3; HT.cols = 1;
    Hp.rows = 3; Hp.cols = 1;
    MATRIX_transpose_1x3(MAT_ADDR_1x3(&k->H), MAT_ADDR_3x1(&HT));                 /* HT = H^T */
    MATRIX_multiply_3x3_3x1(MAT_ADDR_3x3(&k->P), MAT_ADDR_3x1(&HT), MAT_ADDR_3x1(&Hp)); /* Hp = P*HT */

    /* 6) S = H * Hp + Rk   => (1x3)*(3x1) + (1x1) = (1x1) scalar */
    float_point_t S_scalar = 0.0f;
    {
        float_point_t S_1x1;    /* 1x1 buffer for result of 1x3 * 3x1 */
        MATRIX_multiply_1x3_3x1(MAT_ADDR_1x3(&k->H), MAT_ADDR_3x1(&Hp), (uint32_t)(uintptr_t)&S_1x1);
        S_scalar = S_1x1 + k->Rk.data[0][0];
    }

    /* 7) K = Hp / S  -> multiply by invS */
    matrix_t K;
    K.rows = 3; K.cols = 1;
    float_point_t invS = 1.0f / S_scalar;
    MATRIX_scalar_multiply_3x1(MAT_ADDR_3x1(&Hp), invS, MAT_ADDR_3x1(&K));

    /* 8) x = x + K*y */
    matrix_t Ky, x_new;
    Ky.rows = 3; Ky.cols = 1;
    x_new.rows = 3; x_new.cols = 1;
    MATRIX_scalar_multiply_3x1(MAT_ADDR_3x1(&K), y, MAT_ADDR_3x1(&Ky));           /* Ky = K*y */
    MATRIX_add_3x1(MAT_ADDR_3x1(&k->x), MAT_ADDR_3x1(&Ky), MAT_ADDR_3x1(&x_new)); /* x' = x + Ky */
    k->x = x_new;

    /* 9) P = (I - K*H) * P */
    matrix_t KH, I3, I_minus_KH, P_new;
    KH.rows = 3; KH.cols = 3;
    I3.rows = 3; I3.cols = 3;
    I_minus_KH.rows = 3; I_minus_KH.cols = 3;
    P_new.rows = 3; P_new.cols = 3;

    MATRIX_multiply_3x1_1x3(MAT_ADDR_3x1(&K), MAT_ADDR_1x3(&k->H), MAT_ADDR_3x3(&KH)); /* KH = K*H */

    /* I3 = eye(3) */
    I3.data[0][0]=1.0f; I3.data[0][1]=0.0f; I3.data[0][2]=0.0f;
    I3.data[1][0]=0.0f; I3.data[1][1]=1.0f; I3.data[1][2]=0.0f;
    I3.data[2][0]=0.0f; I3.data[2][1]=0.0f; I3.data[2][2]=1.0f;

    MATRIX_subtract_3x3(MAT_ADDR_3x3(&I3), MAT_ADDR_3x3(&KH), MAT_ADDR_3x3(&I_minus_KH)); /* I - KH */
    MATRIX_multiply_3x3(MAT_ADDR_3x3(&I_minus_KH), MAT_ADDR_3x3(&k->P), MAT_ADDR_3x3(&P_new));
    k->P = P_new;

    PROFILER_Stop();
    perf_k2_update_eq = PROFILER_GetValue();
    perf_k2_update_total = perf_k2_update_eq + perf_k2_update_poly;

    return k->x.data[0][0];
}


