/*
 * matrix_v2.h
 *
 *  Created on: Oct 8, 2025
 *      Author: elektronika
 */

#ifndef OPERATIONS_MATRIX_MATRIX_V2_H_
#define OPERATIONS_MATRIX_MATRIX_V2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "matrix_types.h"

/************************************
 * Optimized 3x3 matrix calculation
 * Function arguments are raw addresses
 ************************************/
void MATRIX_Zero(matrix_t* M);
Matrix_Status MATRIX_add_3x3(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_add_3x1(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_subtract_3x3(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);

Matrix_Status MATRIX_multiply_3x3(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_multiply_3x3_3x1(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_multiply_3x3_address(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_multiply_1x3_3x3(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_multiply_1x3_3x1(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);
Matrix_Status MATRIX_multiply_3x1_1x3(uintptr_t mat1Addr, uintptr_t mat2Addr, uintptr_t resultAddr);

Matrix_Status MATRIX_transpose_3x3(uintptr_t matAddr, uintptr_t resultAddr);
Matrix_Status MATRIX_transpose_1x3(uintptr_t matAddr, uintptr_t resultAddr);
Matrix_Status MATRIX_inverse_3x3(uintptr_t matAddr, uintptr_t resultAddr);

Matrix_Status MATRIX_scalar_subtract_3x3(uintptr_t matAddr, float_point_t scalar, uintptr_t resultAddr);
Matrix_Status MATRIX_scalar_multiply_3x3(uintptr_t matAddr, float_point_t scalar, uintptr_t resultAddr);
Matrix_Status MATRIX_scalar_multiply_3x1(uintptr_t matAddr, float_point_t scalar, uintptr_t resultAddr);

#ifdef __cplusplus
}
#endif

#endif // MATRIX_ADDR_H

