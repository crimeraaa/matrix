#ifndef LUA_SLICE_H
#define LUA_SLICE_H

#include "common.h"

#define SLICE_LIBNAME    "slice"
#define SLICE_MTNAME     "cri.slice"

// A 1D array which is a mutable view into the data of some other container.
// For `Matrix`, this will be the `i`th row vector.
typedef struct {
    int len;
    lua_Number *data;
} Slice;

int
luaopen_slice(lua_State *L);

// ( ... ) -> ( ..., self )
Slice *
slice_new(lua_State *L, lua_Number *data, int len);

// ( ... ) -> ( ..., self[index] )
lua_Number
slice_peek(const Slice *self, int index);

// ( ... ) -> ( ... )
lua_Number *
slice_poke(Slice *self, int index);

#endif // LUA_SLICE_H
