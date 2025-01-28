#!/usr/bin/bash

CC=clang
CC_FLAGS='-Wall -Wextra -Wconversion -std=c17 -O1'

LIB_NAME=matrix

set -x

# We require position independent code because linking to `liblua.a` will fail
# otherwise!
$CC $CC_FLAGS -c -fPIC $LIB_NAME.c

# Generate a shared library `.so`, not static library `.a`.
$CC $CC_FLAGS -shared -o $LIB_NAME.so $LIB_NAME.o
