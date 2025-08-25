#!/usr/bin/env sh
aclocal
autoconf
autoheader
automake --add-missing
echo "Ready to run ./configure"