#ifndef LUA_MATRIX_H
#define LUA_MATRIX_H

#include "common.h"

#define MATRIX_LIBNAME    "matrix"
#define MATRIX_MTNAME     "cri.matrix"

// An `m` by `n` matrix.
typedef struct {
    int rows; // Dimension `m`. Once set, must not be changed!
    int cols; // Dimension `n`. Once set, must not be changed!
    lua_Number data[]; // Variable length 1D array. Treat this as row-major.
} Matrix;

Matrix *
matrix_check(lua_State *L, int index);

// ( ... ) -> ( ..., self )
Matrix *
matrix_new(lua_State *L, int rows, int cols);

// ( ... ) -> ( ..., self )
Matrix *
matrix_new_zero(lua_State *L, int rows, int cols);

lua_Number *
matrix_poke(Matrix *matrix, int i, int j);

lua_Number
matrix_peek(const Matrix *matrix, int i, int j);

#endif // LUA_MATRIX_H
