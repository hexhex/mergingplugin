EXTRA_DIST = \
  tests/run-mpc-tests.sh \
  tests/mpcompilertests.test \
  emptyrevisionplan.mp \
  singleanswerset1.mp \
  singleanswerset2.mp \
  multipleanswersets1.mp \
  multipleanswersets2.mp \
  commonsignature.mp \
  invalidcs1.mp \
  invalidcs2.mp \
  invalidbeliefbase1.mp \
  invalidbeliefbase2.mp \
  invalidrp1.mp \
  invalidrp2.mp \
  subrp1.mp \
  subrp2.mp \
  negation1.mp \
  negation2.mp \
  negation3.mp \
  externalsource1.mp \
  externalsource2.mp \
  tests/emptyrevisionplan.as \
  tests/singleanswerset1.as \
  tests/singleanswerset2.as \
  tests/multipleanswersets1.as \
  tests/multipleanswersets2.as \
  tests/commonsignature.as \
  tests/invalidcs1.err \
  tests/invalidcs2.err \
  tests/invalidbeliefbase1.err \
  tests/invalidbeliefbase2.err \
  tests/invalidrp1.err \
  tests/invalidrp2.err \
  tests/subrp1.as \
  tests/subrp2.as \
  tests/negation1.as \
  tests/negation2.as \
  tests/negation3.as \
  tests/externalsource1.as \
  tests/externalsource2.as

TESTS = tests/run-mpc-tests.sh
TESTS_ENVIRONMENT = DLVHEX=dlvhex2 MPCOMPILER=$(top_builddir)/mpcompiler/src/mpcompiler CMPSCRIPT=$(top_srcdir)/examples/compare.sh TESTDIR=$(top_srcdir)/mpcompiler/examples/tests RPCPARAMETERS="" DLVHEXPARAMETERS="--plugindir=$(top_builddir)/src"
