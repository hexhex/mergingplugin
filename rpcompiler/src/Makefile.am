AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = rpcompiler
rpcompiler_CXXFLAGS = $(AM_CFLAGS)
rpcompiler_SOURCES = scanner.l++ parser.ypp parser.h ParseTreeNode.cpp CodeGenerator.cpp ParseTreeNodeIterator.cpp CompilerMain.cpp RPParser.cpp

noinst_LTLIBRARIES = librpcompiler.la
librpcompiler_la_SOURCES = scanner.l++ parser.ypp parser.h ParseTreeNode.cpp CodeGenerator.cpp ParseTreeNodeIterator.cpp RPParser.cpp
librpcompiler_la_CXXFLAGS = $(AM_CFLAGS)
BUILT_SOURCES = parser.h

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


#scanner.cpp: scanner.lex
#	$(LEX) -o scanner.cpp scanner.lex

#scanner.o: scanner.cpp parser.hpp
#	$(CXX) $(CPPFLAGS) -c -o scanner.o scanner.cpp

#parser.cpp parser.hpp: parser.ypp ./../include/ParseTreeNode.h
#	$(YACC) -d -o parser.cpp parser.ypp

#ParseTreeNode.o: ParseTreeNode.cpp ./../include/ParseTreeNode.h
#	$(CXX) $(CPPFLAGS) -c -o ParseTreeNode.o ParseTreeNode.cpp

#CodeGenerator.o: CodeGenerator.cpp ./../include/CodeGenerator.h ./../include/ParseTreeNode.h
#	$(CXX) $(CPPFLAGS) -c -o CodeGenerator.o CodeGenerator.cpp

#ParseTreeNodeIterator.o: ParseTreeNodeIterator.cpp ./../include/ParseTreeNode.h ./../include/ParseTreeNodeIterator.h
#	$(CXX) $(CPPFLAGS) -c -o ParseTreeNodeIterator.o ParseTreeNodeIterator.cpp

#CompilerMain.o: CompilerMain.cpp ./../include/ParseTreeNode.h
#	$(CXX) $(CPPFLAGS) -c -o CompilerMain.o CompilerMain.cpp

#parser.o: parser.cpp
#	$(CXX) $(CPPFLAGS) -c -o parser.o parser.cpp

#RPParser.o: RPParser.cpp
#	$(CXX) $(CPPFLAGS) -c -o RPParser.o RPParser.cpp

#rpcompiler: scanner.o parser.o RPParser.o ParseTreeNode.o CompilerMain.o CodeGenerator.o ParseTreeNodeIterator.o
#	$(CXX) $(AM_CPPFLAGS) -o rpcompiler scanner.o parser.o RPParser.o ParseTreeNode.o CompilerMain.o CodeGenerator.o ParseTreeNodeIterator.o -ll

#all: rpcompiler

#install: rpcompiler
#	mkdir $(DESTDIR)$(plugindir)/rpcompiler
#	cp ./rpcompiler $(DESTDIR)$(plugindir)/rpcompiler/rpcompiler
#	cp ./*.o $(DESTDIR)$(plugindir)/rpcompiler/

#uninstall:
#	rm -r $(DESTDIR)$(plugindir)/rpcompiler

#clean:
#	rm -f scanner
#	rm -f parser
#	rm -f rpcompiler
#	rm -f *.o
#	rm -f parser.cpp
#	rm -f scanner.cpp
#	rm -f parser.hpp
