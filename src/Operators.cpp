#include <Operators.h>
#include <IOperator.h>

#include <stdlib.h>
#include <ltdl.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

// Built-In Operators
#include <UnionOp.h>

using namespace dlvhex::asp;

OperatorAtom::OperatorAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// operator name
	addInputPredicate();	// predicate containing all the answer indices which shall be passed to the operator
	addInputPredicate();	// predicate containing all the key-value pairs which are passed as parameter
	setOutputArity(1);	// answer index of result

	// Provide built-In operators
	builtinOperators["union"] = new UnionOp();
}

OperatorAtom::~OperatorAtom()
{
	// Delete built-In operators
	delete builtinOperators["union"];
}

void
OperatorAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Extract operator name
	std::string opname = query.getInputTuple()[0].getUnquotedString();

	// Extract the predicate which is true for all indices of answers passed to the parameter
	AtomSet opparams;
	std::string predname = query.getInputTuple()[1].getUnquotedString();
	query.getInterpretation().matchPredicate(predname, opparams);
	std::vector<HexAnswer*> answers;
	std::vector<int> answersIndices;
	for (AtomSet::const_iterator it = opparams.begin(); it != opparams.end(); it++){
		int answernr = it->getArgument(1).getInt();
		answers.push_back(&(resultsetCache[answernr].second));
		answersIndices.push_back(answernr);
	}

	// Extract the operator's key-value arguments
	AtomSet arguments;
	std::string argumentspredname = query.getInputTuple()[2].getUnquotedString();
	query.getInterpretation().matchPredicate(argumentspredname, arguments);
	OperatorArguments parameters;
	for (AtomSet::const_iterator it = arguments.begin(); it != arguments.end(); it++){
		std::string key = it->getArgument(0).getUnquotedString();
		std::string value = it->getArgument(1).getUnquotedString();
		parameters.push_back(KeyValuePair(key, value));
	}

	// Assemble unique identifier for this operator application (consisting of operator name and argument indices, see below)
	HexCall hc(HexCall::OperatorCall, opname, answersIndices, parameters);

	// Check if answer is in cache
	int i = 0;
	for (HexAnswerCache::iterator it = resultsetCache.begin(); it != resultsetCache.end(); it++){
		if (it->first == hc){
			// Reuse answer
			Tuple out;
			out.push_back(Term(i));
			answer.addTuple(out);
			return;
		}
		i++;
	}

	// No: Compute it
	// call operator, let opanswer be it's result
	HexAnswer opanswer;

	// If the operator name matches one of the built-in operators, it is executed
	if (builtinOperators.find(opname) != builtinOperators.end()){
		// Built-In operator
		//std::cout << "CALLING INTERNAL OP";
		opanswer = builtinOperators["union"]->apply(parameters.size() + 1, answers, parameters);
	}else{
		// External operator
		//std::cout << "CALLING EXTERNAL OP";

		// Look for the operator library
		lt_dlhandle dlHandle = lt_dlopenext(opname.c_str());							// Absolute path
		if (dlHandle == NULL){
			for (int i = 0; dlHandle == NULL && i < operatorpaths.size(); i++){
				dlHandle = lt_dlopenext((operatorpaths[i] + std::string("/") + opname).c_str());	// Prefix path with one search location after the other
			}
		}
		// Check if the operator library was found
		if (dlHandle == 0){
			throw PluginError((std::string("Operator library \"") + opname + std::string("\" not found")).c_str());
		}
		t_getOperator getOperator = (t_getOperator) lt_dlsym(dlHandle, "getOperator");
		if (getOperator == 0){
			throw PluginError((std::string("Function getOperator not found in operator library \"") + opname + std::string("\"")).c_str());
		}
		// Finally call the operator
		IOperator* externalOperator = getOperator();
		opanswer = externalOperator->apply(parameters.size() + 1, answers, parameters);
	}

	// Put operator's answer into cache
	resultsetCache.push_back(HexAnswerCacheEntry(hc, opanswer));

	// Return answer index
	Tuple out;
	out.push_back(Term(resultsetCache.size() - 1));
	answer.addTuple(out);
}

void OperatorAtom::setSearchPaths(std::vector<std::string> paths){
	this->operatorpaths = paths;
}
