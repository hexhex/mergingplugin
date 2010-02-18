EXTRA_DIST = \
  runhex.hex \
  runhexfile.hex \
  hexprog1.hex \
  accesstuples1.hex \
  accesstuples2.hex \
  accessarguments1.hex \
  accessarguments2.hex \
  multipleanswersets.hex

TESTS = tests/run-mergingplugin-tests.sh
TESTS_ENVIRONMENT = DLVHEX=dlvhex RPCOMPILER=$(top_srcdir)/rpcompiler/src/rpcompiler CMPSCRIPT=$(top_srcdir)/examples/compare.sh TESTDIR=$(top_srcdir)/examples/tests DLVHEXPARAMETERS="--plugindir=$(top_builddir)/src"

SUBDIRS = \
	  testoperators
