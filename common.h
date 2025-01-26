#ifndef COMMON_H
#define COMMON_H

/* https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html */
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else

#define bool    int
#define true    (0 == 0)
#define false   (!true)

#endif

/* Easier to grep. */
#define cast(type)  (type)

#endif /* COMMON_H */
