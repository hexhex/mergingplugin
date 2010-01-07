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
    #   10 -> RP compiler output is expected to be 0; compare RP compiler output
    #   11 -> RP compiler output is expected to be 1; compare RP compiler output
    #   20 -> RP compiler output is expected to be 0; pass compiler output to dlvhex and compare answer sets
    while read REVISIONPLAN REFERENCEOUTPUT REFERENCERETVAL ADDDLVPARAM  ## ADDRPCPARAM
    do
	let ntests++

	REVISIONPLAN=$TESTDIR/$REVISIONPLAN
	REFERENCEOUTPUT=$TESTDIR/$REFERENCEOUTPUT

	if [ ! -f $REVISIONPLAN ] || [ ! -f $REFERENCEOUTPUT ]; then
	    test ! -f $REVISIONPLAN && echo WARN: Could not find revision plan file $REVISIONPLAN
	    test ! -f $REFERENCEOUTPUT && echo WARN: Could not find reference output file $REFERENCEOUTPUT
	    continue
	fi

	if test $REFERENCERETVAL -eq 20
	then
		# COMPARE ANSWER SETS

		# Drop first digit of reference return value since it is only used for the distinction if answer sets or compiler output shall be compared
		REFERENCERETVAL=0

		# run revision plan compiler and pass result to dlvhex with specified parameters
		$RPCOMPILER $RPCPARAMETERS $ADDRPCPARAM $REVISIONPLAN | $DLVHEX $DLVPARAMETERS $ADDDLVPARAM -- | egrep -v "^$" > $TMPFILE

		if cmp -s $TMPFILE $REFERENCEOUTPUT
		then
		    echo PASS: $REVISIONPLAN
		else
		    # and now check which answersets differ

		    pasted=$($MKTEMP)
		    paste $REFERENCEOUTPUT $TMPFILE > $pasted

		    OLDIFS=$IFS
		    IFS=" " # we need the tabs for cut

		    nas=1 # counter for answer sets

	 	    while read
		    do
				# translate both answersets to python lists
				a1=$(echo $REPLY | cut -f1 | sed s/"'"/"\\\'"/g | sed s/"{"/"['"/ | sed s/", "/"', '"/g | sed s/"}"/"']"/)
				a2=$(echo $REPLY | cut -f2 | sed s/"'"/"\\\'"/g | sed s/"{"/"['"/ | sed s/", "/"', '"/g | sed s/"}"/"']"/)

				# check if this is a weak answerset info
				if [ $(echo "$a1" | awk '{ print match($0, "Cost ") }') = 1 ] && [ $(echo "$a2"  | awk '{ print match($0, "Cost ") }') = 1 ] ; then
				    let nas--
				    if [ "$a1" != "$a2" ] ; then
					echo "FAIL: Answer set costs differ: $a1 vs. $a2"
					let failed++
				    fi
				elif cat <<EOF | python
# -*- coding: utf-8 -*-
# now check if set difference yields incomparability
import sys
a1 = $a1
a2 = $a2
z1 = zip(a1,a2)
z2 = zip(z1, range(len(z1)))
z3 = [ e for e in z2 if e[0][0] != e[0][1] ]
for e in z3: print 'In Answerset ' + str($nas) + ' (fact ' + str(e[1]) + '): ' + e[0][0] + ' vs. ' + e[0][1]
s1 = set(a1)
s2 = set(a2)
sys.exit(len(s1.symmetric_difference(s2)))
EOF
				then
					echo "WARN: $RPCOMPILER $RPCPARAMETERS $ADDRPCPARAM $REVISIONPLAN | $DLVHEX $DLVPARAMETERS $ADDDLVPARM -- (answerset $nas has different ordering)"
					let warned++
				else
					echo "FAIL: $RPCOMPILER $RPCPARAMETERS $ADDRPCPARAM $REVISIONPLAN | $DLVHEX $DLVPARAMETERS $DDDLVPARM -- (answerset $nas differs)"
					let failed++
				fi

				let nas++
		    done < $pasted # redirected pasted file to the while loop

		    IFS=$OLDIFS

		    rm -f $pasted
		fi
	else
		# COMPARE COMPILER OUTPUT

		# Drop first digit of reference return value since it is only used for the distinction if answer sets or compiler output shall be compared
		if test $REFERENCERETVAL -eq 10
		then
			REFERENCERETVAL=0
		else
			REFERENCERETVAL=1
		fi
echo "CALLING $REVISIONPLAN $REFERENCEOUTPUT $REFERENCERETVAL $ADDRPCPARAM"

		# run rpcompiler with specified parameters and program
		$RPCOMPILER  $RPCPARAMETERS $ADDPARM $REVISIONPLAN > $TMPFILE
		RPCOMPILERRETVAL=$?

		# compare compiler output with reference result
		if cmp -s $TMPFILE $REFERENCEOUTPUT && test $REFERENCERETVAL -eq $RPCOMPILERRETVAL
		then
			echo PASS: $REVISIONPLAN
		else
			echo FAILED: $REVISIONPLAN
			if ! test $RPCOMPILERRETVAL -eq $REFERENCERETVAL
			then
				echo "Return values differ: $RPCOMPILERRETVAL (should be $REFERENCERETVAL)"
			else
				echo "Outputs differ:"
				diff $REVISIONPLAN $REFERENCEOUTPUT
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
