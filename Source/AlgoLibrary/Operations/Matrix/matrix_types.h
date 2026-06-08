/*
 * matrix_types.h
 *
 *  Created on: May 23, 2024
 *      Author: valentinad
 */

#ifndef MATRIX_TYPES_H_
#define MATRIX_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifdef STM32_BUILD
#include "main.h"
#endif

//#define USE_INLINE

#ifdef USE_INLINE
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE
#endif

/* Here you should set using counter clocking inside functions or outside, for performance testing  */
#define USE_COUNTER_INSIDE_FUNCTIONS 0

/* Set inside function to be for loop or not */
#define USE_FOR_LOOP_INSIDE_FUNCTIONS 1

#define FLOAT_POINT_IS_DOUBLE 1

/* If you change to other value then 3, it will cause for TEST MATRICES main part of code to fail! */
#define MAX_ROWS 3
#define MAX_COLS 3


/*<! Here you should define floating point type: float, float_point_t or long float_point_t */

#if FLOAT_POINT_IS_DOUBLE
typedef double float_point_t;
#else
typedef float float_point_t;
#endif //FLOAT_POINT_IS_DOUBLE

typedef enum {
    MATRIX_SUCCESS,
	MATRIX_ERROR_DIMENSION_MISMATCH,
	MATRIX_ERROR_INVERSION,
	MATRIX_ERROR_NOT_SQUARE_MATRIX,
	MATRIX_ERROR_SINGULAR_MATRIX,
	MATRIX_ERROR_DIVIDE_BY_ZERO
} Matrix_Status;

typedef struct {
    uint32_t rows;
    uint32_t cols;
    float_point_t data[MAX_ROWS][MAX_COLS];
} matrix_t;

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_TYPES_H_ */
