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

using namespace dlvhex::merging;

OperatorAtom::OperatorAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// operator name
	addInputPredicate();	// predicate containing all the answer indices which shall be passed to the operator
	addInputPredicate();	// predicate containing all the key-value pairs which are passed as parameter
	setOutputArity(1);	// answer index of result
}

OperatorAtom::~OperatorAtom()
{
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
	answers.resize(opparams.size());
	answersIndices.resize(opparams.size());
	for (AtomSet::const_iterator it = opparams.begin(); it != opparams.end(); it++){
		int answernrpos = it->getArgument(1).getInt();
		int answernr = it->getArgument(2).getInt();
		answers[answernrpos] = &(resultsetCache[answernr].second);
		answersIndices[answernrpos] = answernr;
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

	// If the operator name matches one of the loaded operators, it is executed
	try{
		// Search for the oprator
		std::map<std::string, IOperator*>::const_iterator itOp = operators.find(opname);
		if (itOp != operators.end()){

			// Found
			IOperator* op = (*itOp).second;
	
			// Finally call the operator
			opanswer = op->apply(answersIndices.size(), answers, parameters);

			// Put operator's answer into cache
			resultsetCache.push_back(HexAnswerCacheEntry(hc, opanswer));

			// Return answer index
			Tuple out;
			out.push_back(Term(resultsetCache.size() - 1));
			answer.addTuple(out);

			return;
		}else{
			// Operator with the specified name was not loaded
			std::stringstream msg;
			msg << "Operator \"" << opname << "\" was not loaded";
			throw IOperator::OperatorException(msg.str());
		}
/*
		if (builtinOperators.find(opname) != builtinOperators.end()){
			// Built-In operator
			//std::cout << "CALLING INTERNAL OP";
			opanswer = builtinOperators[opname]->apply(answersIndices.size(), answers, parameters);
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
			opanswer = externalOperator->apply(answersIndices.size(), answers, parameters);
		}
*/

	}catch(IOperator::OperatorException oe){
		throw PluginError(oe.getMessage());
	}
}

void OperatorAtom::addOperators(std::string lib){

	// Open the specified library
	lt_dlhandle dlHandle = lt_dlopenext(lib.c_str());							// Absolute path

	// Check if the operator library was found
	if (dlHandle == 0){
		throw PluginError((std::string("Operator library \"") + lib + std::string("\" not found")).c_str());
	}
	t_operatorImportFunction operatorImportFunction = (t_operatorImportFunction) lt_dlsym(dlHandle, "OPERATORIMPORTFUNCTION");
	if (operatorImportFunction == 0){
		throw PluginError((std::string("Operator import function (signature: \"std::vector<dlvhex::merging::IOperator*> OPERATORIMPORTFUNCTION()\") was not found in operator library \"") + lib + std::string("\"")).c_str());
	}

	// Finally call the operator import function to retrieve the operators in this library
	std::vector<IOperator*> externalOperators = operatorImportFunction();

	// Add the operators to the operator list
	for (std::vector<IOperator*>::iterator it = externalOperators.begin(); it != externalOperators.end(); it++){
		// Check if the operator name is unique
		if (operators.find((*it)->getName()) != operators.end()){
			// Name is not unique
			throw PluginError((std::string("Operator name\"") + (*it)->getName() + std::string("\" is not unique. A duplicate was find in library \"") + lib + std::string("\".")).c_str());
		}
	}
}
