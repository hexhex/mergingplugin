AUTOMAKE_OPTIONS = gnu 1.10

ACLOCAL_AMFLAGS = -I m4
include doxygen.am

SUBDIRS = \
	  mpcompiler/ \
          include/ \
          src/

# \
#          examples/

EXTRA_DIST = doxygen.am doxygen.cfg documentation.dox

MOSTLYCLEANFILES = $(DX_CLEANFILES)
