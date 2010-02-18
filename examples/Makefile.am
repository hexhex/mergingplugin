EXTRA_DIST = \
  runhex.hex \
  runhexfile.hex \
  accesstuples1.hex \
  accesstuples2.hex \
  accesstuples3.hex \
  accessarguments1.hex \
  accessarguments2.hex \
  accessarguments3.hex \
  multipleanswersets.hex \
  operators1.hex \
  operators2.hex \
  operators3.hex

TESTS = tests/run-mergingplugin-tests.sh
TESTS_ENVIRONMENT = DLVHEX=dlvhex RPCOMPILER=$(top_srcdir)/rpcompiler/src/rpcompiler CMPSCRIPT=$(top_srcdir)/examples/compare.sh TESTDIR=$(top_srcdir)/examples/tests DLVHEXPARAMETERS="--plugindir=$(top_builddir)/src" SYSPLUGINDIR=$(sysplugindir) USERPLUGINDIR=$(userplugindir)

SUBDIRS = \
	  testoperators
