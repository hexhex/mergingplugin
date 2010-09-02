AC_PREREQ(2.64)
AC_INIT([MPCompiler],[1.1.0],[christoph.redl@tuwien.ac.at])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])

#
# where the plugin will be installed by default.
# running configure will take sysplugindir from dlvhex.pc,
# running configure --enable-userinstall will take homeplugindir.
#

AC_ARG_ENABLE(userinstall, 
             [  --enable-userinstall    Install plugin into home directory],
             [INSTALLDIR=$HOME/`pkg-config dlvhex --variable=userplugindir`],
             [INSTALLDIR=`pkg-config dlvhex --variable=sysplugindir`]
             )
AC_SUBST(INSTALLDIR)

AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

LT_INIT

AM_INIT_AUTOMAKE

#
# all needed makefiles
#
AC_CONFIG_FILES([
           Makefile 
           src/Makefile
           examples/Makefile
])

AC_OUTPUT

