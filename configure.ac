AC_PREREQ(2.64)

AC_INIT([dlvhex-mergingplugin],[1.1.0],[christoph.redl@tuwien.ac.at])

AC_CONFIG_SRCDIR([src/MergingPlugin.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])

MAJOR=`echo $PACKAGE_VERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/"`
MINOR=`echo $PACKAGE_VERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/"`
MICRO=`echo $PACKAGE_VERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/"`

AC_DEFINE_UNQUOTED([MERGINGPLUGIN_MAJOR], [$MAJOR], [Major version number of the merging-plugin])
AC_DEFINE_UNQUOTED([MERGINGPLUGIN_MINOR], [$MINOR], [Minor version number of the merging-plugin])
AC_DEFINE_UNQUOTED([MERGINGPLUGIN_MICRO], [$MICRO], [Micro version number of the merging-plugin])


AM_INIT_AUTOMAKE

AC_CANONICAL_HOST

# by default we look into /opt/local and /sw for include files (macports vs. fink)
case $host_os in
     darwin*|macosx*)
	CPPFLAGS="$CPPFLAGS -I/opt/local/include -I/sw/include"
     ;;
esac 


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS([sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# we build a module library
AC_DISABLE_STATIC # we don't need a static library
LT_INIT


# doxygen stuff
#
# these are the defaults, can be overridden by --enable-doxygen-...
DX_HTML_FEATURE([ON])
DX_CHM_FEATURE([OFF])
DX_CHI_FEATURE([OFF])
DX_MAN_FEATURE([OFF])
DX_RTF_FEATURE([OFF])
DX_XML_FEATURE([OFF])
DX_PDF_FEATURE([OFF])
DX_PS_FEATURE([ON])
DX_INIT_DOXYGEN([dlvhex-mergingplugin], [doxygen.cfg], [doc/sdk])

#
# check for other libraries here
#

#
# of course we need dlvhex itself!
#
PKG_CHECK_MODULES([DLVHEX], [dlvhex >= 1.7])

# we need the crypt library for MD5 computation
AC_CHECK_LIB(crypt, crypt, [CRYPTLIB="-lcrypt"])
AC_SUBST(CRYPTLIB)


# checking for boost libs
BOOST_REQUIRE([1.41.0])
BOOST_SMART_PTR
BOOST_STRING_ALGO
BOOST_TOKENIZER

#
# the default system-wide plugin dir $(libdir)/dlvhex/plugins can be
# overridden by setting PLUGIN_DIR=... at configure-time
#
if test "$SYS_PLUGIN_DIR" = ""; then
    sysplugindir="\${libdir}/dlvhex/plugins"
else
    sysplugindir="$SYS_PLUGIN_DIR"
fi

AC_SUBST(sysplugindir)

CPPFLAGS="$CPPFLAGS -DSYS_PLUGIN_DIR='\"$sysplugindir\"'"

#
# the user plugin dir consists of the user home concatenated with
# and USER_PLUGIN_DIR, which is .dlvhex/plugins by default and can be
# overridden by setting USER_PLUGIN_DIR=... at configure-time
#
if test "$USER_PLUGIN_DIR" = ""; then
    userplugindir=".dlvhex/plugins"
else
    userplugindir="$USER_PLUGIN_DIR"
fi

AC_SUBST(userplugindir)

CPPFLAGS="$CPPFLAGS -DUSER_PLUGIN_DIR='\"$userplugindir\"'"

#
# where the plugin will be installed by default.
# running configure will take sysplugindir from dlvhex.pc,
# running configure --enable-userinstall will take homeplugindir.
#

AC_ARG_ENABLE(userinstall, 
             [  --enable-userinstall    Install plugin into home directory],
             [plugindir=$HOME/`pkg-config dlvhex --variable=userplugindir`],
             [plugindir=`pkg-config dlvhex --variable=sysplugindir`]
             )
AC_SUBST([plugindir])

# substitute EXAMPLESDIR in examples
examplesdir=../$srcdir/examples
AC_SUBST([examplesdir])

#
# all needed makefiles
#
AC_CONFIG_FILES([
           Makefile
           include/Makefile
           src/Makefile
           examples/Makefile
           examples/testoperators/Makefile
           examples/testoperators/src/Makefile
           mpcompiler/Makefile 
           mpcompiler/include/Makefile 
           mpcompiler/src/Makefile
           mpcompiler/examples/Makefile
           examples/diagnosis1.mp
           examples/diagnosis2.mp
           examples/diagnosis3.mp
           examples/diagnosis3-dbo.mp
           examples/callhexfile1.hex
           examples/runhexfile.hex
           examples/calledprog.hex
           examples/hexprog1.hex
           examples/fault.obs
           examples/fulladder.dl
           examples/abnormal1.hyp
           examples/abnormal2.hyp
           examples/abnormal3.hyp
])

AC_OUTPUT

