#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "matrix.h"

static void *
xcalloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    assert(ptr != NULL);
    return ptr;
}

static int
row_major_index(const Matrix *A, int i, int j)
{
    assert((0 <= i && i < A->rows) && "Invalid row index!");
    assert((0 <= j && j < A->cols) && "Invalid column index!");
    return i*A->cols + j;
}

static void
set_matrix(Matrix *A, int *acc)
{
    int i;
    int rows = A->rows;
    int cols = A->cols;
    for (i = 0; i < rows; i++) {
        int j;
        for (j = 0; j < cols; j++) {
            *matrix_poke(A, i, j) = *acc;
            *acc += 1;
        }
    }
}

int
main(void)
{
    Matrix A, B, C;
    int acc = 0;

    A = matrix_make(3, 2);
    B = matrix_make(2, 3);
    set_matrix(&A, &acc);
    set_matrix(&B, &acc);
    if (matrix_mul(&A, &B, &C) != MATRIX_OK) {
        printf("Matrix math failed!\n");
        goto cleanup;
    }

    matrix_print(&A);
    matrix_print(&B);
    matrix_print(&C);

    matrix_delete(&C);
cleanup:
    matrix_delete(&B);
    matrix_delete(&A);
    return 0;
}


Matrix
matrix_make(int rows, int cols)
{
    Matrix A;
    A.data = xcalloc(cast(size_t)rows * cast(size_t)cols, sizeof(A.data[0]));
    A.rows = rows;
    A.cols = cols;
    return A;
}

void
matrix_delete(Matrix *A)
{
    free(A->data);
    A->data = NULL;
    A->rows = 0;
    A->cols = 0;
}

void
matrix_print(const Matrix *A)
{
    int *data = A->data;
    int rows = A->rows;
    int cols = A->cols;
    int i;
    printf("%ix%i matrix:\n", rows, cols);
    for (i = 0; i < rows; i++) {
        int j;
        for (j = 0; j < cols; j++) {
            int index = row_major_index(A, i, j);
            printf("%i", data[index]);
            /* printf("[%i, %i] = %i", i, j, data[index]); */
            if (j < cols - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

int
matrix_at(const Matrix *A, int i, int j)
{
    return A->data[row_major_index(A, i, j)];
}

int *
matrix_poke(Matrix *A, int i, int j)
{
    return &A->data[row_major_index(A, i, j)];
}

Matrix_Error
matrix_add(const Matrix *A, const Matrix *B, Matrix *C)
{
    int rows = A->rows;
    int cols = A->cols;
    int i;
    /* Matrix addition requires A and B to be the same dimensions. */
    if (rows != B->rows || cols != B->cols) {
        return MATRIX_ERRDIM;
    }
    *C = matrix_make(rows, cols);
    if (C->data == NULL) {
        return MATRIX_ERRMEM;
    }
    for (i = 0; i < rows; i++) {
        int j;
        for (j = 0; j < cols; j++) {
            *matrix_poke(C, i, j) = matrix_at(A, i, j) + matrix_at(B, i, j);
        }
    }
    return MATRIX_OK;
}

Matrix_Error
matrix_mul(const Matrix *A, const Matrix *B, Matrix *C)
{
    /* See: https://en.wikipedia.org/wiki/Matrix_multiplication */
    int rows = A->rows; /* `m` in `m x n` matrix A. */
    int cols = B->cols; /* `p` in `n x p` matrix B. */
    int dots = A->cols; /* `n` in `m x n` matrix A or `n x p` matrix B. */
    int i;

    /* Matrix multiplication requires A.rows == B.cols and A.cols == B.rows. */
    if (!(rows == cols && dots == B->rows)) {
        return MATRIX_ERRDIM;
    }
    
    *C = matrix_make(rows, cols);
    if (C->data == NULL) {
        return MATRIX_ERRMEM;
    }
    
    for (i = 0; i < rows; i++) {
        int j;
        for (j = 0; j < cols; j++) {
            int dot = 0;
            int k;
            for (k = 0; k < dots; k++) {
                /* C[i, j] += A[i, k] * B[k, j] */
                dot += matrix_at(A, i, k) * matrix_at(B, k, j);
            }
            *matrix_poke(C, i, j) = dot;
        }
    }
    return MATRIX_OK;
}
