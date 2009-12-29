AUTOMAKE_OPTIONS = gnu

#
# replace 'plugin' on the right side by your plugin's name
#
plugin_LTLIBRARIES = libdlvhexasp.la

#
# replace 'plugin' on the left side as above and
# add all sources of your plugin
#
libdlvhexasp_la_SOURCES = ASPPlugin.cpp DLVHexProcess.cpp UnionOp.cpp HexExecution.cpp Operators.cpp InternalTypes.cpp DecisionDiagram.cpp UnfoldOp.cpp MajorityVotingOp.cpp ToBinaryDecisionTreeOp.cpp UserPreferencesOp.cpp OrderBinaryDecisionTreeOp.cpp AvgOp.cpp

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS) \
	$(DLVHEX_CFLAGS)

libdlvhexasp_la_LDFLAGS = -avoid-version -module


libdlvhexasp-static.la: $(libdlvhexasp_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhexasp_la_OBJECTS)

install-static: libdlvhexasp-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s libdlvhexasp-static.la $(DESTDIR)$(plugindir)/libdlvhexasp-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/libdlvhexasp-static.la
