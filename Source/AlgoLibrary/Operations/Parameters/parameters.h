/*
 * parameters.h
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */


#ifndef OPERATIONS_PARAMETERS_PARAMETERS_H_
#define OPERATIONS_PARAMETERS_PARAMETERS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "../Matrix/matrix_types.h"

#define PARAMETERS_USE_PERF  0

// Filip: Modified
#define DoD_ROW 0
#define R_SLOW_ROW 2
#define C_SLOW_ROW 3
#define R_FAST_ROW 4
#define C_FAST_ROW 5
#define R_AVERAGE_ROW 6

typedef struct
{
	float_point_t r0;
	float_point_t rf;
	float_point_t rs;
	float_point_t cf;
	float_point_t cs;
}parameters_t;

uint8_t PARAMETERS_Init();
float_point_t 	PARAMETERS_GetOCV(float_point_t soc, int* perf);
float_point_t 	PARAMETERS_GetOCVDer(float_point_t soc, int* perf);
void PARAMETERS_GetReistanceCapacitanceAlways(float_point_t soc, parameters_t* parameter);
uint8_t PARAMETERS_GetReistanceCapacitance(float_point_t soc, parameters_t* parameter, int* perf);
uint8_t PARAMETERS_GetProcessVariances(float_point_t* qsoc, float_point_t* qfast, float_point_t* qslow);
uint8_t PARAMETERS_GetMeasurementVariances(float_point_t* r);
uint8_t PARAMETERS_GetInitialStateValues(float_point_t* x0, float_point_t* x1, float_point_t* x2);
uint8_t PARAMETERS_GetInitialPValues(float_point_t* p0, float_point_t* p1, float_point_t* p2);
uint8_t PARAMETERS_GetProcessVariancesK0(float_point_t* qsoc);
uint8_t PARAMETERS_GetInitialStateValuesK0(float_point_t* x0);
uint8_t PARAMETERS_GetInitialPValuesK0(float_point_t* p0);
uint8_t PARAMETERS_GetReistanceK0(float_point_t soc, float_point_t* resistance, int* perf);
float_point_t PARAMETERS_ComputeVarianceU(float_point_t R, float_point_t C, float_point_t Ts, float_point_t I);
float_point_t PARAMETERS_SocDeviationAndVariance(float_point_t target_soc);


#ifdef __cplusplus
}
#endif

#endif /* OPERATIONS_PARAMETERS_PARAMETERS_H_ */
