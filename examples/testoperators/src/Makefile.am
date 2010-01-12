AUTOMAKE_OPTIONS = gnu

plugin_LTLIBRARIES = libtestoperators.la

libtestoperators_la_SOURCES = TestOperators.cpp

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS) \
	$(DLVHEX_CFLAGS)

libtestoperators_la_LDFLAGS = -avoid-version -module


libtestoperators-static.la: $(libtestoperators_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libtestoperators_la_OBJECTS)
