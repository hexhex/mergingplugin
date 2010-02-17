#!/bin/bash

# This file contains procedures for comparing the following file types
#   + answer-sets
#   + dot graphs
# It is especially useful for writing testcases for dlvhex plugins
#
# The script expects two filenames as parameters. It will automatically detect the
# file type by checking the filename extension:
#     rp	--> revision plan
#     hex	--> file contains a hex program
#     as	--> file contains answer-sets
#     dot	--> file contains a dot graph
#
# The selection of the comparison algorithm is done according to the following rules:
#
# -- Basic cases --
#     as/as	-> Compare the sets of answer-sets
#     dot/dot	-> Compare the dot graphs
#
# -- Derived cases --
#     rp/rp	-> Run the revision plan compiler on the revision plans, then execute
#		   the resulting hex programs using dlvhex. finally compare their sets
#		   of answer-sets
#     hex/hex	-> Execute both hex programs and compare their sets of answer-sets
#
# -- Extended cases --
#  If two different file types are passed, the "smaller one" is converted into the greater one
#  by calling the appropriate programs. (rp < hex < as < dot)
#  Conversion is done as follows:
#     rp --> [revision plan compiler] --> hex --> [dlvhex] --> [dotconverter] --> dot

#     hex/rp	-> Convert the revision plan into a hex program (using the revision plan compiler),
#		   then execute both programs and compare their sets of answer-sets.
#     as/rp	-> Run the revision plan compiler on the revision plan and execute the resulting
#		   hex program. Then compare the result with the set of answer-sets
#     dot/rp	-> Convert the revision plan into a hex program (using the revision plan compiler),
#		   then execute the program and translate the answer into a dot graph that is compared.
#     hex/as	-> Run dlvhex on the hex program, then compare the sets of answer-sets
#     dot/as	-> Convert the answer-set into a dot graph and compare it.
#
# The third and forth parameter are optional. If present, they need to be one of the
# listed hex extensions and overwrites the default compare mode determined by inspection
# of the filename.
# Example:
#   compare f1.hex f2.hex
# assumes that both files are hex programs, whereas
#   compare f1.hex f2.hex as as
# tells the script that the files rather contain answer-sets despete their extension.
# If only 3 parameters are passed, the forth one is implicitly given (the same as the third).
# 
# Written by Christoph Redl (e0525250@mail.student.tuwien.ac.at)
# February 17, 2010, 21:30


# ================================================== answer-sets ==================================================

# sorts the literals within an answer-set
function sortAnswerSets {
	# remove braces and split the answersset content at comma character (,) and sort the resulting list of literals
	sortedliterals=$(echo $1 | sed 's/^{\(.*\)}$/\1/' | sed 's/, /\n/g' | sort);

	# reassemble sorted answer-set and print it
	as="{"
	first=1
	for literal in $sortedliterals
	do
		if [ $first = 1 ]
		then
			as="$as$literal"
		else
			as="$as,$literal"
		fi
		let "first=0"
	done
	as="$as}"
	echo $as
}

# compares two answer-sets independently from the order of literals
function compareAnswerSets {
	# compare sorted answer-sets
	if [ "$1" = "$2" ]
	then
		return 0
	else
		return 1
	fi
}

# Compares the files given as $1 and $2. They are expected to contain sets of answer-sets (one answer-set per line).
# The order of the answer-sets and the literals within an answer-set is irrelevant.
function compareSetsOfAnswerSets {

	# read answer-sets of source 1
	i=0
	while read line
	do
		as1set[$i]=$(sortAnswerSets "$line")
		let "i=i+1"
	done < $1;

	# read answer-sets of source 2
	i=0
	while read line
	do
		as2set[$i]=$(sortAnswerSets "$line")
		let "i=i+1"
	done < $2;

	err=0

	# check if each answer-set as1 has an equal answer-set in as2
	for as1 in "${as1set[@]}"
	do
		found=false
		for as2 in "${as2set[@]}"
		do
			if compareAnswerSets "$as1" "$as2"
			then
				found=true
			fi
		done
		if [ $found = false ]
		then
			echo "Error: answer-set $as1 does not occur in set of answer-sets 2"
			let "err=err+1"
		fi
	done

	# check if each answer-set as2 has an equal answer-set in as1
	for as2 in "${as2set[@]}"
	do
		found=false
		for as1 in "${as1set[@]}"
		do
			if compareAnswerSets "$as2" "$as1"
			then
				found=true
			fi
		done
		if [ $found = false ]
		then
			echo "Error: answer-set $as2 does not occur in set of answer-sets 1"
			let "err=err+1"
		fi
	done

	if [ $err = 0 ]
	then
		return 0
	else
		return 1
	fi
}


# ================================================== dot graphs ==================================================

# simplifies dot files syntactically by dropping irrelevant tokens (digraph keyword, etc.)
# such that comparing becomes easier
function reduceDotFile {

	# read dot file
	while read line
	do
		# trim each line
		line=$(echo "$line" | sed 's/^[ ]*//')
		line=$(echo "$line" | sed 's/[ ]*$//')
		dot="$dot$line"
	done < $1;

	# remove the digraph{ } construct and reduce the graph to the actual content
	dot=$(echo -e "$dot" | sed "s/^[a-z|A-Z|0-9| ]*{\(.*\)}$/\1/")

	# remove double spaces
	dot=$(echo -e $dot)

	# reduce all sequences of whitespaces to single whitespaces
	dot=$(echo "$dot" | sed "s/;/\n/g")

	# sort the lines
	dot=$(echo -e "$dot" | sort)

	echo -e "$dot"
}

function compareDotFiles {
	dot1=$(echo -e "$(reduceDotFile $1)")
	dot2=$(echo -e "$(reduceDotFile $2)")

	# compare
	if [ "$dot1" = "$dot2" ]
	then
		return 0
	else
		echo "Error: Graphs $1 and $2 differ:"
		diff <(echo "$dot1") <(echo "$dot2")

		return 1
	fi
}


# ================================================== general ==================================================

function checkDlvhex {

	# Check if dlvhex is available
	if ! which $DLVHEX >/dev/null; then
		echo "dlvhex was not found"
		return 1
	fi
}

function checkRequirements {

	# Check if dotconverter is available
	if ! which $DOTCONVERTER >/dev/null; then
		echo "dotconverter was not found"
		return 1
	fi
}

function checkRPCompiler {

	# Check if rpcompiler is available
	if ! which $RPCOMPILER >/dev/null; then
		echo "rpcompiler was not found"
		return 1
	fi
}

function compare {

	# default values
	if [ "$DLVHEX" = "" ]; then
		DLVHEX="dlvhex"
	fi
	if [ "$DOTCONVERTER" = "" ]; then
		DOTCONVERTER="dotconverter"
	fi
	if [ "$RPCOMPILER" = "" ]; then
		RPCOMPILER="rpcompiler"
	fi



	if ! checkDlvhex; then
		return 1
	fi

	# make temporary files for intermediate output
	MKTEMP="mktemp -t tmp.XXXXXXXXXX"
	TMPFILE_AS1=$($MKTEMP)
	TMPFILE_AS2=$($MKTEMP)
	TMPFILE_DOT1=$($MKTEMP)
	TMPFILE_DOT2=$($MKTEMP)

	case $# in
		0)
			echo "  This file contains procedures for comparing the following file types"
			echo "    + answer-sets"
			echo "    + dot graphs"
			echo "  It is especially useful for writing testcases for dlvhex plugins"
			echo " "
			echo "  The script expects two filenames as parameters. It will automatically detect the"
			echo "  file type by checking the filename extension:"
			echo "      rp	--> revision plan"
			echo "      hex	--> file contains a hex program"
			echo "      as	--> file contains answer-sets"
			echo "      dot	--> file contains a dot graph"
			echo " "
			echo "  The selection of the comparison algorithm is done according to the following rules:"
			echo " "
			echo "  -- Basic cases --"
			echo "      as/as	-> Compare the sets of answer-sets"
			echo "      dot/dot	-> Compare the dot graphs"
			echo " "
			echo "  -- Derived cases --"
			echo "      rp/rp	-> Run the revision plan compiler on the revision plans, then execute"
			echo " 		   the resulting hex programs using dlvhex. finally compare their sets"
			echo " 		   of answer-sets"
			echo "      hex/hex	-> Execute both hex programs and compare their sets of answer-sets"
			echo " "
			echo "  -- Extended cases --"
			echo "   If two different file types are passed, the \"smaller one\" is converted into the greater one"
			echo "   by calling the appropriate programs. (rp < hex < as < dot)"
			echo "   Conversion is done as follows:"
			echo "      rp --> [revision plan compiler] --> hex --> [dlvhex] --> [dotconverter] --> dot"
			echo "      hex/rp	-> Convert the revision plan into a hex program (using the revision plan compiler),"
			echo " 		   then execute both programs and compare their sets of answer-sets."
			echo "      as/rp	-> Run the revision plan compiler on the revision plan and execute the resulting"
			echo " 		   hex program. Then compare the result with the set of answer-sets"
			echo "      dot/rp	-> Convert the revision plan into a hex program (using the revision plan compiler),"
			echo " 		   then execute the program and translate the answer into a dot graph that is compared."
			echo "      hex/as	-> Run dlvhex on the hex program, then compare the sets of answer-sets"
			echo "      dot/as	-> Convert the answer-set into a dot graph and compare it."
			echo " "
			echo "  The third and forth parameter are optional. If present, they need to be one of the"
			echo "  listed hex extensions and overwrites the default compare mode determined by inspection"
			echo "  of the filename."
			echo "  Example:"
			echo "    compare f1.hex f2.hex"
			echo "  assumes that both files are hex programs, whereas"
			echo "    compare f1.hex f2.hex as as"
			echo "  tells the script that the files rather contain answer-sets despete their extension."
			echo "  If only 3 parameters are passed, the forth one is implicitly given (the same as the third)."
			echo "  "
			echo "  Written by Christoph Redl (e0525250@mail.student.tuwien.ac.at)"
			echo "  February 17, 2010"
			;;
		2)
			# extract filename extensions
			extension1=$(echo $1 | sed "s/.*\.\([a-z|A-Z|0-9]*$\)/\1/" | tr "[:upper:]" "[:lower:]")
			extension2=$(echo $2 | sed "s/.*\.\([a-z|A-Z|0-9]*$\)/\1/" | tr "[:upper:]" "[:lower:]")
			;;
		3)
			extension1=$3
			extension2=$3
			;;
		4)
			extension1=$3
			extension2=$4
			;;
		*)
			echo "Invalid number of arguments"
			;;
	esac

	# some input types need the rpcompiler
	if [ "$extension1" = "rp" ] || [ "$extension2" = "rp" ]; then
		if ! checkRPCompiler; then
			return 1
		fi
	fi

	# some input types need the dotconverter
	if [ "$extension1" = "dot" ] && [ "$extension2" != "dot" ] || [ "$extension1" != "dot" ] && [ "$extension2" = "dot" ]; then
		if ! checkDotconverter; then
			return 1
		fi
	fi

	# extended cases
	# conversion
	# 	rp < hex < as < dot
	if [ "$extension1" != "$extension2" ]
	then
		case "$extension1/$extension2" in
			"rp/hex")	filter=$($RPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
					$RPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- > $TMPFILE_AS1
					$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
					extension="as"
					;;
			"hex/rp")	filter=$($RPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
					$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
					$RPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- > $TMPFILE_AS2
					extension="as"
					;;
			"rp/as")	filter=$($RPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
					$RPCOMPILER < $1 | $DLVHEX $DLVHEXPARAMETERS --silent --filter=$filter -- > $TMPFILE_AS1
					cp $2 $TMPFILE_AS2
					extension="as"
					;;
			"as/rp")	cp $1 $TMPFILE_AS1
					$RPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS2
					extension="as"
					;;
			"rp/dot")	filter=$($RPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
					$RPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- | $DOTCONVERTER --toas > $TMPFILE_DOT1
					cp $2 $TMPFILE_DOT2
					extension="dot"
					;;
			"dot/rp")	filter=$($RPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
					cp $1 $TMPFILE_DOT1
					$RPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- | $DOTCONVERTER --toas > $TMPFILE_DOT2
					extension="dot"
					;;
			"hex/as")	$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
					cp $2 $TMPFILE_AS2
					extension="as"
					;;
			"as/hex")	cp $1 $TMPFILE_AS1
					$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
					extension="as"
					;;
			"hex/as")	$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
					cp $2 $TMPFILE_AS2
					extension="as"
					;;
			"hex/dot")	$DLVHEX --silent $DLVHEXPARAMETERS $1 | $DOTCONVERTER --toas > $TMPFILE_DOT1
					cp $2 $TMPFILE_DOT2
					extension="dot";
					;;
			"dot/hex")	cp $1 $TMPFILE_DOT1
					$DLVHEX --silent $DLVHEXPARAMETERS $2 | $DOTCONVERTER --toas > $TMPFILE_DOT2
					extension="dot";
					;;
			"as/dot")	$DOTCONVERTER --toas < $1 > $TMPFILE_DOT1
					cp $2 $TMPFILE_DOT2
					extension="dot"
					;;
			"dot/as")	cp $1 $TMPFILE_DOT1
					$DOTCONVERTER --toas < $2 > $TMPFILE_DOT2
					extension="dot"
					;;
			*)		echo "Invalid combination of input types: $extension1/$extension2"
					return 1
					;;
		esac

		echo "Filename extensions differ: $extension1/$extension2. Will use $extension for comparison algorithm selection."
	else
		# derived cases
		case "$extension1" in
			"rp")
				$RPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS1
				$RPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS2
				extension="as"
				;;
			"hex")
				$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
				$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
				extension="as"
				;;
			*)
				# basic cases
				extension=$extension1
				;;
		esac
	fi

	# basic cases
	case "$extension" in
		"dot")
			# compare dot graphs
			compareDotFiles $TMPFILE_DOT1 $TMPFILE_DOT2
			retval=$?
			;;
	 	"as")
			# compare set of answer-sets
			compareSetsOfAnswerSets $TMPFILE_AS1 $TMPFILE_AS2
			retval=$?
			;;
		*)
			echo "Unknown extension: $extension"
			retval=1
			;;
	esac

	# cleanup
	rm -f $TMPFILE_AS1
	rm -f $TMPFILE_AS2
	rm -f $TMPFILE_DOT1
	rm -f $TMPFILE_DOT2

	return $retval
}

# run compare function
compare $*
