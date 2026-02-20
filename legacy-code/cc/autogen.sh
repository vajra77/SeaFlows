#!/usr/bin/env sh

aclocal
autoconf

autoheader

automake --add-missing --copy

./configure "$@"