/*
 * kalman0.h
 *
 *  Created on: Oct 7, 2025
 *      Author: elektronika
 */

#ifndef KALMAN_KALMAN0_H_
#define KALMAN_KALMAN0_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "../Operations/Matrix/matrix_types.h"

/* Glavna (scalar) struktura za Rint model: x = SoC */
typedef struct {
    /* --- State & covariance (1x1) --- */
    float_point_t x;     /* SoC */
    float_point_t P;     /* variance of SoC */

    /* --- Measurement model (1x1) --- */
    float_point_t H;     /* dVOC/dSoC */
    float_point_t Rk;    /* measurement noise variance */

    /* --- Discrete model & noise (1x1) --- */
    float_point_t F;     /* = 1 for Rint */
    float_point_t B0;    /* = -(Ts_s / Q_mAh) */
    float_point_t Qk;    /* process noise variance for SoC */

    float_point_t residual; /* Vmeas - Vpred */

    /* --- Model context --- */
    double  Ts_s;        /* last Ts used to build B0 */
    double  Q_mAh;       /* last capacity used to build B0 */
    uint8_t model_valid; /* 1 if F/B0 reflect current Ts/Q/region */

    float_point_t r0;

    /* --- OCV/dVOC source selection --- */
    uint8_t    use_poly;   /* true => use POLY_Eval_* in .c */
} Kalman0;

/* Inicijalizacija – korisnik puni x, P, Rk spolja po potrebi (scalar varijanta) */
void KALMAN0_Init(Kalman0* k);

/* Predict: radi region search (R0,R1,R2); model build svodi se na B0 i flagove */
uint8_t KALMAN0_Predict(Kalman0* k,
                        float_point_t I_meas_mA,
                        double Ts_s,
                        float_point_t Q_mAh,
                        int* perf);

/* UPDATE korak (merenje napona) – koristi VOC(SoC), dVOC/dSoC i R0 */
float_point_t KALMAN0_Update(Kalman0* k,
                             float_point_t I_meas_mA,
                             float_point_t V_meas);

#ifdef __cplusplus
}
#endif

#endif /* KALMAN_KALMAN0_H_ */
