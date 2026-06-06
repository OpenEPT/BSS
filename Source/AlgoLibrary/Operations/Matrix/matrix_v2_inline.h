/*
 * matrix_v2_inline.h
 *
 *  Created on: Oct 8, 2025
 *      Author: elektronika
 */

#ifndef OPERATIONS_MATRIX_MATRIX_V2_INLINE_H_
#define OPERATIONS_MATRIX_MATRIX_V2_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "matrix_types.h"   /* defines float_point_t, Matrix_Status, MATRIX_SUCCESS, MATRIX_ERROR_SINGULAR_MATRIX */
#include "matrix.h"         /* defines matrix_t with data[][], rows, cols */

/* =========================
 * Helpers / common routines
 * ========================= */

static inline void MATRIX_Zero(matrix_t* M)
{
    if (M == NULL) return;

    for (uint16_t i = 0; i < M->rows; ++i) {
        for (uint16_t j = 0; j < M->cols; ++j) {
            M->data[i][j] = (float_point_t)0.0;
        }
    }
}

/* =========================================================
 * Address-based API (3x3 / 3x1) — static inline definitions
 * ========================================================= */

static inline Matrix_Status MATRIX_add_3x3(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 9; i++) {
        result[i] = mat1[i] + mat2[i];
    }
#else
    result[0] = mat1[0] + mat2[0];
    result[1] = mat1[1] + mat2[1];
    result[2] = mat1[2] + mat2[2];
    result[3] = mat1[3] + mat2[3];
    result[4] = mat1[4] + mat2[4];
    result[5] = mat1[5] + mat2[5];
    result[6] = mat1[6] + mat2[6];
    result[7] = mat1[7] + mat2[7];
    result[8] = mat1[8] + mat2[8];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_add_3x1(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = mat1[i] + mat2[i];
    }
#else
    result[0] = mat1[0] + mat2[0];
    result[1] = mat1[1] + mat2[1];
    result[2] = mat1[2] + mat2[2];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_subtract_3x3(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 9; i++) {
        result[i] = mat1[i] - mat2[i];
    }
#else
    result[0] = mat1[0] - mat2[0];
    result[1] = mat1[1] - mat2[1];
    result[2] = mat1[2] - mat2[2];
    result[3] = mat1[3] - mat2[3];
    result[4] = mat1[4] - mat2[4];
    result[5] = mat1[5] - mat2[5];
    result[6] = mat1[6] - mat2[6];
    result[7] = mat1[7] - mat2[7];
    result[8] = mat1[8] - mat2[8];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_scalar_subtract_3x3(uint32_t matAddr, float_point_t scalar, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 9; i++) {
        result[i] = mat[i] - scalar;
    }
#else
    result[0] = mat[0] - scalar;
    result[1] = mat[1] - scalar;
    result[2] = mat[2] - scalar;
    result[3] = mat[3] - scalar;
    result[4] = mat[4] - scalar;
    result[5] = mat[5] - scalar;
    result[6] = mat[6] - scalar;
    result[7] = mat[7] - scalar;
    result[8] = mat[8] - scalar;
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_3x3(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            result[i * 3 + j] = 0;
            for (uint32_t k = 0; k < 3; k++) {
                result[i * 3 + j] += mat1[i * 3 + k] * mat2[k * 3 + j];
            }
        }
    }
#else
    result[0] = mat1[0] * mat2[0] + mat1[1] * mat2[3] + mat1[2] * mat2[6];
    result[1] = mat1[0] * mat2[1] + mat1[1] * mat2[4] + mat1[2] * mat2[7];
    result[2] = mat1[0] * mat2[2] + mat1[1] * mat2[5] + mat1[2] * mat2[8];

    result[3] = mat1[3] * mat2[0] + mat1[4] * mat2[3] + mat1[5] * mat2[6];
    result[4] = mat1[3] * mat2[1] + mat1[4] * mat2[4] + mat1[5] * mat2[7];
    result[5] = mat1[3] * mat2[2] + mat1[4] * mat2[5] + mat1[5] * mat2[8];

    result[6] = mat1[6] * mat2[0] + mat1[7] * mat2[3] + mat1[8] * mat2[6];
    result[7] = mat1[6] * mat2[1] + mat1[7] * mat2[4] + mat1[8] * mat2[7];
    result[8] = mat1[6] * mat2[2] + mat1[7] * mat2[5] + mat1[8] * mat2[8];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_3x3_3x1(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = 0;
        for (uint32_t j = 0; j < 3; j++) {
            result[i] += mat1[i * 3 + j] * mat2[j];
        }
    }
#else
    result[0] = mat1[0] * mat2[0] + mat1[1] * mat2[1] + mat1[2] * mat2[2];
    result[1] = mat1[3] * mat2[0] + mat1[4] * mat2[1] + mat1[5] * mat2[2];
    result[2] = mat1[6] * mat2[0] + mat1[7] * mat2[1] + mat1[8] * mat2[2];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_1x3_3x3(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = 0;
        for (uint32_t j = 0; j < 3; j++) {
            result[i] += mat1[j] * mat2[j * 3 + i];
        }
    }
#else
    result[0] = mat1[0] * mat2[0] + mat1[1] * mat2[3] + mat1[2] * mat2[6];
    result[1] = mat1[0] * mat2[1] + mat1[1] * mat2[4] + mat1[2] * mat2[7];
    result[2] = mat1[0] * mat2[2] + mat1[1] * mat2[5] + mat1[2] * mat2[8];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_1x3_3x1(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

    result[0] = mat1[0] * mat2[0] + mat1[1] * mat2[1] + mat1[2] * mat2[2];
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_3x1_1x3(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    float_point_t *mat1   = (float_point_t *)mat1Addr;
    float_point_t *mat2   = (float_point_t *)mat2Addr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            result[i * 3 + j] = mat1[i] * mat2[j];
        }
    }
#else
    result[0] = mat1[0] * mat2[0];
    result[1] = mat1[0] * mat2[1];
    result[2] = mat1[0] * mat2[2];
    result[3] = mat1[1] * mat2[0];
    result[4] = mat1[1] * mat2[1];
    result[5] = mat1[1] * mat2[2];
    result[6] = mat1[2] * mat2[0];
    result[7] = mat1[2] * mat2[1];
    result[8] = mat1[2] * mat2[2];
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_scalar_multiply_3x3(uint32_t matAddr, float_point_t scalar, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 9; i++) {
        result[i] = mat[i] * scalar;
    }
#else
    result[0] = mat[0] * scalar;
    result[1] = mat[1] * scalar;
    result[2] = mat[2] * scalar;
    result[3] = mat[3] * scalar;
    result[4] = mat[4] * scalar;
    result[5] = mat[5] * scalar;
    result[6] = mat[6] * scalar;
    result[7] = mat[7] * scalar;
    result[8] = mat[8] * scalar;
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_scalar_multiply_3x1(uint32_t matAddr, float_point_t scalar, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = mat[i] * scalar;
    }
#else
    result[0] = mat[0] * scalar;
    result[1] = mat[1] * scalar;
    result[2] = mat[2] * scalar;
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_transpose_3x3(uint32_t matAddr, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;
    float_point_t *result = (float_point_t *)resultAddr;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            result[j * 3 + i] = mat[i * 3 + j];
        }
    }
#else
    result[0] = mat[0];
    result[1] = mat[3];
    result[2] = mat[6];
    result[3] = mat[1];
    result[4] = mat[4];
    result[5] = mat[7];
    result[6] = mat[2];
    result[7] = mat[5];
    result[8] = mat[8];
#endif
    return MATRIX_SUCCESS;
}

/* NOTE: This is really a copy for 1x3 -> 3x1; naming kept for compatibility. */
static inline Matrix_Status MATRIX_transpose_1x3(uint32_t matAddr, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;     /* expects length 3 */
    float_point_t *result = (float_point_t *)resultAddr;  /* expects length 3 */

    result[0] = mat[0];
    result[1] = mat[1];
    result[2] = mat[2];
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_inverse_3x3(uint32_t matAddr, uint32_t resultAddr)
{
    float_point_t *mat    = (float_point_t *)matAddr;
    float_point_t *result = (float_point_t *)resultAddr;

    float_point_t det = mat[0] * (mat[4] * mat[8] - mat[5] * mat[7]) -
                        mat[1] * (mat[3] * mat[8] - mat[5] * mat[6]) +
                        mat[2] * (mat[3] * mat[7] - mat[4] * mat[6]);

    if (det == (float_point_t)0.0) {
        return MATRIX_ERROR_SINGULAR_MATRIX;
    }

    float_point_t inv_det = (float_point_t)(1.0) / det;

#if USE_FOR_LOOP_INSIDE_FUNCTIONS
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            result[j * 3 + i] =
                (mat[((i + 1) % 3) * 3 + ((j + 1) % 3)] * mat[((i + 2) % 3) * 3 + ((j + 2) % 3)] -
                 mat[((i + 1) % 3) * 3 + ((j + 2) % 3)] * mat[((i + 2) % 3) * 3 + ((j + 1) % 3)]) * inv_det;
        }
    }
#else
    result[0] = (mat[4] * mat[8] - mat[5] * mat[7]) * inv_det;
    result[1] = (mat[2] * mat[7] - mat[1] * mat[8]) * inv_det;
    result[2] = (mat[1] * mat[5] - mat[2] * mat[4]) * inv_det;

    result[3] = (mat[5] * mat[6] - mat[3] * mat[8]) * inv_det;
    result[4] = (mat[0] * mat[8] - mat[2] * mat[6]) * inv_det;
    result[5] = (mat[2] * mat[3] - mat[0] * mat[5]) * inv_det;

    result[6] = (mat[3] * mat[7] - mat[4] * mat[6]) * inv_det;
    result[7] = (mat[1] * mat[6] - mat[0] * mat[7]) * inv_det;
    result[8] = (mat[0] * mat[4] - mat[1] * mat[3]) * inv_det;
#endif
    return MATRIX_SUCCESS;
}

static inline Matrix_Status MATRIX_multiply_3x3_address(uint32_t mat1Addr, uint32_t mat2Addr, uint32_t resultAddr)
{
    /* alias of multiply_3x3 — kept for compatibility */
    return MATRIX_multiply_3x3(mat1Addr, mat2Addr, resultAddr);
}


#ifdef __cplusplus
}
#endif

#endif /* OPERATIONS_MATRIX_MATRIX_V2_INLINE_H_ */
