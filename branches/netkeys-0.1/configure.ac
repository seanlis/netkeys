# -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright (c) 2009 Matthew Iselin

# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.

# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

AC_PREREQ([2.61])

AC_INIT([netkeys], [0.1], [http://code.google.com/p/netkeys/issues/list])

AC_CONFIG_SRCDIR([src/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h:src/config.h.in])

AM_INIT_AUTOMAKE()

AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_GCC_TRADITIONAL

AC_PATH_X

AC_CHECK_LIB([X11], XOpenDisplay, , AC_MSG_ERROR([X11 library not present]))
AC_CHECK_LIB([Xtst], XTestFakeKeyEvent, , AC_MSG_ERROR([XTest library not present]))

AC_CHECK_HEADER([X11/Xlib.h], ,
AC_MSG_ERROR([X11 library present but Xlib.h not found.])
)
AC_CHECK_HEADER([X11/keysym.h], ,
AC_MSG_ERROR([X11 library present but keysym.h not found.])
)
AC_CHECK_HEADER([X11/extensions/XTest.h], ,
AC_MSG_ERROR([XTest library present but XTest.h not found.])
)

# TODO: Handle multiple different methods for sockets
AC_CHECK_HEADERS([sys/socket.h], ,
AC_MSG_ERROR([sys/socket.h not found.])
)
AC_CHECK_HEADERS([netinet/in.h], ,
AC_MSG_ERROR([netinet/in.h not found.])
)
AC_CHECK_HEADERS([arpa/inet.h], ,
AC_MSG_ERROR([arpa/inet.h not found.])
)
AC_CHECK_HEADERS([netdb.h], ,
AC_MSG_ERROR([netdb.h not found.])
)

AC_CHECK_FUNCS(strcpy_s)
AC_CHECK_FUNCS(stricmp)

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

