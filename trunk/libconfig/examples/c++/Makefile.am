
noinst_PROGRAMS = example1 example2 example3

example1_SOURCES = example1.cpp

example2_SOURCES = example2.cpp

example3_SOURCES = example3.cpp

AM_CPPFLAGS = -I $(top_srcdir)/lib

LDADD = $(top_srcdir)/lib/libconfig++.la

EXTRA_DIST = \
	example.cfg \
	*.vcproj
