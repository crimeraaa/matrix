#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
    int *data; /* Heap-allocated 1D integer array. */
    int  rows; /* Number of rows this matrix holds. */
    int  cols; /* Number of columns this matrix holds. */
} Matrix;

typedef enum {
    MATRIX_OK, 
    MATRIX_ERRDIM, /* matrices `A` and `B` have differing #rows or #columns. */
    MATRIX_ERRMEM, /* Failed to allocate memory for a Matrix instance. */
} Matrix_Error;

Matrix
matrix_make(int rows, int cols);

void
matrix_delete(Matrix *A);

void
matrix_print(const Matrix *A);

int
matrix_at(const Matrix *A, int i, int j);

int *
matrix_poke(Matrix *A, int i, int j);

Matrix_Error
matrix_add(const Matrix *A, const Matrix *B, Matrix *C);

Matrix_Error
matrix_mul(const Matrix *A, const Matrix *B, Matrix *C);

#endif /* MATRIX_H */
