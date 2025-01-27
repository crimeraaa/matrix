#ifndef LUA_MATRIX_H
#define LUA_MATRIX_H

#include "lua.h"
#include "lauxlib.h"

#define cast(T)     (T)

#define MT_NAME     "Matrix.Matrix"
#define LIB_NAME    "matrix"

// An `m` by `n` matrix.
typedef struct {
    int rows; // Dimension `m`. Once set, must not be changed!
    int cols; // Dimension `n`. Once set, must not be changed!
    lua_Number data[]; // Variable length 1D array. Treat this as row-major.
} Matrix;

#endif // LUA_MATRIX_H
