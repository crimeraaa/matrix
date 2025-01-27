#include <stdlib.h>
#include <string.h>

#include "matrix.h"

// Assume `Matrix` is always argument 1.
static Matrix *
check_matrix(lua_State *L)
{
    void *user_data = luaL_checkudata(L, 1, MT_NAME);
    luaL_argcheck(L, user_data != NULL, 1, "`Matrix` expected");
    return cast(Matrix *)user_data;
}

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

static lua_Number
peek(const Matrix *matrix, int i, int j)
{
    return matrix->data[i*matrix->cols + j];
}

static lua_Number *
poke(Matrix *matrix, int i, int j)
{
    return &matrix->data[i*matrix->cols + j];
}

// ( rows, cols ) -> ( self )
// https://www.lua.org/pil/28.1.html
static int
matrix_new(lua_State *L)
{
    // Stack: [ rows, cols ]
    int rows = luaL_checkint(L, 1);
    int cols = luaL_checkint(L, 2);
    luaL_argcheck(L, rows > 0, 1, "`rows` must be positive and nonzero");
    luaL_argcheck(L, cols > 0, 2, "`columns` must be positive and nonzero");

    // Stack: [ rows, cols, self ]
    size_t  area   = cast(size_t)rows * cast(size_t)cols;
    Matrix *matrix = lua_newuserdata(L, sizeof(*matrix) + area*sizeof(matrix->data[0]));

    // Stack: [ rows, cols, self, mt ]
    luaL_getmetatable(L, MT_NAME);

    // Stack: [ rows, cols, self ]; setmetatable(self) = mt
    lua_setmetatable(L, 3);
    matrix->rows = rows;
    matrix->cols = cols;

    // Get used to this loop, we'll be doing this a LOT!
    lua_Number *data = matrix->data;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            data[i*cols + j] = 0;
        }
    }

    // Userdata is already on the stack.
    return 1;
}

// ( self, rows, cols ) -> ( self[i][j] )
static int
matrix_at(lua_State *L)
{
    const Matrix *matrix = check_matrix(L);
    int i;
    int j;
    check_indexes(L, matrix, &i, &j);
    lua_pushnumber(L, peek(matrix, i, j));
    return 1;
}

// ( self, rows, cols, n) -> ()
static int
matrix_set(lua_State *L)
{
    Matrix *matrix = check_matrix(L);
    int i;
    int j;
    check_indexes(L, matrix, &i, &j);
    lua_Number n = luaL_checknumber(L, 4);
    *poke(matrix, i, j) = n;
    return 0;
}

static int
matrix_tostring(lua_State *L)
{
    const Matrix *matrix = check_matrix(L);
    int rows = matrix->rows;
    int cols = matrix->cols;
    const lua_Number *data = matrix->data;

    // Use this as a sort of string builder.
    // https://www.lua.org/manual/5.1/manual.html#luaL_Buffer
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    luaL_addchar(&buffer, '[');
    for (int i = 0; i < rows; i++) {
        luaL_addchar(&buffer, '[');
        for (int j = 0; j < cols; j++) {
            lua_pushnumber(L, data[i*cols + j]);
            luaL_addvalue(&buffer);
            if (j < cols - 1) {
                luaL_addstring(&buffer, ", ");
            }
        }
        if (i < rows - 1) {
            luaL_addstring(&buffer, "], ");
        } else {
            luaL_addchar(&buffer, ']');
        }
    }
    luaL_addchar(&buffer, ']');
    luaL_pushresult(&buffer);
    return 1;
}

static int
matrix_rows(lua_State *L)
{
    lua_pushinteger(L, check_matrix(L)->rows);
    return 1;
}

static int
matrix_cols(lua_State *L)
{
    lua_pushinteger(L, check_matrix(L)->cols);
    return 1;
}

static const luaL_Reg
matrix_lib[] = {
    {"new",  &matrix_new},
    {NULL, NULL},
};

// https://www.lua.org/pil/28.3.html
static const luaL_Reg
matrix_mt[] = {
    {"at",          &matrix_at},
    {"set",         &matrix_set},
    {"rows",        &matrix_rows},
    {"cols",        &matrix_cols},
    {"__tostring",  &matrix_tostring},
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
    lua_pushstring(L, "__index");  // [ mt, "__index" ]
    lua_pushvalue(L, -2);          // [ mt, "__index", mt]
    lua_settable(L, -3);           // [ mt ] ; mt["__index"] = mt

    // Here, when `libname` is `NULL`, no new table is created.
    // We assume the package table is on the stack.
    luaL_openlib(L, NULL, matrix_mt, /* nup */ 0);

    // Create and push a new table with `LIB_NAME`.
    luaL_openlib(L, LIB_NAME, matrix_lib, /* nup */ 0);
    return 1;
}
