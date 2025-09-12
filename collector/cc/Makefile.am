AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = include src
AM_CPPFLAGS = -I$(top_builddir)/include

bin_PROGRAMS = seaflows
seaflows_SOURCES = src/main.c src/listener.c src/broker.c src/bucket.c src/sflow.c src/rrdtool.c src/gc.c
seaflows_LDADD = $(LDFLAGS)
