#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(nwtutil, 1.0, lijing@inetlinux.net)
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADER([nwt_config.h:nwt_config.h.in])
AC_CONFIG_AUX_DIR(build)

AC_PROG_CXX
if test -n "$CXX"; then
   CXXFLAGS="-g -Wall"
fi

AC_ARG_WITH(database, 
AC_HELP_STRING([--with-database=DB], [Use which Database mysql or oracle])
AC_HELP_STRING([--with-database=mysql], [Always use version included with Nmap])
)

if test "$with_database" == "mysql"; then
   echo -------------- use mysql
   CXXFLAGS="$CXXFLAGS -DMYSQL"
fi
if test "$with_database" == "oracle"; then
   echo -------------- use oracle
   CXXFLAGS="$CXXFLAGS -DORACLE"
fi

have_pthread=no
have_pcap=no

LIBPTHREAD=-lpthread
LIBPCAP=

AC_CHECK_HEADER(pthread.h,[
	AC_CHECK_LIB(pthread, pthread_create, [have_pthread=yes])])
if test $have_pthread == "no" ; then
   echo "error: no pthread library."
   exit 
fi
AC_SUBST(LIBPTHREAD)

AC_CHECK_HEADER(pcap.h,[
	AC_CHECK_LIB(pcap, pcap_open_live,
	[have_pcap=yes])])
if test $have_pcap == "no"; then
   echo "error: no pcap library."
   LIBPCAP=
fi
AC_SUBST(LIBPCAP)

#1 Checks for programs.
#2 Checks for libraries.
#3 Checks for header files.
#4 Checks for typedefs, structures, and compiler characteristics.
#5 Checks for library functions.

AC_MSG_CHECKING(sizeof int*)
AC_CHECK_SIZEOF([int *])


AC_SUBST(CXXFLAGS)

AC_CONFIG_COMMANDS([check_ok],
	[echo ++++++++++++ this is extra $fubar, and so on.],
	[fubar=$fubar])


AC_CONFIG_FILES([Makefile] [utility/Makefile])
AC_OUTPUT
