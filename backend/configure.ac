AC_INIT([seaflows], [1.1], [f.ferreri@namex.it])
AM_INIT_AUTOMAKE
AC_PROG_CC

AC_ARG_WITH([rrd-dir],
    AS_HELP_STRING([--with-rrd-dir=DIR], [Specify the RRD data directory]),
    [rrd_dir_path=$with_rrd_dir],
    [rrd_dir_path="/data/rrd"])

if test -z "$rrd_dir_path"; then
    AC_MSG_ERROR([--with-rrd-dir must specify a directory])
fi

AC_ARG_WITH([rrdcached-address],
    AS_HELP_STRING([--with-rrdcached-address=ADDR], [Specify the RRD cache daemon address]),
    [rrdcached_address=$with_rrdcached_address],
    [rrdcached_address="127.0.0.1:42217"])

AC_DEFINE_UNQUOTED([RRD_DIR], ["$rrd_dir_path"], [Path to the RRD data directory])
AC_DEFINE_UNQUOTED([RRDCACHED_ADDRES], [ "$rrdcached_address"], [RRD cache daemon address])

AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
