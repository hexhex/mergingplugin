AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = rpcompiler
rpcompiler_CXXFLAGS = $(AM_CFLAGS)
rpcompiler_SOURCES = BisonScanner.l++ BisonParserImpl.ypp BisonParserImpl.h ParseTreeNode.cpp ParseTreeNodeIterator.cpp SpiritParser.cpp IParser.cpp CodeGenerator.cpp CompilerMain.cpp

noinst_LTLIBRARIES = librpcompiler.la
librpcompiler_la_SOURCES = BisonScanner.l++ BisonParserImpl.ypp BisonParserImpl.h SpiritParser.cpp IParser.cpp ParseTreeNode.cpp ParseTreeNodeIterator.cpp CodeGenerator.cpp
librpcompiler_la_CXXFLAGS = $(AM_CFLAGS)
BUILT_SOURCES = BisonParserImpl.h

AM_YFLAGS = -d
AM_CPPFLAGS = \
	-I$(top_srcdir)/rpcompiler/include

librpcompiler_la_LDFLAGS = -avoid-version -module

librpcompiler-static.la: $(rpcompiler_la_OBJECTS)
	$(CXXLINK) --mode=link $(INSTALL) -s librpcompiler-static.la $(DESTDIR)$(plugindir)/librpcompiler-static.la

install-static: rpcompiler librpcompiler-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s rpcompiler $(DESTDIR)$(plugindir)/rpcompiler
	$(LIBTOOL) --mode=install $(INSTALL) -s librpcompiler-static.la $(DESTDIR)$(plugindir)/librpcompiler-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/rpcompiler
