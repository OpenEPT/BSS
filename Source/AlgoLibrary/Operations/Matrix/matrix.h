#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "matrix_types.h"

/************************************ Matrices with scalar value *****************************************/

Matrix_Status MATRIX_scalar_add(matrix_t *mat, float_point_t *scalar, matrix_t *result);
Matrix_Status MATRIX_scalar_subtract(matrix_t *mat, float_point_t *scalar, matrix_t *result);
Matrix_Status MATRIX_scalar_multiply(matrix_t *mat, float_point_t *scalar, matrix_t *result);
Matrix_Status MATRIX_scalar_divide(matrix_t *mat, float_point_t *scalar, matrix_t *result);

/*************************************** Matrices operations *********************************************/
void MATRIX_Zero(matrix_t* M);
/**
 * @brief Function to add 2 matrices and give result matrix.
 *
 * @param mat1    The first operand matrix.
 * @param mat2    The second operand matrix.
 * @param result The resulting added matrix.
 */
Matrix_Status MATRIX_add(matrix_t *mat1, matrix_t *mat2, matrix_t *result);

/**
 * @brief Function to subtract 2 matrices and give result matrix.
 *
 * @param mat1    The first matrix to be subtracted from.
 * @param mat2    The second matrix to be subtrahend.
 * @param result The resulting subtracted matrix.
 */
Matrix_Status MATRIX_subtract(matrix_t *mat1, matrix_t *mat2, matrix_t *result);

/**
 * @brief Function to multiply 2 matrices and give result matrix.
 *
 * @param mat1    The first operand matrix.
 * @param mat2    The second operand matrix.
 * @param result The resulting multiplied matrix.
 */
Matrix_Status MATRIX_multiply(matrix_t *mat1, matrix_t *mat2, matrix_t *result);

/**
 * @brief Function to transpose a matrix.
 *
 * @param mat    The matrix to be transposed.
 * @param result The resulting transposed matrix.
 */
Matrix_Status MATRIX_transpose(matrix_t *mat, matrix_t *result);

/**
 * @brief Function to calculate the inverse matrix using the adjugate matrix.
 *
 * @param mat    The matrix for which the inverse is calculated.
 * @param result The resulting inverse matrix.
 */
Matrix_Status MATRIX_inverse(matrix_t *mat, matrix_t *result);

/**
 * @brief Function to calculate the determinant of a matrix.
 *
 * @param mat   The matrix whose determinant is calculated.
 * @param order The order (dimension) of the matrix to be considered.
 * @return      The determinant value of the matrix.
 */
Matrix_Status MATRIX_determinant(matrix_t *mat, float_point_t *det);


#ifdef __cplusplus
}
#endif


#endif // MATRIX_H
