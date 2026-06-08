
#include "matrix.h"
#include "stdbool.h"

/************************************ Matrices with scalar value *****************************************/

INLINE Matrix_Status MATRIX_scalar_add(matrix_t *mat, float_point_t *scalar, matrix_t *result)
{
    result->rows = mat->rows;
    result->cols = mat->cols;

#if	USE_FOR_LOOP_INSIDE_FUNCTIONS  == 1
    for (uint32_t i = 0; i < mat->rows; i++)
    {
        for (uint32_t j = 0; j < mat->cols; j++)
        {
            result->data[i][j] = mat->data[i][j] + *scalar;
        }
    }
#else
    result->data[0][0] = mat->data[0][0] + *scalar;
    result->data[0][1] = mat->data[0][1] + *scalar;
    result->data[0][2] = mat->data[0][2] + *scalar;
    result->data[1][0] = mat->data[1][0] + *scalar;
    result->data[1][1] = mat->data[1][1] + *scalar;
    result->data[1][2] = mat->data[1][2] + *scalar;
    result->data[2][0] = mat->data[2][0] + *scalar;
    result->data[2][1] = mat->data[2][1] + *scalar;
    result->data[2][2] = mat->data[2][2] + *scalar;
#endif // USE_FOR_LOOP_INSIDE_FUNCTIONS

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_scalar_subtract(matrix_t *mat, float_point_t *scalar, matrix_t *result)
{
    result->rows = mat->rows;
    result->cols = mat->cols;

#if	USE_FOR_LOOP_INSIDE_FUNCTIONS  == 1
    for (uint32_t i = 0; i < mat->rows; i++)
    {
        for (uint32_t j = 0; j < mat->cols; j++)
        {
            result->data[i][j] = mat->data[i][j] - *scalar;
        }
    }
#else
    result->data[0][0] = mat->data[0][0] - *scalar;
    result->data[0][1] = mat->data[0][1] - *scalar;
    result->data[0][2] = mat->data[0][2] - *scalar;
    result->data[1][0] = mat->data[1][0] - *scalar;
    result->data[1][1] = mat->data[1][1] - *scalar;
    result->data[1][2] = mat->data[1][2] - *scalar;
    result->data[2][0] = mat->data[2][0] - *scalar;
    result->data[2][1] = mat->data[2][1] - *scalar;
    result->data[2][2] = mat->data[2][2] - *scalar;
#endif // USE_FOR_LOOP_INSIDE_FUNCTIONS

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_scalar_multiply(matrix_t *mat, float_point_t *scalar, matrix_t *result)
{
    result->rows = mat->rows;
    result->cols = mat->cols;

#if	USE_FOR_LOOP_INSIDE_FUNCTIONS  == 1
    for (uint32_t i = 0; i < mat->rows; i++)
    {
        for (uint32_t j = 0; j < mat->cols; j++)
        {
            result->data[i][j] = mat->data[i][j] * *scalar;
        }
    }
#else
    result->data[0][0] = mat->data[0][0] * *scalar;
    result->data[0][1] = mat->data[0][1] * *scalar;
    result->data[0][2] = mat->data[0][2] * *scalar;
    result->data[1][0] = mat->data[1][0] * *scalar;
    result->data[1][1] = mat->data[1][1] * *scalar;
    result->data[1][2] = mat->data[1][2] * *scalar;
    result->data[2][0] = mat->data[2][0] * *scalar;
    result->data[2][1] = mat->data[2][1] * *scalar;
    result->data[2][2] = mat->data[2][2] * *scalar;
#endif // USE_FOR_LOOP_INSIDE_FUNCTIONS

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_scalar_divide(matrix_t *mat, float_point_t *scalar, matrix_t *result)
{
    if (scalar == 0) {
        return MATRIX_ERROR_DIVIDE_BY_ZERO;
    }

    result->rows = mat->rows;
    result->cols = mat->cols;

#if	USE_FOR_LOOP_INSIDE_FUNCTIONS  == 1
    for (uint32_t i = 0; i < mat->rows; i++)
    {
        for (uint32_t j = 0; j < mat->cols; j++)
        {
            result->data[i][j] = mat->data[i][j] / *scalar;
        }
    }
#else
    result->data[0][0] = mat->data[0][0] / *scalar;
    result->data[0][1] = mat->data[0][1] / *scalar;
    result->data[0][2] = mat->data[0][2] / *scalar;
    result->data[1][0] = mat->data[1][0] / *scalar;
    result->data[1][1] = mat->data[1][1] / *scalar;
    result->data[1][2] = mat->data[1][2] / *scalar;
    result->data[2][0] = mat->data[2][0] / *scalar;
    result->data[2][1] = mat->data[2][1] / *scalar;
    result->data[2][2] = mat->data[2][2] / *scalar;

#endif // USE_FOR_LOOP_INSIDE_FUNCTIONS

    return MATRIX_SUCCESS;
}
INLINE void MATRIX_Zero(matrix_t* M)
{
    if (M == NULL) return;

    /* Iterate over all valid elements */
    for (uint16_t i = 0; i < M->rows; ++i) {
        for (uint16_t j = 0; j < M->cols; ++j) {
            M->data[i][j] = 0.0;
        }
    }
}
/*************************************** Matrices operations  with structure ********************************************/

#if	USE_FOR_LOOP_INSIDE_FUNCTIONS  == 1
/********************************************* Matrix operations with structures and with for loop **************************************************/

INLINE Matrix_Status MATRIX_add(matrix_t *mat1, matrix_t *mat2, matrix_t *result) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    result->rows = mat1->rows;
    result->cols = mat1->cols;

    for (uint32_t i = 0; i < mat1->rows; i++) {
        for (uint32_t j = 0; j < mat1->cols; j++) {
            result->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
        }
    }

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_subtract(matrix_t *mat1, matrix_t *mat2, matrix_t *result) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    result->rows = mat1->rows;
    result->cols = mat1->cols;

    for (uint32_t i = 0; i < mat1->rows; i++) {
        for (uint32_t j = 0; j < mat1->cols; j++) {
            result->data[i][j] = mat1->data[i][j] - mat2->data[i][j];
        }
    }

    return MATRIX_SUCCESS;
}


INLINE Matrix_Status MATRIX_multiply(matrix_t *mat1, matrix_t *mat2, matrix_t *result) {
    if (mat1->cols != mat2->rows) {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    result->rows = mat1->rows;
    result->cols = mat2->cols;

    for (uint32_t i = 0; i < mat1->rows; i++) {
        for (uint32_t j = 0; j < mat2->cols; j++) {
            result->data[i][j] = 0;
            for (uint32_t k = 0; k < mat1->cols; k++) {
                result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_transpose(matrix_t *mat, matrix_t *result) {
    result->rows = mat->cols;
    result->cols = mat->rows;

    for (uint32_t i = 0; i < mat->rows; i++) {
        for (uint32_t j = 0; j < mat->cols; j++) {
            result->data[j][i] = mat->data[i][j];
        }
    }

    return MATRIX_SUCCESS;
}

static float_point_t power(float_point_t base, int exponent) {
    float_point_t result = 1.0;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

INLINE Matrix_Status MATRIX_inverse(matrix_t *mat, matrix_t *result) {
    if (mat->rows != mat->cols) {
        return MATRIX_ERROR_NOT_SQUARE_MATRIX;
    }

    uint32_t n = mat->rows;

    // Calculate determinant
    float_point_t det;
    Matrix_Status det_result = MATRIX_determinant(mat, &det);
    if (det_result != MATRIX_SUCCESS || det == 0.0) {
        return MATRIX_ERROR_SINGULAR_MATRIX; // Singular matrix doesn't have inverse
    }

    // If matrix is dimension 1x1, inverse matrix is 1/determinant
    if (n == 1) {
        result->data[0][0] = 1.0 / mat->data[0][0];
        result->rows = 1;
        result->cols = 1;
        return MATRIX_SUCCESS;
    }

    // If matrix is dimension 2x2, use standard formula for calculating inverse matrix
    if (n == 2) {
        float_point_t inv_det = 1.0 / det;
        result->data[0][0] = mat->data[1][1] * inv_det;
        result->data[0][1] = -mat->data[0][1] * inv_det;
        result->data[1][0] = -mat->data[1][0] * inv_det;
        result->data[1][1] = mat->data[0][0] * inv_det;
        result->rows = 2;
        result->cols = 2;
        return MATRIX_SUCCESS;
    }

    // Calculate cofactors and form cofactor matrix
    matrix_t cofactorMatrix;
    cofactorMatrix.rows = cofactorMatrix.cols = n;

    // Initialize cofactorMatrix to zero
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            cofactorMatrix.data[i][j] = 0.0;
        }
    }

    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            // Calculate cofactor for every element
            matrix_t submatrix;
            submatrix.rows = submatrix.cols = n - 1;
            uint32_t subrow = 0;

            // Initialize submatrix to zero
            for (uint32_t m = 0; m < n - 1; m++) {
                for (uint32_t n = 0; n < n - 1; n++) {
                    submatrix.data[m][n] = 0.0;
                }
            }

            for (uint32_t row = 0; row < n; row++) {
                if (row != i) {
                    uint32_t subcol = 0;  // Reset subcol for each new row
                    for (uint32_t col = 0; col < n; col++) {
                        if (col != j) {
                            submatrix.data[subrow][subcol++] = mat->data[row][col];
                        }
                    }
                    subrow++;
                }
            }

            float_point_t cofactor = 0.0;
            det_result = power(-1, i + j) * MATRIX_determinant(&submatrix, &cofactor);
            if (det_result != MATRIX_SUCCESS) {
                return MATRIX_ERROR_DIMENSION_MISMATCH;
            }
            cofactorMatrix.data[i][j] = (i + j) % 2 == 0 ? cofactor : -cofactor;  // Correct the sign here
        }
    }

    // Transpose cofactor matrix to get adjoint matrix
    matrix_t adjugateMatrix;
    MATRIX_transpose(&cofactorMatrix, &adjugateMatrix);

    // Divide each element of adjugateMatrix by determinant
    float_point_t inv_det = 1.0 / det;
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            result->data[i][j] = adjugateMatrix.data[i][j] * inv_det;
        }
    }
    result->rows = mat->rows;
    result->cols = mat->cols;

    return MATRIX_SUCCESS;
}

#else
/********************************************* Matrix operations with structures and without for loop **************************************************/


INLINE Matrix_Status MATRIX_add(matrix_t *mat1, matrix_t *mat2, matrix_t *result)
{
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols)
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    uint32_t rows = mat1->rows;
    uint32_t cols = mat1->cols;

    result->rows = rows;
    result->cols = cols;

    if (rows == 1 && cols == 1)
    {
        result->data[0][0] = mat1->data[0][0] + mat2->data[0][0];
    }
    else if (rows == 2 && cols == 2)
    {
        result->data[0][0] = mat1->data[0][0] + mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] + mat2->data[0][1];
        result->data[1][0] = mat1->data[1][0] + mat2->data[1][0];
        result->data[1][1] = mat1->data[1][1] + mat2->data[1][1];
    }
    else if (rows == 1 && cols == 3)
    {
        result->data[0][0] = mat1->data[0][0] + mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] + mat2->data[0][1];
        result->data[0][2] = mat1->data[0][2] + mat2->data[0][2];
    }
    else if (rows == 3 && cols == 1)
    {
        result->data[0][0] = mat1->data[0][0] + mat2->data[0][0];
        result->data[1][0] = mat1->data[1][0] + mat2->data[1][0];
        result->data[2][0] = mat1->data[2][0] + mat2->data[2][0];
    }
    else if (rows == 3 && cols == 3)
    {
        result->data[0][0] = mat1->data[0][0] + mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] + mat2->data[0][1];
        result->data[0][2] = mat1->data[0][2] + mat2->data[0][2];
        result->data[1][0] = mat1->data[1][0] + mat2->data[1][0];
        result->data[1][1] = mat1->data[1][1] + mat2->data[1][1];
        result->data[1][2] = mat1->data[1][2] + mat2->data[1][2];
        result->data[2][0] = mat1->data[2][0] + mat2->data[2][0];
        result->data[2][1] = mat1->data[2][1] + mat2->data[2][1];
        result->data[2][2] = mat1->data[2][2] + mat2->data[2][2];
    }
    else
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    return MATRIX_SUCCESS;
}


INLINE Matrix_Status MATRIX_subtract(matrix_t *mat1, matrix_t *mat2, matrix_t *result)
{
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols)
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    uint32_t rows = mat1->rows;
    uint32_t cols = mat1->cols;

    result->rows = rows;
    result->cols = cols;

    // Direct computation for subtraction based on matrix dimension
    if (rows == 1 && cols == 1)
    {
        result->data[0][0] = mat1->data[0][0] - mat2->data[0][0];
    }
    else if (rows == 2 && cols == 2)
    {
        result->data[0][0] = mat1->data[0][0] - mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] - mat2->data[0][1];
        result->data[1][0] = mat1->data[1][0] - mat2->data[1][0];
        result->data[1][1] = mat1->data[1][1] - mat2->data[1][1];
    }
    else if (rows == 1 && cols == 3)
    {
        result->data[0][0] = mat1->data[0][0] - mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] - mat2->data[0][1];
        result->data[0][2] = mat1->data[0][2] - mat2->data[0][2];
    }
    else if (rows == 3 && cols == 1)
    {
        result->data[0][0] = mat1->data[0][0] - mat2->data[0][0];
        result->data[1][0] = mat1->data[1][0] - mat2->data[1][0];
        result->data[2][0] = mat1->data[2][0] - mat2->data[2][0];
    }
    else if (rows == 3 && cols == 3)
    {
        result->data[0][0] = mat1->data[0][0] - mat2->data[0][0];
        result->data[0][1] = mat1->data[0][1] - mat2->data[0][1];
        result->data[0][2] = mat1->data[0][2] - mat2->data[0][2];
        result->data[1][0] = mat1->data[1][0] - mat2->data[1][0];
        result->data[1][1] = mat1->data[1][1] - mat2->data[1][1];
        result->data[1][2] = mat1->data[1][2] - mat2->data[1][2];
        result->data[2][0] = mat1->data[2][0] - mat2->data[2][0];
        result->data[2][1] = mat1->data[2][1] - mat2->data[2][1];
        result->data[2][2] = mat1->data[2][2] - mat2->data[2][2];
    }
    else
    {
        // For larger matrices, perform generic subtraction
        // Don't test this case
    	return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_multiply(matrix_t *mat1, matrix_t *mat2, matrix_t *result)
{
    uint32_t rows1 = mat1->rows;
    uint32_t cols1 = mat1->cols;
    uint32_t rows2 = mat2->rows;
    uint32_t cols2 = mat2->cols;

    if (cols1 != rows2)
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    result->rows = rows1;
    result->cols = cols2;

    if (rows1 == 1 && cols1 == 1 && rows2 == 1 && cols2 == 1)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0];
    }
    else if (rows1 == 1 && cols1 == 3 && rows2 == 3 && cols2 == 1)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0] + mat1->data[0][1] * mat2->data[1][0] + mat1->data[0][2] * mat2->data[2][0];
    }
    else if (rows1 == 1 && cols1 == 3 && rows2 == 3 && cols2 == 3)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0] + mat1->data[0][1] * mat2->data[1][0] + mat1->data[0][2] * mat2->data[2][0];
        result->data[0][1] = mat1->data[0][0] * mat2->data[0][1] + mat1->data[0][1] * mat2->data[1][1] + mat1->data[0][2] * mat2->data[2][1];
        result->data[0][2] = mat1->data[0][0] * mat2->data[0][2] + mat1->data[0][1] * mat2->data[1][2] + mat1->data[0][2] * mat2->data[2][2];
    }
    else if (rows1 == 3 && cols1 == 3 && rows2 == 3 && cols2 == 1)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0] + mat1->data[0][1] * mat2->data[1][0] + mat1->data[0][2] * mat2->data[2][0];
        result->data[1][0] = mat1->data[1][0] * mat2->data[0][0] + mat1->data[1][1] * mat2->data[1][0] + mat1->data[1][2] * mat2->data[2][0];
        result->data[2][0] = mat1->data[2][0] * mat2->data[0][0] + mat1->data[2][1] * mat2->data[1][0] + mat1->data[2][2] * mat2->data[2][0];
    }
    else if (rows1 == 3 && cols1 == 3 && rows2 == 3 && cols2 == 3)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0] + mat1->data[0][1] * mat2->data[1][0] + mat1->data[0][2] * mat2->data[2][0];
        result->data[0][1] = mat1->data[0][0] * mat2->data[0][1] + mat1->data[0][1] * mat2->data[1][1] + mat1->data[0][2] * mat2->data[2][1];
        result->data[0][2] = mat1->data[0][0] * mat2->data[0][2] + mat1->data[0][1] * mat2->data[1][2] + mat1->data[0][2] * mat2->data[2][2];
        result->data[1][0] = mat1->data[1][0] * mat2->data[0][0] + mat1->data[1][1] * mat2->data[1][0] + mat1->data[1][2] * mat2->data[2][0];
        result->data[1][1] = mat1->data[1][0] * mat2->data[0][1] + mat1->data[1][1] * mat2->data[1][1] + mat1->data[1][2] * mat2->data[2][1];
        result->data[1][2] = mat1->data[1][0] * mat2->data[0][2] + mat1->data[1][1] * mat2->data[1][2] + mat1->data[1][2] * mat2->data[2][2];
        result->data[2][0] = mat1->data[2][0] * mat2->data[0][0] + mat1->data[2][1] * mat2->data[1][0] + mat1->data[2][2] * mat2->data[2][0];
        result->data[2][1] = mat1->data[2][0] * mat2->data[0][1] + mat1->data[2][1] * mat2->data[1][1] + mat1->data[2][2] * mat2->data[2][1];
        result->data[2][2] = mat1->data[2][0] * mat2->data[0][2] + mat1->data[2][1] * mat2->data[1][2] + mat1->data[2][2] * mat2->data[2][2];
    }
    else if (rows1 == 1 && cols1 == 1 && rows2 == 1 && cols2 == 3)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0];
        result->data[0][1] = mat1->data[0][0] * mat2->data[0][1];
        result->data[0][2] = mat1->data[0][0] * mat2->data[0][2];
    }
    else if (rows1 == 3 && cols1 == 1 && rows2 == 1 && cols2 == 1)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0];
        result->data[1][0] = mat1->data[1][0] * mat2->data[0][0];
        result->data[2][0] = mat1->data[2][0] * mat2->data[0][0];
    }
    else if (rows1 == 3 && cols1 == 1 && rows2 == 1 && cols2 == 3)
    {
        result->data[0][0] = mat1->data[0][0] * mat2->data[0][0];
        result->data[0][1] = mat1->data[0][0] * mat2->data[0][1];
        result->data[0][2] = mat1->data[0][0] * mat2->data[0][2];
        result->data[1][0] = mat1->data[1][0] * mat2->data[0][0];
        result->data[1][1] = mat1->data[1][0] * mat2->data[0][1];
        result->data[1][2] = mat1->data[1][0] * mat2->data[0][2];
        result->data[2][0] = mat1->data[2][0] * mat2->data[0][0];
        result->data[2][1] = mat1->data[2][0] * mat2->data[0][1];
        result->data[2][2] = mat1->data[2][0] * mat2->data[0][2];
    }
    else
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_transpose(matrix_t *mat, matrix_t *result)
{
    uint32_t rows = mat->rows;
    uint32_t cols = mat->cols;

    result->rows = cols;
    result->cols = rows;

    if (rows == 1 && cols == 1)
    {
        result->data[0][0] = mat->data[0][0];
    }
    else if (rows == 2 && cols == 2)
	{
		result->data[0][0] = mat->data[0][0];
		result->data[0][1] = mat->data[1][0];
		result->data[1][0] = mat->data[0][1];
		result->data[1][1] = mat->data[1][1];
	}
    else if (rows == 1 && cols == 3)
    {
        result->data[0][0] = mat->data[0][0];
        result->data[1][0] = mat->data[0][1];
        result->data[2][0] = mat->data[0][2];
    }
    else if (rows == 3 && cols == 1)
    {
        result->data[0][0] = mat->data[0][0];
        result->data[0][1] = mat->data[1][0];
        result->data[0][2] = mat->data[2][0];
    }
    else if (rows == 3 && cols == 3)
    {
        result->data[0][0] = mat->data[0][0];
        result->data[0][1] = mat->data[1][0];
        result->data[0][2] = mat->data[2][0];
        result->data[1][0] = mat->data[0][1];
        result->data[1][1] = mat->data[1][1];
        result->data[1][2] = mat->data[2][1];
        result->data[2][0] = mat->data[0][2];
        result->data[2][1] = mat->data[1][2];
        result->data[2][2] = mat->data[2][2];
    }
    else
    {
        return MATRIX_ERROR_DIMENSION_MISMATCH;
    }

    return MATRIX_SUCCESS;
}

INLINE Matrix_Status MATRIX_inverse(matrix_t *mat, matrix_t *result)
{
    if (mat->rows != mat->cols)
    {
        return MATRIX_ERROR_NOT_SQUARE_MATRIX;
    }

    uint32_t n = mat->rows;

    // Calculate determinant
    float_point_t det;
    INLINE Matrix_Status det_result = MATRIX_determinant(mat, &det);
    if (det_result != MATRIX_SUCCESS || det == 0.0)
    {
        return MATRIX_ERROR_SINGULAR_MATRIX; // Singular matrix doesn't have inverse
    }

    // If matrix is dimension 1x1, inverse matrix is 1/determinant
    if (n == 1)
    {
        result->data[0][0] = 1.0 / mat->data[0][0];
        result->rows = 1;
        result->cols = 1;
        return MATRIX_SUCCESS;
    }

    // If matrix is dimension 2x2, use standard formula for calculating inverse matrix
    if (n == 2)
    {
        float_point_t inv_det = 1.0 / det;
        result->data[0][0] = mat->data[1][1] * inv_det;
        result->data[0][1] = -mat->data[0][1] * inv_det;
        result->data[1][0] = -mat->data[1][0] * inv_det;
        result->data[1][1] = mat->data[0][0] * inv_det;
        result->rows = 2;
        result->cols = 2;
        return MATRIX_SUCCESS;
    }

    // For 3x3 matrix, compute inverse without loops
    if (n == 3)
    {
        float_point_t inv_det = 1.0 / det;
        result->data[0][0] = (mat->data[1][1] * mat->data[2][2] - mat->data[1][2] * mat->data[2][1]) * inv_det;
        result->data[0][1] = (mat->data[0][2] * mat->data[2][1] - mat->data[0][1] * mat->data[2][2]) * inv_det;
        result->data[0][2] = (mat->data[0][1] * mat->data[1][2] - mat->data[0][2] * mat->data[1][1]) * inv_det;
        result->data[1][0] = (mat->data[1][2] * mat->data[2][0] - mat->data[1][0] * mat->data[2][2]) * inv_det;
        result->data[1][1] = (mat->data[0][0] * mat->data[2][2] - mat->data[0][2] * mat->data[2][0]) * inv_det;
        result->data[1][2] = (mat->data[0][2] * mat->data[1][0] - mat->data[0][0] * mat->data[1][2]) * inv_det;
        result->data[2][0] = (mat->data[1][0] * mat->data[2][1] - mat->data[1][1] * mat->data[2][0]) * inv_det;
        result->data[2][1] = (mat->data[0][1] * mat->data[2][0] - mat->data[0][0] * mat->data[2][1]) * inv_det;
        result->data[2][2] = (mat->data[0][0] * mat->data[1][1] - mat->data[0][1] * mat->data[1][0]) * inv_det;
        result->rows = 3;
        result->cols = 3;
        return MATRIX_SUCCESS;
    }

    // For matrices larger than 3x3, do not calculate, I dont want that case to have
    return MATRIX_SUCCESS;
}
#endif // USE_FOR_LOOP_INSIDE_FUNCTIONS

/******************************************** Determinant of matrix will be used same in both cases(with or without for loop) *************************************************/

INLINE Matrix_Status MATRIX_determinant(matrix_t * mat, float_point_t *det)
{
    if (mat->rows != mat->cols) {
        return MATRIX_ERROR_NOT_SQUARE_MATRIX;
    }

    uint32_t n = mat->rows;

    if (n == 1) {
        *det = mat->data[0][0];
        return MATRIX_SUCCESS;
    }

    if (n == 2) {
        *det = (mat->data[0][0] * mat->data[1][1]) - (mat->data[0][1] * mat->data[1][0]);
        return MATRIX_SUCCESS;
    }

    if (n == 3) {
        float_point_t a = mat->data[0][0], b = mat->data[0][1], c = mat->data[0][2];
        float_point_t d = mat->data[1][0], e = mat->data[1][1], f = mat->data[1][2];
        float_point_t g = mat->data[2][0], h = mat->data[2][1], i = mat->data[2][2];

        *det = a * (e * i - f * h) - 
               b * (d * i - f * g) + 
               c * (d * h - e * g);
        
        return MATRIX_SUCCESS;
    }

    return MATRIX_ERROR_DIMENSION_MISMATCH; 
}
