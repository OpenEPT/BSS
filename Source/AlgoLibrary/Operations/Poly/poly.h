/*
 * poly.h
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "../../Kalman/kalman_config.h"

#ifndef OPERATIONS_POLY_POLY_H_
#define OPERATIONS_POLY_POLY_H_


#define POLY_USE_INTERNAL_POW KALMAN_CONFIG_POLY_USE_INTERNAL_POW
#define POLY_USE_PERF 0


float_point_t POLY_Eval_SoC(float_point_t soc, int* perf);
float_point_t POLY_Eval_SoC_Der(float_point_t soc, int* perf);

#ifdef __cplusplus
}
#endif

#endif /* OPERATIONS_POLY_POLY_H_ */
