#!/bin/bash

# -----------------------------------------------------------------------------
#                                      compare.sh
# -----------------------------------------------------------------------------
#
# This file contains procedures for comparing the following file types
#   + merging plans
#   + hex programs
#   + answer-sets
#   + dot graphs
# It is useful for writing testcases for dlvhex plugins, especially for
#   + mergingplugin
#   + decisiondiagramsplugin
#
# The script expects two mandatory filenames as parameters. It will
# automatically detect the file type by checking the filename extension:
#     mp	--> merging plan
#     hex	--> file contains a hex program
#     as	--> file contains answer-sets
#     dot	--> file contains a dot graph
#     err	--> symbolic representation of errors of any kind (file content
#                   is irrelevant)
#
# The selection of the comparison algorithm is done according to the following
# rules.
# -----------------------------------------------------------------------------
# -- Error cases --
#     A comparison to err succeeds if and only if the other computation fails.
#     In detail:
#       err = err
#       err = any failed computation (with arbitrary output!)
#       err != any successful computation (with arbitrary output!)
#     Thus, for instance, err = rp if any of the tools involved in merging
#     plan evaluation returns an error.
#
# -- Basic cases --
#      + as/as   -> Compare the sets of answer-sets.
#      + dot/dot -> Compare the dot graphs (semantically, see below).
#    When comparing answer-set files, the order of the answer-sets and literals
#    within an answer-set is irrelevant
#    When comparing dot files, the graphs are compared semantically. That is,
#    the internal node names and file structue are irrelevant as long as the
#    encoded graphs are equivalent. This enables an efficient comparison of
#    decision diagrams, since they can contain duplicates of nodes that are
#    named according to this schema.
#
# -- Derived cases --
#     + mp/mp    -> Run the merging plan compiler on the merging plans, then
#                   execute the resulting hex programs using dlvhex. Finally
#		    compare their sets of answer-sets.
#     + hex/hex  -> Execute both hex programs and compare their sets of
#                   answer-sets.
#
# -- Extended cases --
#  If two different file types are passed, the "smaller one" is converted into
#  the greater one by calling the appropriate programs.
#    (with mp < hex < as < dot)
#  Conversion is done as follows:
#     {mp --> [mpcompiler] --> hex --> [dlvhex] --> [graphconverter] --> dot}
#  Therefore:
#     + hex/mp   -> Convert the merging plan into a hex program (using the
#                   merging plan compiler), then execute both programs and
#                   compare their sets of answer-sets.
#     + as/mp    -> Run the merging plan compiler on the merging plan and
#                   execute the resulting hex program. Then compare the result
#                   with the set of answer-sets.
#     + dot/mp   -> Convert the merging plan into a hex program (using the
#                   merging plan compiler), then execute the program and
#                   translate the answer into a dot graph that is compared.
#     + hex/as   -> Run dlvhex on the hex program, then compare the sets of
#                   answer-sets.
#     + dot/as   -> Convert the answer-set into a dot graph and compare it.
# -----------------------------------------------------------------------------
#
# The third and forth parameter are optional. If present, they need to be one
# of the listed hex extensions and overwrites the default compare mode
# determined by inspection of the filename.
# Example:
#   compare f1.hex f2.hex
# assumes that both files are hex programs, whereas
#   compare f1.hex f2.hex as as
# tells the script that the files rather contain answer-sets despete their
# extension. If only 3 parameters are passed, the forth one is implicitly given
# (the same as the third).
# 
# Written by Christoph Redl (e0525250@mail.student.tuwien.ac.at)
# March 23, 2010, 21:18


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
		# restrict predicate and constant names to the part before the first occurrence of underscore
		#literal=$(echo "$literal" | sed 's/_[^,\)\("}]*//g')
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

# Simplifies dot files syntactically by dropping irrelevant tokens (digraph keyword, etc.) such that comparing becomes easier.
# In detail, two graphs are equivalent if their structue coincide. The node names and ordering are irrelevant as long as their
# labels are equal.
function reduceDotFile {
	awkscript='
		{
			# read input file line by line
			file[NR]=$0;
		}

		# print a subgraph recursivly in a unique and comparable format
		function output(nodes, node, level, i, s, children, condition, suboutput, retval, indent, j, tmpval, tmparray, tmparray2){
			retval = ""
			indent = ""
			for (i=1; i <= level; i++){
				indent = indent "   "
			}

			# print node label
			# and the total number of accesses to this node
			#    (this allows to distinct two nodes with the same label from two edges to the same node)
			retval = retval nodes[node] " {" acccount[node]++ "}";

			# find subnodes
			s=0
			for (i=1; i <= NR; i++){
				re=node "[ ]*->";
				if (match(file[i], re)){
					nodestring=file[i];
					gsub(/ /, "", nodestring);
					gsub(/->/, ",", nodestring);
					gsub(/\[label=\"/, ",", nodestring);
					gsub(/\"\];/, "", nodestring);
					split(nodestring, subnode, ",");
					children[++s]=subnode[2]
					condition[s]=subnode[3]
				}
			}

			# sort subnodes by condition and visit them in this order
			delete tmparray;
			for (i=1; i <= s; i++){
				tmparray[children[i]] = condition[i];
			}
			asorti(tmparray, tmparray2);
			i=0;
			for (tmpval in tmparray2){
				++i;
				children[i] = tmparray2[tmpval];
				condition[i] = tmparray[tmparray2[tmpval]];
			}
			delete tmparray;

			# generate output for subnode
			for (i=1; i <= s; i++){
				suboutput[i] = output(nodes, children[i], level + 1);
				tmpval = split(suboutput[i], tmparray, "\n");
				suboutput[i] = indent "{" condition[i] "}";
				# make sure that the indention is correct
				for (j=1; j <= tmpval; j++){
					suboutput[i] = suboutput[i] "\n" indent tmparray[j];
				}
			}
			asort(suboutput);
			for (i=1; i <= s; i++){
				retval = retval "\n" suboutput[i];
			}

			return retval;
		}

		# finds the root node of the graph
		function getRoot(nodes, isroot, re){

			for (key in nodes){
				# check if this node has an ingoing edge
				isroot=1
				for (i=1; i <= NR; i++){
					re="-> " key "[ ]*[\\[;]";
					if (match(file[i], re)){
						# yes, thus it is not the root
						isroot=0
					}
				}
				if (isroot==1){
					# root was found
					return key
				}
			}
		}

		END {
			n=1
			# create a list of nodes
			#   extract nodes from edge definition
			for (i=1; i <= NR; i++){
				if (file[i] ~ /.*->.*;$/){
					nodestring=file[i];
					gsub(/ /, "", nodestring);
					gsub(/->/, ",", nodestring);
					gsub(/\[/, ",", nodestring);
					split(nodestring, node, ",");
					nodes[node[1]]=node[1]
					nodes[node[2]]=node[2]
				}
			}
			#   extract explicitly defined nodes
			for (i=1; i <= NR; i++){
				if (file[i] ~ /^[^>]*;$/){
					# extract node name and classification (the label is irrelevant)
					nodestring=file[i];
					gsub(/ /, "", nodestring);
					gsub(/\[label=\"([^\[\"]*)?/, ",", nodestring);
					gsub(/\"\];/, "", nodestring);
					split(nodestring, node, ",");

					# make an entry in the nodes array
					nodes[node[1]] = node[2];
				}
			}

			# generate comparable format
			r=getRoot(nodes);
			print output(nodes, r, 1);
		}

		';

	awk "$awkscript" $1
}

function compareDotFiles {

	graph1=$(reduceDotFile $1)
	graph2=$(reduceDotFile $2)

	if [ "$graph1" = "$graph2" ]; then
		return 0;
	else
		echo "Error: Graphs $1 and $2 differ:"
		echo "$graph1"
		echo "vs."
		echo "$graph2"

		return 1;
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

function checkGraphConverter {

	# Check if graphconverter is available
	if ! which $GRAPHCONVERTER >/dev/null; then
		echo "graphconverter was not found"
		return 1
	fi
}

function checkMPCompiler {

	# Check if mpcompiler is available
	if ! which $MPCOMPILER >/dev/null; then
		echo "mpcompiler was not found"
		return 1
	fi
}

function compare {

	# default values
	if [ "$DLVHEX" = "" ]; then
		DLVHEX="dlvhex"
	fi
	if [ "$GRAPHCONVERTER" = "" ]; then
		GRAPHCONVERTER="graphconverter"
	fi
	if [ "$MPCOMPILER" = "" ]; then
		MPCOMPILER="mpcompiler"
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
			echo "  -----------------------------------------------------------------------------"
			echo "                                       compare.sh"
			echo "  -----------------------------------------------------------------------------"
			echo " "
			echo "  This file contains procedures for comparing the following file types"
			echo "    + revision plans"
			echo "    + hex programs"
			echo "    + answer-sets"
			echo "    + dot graphs"
			echo "  It is useful for writing testcases for dlvhex plugins, especially for"
			echo "    + mergingplugin"
			echo "    + decisiondiagramsplugin"
			echo " "
			echo "  The script expects two mandatory filenames as parameters. It will"
			echo "  automatically detect the file type by checking the filename extension:"
			echo "      rp	--> revision plan"
			echo "      hex	--> file contains a hex program"
			echo "      as	--> file contains answer-sets"
			echo "      dot	--> file contains a dot graph"
			echo "      err	--> symbolic representation of errors of any kind (file content"
			echo "                    is irrelevant)"
			echo " "
			echo "  The selection of the comparison algorithm is done according to the following"
			echo "  rules."
			echo "  -----------------------------------------------------------------------------"
			echo "  -- Error cases --"
			echo "      A comparison to err succeeds if and only if the other computation fails."
			echo "      In detail:"
			echo "        err = err"
			echo "        err = any failed computation (with arbitrary output!)"
			echo "        err != any successful computation (with arbitrary output!)"
			echo "      Thus, for instance, err = rp if any of the tools involved in merging"
			echo "      plan evaluation returns an error."
			echo " "
			echo "  -- Basic cases --"
			echo "       + as/as   -> Compare the sets of answer-sets."
			echo "       + dot/dot -> Compare the dot graphs (semantically, see below)."
			echo "     When comparing answer-set files, the order of the answer-sets and literals"
			echo "     within an answer-set is irrelevant"
			echo "     When comparing dot files, the graphs are compared semantically. That is,"
			echo "     the internal node names and file structue are irrelevant as long as the"
			echo "     encoded graphs are equivalent. This enables an efficient comparison of"
			echo "     decision diagrams, since they can contain duplicates of nodes that are"
			echo "     named according to this schema."
			echo " "
			echo "  -- Derived cases --"
			echo "      + mp/mp    -> Run the merging plan compiler on the merging plans, then"
			echo "                    execute the resulting hex programs using dlvhex. Finally"
			echo " 		    compare their sets of answer-sets."
			echo "      + hex/hex  -> Execute both hex programs and compare their sets of"
			echo "                    answer-sets."
			echo " "
			echo "  -- Extended cases --"
			echo "   If two different file types are passed, the "smaller one" is converted into"
			echo "   the greater one by calling the appropriate programs."
			echo "     (with rp < hex < as < dot)"
			echo "   Conversion is done as follows:"
			echo "      {mp --> [mpcompiler] --> hex --> [dlvhex] --> [graphconverter] --> dot}"
			echo "   Therefore:"
			echo "      + hex/mp   -> Convert the merging plan into a hex program (using the"
			echo "                    merging plan compiler), then execute both programs and"
			echo "                    compare their sets of answer-sets."
			echo "      + as/mp    -> Run the merging plan compiler on the merging plan and"
			echo "                    execute the resulting hex program. Then compare the result"
			echo "                    with the set of answer-sets."
			echo "      + dot/mp   -> Convert the merging plan into a hex program (using the"
			echo "                    merging plan compiler), then execute the program and"
			echo "                    translate the answer into a dot graph that is compared."
			echo "      + hex/as   -> Run dlvhex on the hex program, then compare the sets of"
			echo "                    answer-sets."
			echo "      + dot/as   -> Convert the answer-set into a dot graph and compare it."
			echo "  -----------------------------------------------------------------------------"
			echo " "
			echo "  The third and forth parameter are optional. If present, they need to be one"
			echo "  of the listed hex extensions and overwrites the default compare mode"
			echo "  determined by inspection of the filename."
			echo "  Example:"
			echo "    compare f1.hex f2.hex"
			echo "  assumes that both files are hex programs, whereas"
			echo "    compare f1.hex f2.hex as as"
			echo "  tells the script that the files rather contain answer-sets despete their"
			echo "  extension. If only 3 parameters are passed, the forth one is implicitly given"
			echo "  (the same as the third)."
			echo "  "
			echo "  Written by Christoph Redl (e0525250@mail.student.tuwien.ac.at)"
			echo "  February 21, 2010, 21:10"
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

	# some input types need the mpcompiler
	if [ "$extension1" = "mp" ] || [ "$extension2" = "mp" ]; then
		if ! checkMPCompiler; then
			return 1
		fi
	fi

	# some input types need the graphconverter
	if [ "$extension1" = "dot" ] && [ "$extension2" != "dot" ]; then
		if ! checkGraphConverter; then
			return 1
		fi
	fi
	if [ "$extension1" != "dot" ] && [ "$extension2" = "dot" ]; then
		if ! checkGraphConverter; then
			return 1
		fi
	fi

	# counter for errors from subprogram calls
	rv=0

	if [ "$extension1" == "err" ] || [ "$extension2" == "err" ]
	then
		# comparisons with errors

		# if both are errors, the result is: equal
		if [ "$extension1" == "err" ] && [ "$extension2" == "err" ]
		then
			retval=0
		else
			# otherwise: compute the "non-error" operand
			if [ "$extension1" == "err" ]; then
				nonerrextension=$extension2
				nonerrfile=$2
			else
				nonerrextension=$extension1
				nonerrfile=$1
			fi
			case "$nonerrextension1" in
				"mp")   $MPCOMPILER < $nonerrfile | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS1
					let rv=rv+$?
					;;
				"hex")  $DLVHEX --silent $DLVHEXPARAMETERS $nonerrfile > $TMPFILE_AS1
					let rv=rv+$?
					;;
			esac
			if [ "$rv" != 0 ]; then
				retval=1
			fi
		fi
	else
		# other cases (non-errors)

		# extended cases
		# conversion
		# 	rp < hex < as < dot
		if [ "$extension1" != "$extension2" ]
		then
			case "$extension1/$extension2" in
				"mp/hex")	filter=$($MPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
						let rv=rv+$?
						$MPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- > $TMPFILE_AS1
						let rv=rv+$?
						$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
						let rv=rv+$?
						extension="as"
						;;
				"hex/mp")	filter=$($MPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
						let rv=rv+$?
						$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
						let rv=rv+$?
						$MPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- > $TMPFILE_AS2
						let rv=rv+$?
						extension="as"
						;;
				"mp/as")	filter=$($MPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
						let rv=rv+$?
						$MPCOMPILER < $1 | $DLVHEX $DLVHEXPARAMETERS --silent --filter=$filter -- > $TMPFILE_AS1
						let rv=rv+$?
						cp $2 $TMPFILE_AS2
						extension="as"
						;;
				"as/mp")	cp $1 $TMPFILE_AS1
						$MPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS2
						let rv=rv+$?
						extension="as"
						;;
				"mp/dot")	filter=$($MPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
						let rv=rv+$?
						$MPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- | $GRAPHCONVERTER as dot > $TMPFILE_DOT1
						let rv=rv+$?
						cp $2 $TMPFILE_DOT2
						extension="dot"
						;;
				"dot/mp")	filter=$($MPCOMPILER < $1 | tail -1 | sed "s/.*filter=\([^ ]*\)[ ].*/\1/")
						let rv=rv+$?
						cp $1 $TMPFILE_DOT1
						$MPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS --filter=$filter -- | $GRAPHCONVERTER as dot > $TMPFILE_DOT2
						let rv=rv+$?
						extension="dot"
						;;
				"hex/as")	$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
						let rv=rv+$?
						cp $2 $TMPFILE_AS2
						extension="as"
						;;
				"as/hex")	cp $1 $TMPFILE_AS1
						$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
						let rv=rv+$?
						extension="as"
						;;
				"hex/as")	$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
						let rv=rv+$?
						cp $2 $TMPFILE_AS2
						extension="as"
						;;
				"hex/dot")	$DLVHEX --silent $DLVHEXPARAMETERS $1 | $GRAPHCONVERTER as dot > $TMPFILE_DOT1
						let rv=rv+$?
						cp $2 $TMPFILE_DOT2
						extension="dot";
						;;
				"dot/hex")	cp $1 $TMPFILE_DOT1
						$DLVHEX --silent $DLVHEXPARAMETERS $2 | $GRAPHCONVERTER as dot > $TMPFILE_DOT2
						let rv=rv+$?
						extension="dot";
						;;
				"as/dot")	$GRAPHCONVERTER as dot < $1 > $TMPFILE_DOT1
						let rv=rv+$?
						cp $2 $TMPFILE_DOT2
						extension="dot"
						;;
				"dot/as")	cp $1 $TMPFILE_DOT1
						$GRAPHCONVERTER as dot < $2 > $TMPFILE_DOT2
						let rv=rv+$?
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
				"mp")
					$MPCOMPILER < $1 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS1
					let rv=rv+$?
					$MPCOMPILER < $2 | $DLVHEX --silent $DLVHEXPARAMETERS -- > $TMPFILE_AS2
					let rv=rv+$?
					extension="as"
					;;
				"hex")
					$DLVHEX --silent $DLVHEXPARAMETERS $1 > $TMPFILE_AS1
					let rv=rv+$?
					$DLVHEX --silent $DLVHEXPARAMETERS $2 > $TMPFILE_AS2
					let rv=rv+$?
					extension="as"
					;;
					# basic cases
				"dot")
					extension=$extension1
					cp $1 $TMPFILE_DOT1
					cp $2 $TMPFILE_DOT2
					;;
				"as")
					extension=$extension1
					cp $1 $TMPFILE_AS1
					cp $2 $TMPFILE_AS2
					;;
			esac
		fi

		# basic cases
		case "$extension" in
			"dot")
				# compare dot graphs
				compareDotFiles $TMPFILE_DOT1 $TMPFILE_DOT2
				retval=$?
				# check if all subprogram calls succeeded
				if [ $rv != 0 ]; then
					retval=1
				fi
				;;
		 	"as")
				# compare set of answer-sets
				compareSetsOfAnswerSets $TMPFILE_AS1 $TMPFILE_AS2
				retval=$?
				# check if all subprogram calls succeeded
				if [ $rv != 0 ]; then
					retval=1
				fi
				;;
			*)
				echo "Unknown extension: $extension"
				retval=1
				;;
		esac
	fi

	# cleanup
	rm -f $TMPFILE_AS1
	rm -f $TMPFILE_AS2
	rm -f $TMPFILE_DOT1
	rm -f $TMPFILE_DOT2

	return $retval
}

# run compare function
compare $*
