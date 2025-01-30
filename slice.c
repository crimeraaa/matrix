#include <string.h>

#include "slice.h"
#include "matrix.h"

static Slice *
check_slice(lua_State *L, int index)
{
    void *user_data = luaL_checkudata(L, index, SLICE_MTNAME);
    luaL_argcheck(L, user_data != NULL, index, "Expected " LUA_QL(SLICE_LIBNAME));
    return cast(Slice *)user_data;
}

#define check_self(L)   check_slice(L, 1)

Slice *
slice_new(lua_State *L, lua_Number *data, int len)
{
    // [ ..., self ]
    Slice *self = cast(Slice *)lua_newuserdata(L, sizeof(*self));
    self->len   = len;
    self->data  = data;
    luaL_getmetatable(L, SLICE_MTNAME); // [ ..., self, mt ]
    lua_setmetatable(L, -2);            // [ ..., self ] ; setmetatable(self, mt)
    return self;
}

// ( self, index ) -> ( value ) ; push self[index]
static int
method_get(lua_State *L)
{
    Slice *self  = check_self(L);
    int    index = luaL_checkint(L, 2);
    if (index < 0) {
        index = self->len + index + 1;
    }
    if (1 <= index && index <= self->len) {
        lua_pushnumber(L, slice_peek(self, index - 1));
    }
    luaL_error(L, "Bad index: %d (self.len=%d)", index, self->len);
    return 1;
}

// ( self, index, value ) -> () ; self[index] = value
static int
method_set(lua_State *L)
{
    Slice *self  = check_self(L);
    int    index = luaL_checkint(L, 2);
    *slice_poke(self, index) = luaL_checknumber(L, 3);
    return 0;
}

lua_Number
slice_peek(const Slice *self, int index)
{
    return *slice_poke(cast(Slice *)self, index);
}

// Convert a 1-based relative Lua index to an absolute C index.
static int
resolve_index(lua_State *L, const Slice *self, int index)
{
    int len      = self->len;
    int absindex = index;
    if (absindex < 0) {
        absindex = len + absindex;
    } else {
        absindex -= 1;
    }
    if (0 <= absindex && absindex < len) {
        return absindex;
    }
    const char *errmsg = lua_pushfstring(L, "Bad index: %d with self.len=%d", index, len);
    luaL_argerror(L, 2, errmsg);
    return 0;
}

lua_Number *
slice_poke(Slice *self, int index)
{
    return &self->data[index];
}

// ( self, index|key ) -> ( self[index|key] )
static int
mt_index(lua_State *L)
{
    const Slice *self = check_self(L);
    if (lua_isnumber(L, 2)) {
        int index = resolve_index(L, self, luaL_checkint(L, 2));
        lua_pushnumber(L, slice_peek(self, index));
        return 1;
    } else if (lua_isstring(L, 2)) {
        size_t len;
        const char *key = lua_tolstring(L, 2, &len);
        if (len == 3 && strcmp(key, "len") == 0) {
            lua_pushinteger(L, cast(lua_Integer)self->len);
            return 1;
        }
        lua_getglobal(L, SLICE_LIBNAME); // [ self, key, slice ]
        lua_pushvalue(L, 2);             // [ self, key, slice, key ]
        lua_rawget(L, -2);               // [ self, key, slice[key] ]
        assertf(L, !lua_isnil(L, -1), "Invalid " LUA_QL(SLICE_LIBNAME) " field: " LUA_QS, key);
        return 1;
    }
    luaL_typerror(L, 2, "number or string");
    return 1;
}

static int
mt_newindex(lua_State *L)
{
    Slice *self  = check_self(L);
    int    index = resolve_index(L, self, luaL_checkint(L, 2));
    *slice_poke(self, index) = luaL_checknumber(L, 3);
    return 0;
}

static int
mt_tostring(lua_State *L)
{
    const Slice *self = check_self(L);
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    luaL_addchar(&buffer, '[');
    for (int i = 0; i < self->len; i++) {
        // `i` is guaranteed to be in range, hence we do not check.
        lua_pushnumber(L, slice_peek(self, i));
        luaL_addvalue(&buffer);
        if (i < self->len - 1) {
            luaL_addstring(&buffer, ", ");
        }
    }
    luaL_addchar(&buffer, ']');
    luaL_pushresult(&buffer);
    return 1;
}

static const luaL_Reg
slice_lib[] = {
    {"get", &method_get},
    {"set", &method_set},
    {NULL, NULL},
};

static const luaL_Reg
slice_mt[] = {
    {"__index",    &mt_index},
    {"__newindex", &mt_newindex},
    {"__tostring", &mt_tostring},
    {NULL, NULL},
};

// ( "matrix" ) -> ( slice )
int
luaopen_slice(lua_State *L)
{
    luaL_newmetatable(L, SLICE_MTNAME); // [ mt ]
    luaL_register(L, NULL, slice_mt);   // [ mt ] ; register `slice_mt` into `mt`
    lua_pop(L, 1);                      // []
    luaL_register(L, SLICE_LIBNAME, slice_lib); // [ slice ]
    return 1;
}
