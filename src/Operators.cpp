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

#include "dlvhex/globals.h"

using namespace dlvhex::merging::plugin;

void OperatorAtom::registerBuiltInOperators(){

	// Add a command of the following type for each built-in operator:
	//
	// 	operators[ptr->getName()] = ptr;
	//
	// where ptr is a pointer to an instance of a class implementing the interface IOperator (defining the abstract methods getName and apply).
	// getName() needs to deliver a unique name for the operator.
	//	
	// Make sure that ptr does not get invalid before the destructor of this OperatorAtom is called.

	// built-in operators
	operators[_union.getName()] = &_union;
	operators[_setminus.getName()] = &_setminus;
	operators[_dalal.getName()] = &_dalal;
	operators[_dbo.getName()] = &_dbo;
	operators[_majorityselection.getName()] = &_majorityselection;
	operators[_relationmerging.getName()] = &_relationmerging;
}

OperatorAtom::OperatorAtom(HexAnswerCache &rsCache) : resultsetCache(rsCache)
{
	addInputConstant();	// operator name
	addInputPredicate();	// predicate containing all the answer indices which shall be passed to the operator
	addInputPredicate();	// predicate containing all the key-value pairs which are passed as parameter
	setOutputArity(1);	// answer index of result

	registerBuiltInOperators();
	if (!Globals::Instance()->getOption("Silent")){
		std::cout << "mergingplugin: Loaded " << operators.size() << " built-in operators" << std::endl;
	}
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

	// retrieve the indices of the answers that are passed to the operator
	query.getInterpretation().matchPredicate(predname, opparams);
	std::vector<HexAnswer*> answers;
	std::vector<int> answersIndices;
	answers.resize(opparams.size());
	for (int i = 0; i < opparams.size(); i++) answers[i] = NULL;
	answersIndices.resize(opparams.size());
	for (AtomSet::const_iterator it = opparams.begin(); it != opparams.end(); it++){
		int answernrpos = it->getArgument(1).getInt();
		int answernr = it->getArgument(2).getInt();
		// index check
		if (answernr < 0 || answernr >= resultsetCache.size()){
			throw PluginError("An invalid answer handle was passed to external atom &operator");
		}
		// check uniqueness of operator arguments
		if (answers[answernrpos] != NULL){
			std::stringstream ss;
			ss << "Multiple answers were passed as " << answernrpos << "-th argument of operator \"" << opname << "\"";
			throw PluginError(ss.str());
		}
		answers[answernrpos] = resultsetCache[answernr];
		answersIndices[answernrpos] = answernr;
	}

	// Extract the operator's key-value arguments
	AtomSet arguments;
	std::string argumentspredname = query.getInputTuple()[2].getUnquotedString();
	query.getInterpretation().matchPredicate(argumentspredname, arguments);
	OperatorArguments parameters;
	for (AtomSet::const_iterator it = arguments.begin(); it != arguments.end(); it++){
		std::string key = it->getArgument(1).getUnquotedString();
		std::string value = it->getArgument(2).getUnquotedString();
		parameters.push_back(KeyValuePair(key, value));
	}

	// If the operator name matches one of the loaded operators, it is executed
	try{
		// Search for the oprator
		IOperator* op = getOperator(opname); // throws an OperatorException of not found

		// Assemble unique identifier for this operator application (consisting of operator name and argument indices, see below)
		HexCall hc(HexCall::OperatorCall, op, debug, silent, answersIndices, parameters);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(Term(resultsetCache[hc]));
		answer.addTuple(out);
	}catch(IOperator::OperatorException& oe){
		throw PluginError(oe.getMessage());
	}catch(std::runtime_error& e){
		throw PluginError(e.what());
	}catch(...){
		throw PluginError("Operator has thrown an exception of unknown type");
	}
}

void OperatorAtom::setMode(bool silentMode, bool debugMode){
	this->silent = silentMode;
	this->debug = debugMode;
}

void OperatorAtom::addOperators(std::string lib){

	// Check if lib specifies a directory or a file
	DIR *dp = opendir(lib.c_str());
	if (dp != NULL){
		// Directory

		if (!silent){
			std::cout << "mergingplugin: Searching for operators in directory \"" << lib << "\"" << std::endl;
		}
		// Search for .so files in this directory
		struct dirent *dirp;
		std::string libname;

		// Open all files in this directory
		while ((dirp = readdir(dp)) != NULL) {
			libname = std::string(dirp->d_name);
			// Does this filename end with .so?
			if (libname.length() >= 3 && libname.substr(libname.length() - 3) == std::string(".so")){
				// Yes: Load the shared library
				addOperators(lib + (lib.length() > 0 && lib[lib.length() - 1] == '/' ? "" : "/") + libname);
			}
		}
		closedir(dp);
	}else{
		// File

		// Open the specified library
		lt_dlhandle dlHandle = lt_dlopenext(lib.c_str());							// Absolute path

		// Check if the operator library was found
		if (dlHandle == 0){
			if (!silent && debug){
				std::cerr << std::string("mergingplugin: Operator library or directory \"") << lib << std::string("\" was specified but not found") << std::endl;
			}
		}else{
			// check if the library contains an operator import function
			t_operatorImportFunction operatorImportFunction = (t_operatorImportFunction) lt_dlsym(dlHandle, "OPERATORIMPORTFUNCTION");
			if (operatorImportFunction == 0){
				// The library does not contain any operators
				if (!silent && debug){
					std::cout << std::string("mergingplugin: Library \"") + lib + std::string("\" was found but does not contain operators since the operator import function (signature: \"std::vector<dlvhex::merging::IOperator*> OPERATORIMPORTFUNCTION()\") is not present.") << std::endl;
				}
			}else{
				// yes: load the operators
				if (!silent){
					std::cout << "mergingplugin: Loading operators form library \"" << lib << "\"" << std::endl;
				}

				// call the operator import function to retrieve the operators in this library
				std::vector<IOperator*> externalOperators = operatorImportFunction();

				// finally add the operators to the operator list
				int opcount = 0;
				for (std::vector<IOperator*>::iterator it = externalOperators.begin(); it != externalOperators.end(); it++){
					// Check if the operator name is unique
					if (operators.find((*it)->getName()) != operators.end()){
						// Name is not unique
						throw PluginError((std::string("Operator name \"") + (*it)->getName() + std::string("\" in library \"") + lib + std::string("\" is not unique. A duplicate was find in library \"") + lib + std::string("\".")).c_str());
					}else{
						if (!silent && debug){
							std::cout << "mergingplugin:    Found operator \"" << (*it)->getName() << "\"" << std::endl;
						}
						operators[(*it)->getName()] = *it;
						opcount++;
					}
				}

				if (!silent && debug){
					std::cout << "mergingplugin: " << opcount << " operators loaded from library \"" << lib << "\"" << std::endl;
				}
			}
		}
	}
}

IOperator* OperatorAtom::getOperator(std::string opname){

	// Search for the oprator
	std::map<std::string, IOperator*>::const_iterator itOp = operators.find(opname);
	if (itOp != operators.end()){
		// Found
		assert(opname == itOp->first);
		return itOp->second;
	}else{
		// Operator with the specified name was not loaded
		std::stringstream msg;
		msg << "Operator \"" << opname << "\" was not loaded";
		throw IOperator::OperatorException(msg.str());
	}
}
