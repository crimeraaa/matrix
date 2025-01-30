#ifndef INTERNAL_COMMON_H
#define INTERNAL_COMMON_H

#include <lua.h>
#include <lauxlib.h>

#include <stdbool.h>

// Taken from Ginger Bill. Easier to grep.
#define cast(T) (T)

#define assertf(L, cond, fmt, ...) \
    (void)((cond) || luaL_error(L, fmt, __VA_ARGS__))


#endif // INTERNAL_COMMON_H
