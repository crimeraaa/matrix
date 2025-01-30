#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "slice.h"
#include "matrix.h"

Matrix *
matrix_check(lua_State *L, int index)
{
    void *user_data = luaL_checkudata(L, index, MATRIX_MTNAME);
    luaL_argcheck(L, user_data != NULL, index, "Expected " LUA_QL(MATRIX_LIBNAME));
    return cast(Matrix *)user_data;
}

#define check_table(L, i)   luaL_checktype(L, i, LUA_TTABLE)
#define check_self(L)       matrix_check(L, 1)

static void
check_indexes(lua_State *L, const Matrix *matrix, int *pi, int *pj)
{
    int i = luaL_checkint(L, 2);
    int j = luaL_checkint(L, 3);

    luaL_argcheck(L, 1 <= i && i <= matrix->rows, 2, "`rows` out of range");
    luaL_argcheck(L, 1 <= j && j <= matrix->cols, 3, "`cols` out of range");

    *pi = i - 1;
    *pj = j - 1;
}

lua_Number *
matrix_poke(Matrix *matrix, int i, int j)
{
    return &matrix->data[i*matrix->cols + j];
}

lua_Number
matrix_peek(const Matrix *matrix, int i, int j)
{
    // Casting away const in this context is safe because we don't mutate anything.
    return *matrix_poke(cast(Matrix *)matrix, i, j);
}

Matrix *
matrix_new(lua_State *L, int rows, int cols)
{
    size_t area = cast(size_t)rows * cast(size_t)cols;

    // Stack: [ ..., self ]
    Matrix *self = lua_newuserdata(L, sizeof(*self) + area*sizeof(self->data[0]));

    // Stack: [ ..., self, mt ]
    luaL_getmetatable(L, MATRIX_MTNAME);

    // Stack: [ ..., self ]; setmetatable(self) = mt
    lua_setmetatable(L, -2);
    self->rows = rows;
    self->cols = cols;

    return self;
}

Matrix *
matrix_new_zero(lua_State *L, int rows, int cols)
{
    // Stack: [ rows, cols, self ]
    Matrix *self = matrix_new(L, rows, cols);
    memset(self->data, 0, cast(size_t)(rows * cols) * sizeof(self->data[0]));
    return self;
}

static void
new_zero(lua_State *L)
{
    // Stack: [ rows, cols ]
    int rows = luaL_checkint(L, 1);
    int cols = luaL_checkint(L, 2);
    luaL_argcheck(L, rows > 0, 1, "`rows` must be positive and nonzero");
    luaL_argcheck(L, cols > 0, 2, "`columns` must be positive and nonzero");
    matrix_new_zero(L, rows, cols);
}

// Assumes the desired table is located at index 1.
static void
get_dimensions(lua_State *L, int *rows, int *cols)
{
    // Stack: [ tbl ]
    *rows = cast(int)lua_objlen(L, 1);

    // Attempt to get a subtable.
    // Stack: [ tbl, tbl[1] ]
    lua_rawgeti(L, 1, 1);
    if (lua_istable(L, -1)) {
        *cols = cast(int)lua_objlen(L, -1);
    } else {
        // 1D array, so treat it as a row-vector.
        *cols = *rows;
        *rows = 1;
    }
    // Stack: [ tbl ]
    lua_pop(L, 1);
}

// ( rows, cols ) -> ( self )
// ( tbl ) -> ( self )
// https://www.lua.org/pil/28.1.html
static int
method_new(lua_State *L)
{
    if (lua_istable(L, 1)) {
        int rows, cols;
        get_dimensions(L, &rows, &cols);

        // [ tbl, self ]
        Matrix *self = matrix_new(L, rows, cols);

        // Easier to work with relative from the top.
        // [ tbl, self, tbl ]
        lua_pushvalue(L, 1);
        if (rows == 1) {
            for (int j = 0; j < cols; j++) {
                lua_rawgeti(L, -1, j + 1);
                *matrix_poke(self, 0, j) = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            // [ tbl, self ]
            lua_pop(L, 1);
            return 1;
        }
        for (int i = 0; i < rows; i++) {
            lua_rawgeti(L, -1, i + 1);
            int tmp = cast(int)lua_objlen(L, -1);
            if (tmp != cols) {
                luaL_error(L, "Bad #cols: expected %d, have %d (row: %d)", cols, tmp, i + 1);
            }
            for (int j = 0; j < cols; j++) {
                lua_rawgeti(L, -1, j + 1);
                *matrix_poke(self, i, j) = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        // [ tbl, self ]
        lua_pop(L, 1);
        return 1;
    } else {
        new_zero(L);
    }
    return 1;
}

// ( self, rows, cols ) -> ( self[i][j] )
static int
method_get(lua_State *L)
{
    const Matrix *matrix = check_self(L);
    int i, j;
    check_indexes(L, matrix, &i, &j);
    lua_pushnumber(L, matrix_peek(matrix, i, j));
    return 1;
}

// ( self, rows, cols, n) -> ()
static int
method_set(lua_State *L)
{
    Matrix *matrix = check_self(L);
    int i, j;
    check_indexes(L, matrix, &i, &j);
    *matrix_poke(matrix, i, j) = luaL_checknumber(L, 4);
    return 0;
}

static int
mt_add(lua_State *L)
{
    const Matrix *a = check_self(L);
    const Matrix *b = matrix_check(L, 2);

    int rows = a->rows;
    int cols = a->cols;
    assertf(L, rows == b->rows, "Bad #rows: (a.rows=%d) != (b.rows=%d)", rows, b->rows);
    assertf(L, cols == b->cols, "Bad #cols: (a.cols=%d) != (b.cols=%d)", cols, b->cols);
    Matrix *c = matrix_new(L, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            *matrix_poke(c, i, j) = matrix_peek(a, i, j) + matrix_peek(b, i, j);
        }
    }
    return 1;
}

static int
mt_mul(lua_State *L)
{
    const Matrix *a = check_self(L);        // m x n matrix
    const Matrix *b = matrix_check(L, 2);   // n x p matrix

    int rows = a->rows; // m
    int cols = b->cols; // p
    int dots = a->cols; // n
    assertf(L, rows == cols,    "Bad #rows: (self.rows=%d) != (b.cols=%d)", rows, cols);
    assertf(L, dots == b->rows, "Bad #cols: (self.cols=%d) != (b.rows=%d)", dots, b->rows);

    Matrix *c = matrix_new(L, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            lua_Number dot = 0;
            for (int k = 0; k < dots; k++) {
                dot += matrix_peek(a, i, k) * matrix_peek(b, k, j);
            }
            *matrix_poke(c, i, j) = dot;
        }
    }
    return 1;
}

static int
mt_tostring(lua_State *L)
{
    const Matrix *matrix = check_self(L);

    // Use this as a sort of string builder.
    // https://www.lua.org/manual/5.1/manual.html#luaL_Buffer
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    luaL_addchar(&buffer, '[');
    for (int i = 0; i < matrix->rows; i++) {
        luaL_addchar(&buffer, '[');
        for (int j = 0; j < matrix->cols; j++) {
            lua_pushnumber(L, matrix_peek(matrix, i, j));
            luaL_addvalue(&buffer);
            if (j < matrix->cols - 1) {
                luaL_addstring(&buffer, ", ");
            }
        }
        if (i < matrix->rows - 1) {
            luaL_addstring(&buffer, "], ");
        } else {
            luaL_addchar(&buffer, ']');
        }
    }
    luaL_addchar(&buffer, ']');
    luaL_pushresult(&buffer);
    return 1;
}

// ( ... ) -> ( ..., self )
static Slice *
push_ith_row_vector(lua_State *L, Matrix *matrix, int row)
{
    int len = matrix->cols;
    return slice_new(L, &matrix->data[row * len], len);
}

// ( self, key ) -> ( matrix[key] )
static int
mt_index(lua_State *L)
{
    Matrix *self = check_self(L);
    if (lua_isnumber(L, 2)) {
        int row = luaL_checkint(L, 2);
        assertf(L, 1 <= row && row <= self->rows, "Bad #row: %d (self.rows=%d)", row, self->cols);
        push_ith_row_vector(L, self, row - 1);
        return 1;
    }

    size_t len;
    const char *key = luaL_checklstring(L, -1, &len);

    // `a.rows` and `b.rows`, without the need to call a function
    if (len == 4) {
        if (strcmp(key, "rows") == 0) {
            lua_pushinteger(L, cast(lua_Integer)self->rows);
            return 1;
        } else if (strcmp(key, "cols") == 0) {
            lua_pushinteger(L, cast(lua_Integer)self->cols);
            return 1;
        }
        // Not `rows` nor `cols`, so try the library table (will likely be nil)
    }
    lua_getglobal(L, MATRIX_LIBNAME); // [ self, key, matrix ]
    lua_pushvalue(L, -2);       // [ self, key, matrix, key ]
    lua_rawget(L, -2);          // [ self, key, matrix, matrix[key] ]
    assertf(L, !lua_isnil(L, -1), "Invalid " LUA_QL(MATRIX_LIBNAME) " field: " LUA_QS, key);
    return 1;
}

static const luaL_Reg
matrix_lib[] = {
    {"new", &method_new},
    {"get", &method_get},
    {"set", &method_set},
    {NULL, NULL},
};

// https://www.lua.org/pil/28.3.html
static const luaL_Reg
matrix_mt[] = {
    {"__index",     &mt_index},
    {"__tostring",  &mt_tostring},
    {"__add",       &mt_add},
    {"__mul",       &mt_mul},
    {NULL, NULL},
};

// ( "matrix" ) -> ( Lib_Matrix )
// https://www.lua.org/pil/26.2.html
extern int
luaopen_matrix(lua_State *L)
{
    // [ "matrix", slice ]
    luaopen_slice(L);


    // Ensure type-safety when converting userdata
    // https://www.lua.org/pil/28.2.html
    // [ "matrix", slice, mt ]
    luaL_newmetatable(L, MATRIX_MTNAME);

    // Here, when `libname` is `NULL`, no new table is created.
    // We assume the package table is on the top of the stack.
    luaL_register(L, NULL, matrix_mt);

    // Create and push a new table with `MATRIX_LIBNAME`.
    // [ "matrix", slice, mt, matrix ]
    luaL_register(L, MATRIX_LIBNAME, matrix_lib);
    return 1;
}
