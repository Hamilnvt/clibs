#!/bin/zsh

set -e

clear

cc -o test test.c -Wall -Wextra -Werror

./test
