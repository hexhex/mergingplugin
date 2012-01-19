#include <HexExecution.h>
#include <DLVHexProcess.h>
#include <HexAnswerCache.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <dlvhex/Registry.hpp>

#include "dlvhex/HexParser.hpp"
#include "dlvhex/InputProvider.hpp"
#include "dlvhex/InternalGrounder.hpp"
#include "dlvhex/InternalGroundDASPSolver.hpp"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/Registry.hpp"
#include "dlvhex/PluginContainer.h"
#include "dlvhex/ASPSolverManager.h"
#include "dlvhex/ASPSolver.h"
#include "dlvhex/State.h"
#include "dlvhex/EvalGraphBuilder.hpp"
#include "dlvhex/EvalHeuristicBase.hpp"
#include "dlvhex/EvalHeuristicOldDlvhex.hpp"
#include "dlvhex/EvalHeuristicTrivial.hpp"
#include "dlvhex/EvalHeuristicEasy.hpp"
#include "dlvhex/EvalHeuristicFromFile.hpp"
#include "dlvhex/OnlineModelBuilder.hpp"
#include "dlvhex/OfflineModelBuilder.hpp"

using namespace dlvhex::merging::plugin;

// -------------------- SimulatorAtom --------------------

std::string SimulatorAtom::getName(int inar, int outar){
	std::stringstream ss;
	ss << "simulator" << inar << "_" << outar;
	return ss.str();
}

SimulatorAtom::SimulatorAtom(ProgramCtx& ctx, int inar, int outar) : PluginAtom(getName(inar, outar), 0), inputArity(inar), outputArity(outar), ctx(ctx){

	addInputConstant();
	for (int i = 0; i < inar; ++i) addInputPredicate();
	setOutputArity(outar);
}

SimulatorAtom::~SimulatorAtom(){
}

void SimulatorAtom::retrieve(const Query& query, Answer& answer) throw (PluginError){


#if 0

	RegistryPtr reg = query.interpretation->getRegistry();

	const Tuple& params = query.input;

	// get ASP filename
	std::string programpath = reg->terms.getByID(params[0]).getUnquotedString();

/*
	DBGLOG(DBG, "Nested program");
	InputProviderPtr ip(new InputProvider());
	ip->addFileInput(programpath);
	ProgramCtx pc = ctx;

	pc.idb.clear();
	pc.edb = InterpretationPtr(new Interpretation(reg));
	pc.changeRegistry(reg);
	ModuleHexParser hp;
	hp.parse(ip, pc);
*/

	// if we access this file for the first time, parse the content
	if (programs.find(programpath) == programs.end()){
		DBGLOG(DBG, "Parsing simulation program");
		InputProviderPtr ip(new InputProvider());
		ip->addFileInput(programpath);
		Logger::Levels l = Logger::Instance().getPrintLevels();	// workaround: verbose causes the parse call below to fail (registry pointer is 0)
		Logger::Instance().setPrintLevels(0);
		programs[programpath] = ctx;
		programs[programpath].changeRegistry(reg);
		programs[programpath].idb.clear();
		programs[programpath].edb = InterpretationPtr(new Interpretation(reg));
		ModuleHexParser hp;
		hp.parse(ip, programs[programpath]);
		Logger::Instance().setPrintLevels(l);


		DBGLOG(DBG, "Setting eval heuristics");
		programs[programpath].evalHeuristic.reset(new EvalHeuristicEasy);
		programs[programpath].modelBuilderFactory = boost::factory<OnlineModelBuilder<FinalEvalGraph>*>();

		DBGLOG(DBG, "Associate PluginAtom instances with ExternalAtom instances");
		programs[programpath].associateExtAtomsWithPluginAtoms(programs[programpath].idb, true);

		DBGLOG(DBG, "State pipeline");
		programs[programpath].changeState(StatePtr(new SafetyCheckState));

		DBGLOG(DBG, "... safetyCheck");
		programs[programpath].safetyCheck();
		DBGLOG(DBG, "... createDependencyGraph");
		programs[programpath].createDependencyGraph();
		DBGLOG(DBG, "... optimizeEDBDependencyGraph");
		programs[programpath].optimizeEDBDependencyGraph();
		DBGLOG(DBG, "... createComponentGraph");
		programs[programpath].createComponentGraph();
		DBGLOG(DBG, "... createEvalGraph");
		programs[programpath].createEvalGraph();
		DBGLOG(DBG, "... setupProgramCtx");
		programs[programpath].setupProgramCtx();

		DBGLOG(DBG, "Clear callbacks");
		programs[programpath].modelCallbacks.clear();
		programs[programpath].finalCallbacks.clear();
	}
	ProgramCtx pc = programs[programpath];

	pc.edb = InterpretationPtr(new Interpretation(reg));
	pc.edb->add(*(programs[programpath].edb));




/*


DBGLOG(DBG, "Setting eval heuristics");
pc.evalHeuristic.reset(new EvalHeuristicEasy);
pc.modelBuilderFactory = boost::factory<OnlineModelBuilder<FinalEvalGraph>*>();

DBGLOG(DBG, "Associate PluginAtom instances with ExternalAtom instances");
pc.associateExtAtomsWithPluginAtoms(programs[programpath].idb, true);

DBGLOG(DBG, "State pipeline");
pc.changeState(StatePtr(new SafetyCheckState));

DBGLOG(DBG, "... safetyCheck");
pc.safetyCheck();
DBGLOG(DBG, "... createDependencyGraph");
pc.createDependencyGraph();
DBGLOG(DBG, "... optimizeEDBDependencyGraph");
pc.optimizeEDBDependencyGraph();
DBGLOG(DBG, "... createComponentGraph");
pc.createComponentGraph();
DBGLOG(DBG, "... createEvalGraph");
pc.createEvalGraph();
DBGLOG(DBG, "... setupProgramCtx");
pc.setupProgramCtx();

DBGLOG(DBG, "Clear callbacks");
pc.modelCallbacks.clear();
pc.finalCallbacks.clear();


*/






	// construct edb
	DBGLOG(DBG, "Constructing EDB");

	// go through all input atoms
	DBGLOG(DBG, "Rewriting input");
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
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, inPredStr.str());
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
DBGLOG(DBG, "EDB: " << *(pc.edb));

	DBGLOG(DBG, "Setting AnswerSetCallback");
	HexAnswerCache::SubprogramAnswerSetCallback* spasc = new HexAnswerCache::SubprogramAnswerSetCallback();
	ModelCallbackPtr spascp = ModelCallbackPtr(spasc);
	pc.modelCallbacks.push_back(spascp);

	DBGLOG(DBG, "evaluate");
	pc.evaluate();

	InterpretationPtr as;
	if (spasc->answersets.size() > 0) as = spasc->answersets[0];

	if (as != InterpretationPtr()){

		// extract parameters from all atoms over predicate "out"
		DBGLOG(DBG, "Rewrting output");
		Term outPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_PREDICATE, "out");
		ID outPredID = reg->storeTerm(outPredTerm);
		for(Interpretation::Storage::enumerator it =
		    as->getStorage().first();
		    it != as->getStorage().end(); ++it){

			ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
			const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

			if (ogatom.tuple[0] == outPredID){
				Tuple t;
				for (int ot = 1; ot < ogatom.tuple.size(); ++ot){
					t.push_back(ogatom.tuple[ot]);
				}
				answer.get().push_back(t);
			}
		}
	}

#endif


	RegistryPtr reg = query.interpretation->getRegistry();

	const Tuple& params = query.input;

	// get ASP filename
	std::string programpath = reg->terms.getByID(params[0]).getUnquotedString();

	// if we access this file for the first time, parse the content
	if (programs.find(programpath) == programs.end()){
		DBGLOG(DBG, "Parsing simulation program");
		InputProviderPtr ip(new InputProvider());
		ip->addFileInput(programpath);
		Logger::Levels l = Logger::Instance().getPrintLevels();	// workaround: verbose causes the parse call below to fail (registry pointer is 0)
		Logger::Instance().setPrintLevels(0);
		programs[programpath].changeRegistry(reg);
		ModuleHexParser hp;
		hp.parse(ip, programs[programpath]);
		Logger::Instance().setPrintLevels(l);
	}
	ProgramCtx& pc = programs[programpath];

	// construct edb
	DBGLOG(DBG, "Constructing EDB");
	InterpretationPtr edb = InterpretationPtr(new Interpretation(*pc.edb));

	// go through all input atoms
	DBGLOG(DBG, "Rewriting input");
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
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, inPredStr.str());
				ID inPredID = reg->storeTerm(inPredTerm);
				OrdinaryAtom oareplace = ogatom;
				oareplace.tuple[0] = inPredID;

				// get ID of replaced atom
				ID oareplaceID = reg->storeOrdinaryGAtom(oareplace);

				// set this atom in the input interpretation
				edb->getStorage().set_bit(oareplaceID.address);
				found = true;
				break;
			}
		}
		assert(found);
	}

	DBGLOG(DBG, "Grounding simulation program");
	ASPProgram program(pc.registry(), pc.idb, edb);
	InternalGrounderPtr ig = InternalGrounderPtr(new InternalGrounder(pc, program));
	ASPProgram gprogram = ig->getGroundProgram();

	DBGLOG(DBG, "Evaluating simulation program");
	InternalGroundDASPSolver igas(pc, gprogram);
	InterpretationPtr as = igas.projectToOrdinaryAtoms(igas.getNextModel());
	if (as != InterpretationPtr()){

		// extract parameters from all atoms over predicate "out"
		DBGLOG(DBG, "Rewrting output");
		Term outPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, "out");
		ID outPredID = reg->storeTerm(outPredTerm);
		for(Interpretation::Storage::enumerator it =
		    as->getStorage().first();
		    it != as->getStorage().end(); ++it){

			ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
			const OrdinaryAtom& ogatom = reg->ogatoms.getByID(ogid);

			if (ogatom.tuple[0] == outPredID){
				Tuple t;
				for (int ot = 1; ot < ogatom.tuple.size(); ++ot){
					t.push_back(ogatom.tuple[ot]);
				}
				answer.get().push_back(t);
			}
		}
	}
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
	InterpretationConstPtr inputfacts = query.interpretation;
	try{
		const Tuple& params = query.input;

		// resolve escape sequences
		program = reg->terms.getByID(params[0]).getUnquotedString();

		// Retrieve command line arguments (if specified)
		if (params.size() > 1){
			cmdargs = reg->terms.getByID(params[1]).getUnquotedString();
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
	InterpretationConstPtr inputfacts = query.interpretation;
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
	}catch(PluginError){
		throw;
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
				Tuple ts;
				ts.push_back(ID::termFromInteger(runningindex));
				Term t(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, "s");
				ts.push_back(reg->storeTerm(t));
				ts.push_back(ID::termFromInteger(/*it->isStronglyNegated() ? 1 :*/ 0));	// TODO: check if the atom is strongly negated
				answer.get().push_back(ts);

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
	}
}

