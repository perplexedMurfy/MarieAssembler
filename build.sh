#!/bin/sh
unset CFLAGS
CFLAGS=""
[ "$1" = "debug" ] && CFLAGS="$CFLAGS -DDEBUG -ggdb3"
BASE="$(dirname $0)"

echo "gcc $CFLAGS -o $BASE/bin/MarieAssembler $BASE/src/linux_MarieAssembler.c $BASE/src/MarieAssembler.c"
gcc $CFLAGS -Wall -Winline -o $BASE/bin/MarieAssembler $BASE/src/linux_MarieAssembler.c $BASE/src/MarieAssembler.c
