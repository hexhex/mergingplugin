#include <HexExecution.h>
#include <DLVHexProcess.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <dlvhex/Registry.hpp>

#include "dlvhex/HexParser.hpp"
#include "dlvhex/InputProvider.hpp"

#include "dlvhex/InternalGrounder.hpp"
#include "dlvhex/InternalGroundDASPSolver.hpp"

using namespace dlvhex::merging::plugin;

// -------------------- SimulatorAtom --------------------

std::string SimulatorAtom::getName(int inar, int outar){
	std::stringstream ss;
	ss << "simulator" << inar << "_" << outar;
	return ss.str();
}

SimulatorAtom::SimulatorAtom(int inar, int outar) : PluginAtom(getName(inar, outar), 0), inputArity(inar), outputArity(outar){

	addInputConstant();
	for (int i = 0; i < inar; ++i) addInputPredicate();
	setOutputArity(outar);
}

SimulatorAtom::~SimulatorAtom(){
}

void SimulatorAtom::retrieve(const Query& query, Answer& answer) throw (PluginError){

	RegistryPtr reg = query.interpretation->getRegistry();

	const Tuple& params = query.input;

	// get ASP filename
	std::string programpath = reg->terms.getByID(params[0]).getUnquotedString();

	// evaluate it
	InputProviderPtr ip(new InputProvider());
	ip->addFileInput(programpath);
	ProgramCtx pc;
	pc.changeRegistry(reg);
	ModuleHexParser hp;
	hp.parse(ip, pc);
std::cout << "INPUT: " << query.interpretation << std::endl;
	// go through all input atoms
	for(Interpretation::Storage::enumerator it =
	    query.interpretation->getStorage().first();
	    it != query.interpretation->getStorage().end(); ++it){

		ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
		const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

		// check if the predicate matches any of the input parameters to simulator atom
		bool found = false;
		for (int inp = 1; inp < params.size(); ++inp){
			if (ogatom.tuple[0] == params[inp]){
				// replace the predicate by "in[inp]"
				std::stringstream inPredStr;
				inPredStr << "in" << inp;
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_PREDICATE, inPredStr.str());
				ID inPredID = reg->storeTerm(inPredTerm);
				OrdinaryAtom oareplace = ogatom;
				oareplace.tuple[0] = inPredID;

				// get ID of replaced atom
				ID oareplaceID = reg->storeOrdinaryGAtom(oareplace);

				// set this atom in the input interpretation
				pc.edb->getStorage().set_bit(oareplaceID.address);
				found = true;
				break;
			}
		}
		assert(found);
	}

	ASPProgram program(pc.registry(), pc.idb, pc.edb);
	InternalGrounderPtr ig = InternalGrounderPtr(new InternalGrounder(pc, program));
	ASPProgram gprogram = ig->getGroundProgram();

	InternalGroundDASPSolver igas(pc, gprogram);
	InterpretationPtr as = igas.projectToOrdinaryAtoms(igas.getNextModel());
	if (as != InterpretationPtr()){

		// extract parameters from all atoms over predicate "out"
		Term outPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_PREDICATE, "out");
		ID outPredID = reg->storeTerm(outPredTerm);
		for(Interpretation::Storage::enumerator it =
		    as->getStorage().first();
		    it != as->getStorage().end(); ++it){

			ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
			const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

std::cout << "check: " << std::endl;
			if (ogatom.tuple[0] == outPredID){
std::cout << "OUTPUT: " << std::endl;
				Tuple t;
				for (int ot = 1; ot < ogatom.tuple.size(); ++ot){
std::cout << ogatom.tuple[ot] << ", ";
					t.push_back(ogatom.tuple[ot]);
				}
std::cout << std::endl;
				answer.get().push_back(t);
			}
		}
	}
//	while ((as = igas.projectToOrdinaryAtoms(igas.getNextModel())) != InterpretationPtr()){
//		result->push_back(InterpretationPtr(new Interpretation(*as)));
//	}
}

// -------------------- HexAtom --------------------

HexAtom::HexAtom(HexAnswerCache &rsCache) : PluginAtom("hex", 1), resultsetCache(rsCache)
{
	addInputConstant();	// program
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

HexAtom::~HexAtom()
{
}

void
HexAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	std::string program;
	std::string cmdargs;
	InterpretationConstPtr inputfacts;
	try{
		const Tuple& params = query.input;

		// resolve escape sequences
		program = reg->terms.getByID(params[0]).getUnquotedString();

		// Retrieve command line arguments
		if (params.size() > 1){
			cmdargs = reg->terms.getByID(params[1]).getUnquotedString();
		}

		// Build hex call identifier
		InterpretationConstPtr inputfacts;
		HexCall hc(HexCall::HexProgram, program, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(ID::termFromInteger(resultsetCache[hc]));
		answer.get().push_back(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << program << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}

// -------------------- HexFileAtom --------------------

HexFileAtom::HexFileAtom(HexAnswerCache &rsCache) : PluginAtom("hexfile", 1), resultsetCache(rsCache)
{
	addInputConstant();	// program path
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

HexFileAtom::~HexFileAtom()
{
}

void
HexFileAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	std::string programpath;
	std::string cmdargs;
	InterpretationConstPtr inputfacts;
	try{
		const Tuple& params = query.input;

		// load program
		programpath = reg->terms.getByID(params[0]).getUnquotedString();

		// Retrieve command line arguments
		if (params.size() > 1){
			cmdargs = reg->terms.getByID(params[1]).getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexFile, programpath, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(ID::termFromInteger(resultsetCache[hc]));
		answer.get().push_back(out);
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << programpath << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}

// -------------------- CallHexAtom --------------------

std::string CallHexAtom::getName(int arity){
	std::stringstream ss;
	ss << "callhex" << arity;
	return ss.str();
}

CallHexAtom::CallHexAtom(HexAnswerCache &rsCache, int ar = 0) : PluginAtom(getName(ar), 1), resultsetCache(rsCache), arity(ar)
{
	addInputConstant();	// program
	for (int i = 0; i < arity; i++){
		addInputPredicate();	// input facts
	}
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

CallHexAtom::~CallHexAtom()
{
}

void
CallHexAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	std::string program;
	std::string cmdargs;
	InterpretationConstPtr inputfacts = query.interpretation;
	try{
		const Tuple& params = query.input;

		// resolve escape sequences
		program = reg->terms.getByID(params[0]).getUnquotedString();

//		// retrieve the facts over the specified predicate and pass them to the subprogram
//		for (int i = 0; i < arity; i++){
//			std::string inputfactspred = params[1 + i].getUnquotedString();
//			query.getInterpretation().matchPredicate(inputfactspred, inputfacts);
//		}

		// Retrieve command line arguments (if specified)
		if (params.size() > 1 + arity){
			cmdargs = reg->terms.getByID(params[1 + arity]).getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexProgram, program, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(ID::termFromInteger(resultsetCache[hc]));
		answer.get().push_back(out);
	}catch(PluginError){
		throw;
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << program << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}


// -------------------- CallHexFileAtom --------------------

std::string CallHexFileAtom::getName(int arity){
	std::stringstream ss;
	ss << "callhexfile" << arity;
	return ss.str();
}

CallHexFileAtom::CallHexFileAtom(HexAnswerCache &rsCache, int ar) : PluginAtom(getName(ar), 1), resultsetCache(rsCache), arity(ar)
{
	addInputConstant();	// program path
	for (int i = 0; i < arity; i++){
		addInputPredicate();	// input facts
	}
	addInputTuple();	// command line arguments (optional)
	setOutputArity(1);	// list of answer-set handles
}

CallHexFileAtom::~CallHexFileAtom()
{
}

void
CallHexFileAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	std::string programpath;
	std::string cmdargs;
	InterpretationConstPtr inputfacts = query.interpretation;
	try{
		const Tuple& params = query.input;

		// load program
		programpath = reg->terms.getByID(params[0]).getUnquotedString();

		// retrieve the facts over the specified predicate and pass them to the subprogram
//		for (int i = 0; i < arity; i++){
//			std::string inputfactspred = params[1 + i].getUnquotedString();
//			query.getInterpretation().matchPredicate(inputfactspred, inputfacts);
//		}

		// Retrieve command line arguments
		if (params.size() > 1 + arity){
			cmdargs = reg->terms.getByID(params[1 + arity]).getUnquotedString();
		}

		// Build hex call identifier
		HexCall hc(HexCall::HexFile, programpath, cmdargs, inputfacts);

		// request entry from cache (this will automatically add it if it's not contained yet)
		Tuple out;
		out.push_back(ID::termFromInteger(resultsetCache[hc]));
		answer.get().push_back(out);
	}catch(PluginError){
		throw;
	}catch(...){
		std::stringstream msg;
		msg << "Nested Hex program \"" << programpath << "\" failed. Command line arguments were: " << cmdargs;
		throw PluginError(msg.str());
	}
}

// -------------------- AnswerSetsAtom --------------------

AnswerSetsAtom::AnswerSetsAtom(HexAnswerCache &rsCache) : PluginAtom("answersets", 1), resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	setOutputArity(1);	// list of answer-set handles
}

AnswerSetsAtom::~AnswerSetsAtom()
{
}

void
AnswerSetsAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	// Retrieve answer index
	int answerindex = query.input[0].address;

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &answersets");
	}else{
		// Return handles to all answer-sets of the given answer (all integers from 0 to the number of answer-sets minus 1)
		int i = 0;
		for (HexAnswer::iterator it = resultsetCache[answerindex].begin(); it != resultsetCache[answerindex].end(); it++){
			Tuple out;
			out.push_back(ID::termFromInteger(i++));
			answer.get().push_back(out);
		}
	}
}


// -------------------- PredicatesAtom --------------------

PredicatesAtom::PredicatesAtom(HexAnswerCache &rsCache) : PluginAtom("predicates", 1), resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	setOutputArity(2);	// list of predicate/arity pairs
}

PredicatesAtom::~PredicatesAtom()
{
}

void
PredicatesAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	// Retrieve answer index and answer-set index
	int answerindex = query.input[0].address;
	int answersetindex = query.input[1].address;

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &predicates");
	}else if(answersetindex < 0 || answersetindex >= resultsetCache[answerindex].size()){
		throw PluginError("An invalid answer-set handle was passed to atom &predicates");
	}else{
		// Go through all atoms of the given answer_set
		for(Interpretation::Storage::enumerator it =
		    (resultsetCache[answerindex])[answersetindex]->getStorage().first();
		    it != (resultsetCache[answerindex])[answersetindex]->getStorage().end(); ++it){

			ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
			const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

			Tuple t;
			t.push_back(ogatom.tuple[0]);
			t.push_back(ID::termFromInteger(ogatom.tuple.size() - 1));
			answer.get().push_back(t);
		}
	}
}


// -------------------- ArgumentsAtom --------------------

ArgumentsAtom::ArgumentsAtom(HexAnswerCache &rsCache) : PluginAtom("arguments", 1), resultsetCache(rsCache)
{
	addInputConstant();	// answer index
	addInputConstant();	// answerset index
	addInputConstant();	// predicate name
	setOutputArity(3);	// list of triples: running-index/arg-index/arg-value
}

ArgumentsAtom::~ArgumentsAtom()
{
}

void
ArgumentsAtom::retrieve(const Query& query, Answer& answer) throw (PluginError)
{
	RegistryPtr reg = query.interpretation->getRegistry();

	// Extract answer index, answerset index and predicate to retrieve
	int answerindex = query.input[0].address;
	int answersetindex = query.input[1].address;
	ID predicate = query.input[2];

	// check index validity
	if (answerindex < 0 || answerindex >= resultsetCache.size()){
		throw PluginError("An invalid answer handle was passed to atom &arguments");
	}else if(answersetindex < 0 || answersetindex >= resultsetCache[answerindex].size()){
		throw PluginError("An invalid answer-set handle was passed to atom &arguments");
	}else{
		int runningindex = 0;

		// Go through all atoms of the given answer_set
		for(Interpretation::Storage::enumerator it =
		    (resultsetCache[answerindex])[answersetindex]->getStorage().first();
		    it != (resultsetCache[answerindex])[answersetindex]->getStorage().end(); ++it){

			ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
			const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

			// If the atom is built upon the given predicate, return it's parameters
			if (ogatom.tuple[0] == predicate){
				// special case of index "s": positive or strongly negated
//				Tuple ts;
//				ts.push_back(ID::termFromInteger(runningindex));
//				ts.push_back(reg->terms.storeAndGetID(Term(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, "s")));
//				ts.push_back(ID::termFromInteger(it->isStronglyNegated() ? 1 : 0));
//				answer.get().push_back(ts);

				// Go through all parameters
				for (int i = 1; i < ogatom.tuple.size(); ++i){
					Tuple t;
					t.push_back(ID::termFromInteger(runningindex));
					t.push_back(ID::termFromInteger(i - 1));
					t.push_back(ogatom.tuple[i]);
					answer.get().push_back(t);
				}
				runningindex++;
			}
		}
/*
		// Go through all atoms of the given answer_set
		int runningindex = 0;
		for (AtomSet::const_iterator it = (*(resultsetCache[answerindex]))[answersetindex].begin(); it != (*(resultsetCache[answerindex]))[answersetindex].end(); it++){
			// If the atom is built upon the given predicate, return it's parameters
			if (it->getPredicate() == predicate){
				// special case of index "s": positive or strongly negated
				ComfortTuple outsign;
				outsign.push_back(ComfortTerm::createInteger(runningindex));
				outsign.push_back(ComfortTerm::createConstant("s"));
				outsign.push_back(ComfortTerm::createInteger(it->isStronglyNegated() ? 1 : 0));
				answer.insert(outsign);

				// Go through all parameters
				for (int argindex = 0; argindex < it->getArity(); argindex++){
					// For each: Return a running index (since the same predicate can occur arbitrary often within an answer-set), the argument index and the value
					ComfortTuple out;
					out.push_back(ComfortTerm::createInteger(runningindex));
					out.push_back(ComfortTerm::createInteger(argindex));
					out.push_back(it->getArgument(argindex + 1));
					answer.insert(out);
				}
				runningindex++;
			}
		}
*/
	}
}

