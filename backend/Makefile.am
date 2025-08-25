AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = include src
AM_CPPFLAGS = -I$(top_builddir)/include
bin_PROGRAMS = seaflows
seaflows_SOURCES = seaflows.c collector.c broker.c bucket.c sflow.c rrdtool.c gc.c
