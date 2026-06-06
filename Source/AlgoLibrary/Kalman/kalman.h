/*
 * kalman.h
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */

#ifndef KALMAN_KALMAN_H_
#define KALMAN_KALMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "../Operations/Matrix/matrix_v2.h"
#include "../Operations/Matrix/matrix_types.h"

/* Hookovi: funkcije za OCV i dVOC/dSOC kad NE koristimo polinom */
typedef float_point_t (*ocv_func_t)(float_point_t soc);
typedef float_point_t (*dvoc_func_t)(float_point_t soc);

/* Glavna struktura za DP model: x=[SoC, v1, v2]^T */
typedef struct {
    matrix_t x;    /* 3x1 */
    matrix_t P;    /* 3x3 */
    matrix_t H;    /* 1x3 */
    matrix_t Rk;   /* 1x1 */

    /* --- Persisted discrete model & scratch --- */
    matrix_t F;    /* 3x3 */
    matrix_t Ft;   /* 3x3 (transpose of F, pre-built) */
    matrix_t B;    /* 3x1 */
    matrix_t Qk;   /* 3x3 (process noise, diag) */

    /* Reusable scratch (to avoid stack locs each call) */
    matrix_t FP;      /* 3x3 */
    matrix_t FP_Ft;   /* 3x3 */
    matrix_t Bu;      /* 3x1 */

    float_point_t residual; /* Vmeas - Vpred */

    /* Model context */
    double Ts_s;      /* sampling time [s] used to build F,B */
    double Q_mAh;     /* capacity used to build B */
    uint8_t model_valid; /* 1 if F/B/Qk/Ft reflect current params */
} Kalman;

/* Inicijalizacija – korisnik puni x, P, Rk spolja po potrebi */
void KALMAN_Init(Kalman* k);

uint8_t KALMAN_Predict(Kalman* k, float_point_t I_meas_mA,  float_point_t Ts_s, float_point_t Q_mAh, int* perf);

/* UPDATE korak (merenje napona): */
float_point_t KALMAN_Update(Kalman* k, float_point_t I_meas_mA, float_point_t V_meas);

#ifdef __cplusplus
}
#endif

#endif /* KALMAN_KALMAN_H_ */
