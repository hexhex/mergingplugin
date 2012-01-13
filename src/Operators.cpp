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

#include <map>

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
/*
	operators[_union.getName()] = &_union;
	operators[_setminus.getName()] = &_setminus;
	operators[_dalal.getName()] = &_dalal;
	operators[_dbo.getName()] = &_dbo;
	operators[_majorityselection.getName()] = &_majorityselection;
	operators[_relationmerging.getName()] = &_relationmerging;
*/
}

OperatorAtom::OperatorAtom(HexAnswerCache &rsCache) : PluginAtom("operator", 0), resultsetCache(rsCache)
{
	addInputConstant();	// operator name
	addInputPredicate();	// predicate containing all the answer indices which shall be passed to the operator
	addInputPredicate();	// predicate containing all the key-value pairs which are passed as parameter
	setOutputArity(1);	// answer index of result

	registerBuiltInOperators();
//	if (!Globals::Instance()->getOption("Silent")){
//		std::cout << "mergingplugin: Loaded " << operators.size() << " built-in operators" << std::endl;
//	}
}


OperatorAtom::~OperatorAtom()
{
}

void
OperatorAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	const Tuple& params = query.input;

	// Extract operator name
	std::string opname = reg->terms.getByID(params[0]).getUnquotedString();

	// Extract the predicate which is true for all indices of answers passed to the parameter
	ID argPred = params[1];

	// go through all input atoms over this predicate
	std::map<int, int> argPairs;
	for(Interpretation::Storage::enumerator it =
	    query.interpretation->getStorage().first();
	    it != query.interpretation->getStorage().end(); ++it){

		ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
		const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

		if (ogatom.tuple[0] == argPred){
			int pos = ogatom.tuple[1].address;
			int handle = ogatom.tuple[2].address;
			if (handle < 0 || handle >= resultsetCache.size()){
				throw PluginError("An invalid answer handle was passed to external atom &operator");
			}
			argPairs[pos] = handle;
		}
	}
	// construct argument vector
	std::vector<int> argumentsIndices(argPairs.size());
	typedef std::pair<int, int> Pair;
	BOOST_FOREACH (Pair p, argPairs){
		argumentsIndices[p.first] = p.second;
	}

	// Extract the operator's key-value arguments
	ID kvPred = params[2];
	OperatorArguments parameters;

	// go through all input atoms over this predicate
	for(Interpretation::Storage::enumerator it =
	    query.interpretation->getStorage().first();
	    it != query.interpretation->getStorage().end(); ++it){

		ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
		const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

		if (ogatom.tuple[0] == kvPred){
			std::string key = reg->terms.getByID(ogatom.tuple[1]).getUnquotedString();
			std::string value = reg->terms.getByID(ogatom.tuple[2]).getUnquotedString();
			parameters.push_back(KeyValuePair(key, value));
		}
	}

	// If the operator name matches one of the loaded operators, it is executed
	try{
		// Search for the oprator
		IOperator* op = getOperator(opname); // throws an OperatorException of not found

		// Assemble unique identifier for this operator application (consisting of operator name and argument indices, see below)
		HexCall hc(HexCall::OperatorCall, op, debug, silent, argumentsIndices, parameters);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(ID::termFromInteger(resultsetCache[hc]));
		answer.get().push_back(out);
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
