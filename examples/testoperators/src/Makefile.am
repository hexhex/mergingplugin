AUTOMAKE_OPTIONS = gnu

plugin_LTLIBRARIES = libdlvhextestoperators.la

libdlvhextestoperators_la_SOURCES = TestOperators.cpp

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS) \
	$(DLVHEX_CFLAGS)

libdlvhextestoperators_la_LDFLAGS = -avoid-version -module


libdlvhextestoperators-static.la: $(libdlvhextestoperators_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhextestoperators_la_OBJECTS)
