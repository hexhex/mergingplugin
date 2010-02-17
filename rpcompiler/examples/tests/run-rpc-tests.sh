#!/bin/bash

#
# dlvhex -- Answer-Set Programming with external interfaces.
# Copyright (C) 2005, 2006, 2007 Roman Schindlauer
# Modified by Christoph Redl in December 2009
# 
# This file is part of dlvhex.
#
# dlvhex is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# dlvhex is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with dlvhex; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA.
#

MKTEMP="mktemp -t tmp.XXXXXXXXXX"
TMPFILE=$($MKTEMP) # global temp. file for answer sets

failed=0
warned=0
ntests=0

echo ============ revision plan tests start ============

for t in $(find $TESTDIR -name '*.test' -type f)
do
    # REFERENCERETVAL
    #   00 -> RP compiler output is expected to be 0; compare return value only
    #   01 -> RP compiler output is expected to be 1; compare return value only
    #   10 -> RP compiler output is expected to be 0; compare RP compiler output
    #   11 -> RP compiler output is expected to be 1; compare RP compiler output
    #   20 -> RP compiler output is expected to be 0; pass compiler output to dlvhex and compare answer sets
    while read INPUT REFOUTPUT REFERENCERETVAL ADDDLVPARAM  ## ADDRPCPARAM
    do
	let ntests++

	INPUT=$TESTDIR/$INPUT
	REFOUTPUT=$TESTDIR/$REFOUTPUT

	if [ ! -f $INPUT ] || [ ! -f $REFOUTPUT ]; then
	    test ! -f $INPUT && echo WARN: Could not find input file $INPUT
	    test ! -f $REFERENCEOUTPUT && echo WARN: Could not find reference output file $REFOUTPUT
	    continue
	fi

	if test $REFERENCERETVAL -eq 20
	then
		# COMPARE ANSWER SETS

		# Drop first digit of reference return value since it is only used for the distinction if answer sets or compiler output shall be compared
		REFERENCERETVAL=0

		$CMPSCRIPT $INPUT $REFOUTPUT > /dev/null
		succ=$?

		if [ $succ ]
		then
			echo "PASS: $INPUT"
		else
			echo "FAIL: $INPUT"
			let failed++
		fi
	else
		# COMPARE COMPILER OUTPUT

		# Drop first digit of reference return value since it is only used for the distinction if answer sets or compiler output shall be compared
		if test $REFERENCERETVAL -eq 10 || test $REFERENCERETVAL -eq 00
		then
			CMPRETVAL=0
		else
			CMPRETVAL=1
		fi

		# run rpcompiler with specified parameters and program
		$RPCOMPILER  $RPCPARAMETERS $ADDPARM $INPUT >& $TMPFILE
		RPCOMPILERRETVAL=$?

		# compare compiler output with reference result
		if (cmp -s $TMPFILE $REFOUTPUT || test $REFERENCERETVAL -eq 00 || test $REFERENCERETVAL -eq 01) && test $CMPRETVAL -eq $RPCOMPILERRETVAL
		then
			echo "PASS: $INPUT"
		else
			echo "FAIL: $INPUT"
			if ! test $RPCOMPILERRETVAL -eq $CMPRETVAL
			then
				echo "Return values differ: $RPCOMPILERRETVAL (should be $CMPRETVAL)"
			else
				echo "Outputs differ:"
				diff $INPUT $REFOUTPUT
			fi
			let failed++
		fi
	fi
    done < $t # redirect test file to the while loop
done

# cleanup
rm -f $TMPFILE

echo ========== revision plan tests completed ==========

echo Tested $ntests revision plans
echo $failed failed tests, $warned warnings

echo ============= revision tests end =============

exit $failed
