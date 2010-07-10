#
# replace 'plugin' on the right side by your plugin's name
#
plugin_LTLIBRARIES = libdlvhexmerging.la

#
# replace 'plugin' on the left side as above and
# add all sources of your plugin
#
libdlvhexmerging_la_SOURCES = MergingPlugin.cpp DLVHexProcess.cpp DlvhexSolver.cpp HexExecution.cpp Operators.cpp HexAnswerCache.cpp OpUnion.cpp OpSetminus.cpp OpDalal.cpp OpMajoritySelection.cpp OpRelationMerging.cpp
libdlvhexmerging_la_LIBADD = $(CRYPTLIB) $(top_srcdir)/mpcompiler/src/libmpcompiler.la

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/mpcompiler/include \
	$(DLVHEX_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CRYPTLIB_CFLAGS)

libdlvhexmerging_la_LDFLAGS = -avoid-version -module


libdlvhexmerging-static.la: $(libdlvhexmerging_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhexmerging_la_OBJECTS)

install-static: libdlvhexmerging-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s libdlvhexmerging-static.la $(DESTDIR)$(plugindir)/libdlvhexmerging-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/libdlvhexmerging-static.la
