#!/usr/bin/bash

CC=clang
CC_FLAGS='-Wall -Wextra -Wconversion -std=c89 -g -fsanitize=address'

CXX=clang++
CXX_FLAGS='-Wall -Wextra -std=c++17 -g -fsanitize=address'


set -x
# $CC $CC_FLAGS -o matrix matrix.c
$CXX $CXX_FLAGS -o matrix++ matrix.cpp
 