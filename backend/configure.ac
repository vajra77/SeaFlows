AC_INIT([seaflows], [1.1], [f.ferreri@namex.it])
AM_INIT_AUTOMAKE(subdir-objects)
AC_PROG_CC

AC_CHECK_LIB([rrd], [rrdc_create],
    [LDFLAGS="$LDFLAGS -lrrd"],
    [AC_MSG_ERROR([RRD library not found. Please install the library and try again.])])

AC_CHECK_LIB([rrd_th], [rrdc_connect],
    [LDFLAGS="$LDFLAGS -lrrd_th"],
    [AC_MSG_ERROR([RRD thread-safe library. Please install the library and try again.])])

AC_ARG_WITH([rrd-dir],
    AS_HELP_STRING([--with-rrd-dir=DIR], [Specify the RRD data directory]),
    [rrd_dir_path=$with_rrd_dir],
    [rrd_dir_path="/data/rrd"])

if test -z "$rrd_dir_path"; then
    AC_MSG_ERROR([--with-rrd-dir must specify a directory])
fi

AC_ARG_WITH([rrd-addr],
    AS_HELP_STRING([--with-rrd-addr=ADDR], [Specify the rrdcached address]),
    [rrd_addr=$with_rrd_addr],
    [rrd_addr="127.0.0.1"])

AC_ARG_WITH([rrd-port],
    AS_HELP_STRING([--with-rrd-port=PORT], [Specify the rrdcached port]),
    [rrd_port=$with_rrd_port],
    [rrd_port="42217"])

AC_DEFINE_UNQUOTED([RRD_DIR], ["$rrd_dir_path"], [Path to the RRD data directory])
AC_DEFINE_UNQUOTED([RRD_ADDRESS], ["$rrd_addr"], [RRD cache daemon address])
AC_DEFINE_UNQUOTED([RRD_PORT], ["$rrd_port"], [RRD cache daemon port])

AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([Makefile include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
