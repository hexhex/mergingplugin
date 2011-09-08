bin_PROGRAMS = mpcompiler
mpcompiler_CXXFLAGS = $(AM_CFLAGS)
mpcompiler_SOURCES = BisonScanner.l++ BisonParserImpl.ypp BisonParserImpl.h ParseTreeNode.cpp ParseTreeNodeIterator.cpp SpiritParser.cpp IParser.cpp CodeGenerator.cpp CompilerMain.cpp

noinst_LTLIBRARIES = libmpcompiler.la
libmpcompiler_la_SOURCES = BisonScanner.l++ BisonParserImpl.ypp BisonParserImpl.h SpiritParser.cpp IParser.cpp ParseTreeNode.cpp ParseTreeNodeIterator.cpp CodeGenerator.cpp
libmpcompiler_la_CXXFLAGS = $(AM_CFLAGS)
BUILT_SOURCES = BisonParserImpl.h

AM_YFLAGS = -d
AM_CPPFLAGS = \
	-I$(top_srcdir)/mpcompiler/include \
  $(BOOST_CPPFLAGS)

libmpcompiler_la_LDFLAGS = -avoid-version -module

librpcompiler-static.la: $(mpcompiler_la_OBJECTS)
	$(CXXLINK) --mode=link $(INSTALL) -s libmpcompiler-static.la $(DESTDIR)$(plugindir)/libmpcompiler-static.la

install-static: mpcompiler libmpcompiler-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s mpcompiler $(DESTDIR)$(plugindir)/mpcompiler
	$(LIBTOOL) --mode=install $(INSTALL) -s libmpcompiler-static.la $(DESTDIR)$(plugindir)/libmpcompiler-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/mpcompiler
