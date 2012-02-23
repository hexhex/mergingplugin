#
# replace 'plugin' on the right side by your plugin's name
#
plugin_LTLIBRARIES = libdlvhexplugin_merging.la

#
# replace 'plugin' on the left side as above and
# add all sources of your plugin
#
libdlvhexplugin_merging_la_SOURCES = MergingPlugin.cpp HexExecution.cpp HexAnswerCache.cpp Operators.cpp OpUnion.cpp OpSetminus.cpp
# DLVHexProcess.cpp DlvhexSolver.cpp OpDalal.cpp OpDBO.cpp OpMajoritySelection.cpp OpRelationMerging.cpp
libdlvhexplugin_merging_la_LIBADD = $(CRYPTLIB) $(top_builddir)/mpcompiler/src/libmpcompiler.la

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/mpcompiler/include \
	$(DLVHEX_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CRYPTLIB_CFLAGS)

libdlvhexplugin_merging_la_LDFLAGS = -avoid-version -module


libdlvhexplugin_merging-static.la: $(libdlvhexplugin_merging_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhexplugin_merging_la_OBJECTS)

install-static: libdlvhexplugin_merging-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s libdlvhexplugin_merging-static.la $(DESTDIR)$(plugindir)/libdlvhexplugin_merging-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/libdlvhexplugin_merging-static.la
