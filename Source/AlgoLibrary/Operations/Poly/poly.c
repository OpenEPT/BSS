/*
 * poly.c
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */
#include <math.h>
#include "../Parameters/parameters.h"
#include "../Parameters/parameters_config.h"
#include "../../Profiler/profiler.h"
#include "poly.h"
#include <math.h>
#include "../Matrix/matrix_types.h"

#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 0
extern float_point_t PARAMETERS_OCV_COEFFICIENTS[];  // defined in parameters_data.c

// Filip: Modified
extern float_point_t PARAMETERS_OCV_COEFFICIENTS_ORDERED[];  // defined in parameters_data.c
extern float_point_t PARAMETERS_OCV_DERIV_COEFFICIENTS[];  // defined in parameters_data.c
#endif

#if POLY_USE_INTERNAL_POW == 1
#if defined(__GNUC__) || defined(__clang__)
#define POLY_UNUSED __attribute__((unused))
#else
#define POLY_UNUSED
#endif
static float_point_t POLY_InternalPow(float_point_t base, int exp) POLY_UNUSED;
static float_point_t POLY_InternalPow(float_point_t base, int exp)
{
    if (exp == 0)
        return 1.0;
    else if (exp < 0)
        return 1.0 / POLY_InternalPow(base, -exp);

    float_point_t result = 1.0;
    float b = base;
    int e = exp;

    while (e > 0) {
        if (e & 1)
            result *= b;
        b *= b;
        e >>= 1;
    }
    return result;
}
#undef POLY_UNUSED
#endif

float_point_t POLY_Eval_SoC(float_point_t soc, int* perf)
{
	int cycles = 0;
	float_point_t ocv = 0.0;

#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 1
	ocv = PARAMETERS_GetOCV(soc, &cycles);
#else
/* Coefficients are stored in DESCENDING powers:
   PARAMETERS_OCV_COEFFICIENTS[0] * soc^N + ... + PARAMETERS_OCV_COEFFICIENTS[N]
   Horner: (((c0*x + c1)*x + c2)*x ... + cN)
*/
    // Filip: Modified
    const int N = PARAMETERS_CONFIG_OCV_POLY_ORDERED;   /* degree */
	float_point_t acc = PARAMETERS_OCV_COEFFICIENTS_ORDERED[0];
	// const int N = PARAMETERS_CONFIG_OCV_POLY_ORDER;   /* degree */
	// float_point_t acc = PARAMETERS_OCV_COEFFICIENTS[0];

#if POLY_USE_PERF == 1
	PROFILER_Reset();
	PROFILER_Start();
#endif

#if USE_FOR_LOOP_INSIDE_FUNCTIONS == 1
    // Filip: Modified
	for (int i = 1; i < N; ++i) {
    // for (int i = 1; i <= N; ++i) {
        acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[i];
		// acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[i];
	}
#else

    // Filip: Modified
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[1];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[2];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[3];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[4];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[5];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[6];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[7];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[8];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[9];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS_ORDERED[10];
	/* acc = PARAMETERS_OCV_COEFFICIENTS[0];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[1];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[2];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[3];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[4];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[5];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[6];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[7];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[8];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[9];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[10];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[11];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[12];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[13];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[14];
	acc = acc * soc + PARAMETERS_OCV_COEFFICIENTS[15]; */
#endif
	ocv = acc;

#if POLY_USE_PERF == 1
	PROFILER_Stop();
	cycles = PROFILER_GetValue();
#endif
#endif /* PARAMETERS_CONFIG_USE_LOOK_UP_TABLE */

	*perf = cycles;
	return ocv;
}
#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 0
extern float_point_t PARAMETERS_OCV_DER_COEFFICIENTS[];  /* defined in parameters_data.c */
#endif

float_point_t POLY_Eval_SoC_Der(float_point_t soc, int* perf)
{
    int cycles = 0;
    float_point_t d_ocv = 0.0;

#if PARAMETERS_CONFIG_USE_LOOK_UP_TABLE == 1
    /* LUT path: pick nearest derivative from PARAMETERS_OCV_INFO row 2 */
    d_ocv = PARAMETERS_GetOCVDer(soc, &cycles);
#else
    /* Polynomial path: derivative coefficients are in DESCENDING powers.
       PARAMETERS_OCV_DER_COEFFICIENTS has N elements where
       N = PARAMETERS_CONFIG_OCV_POLY_ORDER (original degree),
       representing a poly of degree (N-1).
       Use Horner:
         (((c0*x + c1)*x + c2)*x ... + c_{N-1})
    */

    // Filip: Modified
    const int N = PARAMETERS_CONFIG_OCV_DERIV_POLY_ORDERED;   /* count of der coeffs */
    float_point_t acc = PARAMETERS_OCV_DERIV_COEFFICIENTS[0];

    // const int N = PARAMETERS_CONFIG_OCV_POLY_ORDER;   /* count of der coeffs */
    // float_point_t acc = PARAMETERS_OCV_DER_COEFFICIENTS[0];

#if POLY_USE_PERF == 1
    PROFILER_Reset();
    PROFILER_Start();
#endif

#if USE_FOR_LOOP_INSIDE_FUNCTIONS == 1
    for (int i = 1; i < N; ++i) {
        // Filip: Modified
        acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[i];
        // acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[i];
    }
#else

    // Filip: Modified
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[1];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[2];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[3];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[4];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[5];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[6];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[7];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[8];
    acc = acc * soc + PARAMETERS_OCV_DERIV_COEFFICIENTS[9];

    /* acc = PARAMETERS_OCV_DER_COEFFICIENTS[0];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[1];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[2];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[3];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[4];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[5];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[6];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[7];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[8];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[9];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[10];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[11];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[12];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[13];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[14];
    acc = acc * soc + PARAMETERS_OCV_DER_COEFFICIENTS[15]; */
#endif
    d_ocv = acc;

#if POLY_USE_PERF == 1
    PROFILER_Stop();
    cycles = PROFILER_GetValue();
#endif
#endif /* PARAMETERS_CONFIG_USE_LOOK_UP_TABLE */

    if (perf) *perf = cycles;
    return d_ocv;
}


