#include <stdlib.h>
#include <string.h>

#include "matrix.h"

#define assertf(L, cond, fmt, ...) \
    (void)((cond) || luaL_error(L, fmt, __VA_ARGS__))

static Matrix *
check_matrix(lua_State *L, int index)
{
    void *user_data = luaL_checkudata(L, index, MT_NAME);
    luaL_argcheck(L, user_data != NULL, index, "`Matrix` expected");
    return cast(Matrix *)user_data;
}

#define check_table(L, i)   luaL_checktype(L, i, LUA_TTABLE)
#define check_self(L)   check_matrix(L, 1)

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

static lua_Number *
poke(Matrix *matrix, int i, int j)
{
    return &matrix->data[i*matrix->cols + j];
}

static lua_Number
peek(const Matrix *matrix, int i, int j)
{
    // Casting away const in this context is safe because we don't mutate anything.
    return *poke(cast(Matrix *)matrix, i, j);
}

static Matrix *
_matrix_new(lua_State *L, int rows, int cols)
{
    size_t area = cast(size_t)rows * cast(size_t)cols;

    // Stack: [ ..., self ]
    Matrix *self = lua_newuserdata(L, sizeof(*self) + area*sizeof(self->data[0]));

    // Stack: [ ..., self, mt ]
    luaL_getmetatable(L, MT_NAME);

    // Stack: [ ..., self ]; setmetatable(self) = mt
    lua_setmetatable(L, -2);
    self->rows = rows;
    self->cols = cols;

    return self;
}

static void
_matrix_new_zero(lua_State *L)
{
    // Stack: [ rows, cols ]
    int rows = luaL_checkint(L, 1);
    int cols = luaL_checkint(L, 2);
    luaL_argcheck(L, rows > 0, 1, "`rows` must be positive and nonzero");
    luaL_argcheck(L, cols > 0, 2, "`columns` must be positive and nonzero");

    // Stack: [ rows, cols, self ]
    Matrix *self = _matrix_new(L, rows, cols);
    // Get used to this loop, we'll be doing this a LOT!
    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < self->cols; j++) {
            *poke(self, i, j) = 0;
        }
    }
}

// Assumes the desired table is located at index 1.
static void
_get_dims(lua_State *L, int *rows, int *cols)
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
matrix_new(lua_State *L)
{
    if (lua_istable(L, 1)) {
        int rows, cols;
        _get_dims(L, &rows, &cols);

        // [ tbl, self ]
        Matrix *self = _matrix_new(L, rows, cols);

        // Easier to work with relative from the top.
        // [ tbl, self, tbl ]
        lua_pushvalue(L, 1);
        if (rows == 1) {
            for (int j = 0; j < cols; j++) {
                lua_rawgeti(L, -1, j + 1);
                *poke(self, 0, j) = lua_tonumber(L, -1);
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
                *poke(self, i, j) = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        // [ tbl, self ]
        lua_pop(L, 1);
        return 1;
    } else {
        _matrix_new_zero(L);
    }
    return 1;
}

// ( self, rows, cols ) -> ( self[i][j] )
static int
matrix_get(lua_State *L)
{
    const Matrix *matrix = check_self(L);
    int i, j;
    check_indexes(L, matrix, &i, &j);
    lua_pushnumber(L, peek(matrix, i, j));
    return 1;
}

// ( self, rows, cols, n) -> ()
static int
matrix_set(lua_State *L)
{
    Matrix *matrix = check_self(L);
    int i, j;
    check_indexes(L, matrix, &i, &j);
    *poke(matrix, i, j) = luaL_checknumber(L, 4);
    return 0;
}

static int
matrix_add(lua_State *L)
{
    Matrix *a = check_self(L);
    Matrix *b = check_matrix(L, 2);

    int rows = a->rows;
    int cols = a->cols;
    assertf(L, rows == b->rows, "Bad #rows: (a.rows=%d) != (b.rows=%d)", rows, b->rows);
    assertf(L, cols == b->cols, "Bad #cols: (a.cols=%d) != (b.cols=%d)", cols, b->cols);
    Matrix *c = _matrix_new(L, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            *poke(c, i, j) = peek(a, i, j) + peek(b, i, j);
        }
    }
    return 1;
}

static int
matrix_mul(lua_State *L)
{
    // a: m x n, b: n x p
    Matrix *a = check_self(L);
    Matrix *b = check_matrix(L, 2);

    int rows = a->rows;
    int cols = b->cols;
    int dots = a->cols;
    assertf(L, rows == cols,    "Bad #rows: (a.rows=%d) != (b.cols=%d)", rows, cols);
    assertf(L, dots == b->rows, "Bad #cols: (a.cols=%d) != (b.rows=%d)", dots, b->rows);

    Matrix *c = _matrix_new(L, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            lua_Number dot = 0;
            for (int k = 0; k < dots; k++) {
                dot += peek(a, i, k) * peek(b, k, j);
            }
            *poke(c, i, j) = dot;
        }
    }
    return 1;
}

static int
matrix_tostring(lua_State *L)
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
            lua_pushnumber(L, peek(matrix, i, j));
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

// ( self, key ) -> ( matrix[key] )
static int
matrix_index(lua_State *L)
{
    const Matrix *self = check_self(L);
    size_t len;
    const char *key = luaL_checklstring(L, -1, &len);

    // `a.rows` and `b.rows`, without the need to call a function
    if (len == 4) {
        if (strncmp(key, "rows", len) == 0) {
            lua_pushinteger(L, cast(lua_Integer)self->rows);
            return 1;
        } else if (strncmp(key, "cols", len) == 0) {
            lua_pushinteger(L, cast(lua_Integer)self->cols);
            return 1;
        }
    }
    lua_getglobal(L, LIB_NAME); // [ self, key, matrix ]
    lua_pushvalue(L, -2);       // [ self, key, matrix, key ]
    lua_rawget(L, -2);          // [ self, key, matrix, matrix[key] ]
    return 1;
}

static const luaL_Reg
matrix_lib[] = {
    {"new", &matrix_new},
    {"get", &matrix_get},
    {"set", &matrix_set},
    {NULL, NULL},
};

// https://www.lua.org/pil/28.3.html
static const luaL_Reg
matrix_mt[] = {
    {"__index",     &matrix_index},
    {"__tostring",  &matrix_tostring},
    {"__add",       &matrix_add},
    {"__mul",       &matrix_mul},
    {NULL, NULL},
};

// () -> ( Lib_Matrix )
// https://www.lua.org/pil/26.2.html
extern int
luaopen_matrix(lua_State *L)
{
    // Ensure type-safety when converting userdata
    // https://www.lua.org/pil/28.2.html
    luaL_newmetatable(L, MT_NAME); // [ mt ]
    // lua_pushstring(L, "__index");  // [ mt, "__index" ]
    // lua_pushvalue(L, -2);          // [ mt, "__index", mt]
    // lua_settable(L, -3);           // [ mt ] ; mt["__index"] = mt

    // Here, when `libname` is `NULL`, no new table is created.
    // We assume the package table is on the stack.
    luaL_openlib(L, NULL, matrix_mt, /* nup */ 0);

    // Create and push a new table with `LIB_NAME`.
    luaL_openlib(L, LIB_NAME, matrix_lib, /* nup */ 0);
    return 1;
}
