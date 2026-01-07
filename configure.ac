## SPDX-License-Identifier: GPL-2.0+
##
## Copyright 2000-2001 Michal Suszycki <mt_suszycki@yahoo.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.
AC_INIT([whowatch],[1.8.6],[https://github.com/mtsuszycki/whowatch/issues],[whowatch])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/whowatch.c])
AC_CONFIG_HEADER([src/config.h])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

LDFLAGS="$LDFLAGS -rdynamic"

dnl Checks for libraries
AC_CHECK_LIB([kvm], [kvm_openfiles])
CURS=0
AC_CHECK_LIB([ncurses], [scrollok], [CURS=1; LIBS="$LIBS -lncurses"])
if test "$CURS" != "1"; then
  AC_CHECK_LIB([curses], [scrollok], [CURS=1; LIBS="$LIBS -lcurses"])
fi
if test "$CURS" != "1"; then
  AC_MSG_ERROR([Could not find proper curses library])
fi

dnl Checks for header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h sys/ioctl.h sys/time.h unistd.h curses.h kvm.h stdint.h])
AC_CHECK_HEADERS([asm/param.h arpa/inet.h netinet/in.h sys/param.h termios.h utmpx.h])
AC_HEADER_TIME

dnl Check for utmpx functions
AC_CHECK_FUNCS([setutxent getutxent endutxent])

dnl Checks for typedefs, structures, and compiler characteristics
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])

dnl Check for library functions
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_CHECK_FUNCS([select getloadavg bzero gettimeofday inet_ntoa isascii memset regcomp strchr strncasecmp strrchr sysconf])

dnl Checks whether select() modifies the timeval
AC_MSG_CHECKING([whether select() modifies the time value])
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
    int retval;
    fd_set rfds;
    struct timeval tv = {1, 0};
    FD_ZERO(&rfds); FD_SET(0, &rfds);
    select(1, &rfds, 0, 0, &tv);
    return (tv.tv_sec == 0) ? 0 : 1;
}
  ])],
  [AC_MSG_RESULT(yes)
   AC_DEFINE([RETURN_TV_IN_SELECT], [1], [define if select() modifies the time value])
  ],
  [AC_MSG_RESULT(no)]
)

dnl Check whether sysctl() can be used
AC_MSG_CHECKING([whether sysctl() can be used])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
])],
  [AC_MSG_RESULT(yes)
   HAVE_PROCESS_SYSCTL=yes
   AC_DEFINE([HAVE_PROCESS_SYSCTL], [1], [define if sysctl() can be used])
  ],
  [AC_MSG_RESULT(no)]
)

dnl Check members of struct kinfo_proc
AC_CHECK_MEMBERS([struct kinfo_proc.kp_proc], , ,
[#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/proc.h>
])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
